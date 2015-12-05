/**
* @file
* OpenM++ modeling library: run controller class to create new model run and support data exchange
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "model.h"
#include "runController.h"

using namespace std;
using namespace openm;

namespace openm
{
    /** compatibility short name for options file name: -s fileName.ini */
    const char * RunShortKey::optionsFile = "s";

    /** short name for options file name: -ini fileName.ini */
    const char * RunShortKey::optionsFileIni = "ini";

    /** parameters started with "Parameter." treated as value of model scalar input parameters */
    const char * RunOptionsKey::parameterPrefix = "Parameter";

    /** number of sub-samples */
    const char * RunOptionsKey::subSampleCount = "General.Subsamples";

    /** number of modeling threads */
    const char * RunOptionsKey::threadCount = "General.Threads";

    /** database connection string */
    const char * RunOptionsKey::dbConnStr = "OpenM.Database";

    /** model run id */
    const char * RunOptionsKey::runId = "OpenM.RunId";

    /** working set id to get input parameters */
    const char * RunOptionsKey::setId = "OpenM.SetId";

    /** working set name to get input parameters */
    const char * RunOptionsKey::setName = "OpenM.SetName";

    /** modelling task id */
    const char * RunOptionsKey::taskId = "OpenM.TaskId";

    /** modelling task name */
    const char * RunOptionsKey::taskName = "OpenM.TaskName";

    /** profile name to get run options, default is model name */
    const char * RunOptionsKey::profile = "OpenM.OptionsProfile";

    /** use sparse output tables */
    const char * RunOptionsKey::useSparse = "OpenM.SparseOutput";

    /** sparse NULL value */
    const char * RunOptionsKey::sparseNull = "OpenM.SparseNullValue";

    /** trace log to console */
    const char * RunOptionsKey::traceToConsole = "OpenM.TraceToConsole";

    /** trace log to file */
    const char * RunOptionsKey::traceToFile = "OpenM.TraceToFile";

    /** trace log file path */
    const char * RunOptionsKey::traceFilePath = "OpenM.TraceFilePath";

    /** trace log to "stamped" file */
    const char * RunOptionsKey::traceToStamped = "OpenM.TraceToStampedFile";

    /** trace use time-stamp in log "stamped" file name */
    const char * RunOptionsKey::traceUseTs = "OpenM.TraceUseTimeStamp";

    /** trace use pid-stamp in log "stamped" file name */
    const char * RunOptionsKey::traceUsePid = "OpenM.TraceUsePidStamp";

    /** do not prefix trace log messages with date-time */
    const char * RunOptionsKey::traceNoMsgTime = "OpenM.TraceNoMsgTime";
}

/** array of model run option keys. */
static const char * runOptKeyArr[] = {
    RunOptionsKey::subSampleCount,
    RunOptionsKey::threadCount,
    RunOptionsKey::dbConnStr,
    RunOptionsKey::runId,
    RunOptionsKey::setId,
    RunOptionsKey::setName,
    RunOptionsKey::taskId,
    RunOptionsKey::taskName,
    RunOptionsKey::profile,
    RunOptionsKey::useSparse,
    RunOptionsKey::sparseNull,
    RunOptionsKey::traceToConsole,
    RunOptionsKey::traceToFile,
    RunOptionsKey::traceFilePath,
    RunOptionsKey::traceToStamped,
    RunOptionsKey::traceUseTs,
    RunOptionsKey::traceUsePid,
    RunOptionsKey::traceNoMsgTime,
    ArgKey::optionsFile,
    ArgKey::logToConsole,
    ArgKey::logToFile,
    ArgKey::logFilePath,
    ArgKey::logToStamped,
    ArgKey::logUseTs,
    ArgKey::logUsePid,
    ArgKey::logNoMsgTime,
    ArgKey::logSql
};
static const size_t runOptKeySize = sizeof(runOptKeyArr) / sizeof(const char *);

/** array of short and full option names, used to find full option name by short. */
static const pair<const char *, const char *> shortPairArr[] = 
{
    make_pair(RunShortKey::optionsFile, ArgKey::optionsFile),
    make_pair(RunShortKey::optionsFileIni, ArgKey::optionsFile)
};
static const size_t shortPairSize = sizeof(shortPairArr) / sizeof(const pair<const char *, const char *>);

