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
*   * create modeling threads for each sub-value
* - run model event loop (do the simulation)  
* - model shutdown:  
*   * get output results for each sub-value  
*   * write output tables  
*   * do sub-values aggregation    
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

#include "libopenm/omVersion.h"
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

/** model one-time initialization */
OM_RUN_ONCE_HANDLER RunOnceHandler;

/** model run initialization: read input parameters */
OM_RUN_INIT_HANDLER RunInitHandler;

/** model startup method: initialize sub-value */
OM_STARTUP_HANDLER ModelStartupHandler;

/** model event loop: user code entry point */
OM_EVENT_LOOP_HANDLER RunModelHandler;

/** model shutdown method: save output results */
OM_SHUTDOWN_HANDLER ModelShutdownHandler;

/** process shutdown: last entry point from user code before process exit */
OM_RUN_SHUTDOWN_HANDLER RunShutdownHandler;

/** model thread function */
static ExitStatus modelThreadLoop(int i_runId, int i_subCount, int i_subId, RunController * i_runCtrl);

// run modeling threads to calculate sub-values
static ExitStatus runModelThreads(int i_runId, RunController * i_runCtrl);

// exchange between root and child modeling processes or between main therad and modeling threads, sleep if no child activity
static void childExchangeOrSleep(long i_waitTime, RunController * i_runCtrl);

// log model version, runtime version and modeling environment
static void logVersion(const IMsgExec * i_msgExec, const RunController * i_runCtrl);

