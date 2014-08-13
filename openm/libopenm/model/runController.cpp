/**
* @file
* OpenM++ modeling library: run controller class to create new model run
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "model.h"
#include "runController.h"

using namespace std;
using namespace openm;

namespace openm
{
    /** short name for options file name: -s fileName.ini */
    const char * RunShortKey::optionsFile = "s";

    /** compatibility short name for options file name: -sc fileName.ini */
    const char * RunShortKey::optionsFileSc = "sc";

    /** parameters started with "Parameter." treated as value of model scalar input parameters */
    const char * RunOptionsKey::parameterPrefix = "Parameter";

    /** number of sub-samples */
    const char * RunOptionsKey::subSampleCount = "General.Subsamples";

    /** database connection string */
    const char * RunOptionsKey::dbConnStr = "OpenM.Database";

    /** model run id */
    const char * RunOptionsKey::runId = "OpenM.RunId";

    /** working set id to get input parameters */
    const char * RunOptionsKey::setId = "OpenM.SetId";

    /** working set name to get input parameters */
    const char * RunOptionsKey::setName = "OpenM.SetName";

    /** profile name to get run options, default is model name */
    const char * RunOptionsKey::profile = "OpenM.OptionsProfile";

    /** use sparse output tables */
    const char * RunOptionsKey::useSparse = "OpenM.SparseOutput";

    /** sparse null value */
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
    RunOptionsKey::dbConnStr,
    RunOptionsKey::runId,
    RunOptionsKey::setId,
    RunOptionsKey::setName,
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
    make_pair(RunShortKey::optionsFileSc, ArgKey::optionsFile)
};
static const size_t shortPairSize = sizeof(shortPairArr) / sizeof(const pair<const char *, const char *>);

/** create run controller, initialize run options from command line and ini-file. */
RunController::RunController(int argc, char ** argv) :
    runId(0),
    subSampleCount(0),
    subSampleNumber(0),
    isSubCountCmd(false)
{
    // get command line options
    argStore.parseCommandLine(argc, argv, false, true, runOptKeySize, runOptKeyArr, shortPairSize, shortPairArr);

    // is sub-samples count specified on command line
    isSubCountCmd = argStore.isOptionExist(RunOptionsKey::subSampleCount);

    // load options from ini-file and append parameters section
    argStore.loadIniFile(
        argStore.strOption(ArgKey::optionsFile).c_str(), runOptKeySize, runOptKeyArr, RunOptionsKey::parameterPrefix
        );

    // dependency in log options: if LogToFile is true then file name must be non-empty else must be empty
    argStore.adjustLogSettings(argc, argv);
}