/** create run controller, initialize run options from command line and ini-file. */
RunController::RunController(int argc, char ** argv) :
    runId(0),
    subSampleCount(0),
    subFirstNumber(0),
    subPerProcess(1),
    processCount(1),
    maxThreadCount(1),
    modelId(0)
{
    // get command line options
    argStore.parseCommandLine(argc, argv, false, true, runOptKeySize, runOptKeyArr, shortPairSize, shortPairArr);

    // load options from ini-file and append parameters section
    argStore.loadIniFile(
        argStore.strOption(ArgKey::optionsFile).c_str(), runOptKeySize, runOptKeyArr, RunOptionsKey::parameterPrefix
        );

    // dependency in log options: if LogToFile is true then file name must be non-empty else must be empty
    argStore.adjustLogSettings(argc, argv);
}

/** model run controler: create new run and input parameters in database and support data exchange. 
*
* - (a) determine run id, number of subsamples, processes, threads
* - (b) create new "run" in database (if required)
* - (c) determine run options and save it in database (if required)
* - (d) find id of source working set for input parameters
* - (e) prepare model input parameters
* - (f) load metadata tables from db and broadcast it to child processes
* - (g) receive accumulators data from to child processes and write into database
*
* (a) determine run id, number of subsamples, processes, threads
* --------------------------------------------------------------
*
* it can be following cases:
*
* Number of subsamples by default = 1 and it can be specified 
* using command-line argument, ini-file or profile table in database
*
* Number of modelling threads by default = number of subsamples per modelling process.
* It can be less if specified by command-line argument (or ini-file or profile table in database)
* and in that case subsamples would be run sequentially. 
*
* Typically new run id and run input parameters created unless run id specified (see "custom run" below)
*
* 1. if MPI not used then
*
*   number of modelling processes = 1 \n
*   number of threads = number of model subsamples, if not limited by model run argument
*
* 2. if MPI is used then
*
*   number of processes = MPI world size \n
*   number of subsamples per process = MPI world size / number of processes \n
*   if MPI world size % number of processes != 0 then remainder of subsamples calculated by root process
*   number of threads = number of subsamples per process, if not limited by model run argument
*
* 3. "custom run" case scenario:
*
*   run id > 0 expliictly specified as run option (i.e. on command line) \n
*   run_id key exist in run_lst table \n
*   number of subsamples = sub_count column value of run_lst table \n
*   process subsample number = sub_started column value of run_lst table
*   number of threads can not be specified and must be 1 by default
*   modelling process will calculate only one subsample and exit.
*       
* (b) create new "run" in database (if required)
* ----------------------------------------------
* "root" process (process with subsample number =0) creates "new run" in database:
* 
*   - insert new row with run_id key into run_lst (except of "custom run" case)  
*   - insert run options into run_option table under run_id  
*   - check destination run id:
*       - it must be newly created run
*       - or empty existing run: in run_lst table value of sub_completed =0
*
* (c) determine run options and save it in database (if required)
* ---------------------------------------------------------------
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
* (d) find id of source working set for input parameters
* ------------------------------------------------------
* use following to find input parameters set id: \n
*   - if task id or task name specified then find set id in task_run \n
*   - if set id specified as run option then use such set id \n
*   - if set name specified as run option then find set id by name \n
*   - else use min(set id) as default set of model parameters
*
* (e) prepare model input parameters
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
* 
* (f) load metadata tables from db and broadcast it to child processes
* --------------------------------------------------------------------
* read all model metadata from database and (if required) broadcast it to all subsample processes.
*/
MetaRunHolder * RunController::init(bool i_isMpiUsed, IDbExec * i_dbExec, IMsgExec * i_msgExec)
{
    // get main run control values: run id, number of subsamples, threads, processes
    runId = argStore.intOption(RunOptionsKey::runId, 0);                    // "custom run": run id must exist in database
    processCount = i_isMpiUsed ? i_msgExec->worldSize() : 1;                // number of processes: MPI world size
    subSampleCount = argStore.intOption(RunOptionsKey::subSampleCount, 1);  // number of subsamples from command line or ini-file
    maxThreadCount = argStore.intOption(RunOptionsKey::threadCount, 1);     // max number of modeling threads

    // basic validation
    if (subSampleCount <= 0 || maxThreadCount <= 0 || processCount <= 0 || subSampleCount < processCount)
        throw ModelException("Invalid number of subsamples: %d or threads: %d or processes: %d", subSampleCount, maxThreadCount, processCount);
    if (runId < 0) throw ModelException("Invalid run id: %d", runId);

    // calculate number of subsamples and first subsample number
    subPerProcess = subSampleCount / processCount;                          // number of subsamples for each modeling process
    subFirstNumber = i_isMpiUsed ? i_msgExec->rank() * subPerProcess : 0;   // first subsample number for current modeling process

    if (subFirstNumber < 0 || subPerProcess <= 0 || subFirstNumber + subPerProcess > subSampleCount)
        throw ModelException("Invalid subsample number: %d or number of subsamples per process: %d", subFirstNumber, subPerProcess);

    // if run id specified then only one subsample calculated and thread count must be 1 (custom run)
    if (runId > 0) {
        subPerProcess = 1;
        if (processCount != 1 || maxThreadCount != 1)
            throw ModelException("Invalid run id %d or number of threads: %d or processes: %d", runId, maxThreadCount, processCount);
    }

    // create new metadata rows storage
    unique_ptr<MetaRunHolder> metaStore(new MetaRunHolder);

    if (!i_isMpiUsed || i_msgExec->isRoot()) {

        // it must be direct database connection: non-MPI or at root MPI process
        if (i_dbExec == nullptr) throw ModelException("invalid (NULL) database connection");

        // load metadata table rows, except of run_option, which is may not created yet
        readMetaTables(i_dbExec, metaStore.get());

        // create new run or next subsample for existing run
        createRunSubsample(i_isMpiUsed, i_dbExec, i_msgExec, metaStore.get());

        // load run_option table rows
        metaStore->runOption.reset(IRunOptionTable::create(i_dbExec, runId));
    }

    // broadcast metadata: run id, subsample count and meta tables from root to all other processes
    if (i_isMpiUsed) {
        broadcastMetaData(i_msgExec, metaStore.get());

        // create list of accumulators to be received from child modelling processes 
        if (i_msgExec->isRoot()) initAccReceiveList(metaStore.get());
    }

    return metaStore.release();
}