/** main entry point */
int main(int argc, char ** argv) 
{
    try {
        // get model run options from command line and ini-file
        const ArgReader argOpts = MetaLoader::getRunOptions(argc, argv);

        // adjust log file(s) with actual log settings specified in model run options file
        string lfPath = argOpts.strOption(ArgKey::logFilePath);
        if (lfPath.empty()) {
            lfPath = string(OM_MODEL_NAME) + ".log";
        }
        theLog->init(
            argOpts.boolOption(ArgKey::logToConsole) || !argOpts.isOptionExist(ArgKey::logToConsole),
            lfPath.c_str(),
            argOpts.boolOption(ArgKey::logToFile) || !argOpts.isOptionExist(ArgKey::logToFile),
            argOpts.boolOption(ArgKey::logUseTs),
            argOpts.boolOption(ArgKey::logUsePid),
            argOpts.boolOption(ArgKey::logNoMsgTime),
            argOpts.boolOption(ArgKey::logSql)
            );

        // read language specific messages from path/to/exe/modelName.message.ini
        IniFileReader::loadMessages(
            makeFilePath(baseDirOf((argc > 0 ? argv[0] : "")).c_str(), OM_MODEL_NAME, ".message.ini").c_str(),
            argOpts.strOption(RunOptionsKey::messageLang)
        );
        theLog->logMsg(OM_MODEL_NAME);  // startup message: model name

        // if trace log file enabled setup trace file name
        string tfPath;
        if (argOpts.boolOption(RunOptionsKey::traceToFile)) {
            tfPath = argOpts.strOption(RunOptionsKey::traceFilePath);
            if (tfPath.empty()) tfPath = string(OM_MODEL_NAME) + ".trace.txt";
        }
        theTrace->init(
            argOpts.boolOption(RunOptionsKey::traceToConsole),
            tfPath.c_str(),
            argOpts.boolOption(RunOptionsKey::traceToFile),
            argOpts.boolOption(RunOptionsKey::traceUseTs),
            argOpts.boolOption(RunOptionsKey::traceUsePid),
            argOpts.boolOption(RunOptionsKey::traceNoMsgTime) || !argOpts.isOptionExist(RunOptionsKey::traceNoMsgTime)
            );

        // init message interface
        unique_ptr<IMsgExec> msgExec(IMsgExec::create(argc, argv, theModelRunState));

        if (argOpts.boolOption(RunOptionsKey::logRank)) theLog->setRank(msgExec->rank(), msgExec->worldSize());
        if (argOpts.boolOption(RunOptionsKey::traceRank)) theTrace->setRank(msgExec->rank(), msgExec->worldSize());

        try {
            // get db-connection string or use default if not specified
            string connectionStr = argOpts.strOption(
                RunOptionsKey::dbConnStr,
                string("Database=") + OM_MODEL_NAME + ".sqlite; Timeout=86400; OpenMode=ReadWrite;"
            );

            // open db-connection
            unique_ptr<IDbExec> dbExec(
                (!isMpiUsed || msgExec->isRoot()) ? IDbExec::create(SQLITE_DB_PROVIDER, connectionStr) : nullptr
            );

            // load metadata tables and broadcast it to all modeling processes
            unique_ptr<RunController> runCtrl(RunController::create(argOpts, isMpiUsed, dbExec.get(), msgExec.get()));

            // log model version, runtime version and modeling environment
            logVersion(msgExec.get(), runCtrl.get());

            // model one-time initialization
            if (RunOnceHandler != NULL) RunOnceHandler(runCtrl.get());

            if (string rs = runCtrl->argOpts().strOption(ArgKey::runStamp); !rs.empty()) {
                theLog->logFormatted("Run: %s", rs.c_str());
            }

            // run the model until modeling task completed
            ExitStatus e = ExitStatus::OK;

            while (!theModelRunState->isShutdownOrFinal()) {

                // create next run id: find model input data set
                int runId = runCtrl->nextRun();
                if (runId <= 0) {
                    childExchangeOrSleep(OM_WAIT_SLEEP_TIME, runCtrl.get());    // exchange between root and child processes or between main thread and modeling threads
                    continue;                                                   // no input: completed or waiting for additional input
                }
                theLog->logFormatted("Run: %d %s", runId, runCtrl->strOption(RunOptionsKey::runName).c_str());

                // initialize model run: read input parameters
                RunInitHandler(runCtrl.get());

                // do the modeling: run modeling threads to calculate sub-values
                e = runModelThreads(runId, runCtrl.get());
                if (e != ExitStatus::OK) {
                    theModelRunState->updateStatus(ModelStatus::error); // initiate process exit by error
                    break;
                }

                // run completed OK, receive and write the data
                runCtrl->shutdownRun(runId);
            }

            // if model completed OK at local process then wait for all child to be completed and do final cleanup
            // else shutdown process with error
            if (e == ExitStatus::OK && !theModelRunState->isError()) {
                runCtrl->shutdownWaitAll();
                if (RunShutdownHandler != NULL) RunShutdownHandler(false, runCtrl.get());
            }
            else {
                runCtrl->shutdownOnExit(ModelStatus::error);
                if (RunShutdownHandler != NULL) {
                    RunShutdownHandler(true, runCtrl.get());
                }
                return (int)(e != ExitStatus::OK ? e : ExitStatus::FAIL);
            }
        }
        catch (SimulationException & ex) {
            theLog->logErr(ex, "Simulation error");
            return (int)ExitStatus::SIMULATION_ERROR;
        }
        catch (ModelException & ex) {
            theLog->logErr(ex, "Model error");
            return (int)ExitStatus::MODEL_ERROR;
        }
        catch (DbException & ex) {
            theLog->logErr(ex, "DB error");
            return (int)ExitStatus::DB_ERROR;
        }
    }
    catch (MsgException & ex) {
        theLog->logErr(ex, "Messaging error");
        return (int)ExitStatus::MSG_ERROR;
    }
    catch (HelperException & ex) {
        theLog->logErr(ex, "Helper error");
        return (int)ExitStatus::HELPER_ERROR;
    }
    catch (exception & ex) {
        theLog->logErr(ex);
        return (int)ExitStatus::FAIL;
    }
    catch (...) {    // exit with failure on unhandled exception
        theLog->logMsg("FAILED", OM_FILE_LINE);
        return (int)ExitStatus::FAIL;
    }

    theLog->logMsg("Done.");
    return (int)ExitStatus::OK;
}

