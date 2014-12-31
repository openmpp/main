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
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "libopenm/common/omFile.h"
#include "model.h"
using namespace openm;

// if message library is MPI based then use database only at the root model process
#ifdef OM_MSG_MPI
    static const bool isMpiUsed = true;
#else
    static const bool isMpiUsed = false;
#endif

/** model startup method: initialize parameters */
OM_STARTUP_HANDLER ModelStartupHandler;

/** model event loop: user code entry point */
OM_EVENT_LOOP_HANDLER RunModelHandler;

/** model shutdown method: save output results */
OM_SHUTDOWN_HANDLER ModelShutdownHandler;

/** model thread function */
static bool modelThreadLoop(
    bool i_isMpiUsed,
    int i_runId,
    int i_subCount,
    int i_subNumber,
    IDbExec * i_dbExec,
    IMsgExec * i_msgExec,
    const MetaRunHolder * i_metaStore
    );

/** main entry point */
int main(int argc, char ** argv) 
{
    try {
        // get model run options from command line and ini-file
        RunController runCtrl(argc, argv);

        // adjust log file(s) with actual log settings specified in model run options file
        theLog->init(
            runCtrl.argOpts().boolOption(ArgKey::logToConsole) || !runCtrl.argOpts().isOptionExist(ArgKey::logToConsole),
            runCtrl.argOpts().strOption(ArgKey::logFilePath).c_str(),
            runCtrl.argOpts().boolOption(ArgKey::logUseTs),
            runCtrl.argOpts().boolOption(ArgKey::logUsePid),
            runCtrl.argOpts().boolOption(ArgKey::logNoMsgTime),
            runCtrl.argOpts().boolOption(ArgKey::logSql)
            );
        theLog->logMsg("Model", OM_MODEL_NAME);

        // init message interface
        unique_ptr<IMsgExec> msgExec(IMsgExec::create(argc, argv));

        // get db-connection string or use default if not specified
        string connectionStr = runCtrl.argOpts().strOption(
            RunOptionsKey::dbConnStr, 
            string("Database=") + OM_MODEL_NAME + ".sqlite; Timeout=86400; OpenMode=ReadWrite;"
            );

        // open db-connection
        unique_ptr<IDbExec> dbExec( 
            (!isMpiUsed || msgExec->isRoot()) ? IDbExec::create(connectionStr) : nullptr
            );

        // create new model run in database or use exising
        // initialize run id and subsample number
        // store run options in database with new run id
        // copy input parameters to run the model
        // load metadata tables and broadcast it to all modeling processes
        unique_ptr<MetaRunHolder> metaStore(runCtrl.init(isMpiUsed, dbExec.get(), msgExec.get()));

        // create and run modeling threads
        vector<future<bool> > futureVec;
        for (int nTh = 0; nTh < runCtrl.threadCount; nTh++) {

            futureVec.push_back(std::move(std::async(
                launch::async,
                modelThreadLoop,
                isMpiUsed,
                runCtrl.runId,
                runCtrl.subSampleCount,
                runCtrl.subBaseNumber + nTh,
                dbExec.get(),
                msgExec.get(),
                metaStore.get()
                )));
        }

        // get results from modeling threads
        bool isAnyOk = false;
        bool isAnyFailed = false;
        for (int nTh = 0; nTh < runCtrl.threadCount; nTh++) {
            bool isOk = futureVec[nTh].get();
            isAnyOk = isAnyOk || isOk;
            isAnyFailed = isAnyFailed || !isOk;
        }

        // mark run as failure and exit with error
        if (isAnyFailed) {
            ModelBase::shutdownOnFail(isMpiUsed, runCtrl.runId, dbExec.get(), msgExec.get());
            throw ModelException("modeling FAILED");
        }

        // final cleanup
        ModelBase::shutdown(
            isMpiUsed, runCtrl.runId, runCtrl.subSampleCount, dbExec.get(), msgExec.get(), metaStore.get()
            );
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
bool modelThreadLoop(
    bool i_isMpiUsed,
    int i_runId,
    int i_subCount,
    int i_subNumber,
    IDbExec * i_dbExec,
    IMsgExec * i_msgExec,
    const MetaRunHolder * i_metaStore
    )
{
    try {
#ifdef _DEBUG
        theLog->logFormatted("Subsample %d", i_subNumber);
#endif      
        // create the model
        unique_ptr<IModel> model(ModelBase::create(
            i_isMpiUsed, i_runId, i_subCount, i_subNumber, i_dbExec, i_msgExec, i_metaStore
            ));

        // initialize parameters
        ModelStartupHandler(model.get());

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
