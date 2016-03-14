/**
* @file
* OpenM++ modeling library: model metadata loader to read and broadcast metadata and run options.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "model.h"

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

    /** modeling task id */
    const char * RunOptionsKey::taskId = "OpenM.TaskId";

    /** modeling task name */
    const char * RunOptionsKey::taskName = "OpenM.TaskName";
        
    /** modeling task under external supervision */
    const char * RunOptionsKey::taskWait = "OpenM.TaskWait";

    /** modeling task run id */
    const char * RunOptionsKey::taskRunId = "OpenM.TaskRunId";

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

/** last cleanup */
MetaLoader::~MetaLoader(void) throw() { }

/** initialize run options from command line and ini-file. */
const ArgReader MetaLoader::getRunOptions(int argc, char ** argv)
{
    // get command line options
    ArgReader ar;
    ar.parseCommandLine(argc, argv, false, true, runOptKeySize, runOptKeyArr, shortPairSize, shortPairArr);

    // load options from ini-file and append parameters section
    ar.loadIniFile(
        ar.strOption(ArgKey::optionsFile).c_str(), runOptKeySize, runOptKeyArr, RunOptionsKey::parameterPrefix
        );

    // dependency in log options: if LogToFile is true then file name must be non-empty else must be empty
    ar.adjustLogSettings(argc, argv);

    return ar;
}

/** return basic model run options */
const RunOptions MetaLoader::modelRunOptions(int i_subCount, int i_subNumber) const 
{ 
    RunOptions opts(baseRunOpts);
    opts.subSampleCount = i_subCount;
    opts.subSampleNumber = i_subNumber;
    return opts; 
}

// read metadata tables from db, except of run_option table
const ModelDicRow * MetaLoader::readMetaTables(IDbExec * i_dbExec, MetaRunHolder * io_metaStore)
{
    // find model and set model id
    io_metaStore->modelDic.reset(IModelDicTable::create(i_dbExec, OM_MODEL_NAME, OM_MODEL_TIMESTAMP));

    const ModelDicRow * mdRow = io_metaStore->modelDic->byNameTimeStamp(OM_MODEL_NAME, OM_MODEL_TIMESTAMP);
    if (mdRow == nullptr) throw DbException("model %s not found in the database", OM_MODEL_NAME);

    // read metadata tables
    io_metaStore->typeDic.reset(ITypeDicTable::create(i_dbExec, mdRow->modelId));
    io_metaStore->typeEnumLst.reset(ITypeEnumLstTable::create(i_dbExec, mdRow->modelId));
    io_metaStore->paramDic.reset(IParamDicTable::create(i_dbExec, mdRow->modelId));
    io_metaStore->paramDims.reset(IParamDimsTable::create(i_dbExec, mdRow->modelId));
    io_metaStore->tableDic.reset(ITableDicTable::create(i_dbExec, mdRow->modelId));
    io_metaStore->tableDims.reset(ITableDimsTable::create(i_dbExec, mdRow->modelId));
    io_metaStore->tableAcc.reset(ITableAccTable::create(i_dbExec, mdRow->modelId));
    io_metaStore->tableExpr.reset(ITableExprTable::create(i_dbExec, mdRow->modelId));

    return mdRow;
}

// broadcast metadata tables from root to all modeling processes
void MetaLoader::broadcastMetaData(int i_groupOne, IMsgExec * i_msgExec, MetaRunHolder * io_metaStore)
{
    if (i_msgExec == nullptr) throw ModelException("invalid (NULL) message passing interface");

    // broadcast metadata tables
    broadcastMetaTable<IModelDicTable>(i_groupOne, i_msgExec, MsgTag::modelDic, io_metaStore->modelDic);
    broadcastMetaTable<ITypeDicTable>(i_groupOne, i_msgExec, MsgTag::typeDic, io_metaStore->typeDic);
    broadcastMetaTable<ITypeEnumLstTable>(i_groupOne, i_msgExec, MsgTag::typeEnumLst, io_metaStore->typeEnumLst);
    broadcastMetaTable<IParamDicTable>(i_groupOne, i_msgExec, MsgTag::parameterDic, io_metaStore->paramDic);
    broadcastMetaTable<IParamDimsTable>(i_groupOne, i_msgExec, MsgTag::parameterDims, io_metaStore->paramDims);
    broadcastMetaTable<ITableDicTable>(i_groupOne, i_msgExec, MsgTag::tableDic, io_metaStore->tableDic);
    broadcastMetaTable<ITableDimsTable>(i_groupOne, i_msgExec, MsgTag::tableDims, io_metaStore->tableDims);
    broadcastMetaTable<ITableAccTable>(i_groupOne, i_msgExec, MsgTag::tableAcc, io_metaStore->tableAcc);
    broadcastMetaTable<ITableExprTable>(i_groupOne, i_msgExec, MsgTag::tableExpr, io_metaStore->tableExpr);
}

