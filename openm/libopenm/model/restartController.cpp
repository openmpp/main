/**
* @file
* OpenM++ modeling library:
* process run controller class for model "restart run" to calculate outstanding subsamples for existing run.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "model.h"
#include "modelHelper.h"
#include "runControllerImpl.h"

using namespace std;
using namespace openm;

/** initialize modeling process to calculate outstanding subsamples for existing run.
*
* - (a) get run id and number of threads
* - (b) merge command line and ini-file options with db profile table
* - (d) load metadata tables from database
*
* (a) get run id and number of threads
* ------------------------------------
* Run id must be specified by command-line argument or .ini-file 
* and that run id must already exist in database.
* 
* Number of modeling threads by default = 1 and subsamples run sequentially in single thread.
* If more threads specified (i.e. by command-line argument) then subsamples run in parallel.
*
* For example: \n
*   model.exe -OpenM.RunId 10 \n
*   model.exe -OpenM.RunId 10 -General.Threads 4
*
* (b) merge command line and ini-file options with db profile table
*------------------------------------------------------------------
* model run options can be specified as (in order of priority):
*
*   - command line arguments, i.e.: \n
*       model.exe -Parameter.Population 1234
*
*   - through ini-file: \n
*       model.exe -s modelOne.ini
*
*   - as rows of profile_option table, default profile_name='model name'
*     profile name also can be command line argument or ini-file entry, i.e.: \n
*       model.exe -OpenM.OptionsProfile TestProfile
*
*   - some options have hard coded default values which used if nothing above is specified
*
*   run options which name starts with "Parameter." are treated as
*   value of scalar input parameter (see below). \n
*   if there is no input parameter with such name then exception raised. \n
*   for example, if command line is: \n
*       model.exe -Parameter.Population 1234 \n
*   and model does not have "Population" parameter then execution aborted.
*/
void RestartController::init(void)
{
    if (dbExec == nullptr) throw ModelException("invalid (NULL) database connection");

    // load metadata table rows, except of run_option, which is may not created yet
    metaStore.reset(new MetaRunHolder());
    modelId = readMetaTables(dbExec, metaStore.get());

    // merge command line and ini-file arguments with profile_option table values
    mergeProfile(dbExec);

    // "restart run": run id must be defined
    runId = argOpts().intOption(RunOptionsKey::runId, 0);

    if (runId <= 0) throw ModelException("Invalid run id: %d", runId);

    // get main run control values: number of subsamples, threads
    // if number of subsamples sepcified then it must be same as run_lst.sub_count database value
    threadCount = argOpts().intOption(RunOptionsKey::threadCount, 1);       // max number of modeling threads
    subSampleCount = argOpts().intOption(RunOptionsKey::subSampleCount, 1); // number of subsamples from command line or ini-file

    // basic validation: only single process allowed for "restart run"
    if (subSampleCount <= 0) throw ModelException("Invalid number of subsamples: %d", subSampleCount);
    if (threadCount <= 0) throw ModelException("Invalid number of modeling threads: %d", threadCount);

    isSubDone.init(subSampleCount);     // reset write status for subsamples
}

/** restart existing model run.
*
* - (a) find model run and get number of outstanding subsamples
* - (b) delete any unfinished subsample(s)
* - (c) prepare model input parameters (if required)
*
* (a) find model run and get number of outstanding subsamples
* -----------------------------------------------------------
* model run id supplied by command-line or ini-file argument and must exist in run_lst table. \n
* subsample to restart from is value of run_lst.sub_restart column.
*
* number of outstanding subsamples = total number of subsamples - run_lst.sub_restart
*
* if number of outstanding subsamples <= 0 and run_lst.status != success \n
* then output tables aggregated values re-calculated
*
* (b) delete any unfinished subsample(s)
* --------------------------------------
* - delete from all accumulator tables where sub_id >= sub_restart
* - delete from output aggregated value tables where run_id = run id to restart
*
* (c) prepare model input parameters
* ----------------------------------
* it creates a copy of input paramters from source working set under destination run_id
*
* search for input parameter value in following order: \n
*   - use value of parameter specified as command line or ini-file argument
*   - use value of parameter from working set of model parameters
*   - use value of parameter from profile_option table or any other run options source
*   - if working set based on model run then search by base run id to get parameter value
*   - else raise an exception
*
*   any scalar parameter value can be overriden by model run option with "Parameter" prefix \n
*   for example, command line: \n
*       model.exe -Parameter.Population 1234 \n
*   means input parameter with name "Population" will be =1234 \n
*   in that case working set value of "Population" input parameter ignored \n
*   because command line options have higher priority than database values.
*/
int RestartController::nextRun(void)
{
    if (dbExec == nullptr) throw ModelException("invalid (NULL) database connection");

    // get number of subsamples and subsample number to restart from
    // delete any existing output accumulators where sub_id >= restart subsample number
    // delete output aggregated values for that run id
    if (!cleanupRestartSubsample()) {
        theModelRunState.updateStatus(ModelStatus::shutdown);
        runId = 0;
        return 0;           // run already completed
    }

    // if all subsamples calculated and run status not success 
    // then do run shutdown to re-calculate output aggregated values
    if (subFirstNumber >= subSampleCount) {

        shutdownRun(runId);

        theModelRunState.updateStatus(ModelStatus::shutdown);
        runId = 0;
        return 0;           // run completed
    }

    // reset write status for subsamples and set for subsamples below restart
    isSubDone.init(subSampleCount);
    for (int k = 0; k < subFirstNumber; k++) {
        isSubDone.setAt(k);
    }

    return runId;
}