// create new run or next subsample for existing run
// create input parameters and run options for all input working sets
void RunController::createRunSubsample(
    bool i_isMpiUsed, IDbExec * i_dbExec, IMsgExec * i_msgExec, MetaRunHolder * io_metaStore
    )
{
    // get model info
    const ModelDicRow * mdRow = io_metaStore->modelDic->byNameTimeStamp(OM_MODEL_NAME, OM_MODEL_TIMESTAMP);
    if (mdRow == nullptr) throw DbException("model %s not found in the database", OM_MODEL_NAME);

    modelId = mdRow->modelId;

    // update in transaction scope
    unique_lock<recursive_mutex> lck = i_dbExec->beginTransactionThreaded();

    // create new run:
    // it must be done for root MPI process or if MPI not used and run id not specified
    bool isNewRunCreated = false;

    if ((i_isMpiUsed && i_msgExec->isRoot()) || (!i_isMpiUsed && runId <= 0)) {

        // get next run id
        i_dbExec->update("UPDATE id_lst SET id_value = id_value + 1 WHERE id_key = 'run_id_set_id'");

        runId = i_dbExec->selectToInt("SELECT id_value FROM id_lst WHERE id_key = 'run_id_set_id'", 0);
        if (runId <= 0)
            throw DbException("invalid run id: %d", runId);

        string dtStr = toDateTimeString(theLog->timeStampSuffix()); // get log date-time as string

        // create new run
        i_dbExec->update(
            "INSERT INTO run_lst (run_id, model_id, run_name, sub_count, sub_started, sub_completed, create_dt, status, update_dt)" \
            " VALUES (" +
            to_string(runId) + ", " +
            to_string(modelId) + ", " +
            toQuoted(string(OM_MODEL_NAME) + " " + dtStr) + ", " +
            to_string(subSampleCount) + ", " +
            to_string(subSampleCount) + ", " +
            "0, " +
            toQuoted(dtStr) + ", "
            "'p', " +
            toQuoted(dtStr) + ")"
            );
        isNewRunCreated = true;
    }
    else { // create next subsample: if MPI is not used and number of subsamples >1 then run id must already exist in database

        if (runId <= 0)  throw DbException("invalid run id: %d", runId);

        // actual subsample number: get next subsample number from database
        string sRunId = to_string(runId);
        i_dbExec->update(
            "UPDATE run_lst SET status = 'p', sub_started = sub_started + 1 WHERE run_id = " + sRunId
            );

        subFirstNumber = i_dbExec->selectToInt("SELECT sub_started - 1 FROM run_lst WHERE run_id = " + sRunId, -1);
        if (subFirstNumber < 0)
            throw DbException("invalid run id: %d", runId);     // run id not found

        int nSubCount = i_dbExec->selectToInt("SELECT sub_count FROM run_lst WHERE run_id = " + sRunId, 0);
        if (subSampleCount > 1 && nSubCount != subSampleCount)
            throw DbException("invalid number of sub-samples: %d, it must be %d (run id: %d)", subSampleCount, nSubCount, runId);

        subSampleCount = nSubCount;     // actual number of subsamples

        if (subFirstNumber >= subSampleCount)
            throw DbException("invalid sub-sample number: %d, it must be less than %d (run id: %d)", subFirstNumber, subSampleCount, runId);

        // validate destination run: it must be new run or empty run
        if (subFirstNumber == 0) {
            int nCompleted = i_dbExec->selectToInt("SELECT sub_completed FROM run_lst WHERE run_id = " + sRunId, 0);
            if (nCompleted != 0)
                throw DbException("destination run must be empty, but it already have %d completed subsamples (run id: %d)", nCompleted, runId);
        }
    }

    // first subsample number =0 (ie: MPI root process)
    // save run options in run_option table
    // copy input parameters from working set and "base" run into new run id
    if (subFirstNumber == 0) {

        // merge run options values from command line, ini-file and profile_option table
        // save run options in run_option table
        createRunOptions(i_dbExec, mdRow, io_metaStore);

        // copy input parameters from "base" run and working set into new run id
        createRunParameters(isNewRunCreated, i_dbExec, mdRow, io_metaStore);
    }

    // completed: commit the results
    i_dbExec->commit();
}