/** complete model run initialization: create run and input parameters in database. 
*
* - (a) determine run id, number of subsamples and process subsample number
* - (b) create new "run" in database (if required)
* - (c) determine run options and save it in database (if required)
* - (d) prepare model input parameters
* - (e) load metadata tables from db
*
* (a) determine run id, number of subsamples and process subsample number
* -----------------------------------------------------------------------
* it can be following cases:
* 
* 1. if MPI is used then 
* 
*   new run id created by root process \n
*   number of subsamples = MPI world size \n
*   process subsample number = MPI process rank
*   
* 2. if MPI not used then it must be one of:
* 
*   - "custom run" case scenario: \n
*       run id > 0 expliictly specified as run option (i.e. on command line) \n
*       run_id key exist in run_lst table \n
*       number of subsamples = sub_count column value of run_lst table \n
*       process subsample number = sub_started column value of run_lst table
*     
*   - "debug" case scenario: \n
*       run id = 0 and number of subsamples = 1 \n
*       in "debug" case new run id will be created
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
*   - some options have hard default values which used if nothing above is specified
*
*   - find id of source working set for input parameters: \n
*     - if set id specified as run option then use such set id \n
*     - if set name specified as run option then find set id by name \n
*     - else use min(set id) as default set of model parameters
*
*   run options which name starts with "Parameter." are treated as
*   value of scalar input parameter (see below). \n
*   if there is no input parameter with such name then exception raised. \n
*   for example, if command line is: \n
*       model.exe -Parameter.Population 1234 \n
*   and model does not have "Population" parameter then execution aborted.
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
* 
* (e) load metadata tables from db
* ------------------------------------------------------------------------
* read all model metadata from database and (if required) broadcast it to all subsample processes.
*/
MetaRunHolder * RunController::init(bool i_isMpiUsed, IDbExec * i_dbExec, IMsgExec * i_msgExec)
{
    // get run id, number of subsamples and process subsample number
    bool isNewRunCreated = false;
    runId = i_isMpiUsed ? 0 : argStore.intOption(RunOptionsKey::runId, 0);  // if not MPI then run id must exist in database
    int nSubCount = argStore.intOption(RunOptionsKey::subSampleCount, 1);   // number of subsamples specified as run option
    subSampleCount = i_isMpiUsed ? i_msgExec->worldSize() : nSubCount;      // actual number of subsamples: MPI world size or run option or =1 by default
    subSampleNumber = i_isMpiUsed ? i_msgExec->rank() : 0;                  // subsample number: MPI rank or =0 in all other cases

    // validate number of subsamples and check for command line options conflict 
    if (subSampleCount <= 0) throw ModelException("Invalid number of subsamples: %d", subSampleCount);
    if (subSampleNumber < 0) throw ModelException("Invalid subsample number: %d", subSampleNumber);

    if (i_isMpiUsed && isSubCountCmd && nSubCount != subSampleCount) 
        throw ModelException("Invalid number of subsamples: %d, expected to be: %d", nSubCount, subSampleCount);

    // if MPI not used then it must be one of:
    //   expliictly specified run id > 0 and any number of subsamples specified in run_lst table
    //   or "debug" case scenario: run id = 0 and number of subsamples = 1
    //   in "debug" case new run id will be created
    if (!i_isMpiUsed && (runId < 0 || (runId >= 0 && subSampleCount != 1))) 
        throw ModelException("Invalid run id %d or number of subsamples: %d", runId, subSampleCount);

    // create new metadata rows storage
    unique_ptr<MetaRunHolder> metaStore(new MetaRunHolder);

    // direct database connection: non-MPI or at root MPI process
    if (!i_isMpiUsed || i_msgExec->isRoot()) {

        // load metadata table rows, except of run_option, which is may not created yet
        readMetaTables(i_dbExec, metaStore.get());
        
        const ModelDicRow * mdRow = metaStore->modelDic->byNameTimeStamp(OM_MODEL_NAME, OM_MODEL_TIMESTAMP);
        if (mdRow == NULL) throw DbException("model %s not found in the database", OM_MODEL_NAME);

        int modelId = mdRow->modelId;

        // update in transaction scope
        i_dbExec->beginTransaction();

        // create new run:
        // it must be done for root MPI process or if MPI not used and run id not specified
        if ((i_isMpiUsed && i_msgExec->isRoot()) || (!i_isMpiUsed && runId <= 0)) {

            // get next run id
            i_dbExec->update("UPDATE id_lst SET id_value = id_value + 1 WHERE id_key = 'run_id_set_id'");

            runId = i_dbExec->selectToInt("SELECT id_value FROM id_lst WHERE id_key = 'run_id_set_id'", 0);
            if (runId <= 0) 
                throw DbException("invalid run id: %d", runId);

            string dtStr = toDateTimeString(theLog->timeStampSuffix()); // get log date-time as string

            // create new run
            i_dbExec->update(
                "INSERT INTO run_lst (run_id, model_id, run_name, sub_count, sub_started, sub_completed, create_dt) VALUES (" + 
                to_string(runId) + ", " + 
                to_string(modelId) + ", " + 
                toQuoted(string(OM_MODEL_NAME) + " " + dtStr) + ", " + 
                to_string(subSampleCount) + ", " + 
                to_string(subSampleCount) + ", " + 
                "0, " + 
                toQuoted(dtStr) + ")"
                );
            isNewRunCreated = true;
        }
        else { // create next subsample: if MPI is not used and number of subsamples >1 then run id must already exist in database

            if (runId <= 0)  throw DbException("invalid run id: %d", runId);

            // actual subsample number: get next subsample number from database
            i_dbExec->update(
                "UPDATE run_lst SET sub_started = sub_started + 1 WHERE run_id = " + to_string(runId)
                );

            subSampleNumber = i_dbExec->selectToInt("SELECT sub_started FROM run_lst WHERE run_id = " + to_string(runId), 0);
            if (subSampleNumber <= 0) 
                throw DbException("invalid run id: %d", runId);     // run id not found

            subSampleNumber--;    // subsample number is zero-based

            subSampleCount = i_dbExec->selectToInt("SELECT sub_count FROM run_lst WHERE run_id = " + to_string(runId), 0);
            if (subSampleNumber >= subSampleCount) 
                throw DbException("invalid sub-sample number: %d, it must be less than %d (run id: %d)", subSampleNumber, subSampleCount, runId);
        }

        // "root" process: process where subsampe number =0 (ie: MPI root process)
        // validate destination run: it must be new run or empty run
        // save run options in run_option table
        // copy input parameters from working set and "base" run into new run id
        if (subSampleNumber == 0) {

            // validate destination run: it must be new run or empty run
            string sRunId = to_string(runId);
            if (!isNewRunCreated) {
                int nCompleted = i_dbExec->selectToInt("SELECT sub_completed FROM run_lst WHERE run_id = " + sRunId, 0);
                if (nCompleted != 0) 
                    throw DbException("destination run must be empty, but it already have %d completed subsamples (run id: %d)", nCompleted, runId);
            }

            // merge run options values from command line, ini-file and profile_option table
            // save run options in run_option table
            createRunOptions(i_dbExec, mdRow, metaStore.get());

            // copy input parameters from "base" run and working set into new run id
            createRunParameters(isNewRunCreated, i_dbExec, mdRow, metaStore.get());
        }

        // completed: commit the results
        i_dbExec->commit();

        // set run id (if new created) and load run_option table rows
        metaStore->runId = runId;
        metaStore->runOption.reset(IRunOptionTable::create(i_dbExec, runId));
    }

    // broadcast metadata rows from root to all other processes
    if (i_isMpiUsed) {
        broadcastMetaTables(i_msgExec, metaStore.get());
    }
    return metaStore.release();
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
        if (optRow != NULL) {
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
        if (optRow != NULL) {
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
        if (paramRow == NULL) 
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
    int modelId = i_mdRow->modelId;
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
    if (i_dbExec == NULL) throw ModelException("invalid (NULL) database connection");

    io_metaStore->modelDic.reset(IModelDicTable::create(i_dbExec, OM_MODEL_NAME, OM_MODEL_TIMESTAMP));

    const ModelDicRow * mdRow = io_metaStore->modelDic->byNameTimeStamp(OM_MODEL_NAME, OM_MODEL_TIMESTAMP);
    if (mdRow == NULL) throw DbException("model %s not found in the database", OM_MODEL_NAME);

    io_metaStore->typeDic.reset(ITypeDicTable::create(i_dbExec, mdRow->modelId));
    io_metaStore->typeEnumLst.reset(ITypeEnumLstTable::create(i_dbExec, mdRow->modelId));
    io_metaStore->paramDic.reset(IParamDicTable::create(i_dbExec, mdRow->modelId));
    io_metaStore->paramDims.reset(IParamDimsTable::create(i_dbExec, mdRow->modelId));
    io_metaStore->tableDic.reset(ITableDicTable::create(i_dbExec, mdRow->modelId));
    io_metaStore->tableDims.reset(ITableDimsTable::create(i_dbExec, mdRow->modelId));
    io_metaStore->tableAcc.reset(ITableAccTable::create(i_dbExec, mdRow->modelId));
    io_metaStore->tableUnit.reset(ITableUnitTable::create(i_dbExec, mdRow->modelId));

    // set run id: initialized for root process
    io_metaStore->runId = runId;
}

// broadcast metadata tables from root to all modeling processes
void RunController::broadcastMetaTables(IMsgExec * i_msgExec, MetaRunHolder * io_metaStore)
{
    if (i_msgExec == NULL) throw ModelException("invalid (NULL) message passing interface");

    // broadcast run id
    i_msgExec->bcast(i_msgExec->rootRank, typeid(int), 1, &runId);

    // broadcast metadata tables
    broadcastTable<IModelDicTable>(io_metaStore->modelDic, i_msgExec, MsgTag::modelDic);
    broadcastTable<IRunOptionTable>(io_metaStore->runOption, i_msgExec, MsgTag::runOption);
    broadcastTable<ITypeDicTable>(io_metaStore->typeDic, i_msgExec, MsgTag::typeDic);
    broadcastTable<ITypeEnumLstTable>(io_metaStore->typeEnumLst, i_msgExec, MsgTag::typeEnumLst);
    broadcastTable<IParamDicTable>(io_metaStore->paramDic, i_msgExec, MsgTag::parameterDic);
    broadcastTable<IParamDimsTable>(io_metaStore->paramDims, i_msgExec, MsgTag::parameterDims);
    broadcastTable<ITableDicTable>(io_metaStore->tableDic, i_msgExec, MsgTag::tableDic);
    broadcastTable<ITableDimsTable>(io_metaStore->tableDims, i_msgExec, MsgTag::tableDims);
    broadcastTable<ITableAccTable>(io_metaStore->tableAcc, i_msgExec, MsgTag::tableAcc);
    broadcastTable<ITableUnitTable>(io_metaStore->tableUnit, i_msgExec, MsgTag::tableUnit);

    // set run id: it is broadcasted by MPI
    io_metaStore->runId = runId;
}

// broadcast meta table db rows
template <typename MetaTbl>
void RunController::broadcastTable(unique_ptr<MetaTbl> & i_tableUptr, IMsgExec * i_msgExec, MsgTag i_msgTag)
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

