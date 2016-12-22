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
* Model part is a top level portion of runtime to control openM++ model model life cycle.
* It implement IModel and IRun public interfaces to read model input, write output results, update progress, etc.
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
#include "libopenm/common/iniReader.h"
#include "libopenm/common/argReader.h"
#include "helper.h"
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

// communicate with child modeling processes or sleep if no child activity
static void childExchangeOrSleep(long i_sleepTime, RunController * i_runCtrl);

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

        // read language specific messages from path/to/exe/modelName.message.ini
        IniFileReader::loadMessages(
            makeFilePath(baseDirOf((argc > 0 ? argv[0] : "")).c_str(), OM_MODEL_NAME, ".message.ini").c_str()
        );
        theLog->logMsg(OM_MODEL_NAME);

        // if trace log file enabled setup trace file name
        string traceFilePath;
        if (argOpts.boolOption(RunOptionsKey::traceToFile)) {
            traceFilePath = argOpts.strOption(RunOptionsKey::traceFilePath);
            if (traceFilePath.empty()) traceFilePath = makeDefaultPath(argv[0], ".trace.txt");
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
            (!isMpiUsed || msgExec->isRoot()) ? IDbExec::create(SQLITE_DB_PROVIDER, connectionStr) : nullptr
            );

        // read language specific messages for the log and broadcast it to all modeling processes
        MetaLoader::initMessages(isMpiUsed, dbExec.get(), msgExec.get());

        // load metadata tables and broadcast it to all modeling processes
        unique_ptr<RunController> runCtrl(RunController::create(argOpts, isMpiUsed, dbExec.get(), msgExec.get()));

        // run the model until modeling task completed
        while(!theModelRunState.isShutdownOrExit()) {

            // create next run id: find model input data set
            int runId = runCtrl->nextRun();
            if (runId <= 0) {
                childExchangeOrSleep(OM_WAIT_SLEEP_TIME, runCtrl.get());    // communicate with child processes, if any, or sleep
                continue;                                                   // no input: completed or waiting for additional input
            }
            theLog->logFormatted("Run: %d", runId);

            // initialize model run: read input parameters
            RunInitHandler(runCtrl.get());

            // do the modeling: run modeling threads to calculate subsamples
            bool isRunOk = runModelThreads(runId, runCtrl.get());
            if (!isRunOk) {
                runCtrl->shutdownOnExit(ModelStatus::error);
                throw ModelException("modeling FAILED");
            }

            // run completed OK, receive and write the data
            runCtrl->shutdownRun(runId);
        }

        // model completed OK at local process
        // wait for all child to be completed and do final cleanup
        runCtrl->shutdownWaitAll();
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
    catch(...) {    // exit with failure on unhandled exception
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
    catch (SimulationException & ex) {
        theLog->logErr(ex, "Simulation error");
        return false;
    }
    catch (exception & ex) {
        theLog->logErr(ex);
        return false;
    }
    catch (...) {    // exit with failure on unhandled exception
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
    while (nextSub < i_runCtrl->selfSubCount || modelFutureLst.size() > 0) {

        // create and start new modeling threads
        while (nextSub < i_runCtrl->selfSubCount && (int)modelFutureLst.size() < i_runCtrl->threadCount) {
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
            if (mfIt->wait_for(chrono::milliseconds(OM_RUN_POLL_TIME)) != future_status::ready) {
                ++mfIt;
                continue;
            }
            isAnyCompleted = true;

            // modeling completed: get result success and remove thread from the list
            bool isOk = mfIt->get();
            mfIt = modelFutureLst.erase(mfIt);

            if (!isOk) return false;    // exit with error if model failed
        }

        // wait if no modeling progress and any threads still running
        if (!isAnyCompleted && modelFutureLst.size() > 0) {
            childExchangeOrSleep(2L * OM_ACTIVE_SLEEP_TIME, i_runCtrl); // communicate with child processes, if any, or sleep
        }
    }

    return true;    // modeling completed OK
}

// communicate with child modeling processes or sleep if no child activity
void childExchangeOrSleep(long i_sleepTime, RunController * i_runCtrl)
{
    long nExchange = 1 + i_sleepTime / OM_ACTIVE_SLEEP_TIME;

    bool isAnyChildActivity = false;
    do {
        isAnyChildActivity = i_runCtrl->childExchange();    // communicate with child processes, if any

        if (isAnyChildActivity) 
            this_thread::sleep_for(chrono::milliseconds(OM_ACTIVE_SLEEP_TIME));
    }
    while (isAnyChildActivity && --nExchange > 0);

    if (nExchange > 0 && i_runCtrl->processCount > 1 && !theModelRunState.isShutdownOrExit()) 
        this_thread::sleep_for(chrono::milliseconds(nExchange * OM_ACTIVE_SLEEP_TIME)); // sleep more if no child activity
}