// create run options and save it in run_option table:
// merge command line and ini-file arguments with profile_option table values
// use default values for basic run options, i.e. SparseOutput = false
// raise exception if any of "Parameter." run option name 
// is not in the list of model input parameters or is not scalar
// find id of source working set for input parameters:
//   if set id specified as run option then use such set id
//   if set name specified as run option then find set id by name
//   else use min(set id) as default set of model parameters
void RunController::createRunOptions(
    IDbExec * i_dbExec, const ModelDicRow * i_mdRow, const MetaRunHolder * i_metaStore
    )
{
    // hard-coded default run options
    RunOptions emptyOpts;
    NoCaseMap defaultOpt;

    defaultOpt[RunOptionsKey::useSparse] = emptyOpts.useSparse ? "true" : "false";
    defaultOpt[RunOptionsKey::sparseNull] = toString(emptyOpts.nullValue);

    // load default run options from profile options, default profile name = model name
    string profileName = argStore.strOption(RunOptionsKey::profile, OM_MODEL_NAME);
    unique_ptr<IProfileOptionTable> profileTbl(
        IProfileOptionTable::create(i_dbExec, profileName)
        );

    // update run options: merge command line and ini-file with profile and hard-coded default values
    for (size_t nOpt = 0; nOpt < runOptKeySize; nOpt++) {

        if (argStore.isOptionExist(runOptKeyArr[nOpt])) continue;   // option specified at command line or ini-file

        // find option in profile_option table
        const ProfileOptionRow * optRow = profileTbl->byKey(profileName, runOptKeyArr[nOpt]);
        if (optRow != nullptr) {
            argStore.args[runOptKeyArr[nOpt]] = optRow->value;      // add option from database
        }
        else {  // no database value for that option key, use hard-coded default
            NoCaseMap::const_iterator defIt = defaultOpt.find(runOptKeyArr[nOpt]);
            if (defIt != defaultOpt.cend()) argStore.args[runOptKeyArr[nOpt]] = defIt->second;
        }
    }

    // update "Parameter." options: merge command line and ini-file with profile table
    vector<ParamDicRow> paramVec = i_metaStore->paramDic->rows();

    for (vector<ParamDicRow>::const_iterator paramIt = paramVec.cbegin(); paramIt != paramVec.cend(); ++paramIt) {

        string argName = string(RunOptionsKey::parameterPrefix) + "." + paramIt->paramName;

        if (argStore.isOptionExist(argName.c_str())) continue;  // parameter specified at command line or ini-file

        // find option in profile_option table
        const ProfileOptionRow * optRow = profileTbl->byKey(profileName, argName.c_str());
        if (optRow != nullptr) {
            argStore.args[argName] = optRow->value;             // add option from database
        }
    }

    // find set id of parameters workset, default is first set id for that model
    int setId = argStore.intOption(RunOptionsKey::setId, 0);
    string setName = argStore.strOption(RunOptionsKey::setName);

    if (setId > 0) {
        int cnt = i_dbExec->selectToInt(
            "SELECT COUNT(*) FROM workset_lst WHERE set_id = " + to_string(setId), 0
            );
        if (cnt <= 0) throw DbException("working set id not found in database: %d", setId);
    }
    else {  // find set id by name, if setName option specified 

        if (!setName.empty()) {
            setId = i_dbExec->selectToInt(
                "SELECT MIN(set_id) FROM workset_lst WHERE model_id = " + to_string(i_mdRow->modelId) +
                " AND set_name = " + toQuoted(setName), 
                0);
            if (setId <= 0) 
                throw DbException("model %s, id: %d does not contain working set with name: %s", i_mdRow->name.c_str(), i_mdRow->modelId, setName.c_str());
        }
    }

    if (setId <= 0) {   // use default working set for the model
        setId = i_dbExec->selectToInt(
            "SELECT MIN(set_id) FROM workset_lst WHERE model_id = " + to_string(i_mdRow->modelId), 0
            );
        if (setId <= 0)
            throw DbException("model %s, id: %d must have at least one working set", i_mdRow->name.c_str(), i_mdRow->modelId);
    }

    argStore.args[RunOptionsKey::setId] = to_string(setId);     // add set id to run options

    // get set name and store it as run option
    if (setName.empty()) {
        setName = i_dbExec->selectToStr(
            "SELECT set_name FROM workset_lst WHERE set_id = " + to_string(setId)
            );
    }
    argStore.args[RunOptionsKey::setName] = setName;            // add set name to run options

    // validate "Parameter." options: it must be name of scalar input parameter
    string parPrefix = string(RunOptionsKey::parameterPrefix) + ".";
    size_t nPrefix = parPrefix.length();

    for (NoCaseMap::const_iterator propIt = argStore.args.cbegin(); propIt != argStore.args.cend(); propIt++) {

        if (!equalNoCase(propIt->first.c_str(), parPrefix.c_str(), nPrefix)) continue;  // it is not a "Parameter."

        if (propIt->first.length() <= nPrefix) throw ModelException("invalid (empty) parameter name argument specified");

        // find parameter by name: it must be a model parameter
        string sName = propIt->first.substr(nPrefix);

        const ParamDicRow * paramRow = i_metaStore->paramDic->byModelIdName(i_mdRow->modelId, sName);
        if (paramRow == nullptr) 
            throw DbException("parameter %s is not an input parameter of model %s, id: %d", sName.c_str(), i_mdRow->name.c_str(), i_mdRow->modelId);

        // check the rank: it must scalar
        if (paramRow->rank != 0) throw DbException("parameter %s is not a scalar", sName.c_str());

        // parameter value can not be empty
        if (propIt->second.empty()) throw ModelException("invalid (empty) value specified for parameter %s", sName.c_str());
    }

    // save options in database
    for (NoCaseMap::const_iterator propIt = argStore.args.cbegin(); propIt != argStore.args.cend(); propIt++) {

        // skip run id and connection string: it is already in database
        if (propIt->first == RunOptionsKey::runId || propIt->first == RunOptionsKey::dbConnStr) continue;

        // remove subsample count from run_option, it is stored in run_lst
        if (propIt->first == RunOptionsKey::subSampleCount) continue;

        i_dbExec->update(
            "INSERT INTO run_option (run_id, option_key, option_value) VALUES (" +
            to_string(runId) + ", " + toQuoted(propIt->first) + ", " + toQuoted(propIt->second) + ")"
            );
    }
}