/** model thread function */
ExitStatus modelThreadLoop(int i_runId, int i_subCount, int i_subId, RunController * i_runCtrl)
{
    ExitStatus e = ExitStatus::FAIL;
    try {
#ifdef _DEBUG
        theLog->logFormatted("Sub-value: %d", i_subId);
#endif      
        // create the model
        unique_ptr<IModel> model(
            ModelBase::create(i_runId, i_subCount, i_subId, i_runCtrl, i_runCtrl->meta())
            );
        i_runCtrl->runStateStore().add(i_runId, i_subId);

        // initialize model sub-value
        if (ModelStartupHandler != NULL) ModelStartupHandler(model.get());

        // do the modeling
        RunModelHandler(model.get());

        // write output tables and update final run status
        ModelShutdownHandler(model.get());
        i_runCtrl->runStateStore().updateStatus(i_runId, i_subId, ModelStatus::done, true);

        return ExitStatus::OK;      // model sub-value completed OK
    }
    catch (SimulationException & ex) {
        theLog->logErr(ex, "Simulation error");
        e = ExitStatus::SIMULATION_ERROR;
    }
    catch (ModelException & ex) {
        theLog->logErr(ex, "Model error");
        e = ExitStatus::MODEL_ERROR;
    }
    catch (DbException & ex) {
        theLog->logErr(ex, "DB error");
        e = ExitStatus::DB_ERROR;
    }
    catch (MsgException & ex) {
        theLog->logErr(ex, "Messaging error");
        e = ExitStatus::MSG_ERROR;
    }
    catch (HelperException & ex) {
        theLog->logErr(ex, "Helper error");
        e = ExitStatus::HELPER_ERROR;
    }
    catch (exception & ex) {
        theLog->logErr(ex);
        e = ExitStatus::FAIL;
    }
    catch (...) {    // exit with failure on unhandled exception
        theLog->logMsg("FAILED", OM_FILE_LINE);
        e = ExitStatus::FAIL;
    }

    // modeling thread failed, initiate shutdown of modeling process
    i_runCtrl->runStateStore().updateStatus(i_runId, i_subId, ModelStatus::error, true);
    return e;
}

