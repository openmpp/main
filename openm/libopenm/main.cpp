/**
* @file
* OpenM++ model: main entry point
*  
* @mainpage OpenM++ runtime library (libopenm)
* 
* OpenM++ runtime library provides life cycle support to run the models:
* - main() model entry point  
* - model initialization:    
*   * read model input parameters  
*   * create modeling threads for each subsample
* - run model event loop (do the simulation)  
* - model shutdown:  
*   * get output results for each subsample  
*   * write output tables  
*   * do subsamples aggregation    
* 
* There are four isolated parts in runtime library:
* - model part: sources are in openm/model
* - data access part: sources are in openm/db
* - message passing part: sources are in openm/msg  
* - common helper utilities part: sources are in openm/common  
*  
* Model part is a top level base class and IModel public interface for openM++ models.
* It control model life cycle, creates data access and message passing objects to organize model execution.
* 
* Data access and message passing are isolated parts of runtime and accessible through 
* openm::IDbExec and openm::IMsgExec public interfaces.
* 
* Common portion of runtime contains set of helper functions and utility classes, like openm::ILog.
* They are used everywhere inside of the model runtime code.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/common/omFile.h"
#include "model.h"
using namespace openm;

// if message library is MPI based then use database only at the root model process
#ifdef OM_MSG_MPI
    static const bool isMpiUsed = true;
#else
    static const bool isMpiUsed = false;
#endif

/** model run initialization: read input parameters */
OM_RUN_INIT_HANDLER RunInitHandler;

/** model startup method: initialize subsample */
OM_STARTUP_HANDLER ModelStartupHandler;

/** model event loop: user code entry point */
OM_EVENT_LOOP_HANDLER RunModelHandler;

/** model shutdown method: save output results */
OM_SHUTDOWN_HANDLER ModelShutdownHandler;

/** model thread function */
static bool modelThreadLoop(int i_runId, int i_subCount, int i_subNumber, RunController * i_runCtrl);

// run modeling threads to calculate subsamples
static bool runModelThreads(int i_runId, RunController * i_runCtrl);

/** main entry point */
int main(int argc, char ** argv) 
{
    try {
        // get model run options from command line and ini-file
        const ArgReader argOpts = MetaLoader::getRunOptions(argc, argv);

        // adjust log file(s) with actual log settings specified in model run options file
        theLog->init(
            argOpts.boolOption(ArgKey::logToConsole) || !argOpts.isOptionExist(ArgKey::logToConsole),
            argOpts.strOption(ArgKey::logFilePath).c_str(),
            argOpts.boolOption(ArgKey::logUseTs),
            argOpts.boolOption(ArgKey::logUsePid),
            argOpts.boolOption(ArgKey::logNoMsgTime),
            argOpts.boolOption(ArgKey::logSql)
            );
        theLog->logMsg("Model", OM_MODEL_NAME);

        // if trace log file enabled setup trace file name
        string traceFilePath;
        if (argOpts.boolOption(RunOptionsKey::traceToFile)) {
            traceFilePath = argOpts.strOption(RunOptionsKey::traceFilePath);
            if (traceFilePath.empty()) traceFilePath = argOpts.strOption(RunOptionsKey::setName) + ".txt";
        }
        theTrace->init(
            argOpts.boolOption(RunOptionsKey::traceToConsole),
            traceFilePath.c_str(),
            argOpts.boolOption(RunOptionsKey::traceUseTs),
            argOpts.boolOption(RunOptionsKey::traceUsePid),
            argOpts.boolOption(RunOptionsKey::traceNoMsgTime) || !argOpts.isOptionExist(RunOptionsKey::traceNoMsgTime)
            );

        // init message interface
        unique_ptr<IMsgExec> msgExec(IMsgExec::create(argc, argv));

        // get db-connection string or use default if not specified
        string connectionStr = argOpts.strOption(
            RunOptionsKey::dbConnStr,
            string("Database=") + OM_MODEL_NAME + ".sqlite; Timeout=86400; OpenMode=ReadWrite;"
            );

        // open db-connection
        unique_ptr<IDbExec> dbExec(
            (!isMpiUsed || msgExec->isRoot()) ? IDbExec::create(connectionStr) : nullptr
            );

        // load metadata tables and broadcast it to all modeling processes
        unique_ptr<RunController> runCtrl(RunController::create(argOpts, isMpiUsed, dbExec.get(), msgExec.get()));

        if (theModelRunState.isExit()) {        // exit if initial status not a continue status
            if (theModelRunState.isError()) {
                theLog->logMsg("Error at model initialization");
                return EXIT_FAILURE;
            }
            theLog->logMsg("Done.");
            return EXIT_SUCCESS;
        }

        // run the model until modeling task completed
        for (int runId = 0; (runId = runCtrl->nextRun()) > 0; ) {

            // initilaze model run: read input parameters
            RunInitHandler(runCtrl.get());

            // do the modeling: run modeling threads to calculate subsamples
            bool isRunOk = runModelThreads(runId, runCtrl.get());
            if (!isRunOk) {
                runCtrl->shutdownOnExit(ModelStatus::error);
                throw ModelException("modeling FAILED");
            }

            // model run completed OK, receive and write the data
            runCtrl->shutdownRun();
        }

        // all model run completed OK, do final cleanup
        runCtrl->shutdown();
    }
    catch(HelperException & ex) {
        theLog->logErr(ex, "Helper error");
        return EXIT_FAILURE;
    }
    catch(DbException & ex) {
        theLog->logErr(ex, "DB error");
        return EXIT_FAILURE;
    }
    catch(MsgException & ex) {
        theLog->logErr(ex, "Messaging error");
        return EXIT_FAILURE;
    }
    catch(ModelException & ex) {
        theLog->logErr(ex, "Model error");
        return EXIT_FAILURE;
    }
    catch(exception & ex) {
        theLog->logErr(ex);
        return EXIT_FAILURE;
    }
    catch(...) {    // exit with faulure on unhandled exception
        theLog->logMsg("FAILED", OM_FILE_LINE);
        return EXIT_FAILURE;
    }

    theLog->logMsg("Done.");
    return EXIT_SUCCESS;
}