// copy input parameters from working set and "base" run into new run id
// search for input parameter value in following order:
//   use value of parameter specified as command line or ini-file argument
//   use value of parameter from profile_option table or any other run options source
//   use value of parameter from working set of model parameters
//   if working set based on model run then search by base run id to get parameter value
//   else raise an exception
void RunController::createRunParameters(
    bool i_isNewRunCreated, IDbExec * i_dbExec, const ModelDicRow * i_mdRow, const MetaRunHolder * i_metaStore
    )
{
    // find input parameters workset
    int setId = argStore.intOption(RunOptionsKey::setId, 0);
    if (setId <= 0) 
        throw DbException("invalid set id or model working sets not found in database");

    // increase read only flag to "lock" workset until parameters copy not done
    i_dbExec->update(
        "UPDATE workset_lst SET is_readonly = is_readonly + 1 WHERE set_id = " + to_string(setId)
        );

    // workset must exist
    vector<WorksetLstRow> wsVec = IWorksetLstTable::byKey(i_dbExec, setId);
    if (wsVec.empty()) 
        throw DbException("workset must exist (set id: %d)", setId);

    // validate workset: it must be read-only and must be from the same model
    if (!wsVec[0].isReadonly) throw DbException("workset must be read-only (set id: %d)", setId);
    if (wsVec[0].modelId != modelId)
        throw DbException("invalid workset model id: %d, expected: %d (set id: %d)", wsVec[0].modelId, modelId, setId);

    // get list of model parameters and list of parameters included into workset
    vector<ParamDicRow> paramVec = i_metaStore->paramDic->rows();
    vector<WorksetParamRow> wsParamVec = IWorksetParamTable::select(i_dbExec, setId);

    // if base run does not exist then workset must include all model parameters
    int baseRunId = wsVec[0].runId;
    bool isBaseRunExist = baseRunId > 0;
    // if (!isBaseRunExist && paramVec.size() != wsParamVec.size()) throw DbException("workset must include all model parameters (set id: %d)", setId);

    // copy parameters from run options, source set or base run into destination run
    string sRunId = to_string(runId);

    for (vector<ParamDicRow>::const_iterator paramIt = paramVec.cbegin(); paramIt != paramVec.cend(); ++paramIt) {

        string paramTblName = i_mdRow->modelPrefix + i_mdRow->paramPrefix + paramIt->dbNameSuffix;
        string setTblName = i_mdRow->modelPrefix + i_mdRow->setPrefix + paramIt->dbNameSuffix;
        string argName = string(RunOptionsKey::parameterPrefix) + "." + paramIt->paramName;

        // skip if parameter already exists for destination run
        if (!i_isNewRunCreated) {
            if (0 < i_dbExec->selectToLong("SELECT COUNT(*) FROM " + paramTblName + "  WHERE run_id = " + sRunId, 0)) continue;
        }

        // calculate parameter source: command line (or ini-file), workset, based run, run options
        bool isFromSet = WorksetParamRow::byKey(setId, paramIt->paramId, wsParamVec) != wsParamVec.cend();
        bool isArgOption = argStore.isOptionExist(argName.c_str());

        // get dimensions name
        int nRank = paramIt->rank;

        vector<ParamDimsRow> paramDimVec = i_metaStore->paramDims->byModelIdParamId(modelId, paramIt->paramId);
        if (nRank < 0 || nRank != (int)paramDimVec.size()) 
            throw DbException("invalid parameter rank or dimensions not found for parameter: %s", paramIt->paramName.c_str());

        string sDimLst = "";
        for (int nDim = 0; nDim < nRank; nDim++) {
            sDimLst += paramDimVec[nDim].name + ", ";
        }

        // execute insert to copy parameter from run parameters, workset or base run
        bool isInserted = false;
        if (isArgOption) {
            
            // find parameter type 
            const TypeDicRow * typeRow = i_metaStore->typeDic->byKey(modelId, paramIt->typeId);
            if (typeRow == nullptr)
                throw DbException("invalid (not found) type of parameter: %s", paramIt->paramName.c_str());

            // get parameter value as sql string 
            string sVal = argStore.strOption(argName.c_str());
            if (equalNoCase(typeRow->name.c_str(), "file")) sVal = toQuoted(sVal);  // "file" type is VARCHAR

            // insert the value
            i_dbExec->update(
                "INSERT INTO " + paramTblName + " (run_id, value) VALUES (" + sRunId + ", " + sVal + ")"
                );
            isInserted = true;
        }
        if (!isInserted && isFromSet) {
            i_dbExec->update(
                "INSERT INTO " + paramTblName + " (run_id, " + sDimLst + " value)" + 
                " SELECT " + sRunId + ", " + sDimLst + " value" +
                " FROM " + setTblName + " WHERE set_id = " + to_string(setId)
                );
            isInserted = true;
        }
        if (!isInserted && isBaseRunExist) {
            i_dbExec->update(
                "INSERT INTO " + paramTblName + " (run_id, " + sDimLst + " value)" + 
                " SELECT " + sRunId + ", " + sDimLst + " value" +
                " FROM " + paramTblName + " WHERE run_id = " + to_string(baseRunId)
                );
            isInserted = true;
        }
        if (!isInserted) {
            if (nRank <= 0)
                throw DbException("parameter %d not found: %s, it must specified as model run option or be included in workset (set id: %d), ", paramIt->paramId, paramIt->paramName.c_str(), setId);
            else
                throw DbException("parameter %d not found: %s, workset (set id: %d) must include this parameter", paramIt->paramId, paramIt->paramName.c_str(), setId);
        }
    }

    // unlock workset
    i_dbExec->update("UPDATE workset_lst SET is_readonly = 1 WHERE set_id = " + to_string(setId));
}