// broadcast int value from root to group of modeling processes
void MetaLoader::broadcastInt(int i_groupOne, IMsgExec * i_msgExec, int * io_value)
{
    if (i_msgExec == nullptr) throw ModelException("invalid (NULL) message passing interface");

    i_msgExec->bcast(i_groupOne, typeid(int), 1, io_value);
}

// broadcast run options from root to group of modeling processes
void MetaLoader::broadcastRunOptions(int i_groupOne, IMsgExec * i_msgExec)
{
    if (i_msgExec == nullptr) throw ModelException("invalid (NULL) message passing interface");

    i_msgExec->bcast(i_groupOne, typeid(int), 1, &baseRunOpts.subSampleCount);
    i_msgExec->bcast(i_groupOne, typeid(int), 1, &baseRunOpts.subSampleNumber);
    i_msgExec->bcast(i_groupOne, typeid(bool), 1, &baseRunOpts.useSparse);
    i_msgExec->bcast(i_groupOne, typeid(double), 1, &baseRunOpts.nullValue);
}

// broadcast meta table db rows
template <typename MetaTbl>
void MetaLoader::broadcastMetaTable(int i_groupOne, IMsgExec * i_msgExec, MsgTag i_msgTag, unique_ptr<MetaTbl> & io_tableUptr)
{
    unique_ptr<IPackedAdapter> packAdp(IPackedAdapter::create(i_msgTag));

    if (i_msgExec->isRoot()) {
        IRowBaseVec & rv = io_tableUptr->rowsRef();
        i_msgExec->bcastPacked(i_groupOne, rv, *packAdp);
    }
    else {
        IRowBaseVec rv;
        i_msgExec->bcastPacked(i_groupOne, rv, *packAdp);
        io_tableUptr.reset(MetaTbl::create(rv));
    }
}

// merge command line and ini-file arguments with profile_option table values
// use default values for basic run options, i.e. SparseOutput = false
// raise exception if any of "Parameter." run option name 
// is not in the list of model input parameters or is not scalar
void MetaLoader::mergeProfile(IDbExec * i_dbExec, const ModelDicRow * i_modelRow)
{
    // initial values are hard-coded default run options
    NoCaseMap defaultOpt;

    defaultOpt[RunOptionsKey::useSparse] = baseRunOpts.useSparse ? "true" : "false";
    defaultOpt[RunOptionsKey::sparseNull] = toString(baseRunOpts.nullValue);
    defaultOpt[RunOptionsKey::threadCount] = "1";

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
    vector<ParamDicRow> paramVec = metaStore->paramDic->rows();

    for (vector<ParamDicRow>::const_iterator paramIt = paramVec.cbegin(); paramIt != paramVec.cend(); ++paramIt) {

        string argName = string(RunOptionsKey::parameterPrefix) + "." + paramIt->paramName;

        if (argStore.isOptionExist(argName.c_str())) continue;  // parameter specified at command line or ini-file

        // find option in profile_option table
        const ProfileOptionRow * optRow = profileTbl->byKey(profileName, argName.c_str());
        if (optRow != nullptr) {
            argStore.args[argName] = optRow->value;             // add option from database
        }
    }

    // validate "Parameter." options: it must be name of scalar input parameter
    string parPrefix = string(RunOptionsKey::parameterPrefix) + ".";
    size_t nPrefix = parPrefix.length();

    for (NoCaseMap::const_iterator propIt = argStore.args.cbegin(); propIt != argStore.args.cend(); propIt++) {

        if (!equalNoCase(propIt->first.c_str(), parPrefix.c_str(), nPrefix)) continue;  // it is not a "Parameter."

        if (propIt->first.length() <= nPrefix) throw ModelException("invalid (empty) parameter name argument specified");

        // find parameter by name: it must be a model parameter
        string sName = propIt->first.substr(nPrefix);

        const ParamDicRow * paramRow = metaStore->paramDic->byModelIdName(i_modelRow->modelId, sName);
        if (paramRow == nullptr)
            throw DbException("parameter %s is not an input parameter of model %s, id: %d", sName.c_str(), i_modelRow->name.c_str(), i_modelRow->modelId);

        // check the rank: it must scalar
        if (paramRow->rank != 0) throw DbException("parameter %s is not a scalar", sName.c_str());

        // parameter value can not be empty
        if (propIt->second.empty()) throw ModelException("invalid (empty) value specified for parameter %s", sName.c_str());
    }

    // merge model run options
    baseRunOpts.useSparse = argStore.boolOption(RunOptionsKey::useSparse);
    baseRunOpts.nullValue = argStore.doubleOption(RunOptionsKey::sparseNull, DBL_EPSILON);
}