// find subsample to restart the run and update run status
// delete any existing output accumulators where sub_id >= restart subsample number
// delete run_table rows for that run_id
// delete output aggregated values for that run id
bool RestartController::cleanupRestartSubsample(void)
{
    // get current status of that run
    vector<RunLstRow> runVec = IRunLstTable::byKey(dbExec, runId);
    if (runVec.empty()) throw DbException("run id: %d not found in the database", runId);

    const RunLstRow & runRow = runVec[0];

    // if run already completed then exit
    if (runRow.status == RunStatus::done) return false;

    // get number of subsamples and subsample number to restart from
    int nSubCount = runRow.subCount;
    if (nSubCount <= 0) throw ModelException("Invalid number of subsamples: %d", nSubCount);
    if (subSampleCount > 1 && nSubCount != subSampleCount)
        throw DbException("invalid number of sub-samples: %d, it must be %d (run id: %d)", subSampleCount, nSubCount, runId);

    subSampleCount = nSubCount;
    subFirstNumber = (runRow.subRestart > 0) ? runRow.subRestart : 0;
    if (subFirstNumber > subSampleCount) subFirstNumber = subSampleCount;

    selfSubCount = subSampleCount - subFirstNumber;     // all subsamples calculated by current process

#ifdef _DEBUG
    theLog->logFormatted("Restart from subsample %d", subFirstNumber);
#endif      

    // update in transaction scope
    {
        unique_lock<recursive_mutex> lck = dbExec->beginTransactionThreaded();

        string sRunId = to_string(runId);
        string dtStr = makeDateTime(chrono::system_clock::now());

        dbExec->update(
            "UPDATE run_lst SET status = " + toQuoted(RunStatus::progress) + "," +
            " sub_started = " + to_string(subSampleCount) + "," +
            " sub_completed = " + to_string(subFirstNumber) + "," +
            " sub_restart = " + to_string(subFirstNumber) + "," +
            " update_dt = " + toQuoted(dtStr) +
            " WHERE run_id = " + sRunId
        );

        // delete all run_table rows to remove potentially incomplete output values digests
        dbExec->update(
            "DELETE FROM run_table WHERE run_id = " + to_string(runId)
        );

        // delete all output aggregated values
        const vector<TableDicRow> tblVec = metaStore->tableDic->byModelId(modelId);

        for (const TableDicRow & tblRow : tblVec) {
            dbExec->update(
                "DELETE FROM " + tblRow.dbExprTable + " WHERE run_id = " + to_string(runId)
            );
        }

        // delete all accumulators starting from first subsample number
        if (subFirstNumber < subSampleCount) {

            for (const TableDicRow & tblRow : tblVec) {
                dbExec->update(
                    "DELETE FROM " + tblRow.dbAccTable +
                    " WHERE run_id = " + to_string(runId) +
                    " AND sub_id >= " + to_string(subFirstNumber)
                );
            }
        }

        // completed: commit the changes
        dbExec->commit();
    }
    return true;
}

/**
* read input parameter values.
*
* @param[in]     i_name      parameter name
* @param[in]     i_type      parameter type
* @param[in]     i_size      parameter size (number of parameter values)
* @param[in,out] io_valueArr array to return parameter values, size must be =i_size
*/
void RestartController::readParameter(const char * i_name, const type_info & i_type, size_t i_size, void * io_valueArr)
{
    if (i_name == NULL || i_name[0] == '\0') throw ModelException("invalid (empty) input parameter name");

    try {
        // read parameter from db
        unique_ptr<IParameterReader> reader(
            IParameterReader::create(runId, i_name, dbExec, metaStore.get())
            );
        reader->readParameter(dbExec, i_type, i_size, io_valueArr);
    }
    catch (exception & ex) {
        throw ModelException("Failed to read input parameter: %s. %s", i_name, ex.what());
    }
}

/** write output table accumulators.
*
* @param[in]     i_runOpts      model run options
* @param[in]     i_isLastTable  if true then it is last output table to write
* @param[in]     i_name         output table name
* @param[in]     i_size         number of cells for each accumulator
* @param[in,out] io_accValues   accumulator values
*/
void RestartController::writeAccumulators(
    const RunOptions & i_runOpts,
    bool i_isLastTable,
    const char * i_name,
    size_t i_size,
    forward_list<unique_ptr<double> > & io_accValues
    )
{
    // write accumulators into database
    doWriteAccumulators(runId, dbExec, i_runOpts, i_name, i_size, io_accValues);

    // if all accumulators of subsample completed then update restart subsample number
    if (i_isLastTable) {
        isSubDone.setAt(i_runOpts.subSampleNumber);     // mark that subsample as completed
        updateRestartSubsample(runId, dbExec, isSubDone.countFirst());
    }
}