/** model thread function */
bool modelThreadLoop(int i_runId, int i_subCount, int i_subNumber, RunController * i_runCtrl)
{
    try {
#ifdef _DEBUG
        theLog->logFormatted("Subsample %d", i_subNumber);
#endif      
        // create the model
        unique_ptr<IModel> model(
            ModelBase::create(i_runId, i_subCount, i_subNumber, i_runCtrl, i_runCtrl->meta())
            );

        // initialize model subsample
        if (ModelStartupHandler != NULL) ModelStartupHandler(model.get());

        // do the modeling
        RunModelHandler(model.get());

        // write output tables
        ModelShutdownHandler(model.get());
    }
    catch (HelperException & ex) {
        theLog->logErr(ex, "Helper error");
        return false;
    }
    catch (DbException & ex) {
        theLog->logErr(ex, "DB error");
        return false;
    }
    catch (MsgException & ex) {
        theLog->logErr(ex, "Messaging error");
        return false;
    }
    catch (ModelException & ex) {
        theLog->logErr(ex, "Model error");
        return false;
    }
    catch (exception & ex) {
        theLog->logErr(ex);
        return false;
    }
    catch (...) {    // exit with faulure on unhandled exception
        theLog->logMsg("FAILED", OM_FILE_LINE);
        return false;
    }

    return true;    // modeling thread completed OK
}

// run modeling threads to calculate subsamples
bool runModelThreads(int i_runId, RunController * i_runCtrl)
{
    list<future<bool> > modelFutureLst;     // modeling threads

    int nextSub = 0;
    int maxSub = i_runCtrl->selfSubCount;

    while (nextSub < maxSub || modelFutureLst.size() > 0) {

        // create and start new modeling threads
        while (nextSub < maxSub && (int)modelFutureLst.size() < i_runCtrl->threadCount) {
            modelFutureLst.push_back(std::move(std::async(
                launch::async,
                modelThreadLoop,
                i_runId,
                i_runCtrl->subSampleCount,
                i_runCtrl->subFirstNumber + nextSub,
                i_runCtrl
                )));
            nextSub++;
        }

        // wait for modeling threads completion
        bool isAnyCompleted = false;
        for (auto mfIt = modelFutureLst.begin(); mfIt != modelFutureLst.end(); ) {

            // skip thread if modeling not completed yet
            if (mfIt->wait_for(chrono::milliseconds(OM_PROGRESS_WAIT_TIME)) != future_status::ready) {
                ++mfIt;
                continue;
            }
            isAnyCompleted = true;

            // modeling completed: get result sucess and remove thread form the list
            bool isOk = mfIt->get();
            mfIt = modelFutureLst.erase(mfIt);

            if (!isOk) return false;    // exit with error if model failed
        }

        // try to receive output subsamples
        i_runCtrl->receiveSubSamples();

        // wait if no modeling progress and any threads still running
        if (!isAnyCompleted && modelFutureLst.size() > 0) {
            this_thread::sleep_for(chrono::milliseconds(OM_PROGRESS_SLEEP_TIME));
            continue;
        }
    }

    return true;    // modeling completed OK
}