// create task run entry in database
int MetaLoader::createTaskRun(int i_taskId, IDbExec * i_dbExec) 
{
    // update in transaction scope
    unique_lock<recursive_mutex> lck = i_dbExec->beginTransactionThreaded();

    // get next task run id
    i_dbExec->update("UPDATE id_lst SET id_value = id_value + 1 WHERE id_key = 'task_run_id'");

    int taskRunId = i_dbExec->selectToInt("SELECT id_value FROM id_lst WHERE id_key = 'task_run_id'", 0);
    if (taskRunId <= 0)
        throw DbException("invalid task run id: %d", taskRunId);

    string dtStr = toDateTimeString(theLog->timeStampSuffix()); // get log date-time as string

    // create new run
    i_dbExec->update(
        "INSERT INTO task_run_lst (task_run_id, task_id, sub_count, create_dt, status, update_dt)" \
        " VALUES (" +
        to_string(taskRunId) + ", " +
        to_string(i_taskId) + ", " +
        to_string(subSampleCount) + ", " +
        toQuoted(dtStr) + ", " +
        toQuoted(RunStatus::init) + ", " +
        toQuoted(dtStr) + ")"
        );

    // completed: commit the changes
    i_dbExec->commit();

    argStore.args[RunOptionsKey::taskRunId] = to_string(taskRunId);
    return taskRunId;
}

// find modeling task, if specified
int MetaLoader::findTask(IDbExec * i_dbExec, const ModelDicRow * i_modelRow)
{
    // find task id or name if specified as run options
    int taskId = argStore.intOption(RunOptionsKey::taskId, 0);
    string taskName = argStore.strOption(RunOptionsKey::taskName);

    if (taskId > 0) {
        int cnt = i_dbExec->selectToInt(
            "SELECT COUNT(*) FROM task_lst WHERE task_id = " + to_string(taskId), 0
            );
        if (cnt <= 0) throw DbException("task id not found in database: %d", taskId);
    }
    else {  // find task id by name, if taskName option specified 

        if (!taskName.empty()) {
            taskId = i_dbExec->selectToInt(
                "SELECT MIN(task_id) FROM task_lst WHERE model_id = " + to_string(i_modelRow->modelId) +
                " AND task_name = " + toQuoted(taskName),
                0);
            if (taskId <= 0)
                throw DbException("model %s, id: %d does not contain task with name: %s", i_modelRow->name.c_str(), i_modelRow->modelId, taskName.c_str());
        }
    }

    // if task found then add it to run options
    if (taskId > 0) {

        if (taskName.empty()) {
            taskName = i_dbExec->selectToStr(
                "SELECT task_name FROM task_lst WHERE task_id = " + to_string(taskId)
                );
        }
        argStore.args[RunOptionsKey::taskId] = to_string(taskId);
        argStore.args[RunOptionsKey::taskName] = taskName;
    }

    return taskId;
}