// read metadata tables from db, except of run_option table
void RunController::readMetaTables(IDbExec * i_dbExec, MetaRunHolder * io_metaStore)
{
    // find model and set model id
    io_metaStore->modelDic.reset(IModelDicTable::create(i_dbExec, OM_MODEL_NAME, OM_MODEL_TIMESTAMP));

    const ModelDicRow * mdRow = io_metaStore->modelDic->byNameTimeStamp(OM_MODEL_NAME, OM_MODEL_TIMESTAMP);
    if (mdRow == nullptr) throw DbException("model %s not found in the database", OM_MODEL_NAME);

    modelId = mdRow->modelId;

    // read metadata tables
    io_metaStore->typeDic.reset(ITypeDicTable::create(i_dbExec, modelId));
    io_metaStore->typeEnumLst.reset(ITypeEnumLstTable::create(i_dbExec, modelId));
    io_metaStore->paramDic.reset(IParamDicTable::create(i_dbExec, modelId));
    io_metaStore->paramDims.reset(IParamDimsTable::create(i_dbExec, modelId));
    io_metaStore->tableDic.reset(ITableDicTable::create(i_dbExec, modelId));
    io_metaStore->tableDims.reset(ITableDimsTable::create(i_dbExec, modelId));
    io_metaStore->tableAcc.reset(ITableAccTable::create(i_dbExec, modelId));
    io_metaStore->tableExpr.reset(ITableExprTable::create(i_dbExec, modelId));
}

