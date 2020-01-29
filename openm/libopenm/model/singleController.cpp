/**
* @file
* OpenM++ modeling library: 
* single process run controller class to create new model run(s), read input parameters and write output tables.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "model.h"
#include "modelHelper.h"
#include "runControllerImpl.h"

using namespace std;
using namespace openm;

/** initialize modeling process.
*
* - (a) get number of sub-values, processes, threads
* - (b) merge command line and ini-file options with db profile table
* - (c) initialize task run if required
* - (d) load metadata tables from database
*
* (a) get number of sub-values, processes, threads
* ------------------------------------------------
*
* Number of sub-values by default = 1 and it can be specified
* using command-line argument, ini-file or profile table in database
*
* Number of modeling threads by default = 1 and sub-values run sequentially in single thread.
* If more threads specified (i.e. by command-line argument) then sub-values run in parallel.
*
* For example: \n
*   model.exe -OpenM.SubValues 8 \n
*   model.exe -OpenM.SubValues 8 -OpenM.Threads 4 \n
*   mpiexec -n 2 model.exe -OpenM.SubValues 31 -OpenM.Threads 7
*
* Number of modeling processes expected to be 1 \n
* Number of sub-values calculated by process is same as total number of sub-values
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
*   - as rows of profile_option table: \n
*     profile name can be command line argument or ini-file entry, i.e.: \n
*       model.exe -OpenM.Profile TestProfile
*
*   - some options have hard coded default values which used if nothing above is specified
*
*   run options which name starts with "Parameter." are treated as
*   value of scalar input parameter (see below). \n
*   if there is no input parameter with such name then exception raised. \n
*   for example, if command line is: \n
*       model.exe -Parameter.Population 1234 \n
*   and model does not have "Population" parameter then execution aborted.
*
* (c) initialize task run
* -----------------------
* modeling task can be specified by name or by task id:
*
*   model.exe -OpenM.SubValues 16 -OpenM.TaskName someTask \n
*   model.exe -OpenM.SubValues 16 -OpenM.Taskid 1 \n
*   model.exe -OpenM.SubValues 16 -OpenM.TaskName someTask -OpenM.TaskWait true
*
* if task specified then model would run for each input working set of parameters \n
* if task "wait" is true then modeling task is run under external supervision: \n
*   - model must wait until task status set as "completed" by other external process \n
*   - external process can append new inputs into task_set table and model will continue to run \n
*/
void SingleController::init(void)
{
    if (dbExec == nullptr) throw ModelException("invalid (NULL) database connection");

    // load metadata table rows, except of run_option, which is may not created yet
    // load messages from database
    metaStore.reset(new MetaHolder);
    modelId = readMetaTables(dbExec, metaStore.get(), OM_MODEL_NAME, OM_MODEL_DIGEST);
    loadMessages(dbExec);

    // merge command line and ini-file arguments with profile_option table values
    mergeOptions(dbExec);

    // get main run control values: number of sub-values, threads
    subValueCount = argOpts().intOption(RunOptionsKey::subValueCount, 1);   // number of sub-values from command line or ini-file
    threadCount = argOpts().intOption(RunOptionsKey::threadCount, 1);       // max number of modeling threads
    isWaitTaskRun = argOpts().boolOption(RunOptionsKey::taskWait);          // if true then task run under external supervision

    // basic validation: single process expected
    if (subValueCount <= 0) throw ModelException("Invalid number of sub-values: %d", subValueCount);
    if (threadCount <= 0) throw ModelException("Invalid number of modeling threads: %d", threadCount);

    // all sub-values calculated at current process
    subFirstId = 0;
    selfSubCount = subValueCount;
    isSubDone.init(subValueCount);

    // if this is modeling task then find it in database
    // and create task run entry in database
    taskId = findTask(dbExec);
    if (taskId > 0) taskRunId = createTaskRun(taskId, dbExec);
}

/** create new run and input parameters in database.
*
* - (a) find id of source working set for input parameters
* - (b) create new "run" in database (if required)
* - (c) prepare model input parameters
*
* (a) find id of source working set for input parameters
* ------------------------------------------------------
* use following to find input parameters set id: \n
*   - if task id or task name specified then find task id in task_lst \n
*   - if set id specified as run option then use such set id \n
*   - if set name specified as run option then find set id by name \n
*   - else use min(set id) as default set of model parameters
*
* (b) create new "run" in database
* --------------------------------
* root process creates "new run" in database:
*
*   - insert new row with new run_id key into run_lst
*   - insert run options into run_option table under run_id
*   - save run options in database
*
* (c) prepare model input parameters
* ----------------------------------
* it creates a copy of input paramters from source working set under destination run_id
*
* search for input parameter value in following order: \n
*   - use value of parameter specified as command line or ini-file argument or from profile_option table
*   - use parameter.csv file if parameters csv directory specified
*   - use value of parameter from working set of model parameters
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
int SingleController::nextRun(void)
{
    if (dbExec == nullptr) throw ModelException("invalid (NULL) database connection");

    // create new run:
    // find next working set of input parameters
    // if this is a modeling task then next set from that task
    // else if no run completed then get set by id or name or as default set for the model
    auto [nSetId, nRunId, mStatus] = createNewRun(taskRunId, isWaitTaskRun, dbExec);
    runId = nRunId;

    mStatus = theModelRunState->updateStatus(mStatus);  // update model status: progress, wait, shutdown

    if (RunState::isShutdownOrFinal(mStatus) || nSetId <= 0 || nRunId <= 0) {
        return 0;   // all done: all sets from task or single run completed
    }

    // reset write status for sub-values
    isSubDone.reset();

    return runId;
}

/**
* read input parameter values.
*
* @param[in]     i_name      parameter name
* @param[in]     i_subId     parameter sub-value index
* @param[in]     i_type      parameter type
* @param[in]     i_size      parameter size (number of parameter values)
* @param[in,out] io_valueArr array to return parameter values, size must be =i_size
*/
void SingleController::readParameter(const char * i_name, int i_subId, const type_info & i_type, size_t i_size, void * io_valueArr)
{
    if (i_name == nullptr || i_name[0] == '\0') throw ModelException("invalid (empty) input parameter name");

    try {
        // read parameter from db
        unique_ptr<IParameterReader> reader(
            IParameterReader::create(runId, i_name, dbExec, meta())
            );
        reader->readParameter(dbExec, i_subId, i_type, i_size, io_valueArr);
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
void SingleController::writeAccumulators(
    const RunOptions & i_runOpts,
    bool i_isLastTable,
    const char * i_name,
    size_t i_size,
    forward_list<unique_ptr<double> > & io_accValues
    )
{
    // write accumulators into database
    doWriteAccumulators(runId, dbExec, i_runOpts, i_name, i_size, io_accValues);

    // if all accumulators of sub-value completed then update restart sub-value index
    if (i_isLastTable) {
        isSubDone.setAt(i_runOpts.subValueId);      // mark that sub-value as completed
        updateRestartSubValueId(runId, dbExec, isSubDone.countFirst());
    }
}

/** communicate with child threads to receive status update. */
bool SingleController::childExchange(void)
{
    if (dbExec == nullptr) throw ModelException("invalid (NULL) database connection");

    auto stm = runStateStore().saveUpdated();
    if (stm.size() > 0) {
        updateRunState(dbExec, stm);
        return true;
    }
    return false;
}