// find id and name of source working set for input parameters:
//   if set id specified as run option then use such set id
//   if set name specified as run option then find set id by name
//   else use min(set id) as default set of model parameters
int MetaLoader::findWorkset(int i_setId, IDbExec * i_dbExec, const ModelDicRow * i_modelRow) const
{
    // find set id of parameters workset, default is first set id for that model
    int setId = (i_setId > 0) ? i_setId : argStore.intOption(RunOptionsKey::setId, 0);
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
                "SELECT MIN(set_id) FROM workset_lst WHERE model_id = " + to_string(i_modelRow->modelId) +
                " AND set_name = " + toQuoted(setName),
                0);
            if (setId <= 0)
                throw DbException("model %s, id: %d does not contain working set with name: %s", i_modelRow->name.c_str(), i_modelRow->modelId, setName.c_str());
        }
    }

    // if set id not defined then use default working set for the model
    if (setId <= 0) {
        setId = i_dbExec->selectToInt(
            "SELECT MIN(set_id) FROM workset_lst WHERE model_id = " + to_string(i_modelRow->modelId), 0
            );
        if (setId <= 0)
            throw DbException("model %s, id: %d must have at least one working set", i_modelRow->name.c_str(), i_modelRow->modelId);
    }

    return setId;
}

// set modeling groups size, group count and process rank in group
ProcessGroupDef::ProcessGroupDef(int i_subSampleCount, int i_threadCount, int i_worldSize, int i_worldRank) :
    ProcessGroupDef() 
{
    groupSize = i_subSampleCount / i_threadCount;
    if (groupSize <= 0) groupSize = 1;
    if (groupSize > i_worldSize) groupSize = i_worldSize;

    groupCount = (groupSize > 1) ? i_worldSize / groupSize : (i_worldSize - 1) / groupSize;
    if (groupCount <= 0) groupCount = 1;

    // one-based group number, root is in the last group
    groupOne = (i_worldRank > 0) ? 1 + (i_worldRank - 1) / groupSize : groupCount;

    // "active" root: use root process for modeling, else dedicate it for data exchange only
    isRootActive = groupSize > 1 && i_worldSize <= groupSize * groupCount;

    // get process rank among other active modeling processes in the group
    bool isInLastGroup = groupOne >= groupCount;

    activeRank = 
        (i_worldRank > 0 && (!isInLastGroup || !isRootActive)) ?
        ((i_worldRank - 1) % groupSize) : 
        i_worldRank % groupSize;

    // is current process active: 
    // "active" process means it is used for modeling 
    // if process count == group size * group count then all processes used for modeliung, including root
    // else root process dedicated to data exchange and children > (group size * group count) unused (not active)
    // isActive = (i_worldRank == 0 && isRootActive) || (0 < i_worldRank && i_worldRank <= groupSize * groupCount);
}

// set initial run group size, assign process ranks and initial state state
RunGroup::RunGroup(int i_groupOne, int i_subSampleCount, const ProcessGroupDef & i_rootGroupDef) : RunGroup() 
{ 
    groupOne = i_groupOne;
    firstChildRank = 1 + (i_groupOne - 1) * i_rootGroupDef.groupSize;
    childCount = 
        (i_groupOne >= i_rootGroupDef.groupCount && i_rootGroupDef.isRootActive) ? 
        i_rootGroupDef.groupSize - 1 : 
        i_rootGroupDef.groupSize;

    isUseRoot = (i_groupOne == i_rootGroupDef.groupOne) ? i_rootGroupDef.isRootActive : false;
    isSubDone.assign(i_subSampleCount, false); 
}

// set group state for next run
void RunGroup::nextRun(int i_runId, int i_setId, ModelStatus i_status, int i_subSampleCount)
{ 
    runId = i_runId;
    setId = i_setId;
    state.updateStatus(i_status);
    isSubDone.assign(i_subSampleCount, false);
}

// return child world rank where subsample is calculated
int RunGroup::rankBySubsampleNumber(int i_subNumber, int i_subSampleCount, int i_groupSize) const
{
    int nSubPerProc = i_subSampleCount / i_groupSize;   // subsamples per modeling process
    int nProc = i_subNumber / nSubPerProc;              // process index in group
    if (nProc >= i_groupSize) nProc = i_groupSize - 1;  // last process also calculate the rest

    int nRank = firstChildRank + nProc;     // world rank to calculate subsample

    // if root process is "active" (is used for modeling) then first subsamples calculated at root 
    if (isUseRoot) {
        nRank = i_subNumber < nSubPerProc ? 0 : firstChildRank + nProc - 1;
    }
    return nRank;
}