// broadcast metadata: run id, subsample count, subsample number and meta tables from root to all modelling processes
void RunController::broadcastMetaData(IMsgExec * i_msgExec, MetaRunHolder * io_metaStore)
{
    if (i_msgExec == nullptr) throw ModelException("invalid (NULL) message passing interface");

    // broadcast model id, run id, subsample count and thread from root to all model processes
    i_msgExec->bcast(i_msgExec->rootRank, typeid(int), 1, &modelId);
    i_msgExec->bcast(i_msgExec->rootRank, typeid(int), 1, &runId);
    i_msgExec->bcast(i_msgExec->rootRank, typeid(int), 1, &subSampleCount);
    i_msgExec->bcast(i_msgExec->rootRank, typeid(int), 1, &maxThreadCount);

    // broadcast metadata tables
    broadcastMetaTable<IModelDicTable>(io_metaStore->modelDic, i_msgExec, MsgTag::modelDic);
    broadcastMetaTable<ITypeDicTable>(io_metaStore->typeDic, i_msgExec, MsgTag::typeDic);
    broadcastMetaTable<ITypeEnumLstTable>(io_metaStore->typeEnumLst, i_msgExec, MsgTag::typeEnumLst);
    broadcastMetaTable<IParamDicTable>(io_metaStore->paramDic, i_msgExec, MsgTag::parameterDic);
    broadcastMetaTable<IParamDimsTable>(io_metaStore->paramDims, i_msgExec, MsgTag::parameterDims);
    broadcastMetaTable<ITableDicTable>(io_metaStore->tableDic, i_msgExec, MsgTag::tableDic);
    broadcastMetaTable<ITableDimsTable>(io_metaStore->tableDims, i_msgExec, MsgTag::tableDims);
    broadcastMetaTable<ITableAccTable>(io_metaStore->tableAcc, i_msgExec, MsgTag::tableAcc);
    broadcastMetaTable<ITableExprTable>(io_metaStore->tableExpr, i_msgExec, MsgTag::tableExpr);
    broadcastMetaTable<IRunOptionTable>(io_metaStore->runOption, i_msgExec, MsgTag::runOption);
}

