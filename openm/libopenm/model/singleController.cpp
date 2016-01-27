/**
* @file
* OpenM++ modeling library: 
* single process run controller class to create new model run(s), read input parameters and write output tables.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "model.h"

using namespace std;
using namespace openm;

/** initialize modeling process.
*
* - (a) get number of subsamples, processes, threads
* - (b) merge command line and ini-file options with db profile table
* - (c) initialize task run if required
* - (d) load metadata tables from database
*
* (a) get number of subsamples, processes, threads
* ------------------------------------------------
*
* Number of subsamples by default = 1 and it can be specified
* using command-line argument, ini-file or profile table in database
*
* Number of modeling threads by default = 1 and subsamples run sequentially in single thread.
* If more threads specified (i.e. by command-line argument) then subsamples run in parallel.
*
* For example: \n
*   model.exe -General.Subsamples 8 \n
*   model.exe -General.Subsamples 8 -General.Threads 4 \n
*   mpiexec -n 2 model.exe -General.Subsamples 31 -General.Threads 7
*
* Number of modeling processes expected to be 1
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
*
* (c) initialize task run
* -----------------------
* modeling task can be specified by name or by task id:
*
*   model.exe -General.Subsamples 16 -OpenM.TaskName someTask \n
*   model.exe -General.Subsamples 16 -OpenM.Taskid 1 \n
*
* if task specified then model would run for each input working set of parameters
*/
void SingleController::init(void)
{
    if (dbExec == nullptr) throw ModelException("invalid (NULL) database connection");

    // load metadata table rows, except of run_option, which is may not created yet
    metaStore.reset(new MetaRunHolder);
    const ModelDicRow * mdRow = readMetaTables(dbExec, metaStore.get());
    modelId = mdRow->modelId;

    // merge command line and ini-file arguments with profile_option table values
    mergeProfile(dbExec, mdRow);

    // get main run control values: number of subsamples, threads
    subSampleCount = argOpts().intOption(RunOptionsKey::subSampleCount, 1); // number of subsamples from command line or ini-file
    threadCount = argOpts().intOption(RunOptionsKey::threadCount, 1);       // max number of modeling threads

    // basic validation: single process expected
    if (subSampleCount <= 0) throw ModelException("Invalid number of subsamples: %d", subSampleCount);
    if (processCount != 1) throw ModelException("Invalid number of modeling processes: %d (it must be 1)", processCount);
    if (threadCount <= 0) throw ModelException("Invalid number of modeling threads: %d", threadCount);
    //if (subSampleCount < processCount)
    //    throw ModelException("Error: number of subsamples: %d less than number of processes: %d", subSampleCount, processCount);

    // if this is modeling task then find it in database
    taskId = findTask(dbExec, mdRow);

    if (taskId > 0) {
        taskRunLst = readTask(taskId, mdRow, dbExec);

        // create task run entry in database
        taskRunId = createTaskRun(taskId, dbExec);
    }
}

/** create new run and input parameters in database.
*
* - (a) get number of subsamples calculated by current process
* - (b) find id of source working set for input parameters
* - (c) create new "run" in database (if required)
* - (d) prepare model input parameters
*
* (a) get number of subsamples calculated by current process
* ----------------------------------------------------------
* number of subsamples calculated by process is same as total number of subsamples
*
* (b) find id of source working set for input parameters
* ------------------------------------------------------
* use following to find input parameters set id: \n
*   - if task id or task name specified then find task id in task_lst \n
*   - if set id specified as run option then use such set id \n
*   - if set name specified as run option then find set id by name \n
*   - else use min(set id) as default set of model parameters
*
* (c) create new "run" in database
* --------------------------------
* root process creates "new run" in database:
*
*   - insert new row with new run_id key into run_lst
*   - insert run options into run_option table under run_id
*   - save run options in database
*
* (d) prepare model input parameters
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
int SingleController::nextRun(void)
{
    if (dbExec == nullptr) throw ModelException("invalid (NULL) database connection");

    // find next working set of input parameters
    // if this is a modeling task then next set from that task
    // else if no run completed then get set by id or name or as default set for the model
    setId = 0;
    if (taskId > 0 || runId <= 0) setId = nextSetId(taskId, dbExec, taskRunLst);

    if (setId <= 0) return 0;       // all done: all sets from task or single run

    // all subsamples calculated at current process
    subFirstNumber = 0;
    selfSubCount = subPerProcess = subSampleCount;

    //if (subFirstNumber < 0 || subPerProcess <= 0 || selfSubCount <= 0 || subFirstNumber + selfSubCount > subSampleCount)
    //    throw ModelException(
    //        "Invalid first subsample number: %d or number of subsamples: %d or subsamples per process: %d", subFirstNumber, selfSubCount, subPerProcess
    //        );

    // create new run and load run_option table rows
    runId = createNewRun(setId, taskId, taskRunId, dbExec, taskRunLst);
    metaStore->runOption.reset(IRunOptionTable::create(dbExec, runId));

    // reset write status for subsamples
    subDoneVec.assign(subSampleCount, false);

    return runId;
}

/**
* read input parameter values.
*
* @param[in]     i_name      parameter name
* @param[in]     i_type      parameter type
* @param[in]     i_size      parameter size (number of parameter values)
* @param[in,out] io_valueArr array to return parameter values, size must be =i_size
*/
void SingleController::readParameter(const char * i_name, const type_info & i_type, long long i_size, void * io_valueArr)
{
    if (i_name == NULL || i_name[0] == '\0') throw ModelException("invalid (empty) input parameter name");

    try {
        // read parameter from db
        unique_ptr<IParameterReader> reader(
            IParameterReader::create(modelId, runId, i_name, dbExec, metaStore.get())
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
* @param[in]     i_isLast       if true then it is last output table to write
* @param[in]     i_name         output table name
* @param[in]     i_size         number of cells for each accumulator
* @param[in,out] io_accValues   accumulator values
*/
void SingleController::writeAccumulators(
    const RunOptions & i_runOpts,
    bool i_isLast,
    const char * i_name,
    long long i_size,
    forward_list<unique_ptr<double> > & io_accValues
    )
{
    // write accumulators into database
    doWriteAccumulators(runId, dbExec, i_runOpts, i_name, i_size, io_accValues);

    // if all accumulators of subsample completed then update restart subsample number
    if (i_isLast) {
        subDoneVec[i_runOpts.subSampleNumber] = true;    // mark that subsample as completed
        updateRestartSubsample(runId, dbExec, subDoneVec);
    }
}