// run modeling threads to calculate sub-values
ExitStatus runModelThreads(int i_runId, RunController * i_runCtrl)
{
    list<future<ExitStatus>> modelFutureLst;    // modeling threads

    int nextSub = 0;
    while (nextSub < i_runCtrl->selfSubCount || modelFutureLst.size() > 0) {

        // create and start new modeling threads
        while (nextSub < i_runCtrl->selfSubCount && (int)modelFutureLst.size() < i_runCtrl->threadCount) {
            modelFutureLst.push_back(
                std::move(std::async(
                    launch::async,
                    modelThreadLoop,
                    i_runId,
                    i_runCtrl->subValueCount,
                    i_runCtrl->subFirstId + nextSub,
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

            // modeling completed: get result success or error
            ExitStatus e = mfIt->get();
            if (e != ExitStatus::OK) {
                return e;   // exit with error to initiate shutdown of modeling process
            }
            mfIt = modelFutureLst.erase(mfIt);  // remove thread from the list
        }

        // wait if no modeling progress and any threads still running
        if (!isAnyCompleted && modelFutureLst.size() > 0) {
            childExchangeOrSleep(2L * OM_ACTIVE_SLEEP_TIME, i_runCtrl); // communicate with child processes and threads, if any, or sleep
        }
    }

    // all process sub-values completed OK
    return ExitStatus::OK;
}

// exchange between root and child modeling processes or between main therad and modeling threads, sleep if no child activity
void childExchangeOrSleep(long i_waitTime, RunController * i_runCtrl)
{
    long nExchange = 1 + i_waitTime / OM_ACTIVE_SLEEP_TIME;

    bool isAnyChildActivity = false;
    do {
        if (theModelRunState->isFinal()) return;   // model completed

        isAnyChildActivity = i_runCtrl->childExchange();    // exchange between root and child processes and threads, if any
        if (isAnyChildActivity) {
            this_thread::sleep_for(chrono::milliseconds(OM_ACTIVE_SLEEP_TIME));
        }
    }
    while (isAnyChildActivity && --nExchange > 0);

    if (nExchange > 0 && i_runCtrl->processCount > 1 && !theModelRunState->isFinal()) {
        this_thread::sleep_for(chrono::milliseconds(nExchange * OM_ACTIVE_SLEEP_TIME)); // sleep more if no child activity
    }
}

/**
* OpenM++ build environment to report at run time as part of the model version message
*
* - platform name: Windows 32 bit, Windows 64 bit, Linux or Apple (MacOS)
* - configuration: Release or Debug
* - MPI usage enabled or not
*/
#if !defined(_WIN32) && !defined(__linux__) && !defined(__APPLE__)
static const char * libTargetOsName = "";
#else
#ifdef _WIN32
#ifdef _WIN64
static const char * libTargetOsName = "Windows 64 bit";
#else
static const char * libTargetOsName = "Windows 32 bit";
#endif
#endif // _WIN32

#ifdef __linux__
static const char * libTargetOsName = "Linux";
#endif

#ifdef __APPLE__
#include <TargetConditionals.h>
#if defined(TARGET_OS_OSX) && TARGET_OS_OSX
static const char * libTargetOsName = "macOS";
#else
static const char * libTargetOsName = "Apple OS";
#endif
#endif // __APPLE__
#endif

#ifdef NDEBUG
static const char * libTargetConfigName = "Release";
#else
static const char * libTargetConfigName = "Debug";
#endif // NDEBUG

#ifdef OM_MSG_MPI
static const char * libTargetMpiUseName = "MPI";
#else
static const char * libTargetMpiUseName = "";
#endif // OM_MSG_MPI

// log model version, runtime version and modeling environment:
//  - number of modeling processes and threads
//  - openM++ root environment variable, if defined
//  - model root environment variable, if defined
void logVersion(const IMsgExec * i_msgExec, const RunController * i_runCtrl)
{
    theLog->logMsg("Model version  ", i_runCtrl->meta()->modelRow->version.c_str());
    theLog->logMsg("Model created  ", i_runCtrl->meta()->modelRow->createDateTime.c_str());
    theLog->logMsg("Model digest   ", OM_MODEL_DIGEST);
#ifdef OM_RUNTIME_VERSION
    theLog->logMsg("OpenM++ version", OM_RUNTIME_VERSION);
#endif
    theLog->logFormatted("OpenM++ build  : %s %s %s", libTargetOsName, libTargetConfigName, libTargetMpiUseName);

    if (!isMpiUsed && i_runCtrl->threadCount > 1) {
        theLog->logFormatted("Run model with %d modeling thread(s)", i_runCtrl->threadCount);
    }
    if (isMpiUsed && i_msgExec->isRoot()) {
        if (i_msgExec->worldSize() > 1) {
            theLog->logFormatted("Parallel run of %d modeling processes, %d thread(s) each", i_msgExec->worldSize(), i_runCtrl->threadCount);
        }
        else {
            theLog->logFormatted("Run single model process with %d modeling thread(s)", i_runCtrl->threadCount);
        }
    }

    // report model environment roots, if such envrinment variables defined, example:
    //  OM_ROOT=../../../ 
    //  OM_MyModel=../../
    {
        if (const char * val = getenv("OM_ROOT")) {
            theLog->logFormatted("OM_ROOT=%s", val);
        }
        string merName = string("OM_") + OM_MODEL_NAME;
        if (const char * val = getenv(merName.c_str())) {
            theLog->logFormatted("%s=%s", merName.c_str(), val);
        }
    }
}