// broadcast meta table db rows
template <typename MetaTbl>
void RunController::broadcastMetaTable(unique_ptr<MetaTbl> & i_tableUptr, IMsgExec * i_msgExec, MsgTag i_msgTag)
{
    unique_ptr<IPackedAdapter> packAdp(IPackedAdapter::create(i_msgTag));

    if(i_msgExec->isRoot()) {
        IRowBaseVec & rv = i_tableUptr->rowsRef();
        i_msgExec->bcastPacked(i_msgExec->rootRank, rv, *packAdp);
    }
    else {
        IRowBaseVec rv;
        i_msgExec->bcastPacked(i_msgExec->rootRank, rv, *packAdp);
        i_tableUptr.reset(MetaTbl::create(rv));
    }
}

// create list of accumulators to be received from child modelling processes:
// root process calculating first subPerProcess subsamples 
// and remainder of subSampleCount / processCount
void RunController::initAccReceiveList(const MetaRunHolder * i_metaStore)
{
    if (subSampleCount <= subPerProcess) return;    // exit if all subsamples was produced at root model process

    const vector<TableAccRow> accVec = i_metaStore->tableAcc->byModelId(modelId);

    int tblId = -1;
    long long valSize = 0;
    for (int nAcc = 0; nAcc < (int)accVec.size(); nAcc++) {

        // get accumulator data size
        if (tblId != accVec[nAcc].tableId) {
            tblId = accVec[nAcc].tableId;
            valSize = IOutputTableWriter::sizeOf(modelId, i_metaStore, tblId);
        }

        for (int nSub = subPerProcess; nSub < processCount * subPerProcess; nSub++) {
            accRecvVec.push_back(AccReceiveItem(nSub, tblId, accVec[nAcc].accId, nAcc, valSize));
        }
    }
}

/** receive accumulators of output tables subsamples and write into database.
*
* @return  true if not all accumulators for all subsamples received yet (if data not ready)
*/
bool RunController::receiveSubSamples(bool i_isMpiUsed, IDbExec * i_dbExec, IMsgExec * i_msgExec, const MetaRunHolder * i_metaStore)
{
    if (!i_isMpiUsed || !i_msgExec->isRoot()) return false; // exit if this is not a root process
    if (accRecvVec.empty()) return false;                   // exit if nothing to receive

    // get sparse settings
    bool isSparse = i_metaStore->runOption->boolValue(RunOptionsKey::useSparse);
    double nullValue = i_metaStore->runOption->doubleValue(RunOptionsKey::sparseNull, DBL_EPSILON);

    // try to receive and save accumulators
    bool isReceived = false;
    for (AccReceiveItem & accRecv : accRecvVec) {

        if (!accRecv.isReceived) {      // if accumulator not yet received

            // allocate buffer to receive the data
            unique_ptr<double> valueUptr(new double[(int)accRecv.valueSize]);
            double * valueArr = valueUptr.get();

            // try to received
            accRecv.isReceived = i_msgExec->tryReceive(
                (accRecv.subNumber / subPerProcess),
                (MsgTag)(((int)MsgTag::outSubsampleBase + accRecv.accIndex) * subSampleCount + accRecv.subNumber),
                typeid(double),
                accRecv.valueSize,
                valueArr
                );
            if (!accRecv.isReceived) continue;

            // accumulator received: write it into database
            const TableDicRow * tblRow = i_metaStore->tableDic->byKey(modelId, accRecv.tableId);
            if (tblRow == nullptr) throw new DbException("output table not found in table dictionary, id: %d", accRecv.tableId);

            unique_ptr<IOutputTableWriter> writer(IOutputTableWriter::create(
                modelId,
                runId,
                tblRow->tableName.c_str(),
                i_dbExec,
                i_metaStore,
                subSampleCount,
                isSparse,
                nullValue
                ));
            writer->writeAccumulator(i_dbExec, accRecv.subNumber, accRecv.accId, accRecv.valueSize, valueArr);

            isReceived = true;
        }
    }

    // check if anything left to receive
    bool isAnyToRecv = std::any_of(
        accRecvVec.cbegin(),
        accRecvVec.cend(),
        [](AccReceiveItem i_recv) -> bool { return !i_recv.isReceived; }
    );

    // no data received: if any accumulators outstanding then sleep before try again
    if (!isReceived && isAnyToRecv) this_thread::sleep_for(chrono::milliseconds(OM_RECV_SLEEP_TIME));

    return isAnyToRecv;
}
