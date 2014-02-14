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
    /** number of sub-samples */
    const char * RunOptionsKey::subSampleCount = "General.Subsamples";

    /** starting seed for random number generator */
    const char * RunOptionsKey::seed = "General.StartingSeed";

    /** number of cases */
    const char * RunOptionsKey::cases = "General.Cases";

    /** simulation end time */
    const char * RunOptionsKey::endTime = "General.SimulationEnd";

    /** database connection string */
    const char * RunOptionsKey::dbConnStr = "OpenM.Database";

    /** model run id */
    const char * RunOptionsKey::runId = "OpenM.RunId";

    /** working set id to get input parameters */
    const char * RunOptionsKey::setId = "OpenM.SetId";

    /** profile name to get run options, default is model name */
    const char * RunOptionsKey::profile = "OpenM.OptionsProfile";

    /** use sparse output tables */
    const char * RunOptionsKey::useSparse = "OpenM.SparseOutput";

    /** sparse null value */
    const char * RunOptionsKey::sparseNull = "OpenM.SparseNullValue";

    /** short name for options file name: -s fileName.ini */
    const char * RunShortKey::optionsFile = "s";

    /** compatibility short name for options file name: -sc fileName.ini */
    const char * RunShortKey::optionsFileSc = "sc";
}

/** array of model run option keys. */
static const char * runOptKeyArr[] = {
    RunOptionsKey::subSampleCount,
    RunOptionsKey::seed,
    RunOptionsKey::cases,
    RunOptionsKey::endTime,
    RunOptionsKey::dbConnStr,
    RunOptionsKey::runId,
    RunOptionsKey::setId,
    RunOptionsKey::profile,
    RunOptionsKey::useSparse,
    RunOptionsKey::sparseNull,
    ArgKey::optionsFile,
    ArgKey::logToConsole,
    ArgKey::logToFile,
    ArgKey::logFilePath,
    ArgKey::logToStamped,
    ArgKey::logUseTs,
    ArgKey::logUsePid,
    ArgKey::logNoTimeConsole,
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
    argStore.parseCommandLine(argc, argv, true, false, runOptKeySize, runOptKeyArr, shortPairSize, shortPairArr);

    // is sub-samples count specified on command line
    isSubCountCmd = argStore.isOptionExist(RunOptionsKey::subSampleCount);

    // load options from ini-file
    argStore.loadIniFile(argStore.strOption(ArgKey::optionsFile).c_str(), runOptKeySize, runOptKeyArr);

    // dependency in log options: if LogToFile is true then file name must be non-empty else must be empty
    argStore.adjustLogSettings(argc, argv);
}

/** complete model run initialization: create run and input parameters in database. 
*
* - (a) determine run id, number of subsamples and process subsample number
* - (b) create new "run" in database (if required)
* - (c) prepare model input parameters
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
* (c) prepare model input parameters
* ----------------------------------
* it creates a copy of input paramters from source working set under destination run_id
* 
* there are two cases to find source working set of input parameters: \n
* if working set id specified as run option \n
*   - then use workset (set id) and workset base run (base run id) as source \n
*   - else use min(set id) as default set of model parameters
*/
void RunController::init(bool i_isMpiUsed, IDbExec * i_dbExec, IMsgExec * i_msgExec)
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

    // direct database connection: non-MPI or at root MPI process
    if (!i_isMpiUsed || i_msgExec->isRoot()) {

        // find the model
        unique_ptr<IModelDicTable> mdTbl(IModelDicTable::create(i_dbExec, OM_MODEL_NAME, OM_MODEL_TIMESTAMP));
        const ModelDicRow * mdRow = mdTbl->byNameTimeStamp(OM_MODEL_NAME, OM_MODEL_TIMESTAMP);
        if (mdRow == NULL) throw DbException("model %s not found in the database", OM_MODEL_NAME);

        int modelId = mdRow->modelId;

        // update in transaction scope
        i_dbExec->beginTransaction();

        // create new run:
        // it must be done for root MPI process or if MPI not used and run id not specified
        if ((i_isMpiUsed && i_msgExec->isRoot()) || (!i_isMpiUsed && runId <= 0)) {

            // get next run id
            i_dbExec->update("UPDATE id_lst SET id_value = id_value + 1 WHERE id_key = 'run_set'");

            runId = i_dbExec->selectToInt("SELECT id_value FROM id_lst WHERE id_key = 'run_set'", 0);
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

            // copy input parameters from "base" run and working set into new run id
            createRunParameters(isNewRunCreated, i_dbExec, mdRow);

            // save run options in run_option table
            saveRunOptions(i_dbExec, mdRow);
        }

        // completed: commit the results
        i_dbExec->commit();
    }
}

// copy input parameters from working set and "base" run into new run id
// if working set id specified as run option
//   - then use workset (set id) and workset base run (base run id) as source
//   - else use min(set id) as default set of model parameters
void RunController::createRunParameters(bool i_isNewRunCreated, IDbExec * i_dbExec, const ModelDicRow * i_mdRow)
{
    // find input parameters workset
    int modelId = i_mdRow->modelId;
    int setId = argStore.intOption(RunOptionsKey::setId, 0);

    // increase read only flag to "lock" workset until parameters copy not done
    if (setId > 0) {
        i_dbExec->update(
            "UPDATE workset_lst SET is_readonly = is_readonly + 1 WHERE set_id = " + to_string(setId)
            );
    }
    else {  // no set id specified: use min set_id of the model

        // lock workset to prevent editing during copy parameters
        i_dbExec->update(
            "UPDATE workset_lst SET is_readonly = is_readonly + 1" \
            " WHERE set_id =" \
            " (SELECT MIN(set_id) FROM workset_lst WHERE model_id = " + to_string(modelId) + ")"
            );

        // use min set id of the model to get the data
        setId = i_dbExec->selectToInt(
            "SELECT MIN(set_id) FROM workset_lst WHERE model_id = " + to_string(modelId), 0
            );
    }

    // check set id: it must be defined
    if (setId <= 0) throw DbException("invalid set id or model working sets not found in database");

    // workset must exist
    vector<WorksetLstRow> wsVec = IWorksetLstTable::byKey(i_dbExec, setId);
    if (wsVec.empty()) 
        throw DbException("workset must exist (set id: %d)", setId);

    // validate workset: it must be read-only and must be from the same model
    if (!wsVec[0].isReadonly) throw DbException("workset must be read-only (set id: %d)", setId);
    if (wsVec[0].modelId != modelId)
        throw DbException("invalid workset model id: %d, expected: %d (set id: %d)", wsVec[0].modelId, modelId, setId);

    // get list of model parameters and list of parameters included into workset
    vector<ParamDicRow> paramVec = IParamDicTable::create(i_dbExec, modelId)->rows();
    vector<WorksetParamRow> wsParamVec = IWorksetParamTable::select(i_dbExec, setId);

    // if base run does not exist then workset must include all model parameters
    int baseRunId = wsVec[0].runId;
    bool isBaseRunExist = baseRunId > 0;
    if (!isBaseRunExist && paramVec.size() != wsParamVec.size()) throw DbException("workset must include all model parameters (set id: %d)", setId);

    // copy parameters from source set or base run into destination run
    unique_ptr<IParamDimsTable> paramDimsTbl( IParamDimsTable::create(i_dbExec, modelId) );
    string sRunId = to_string(runId);
    string sSetId = to_string(setId);

    // for each parameter make data copy by sql insert
    for (vector<ParamDicRow>::const_iterator paramIt = paramVec.cbegin(); paramIt != paramVec.cend(); ++paramIt) {

        string paramTblName = i_mdRow->modelPrefix + i_mdRow->paramPrefix + paramIt->dbNameSuffix;
        string setTblName = i_mdRow->modelPrefix + i_mdRow->setPrefix + paramIt->dbNameSuffix;

        // skip if parameter already exists for destination run
        if (!i_isNewRunCreated) {
            if (0 < i_dbExec->selectToLong("SELECT COUNT(*) FROM " + paramTblName + "  WHERE run_id = " + sRunId, 0)) continue;
        }

        // get dimensions name
        int nRank = paramIt->rank;

        vector<ParamDimsRow> paramDimVec = paramDimsTbl->byModelIdParamId(modelId, paramIt->paramId);
        if (nRank < 0 || nRank != (int)paramDimVec.size()) 
            throw DbException("invalid parameter rank or dimensions not found for parameter: %s", paramIt->paramName.c_str());

        string sDimLst = "";
        for (int nDim = 0; nDim < nRank; nDim++) {
            sDimLst += paramDimVec[nDim].name + ", ";
        }

        // if base run does not exist then workset must include all model parameters
        bool isFromSet = WorksetParamRow::byKey(setId, paramIt->paramId, wsParamVec) != wsParamVec.cend();

        if (!isFromSet && !isBaseRunExist) 
            throw DbException("workset must include all model parameters (set id: %d), parameter %d not found: %s", setId, paramIt->paramId, paramIt->paramName.c_str());

        // make sql insert to copy parameter from base run or workset
        string sql = 
            "INSERT INTO " + paramTblName + " (run_id, " + sDimLst + " value) SELECT " + sRunId + ", " + sDimLst + " value" + 
            (isFromSet ? 
                " FROM " + setTblName + " WHERE set_id = " + sSetId : 
                " FROM " + paramTblName + " WHERE run_id = " + to_string(baseRunId)
            );

        // execute insert to copy parameter
        i_dbExec->update(sql);
    }

    // unlock workset
    i_dbExec->update("UPDATE workset_lst SET is_readonly = 1 WHERE set_id = " + sSetId);

    // save source set id in run_options
    argStore.args[RunOptionsKey::setId] = sSetId;
}

// save run options in run_option table
void RunController::saveRunOptions(IDbExec * i_dbExec, const ModelDicRow * i_mdRow)
{
    // copy initial run options from profile options, default profile name = model name
    i_dbExec->update(
        "INSERT INTO run_option (run_id, option_key, option_value)" \
        " SELECT " + to_string(runId) + ", option_key, option_value" +
        " FROM profile_option WHERE profile_name = " + toQuoted(argStore.strOption(RunOptionsKey::profile, OM_MODEL_NAME))
        );

    // get model type and default run options for that model type
    NoCaseMap defaultOpt = 
        (i_mdRow->type == ModelType::timeBased) ? TimeModel::defaultOptions() : CaseModel::defaultOptions();

    // update run options: merge command line and ini-file with database and default values
    unique_ptr<IRunOptionTable> optTbl(IRunOptionTable::create(i_dbExec, runId));

    for (size_t nOpt = 0; nOpt < runOptKeySize; nOpt++) {

        if (argStore.isOptionExist(runOptKeyArr[nOpt])) continue;          // option specified at command line or ini-file

        // find option in database
        const RunOptionRow * optRow = optTbl->byKey(runId, runOptKeyArr[nOpt]);
        if (optRow != NULL) {
            argStore.args[runOptKeyArr[nOpt]] = optRow->value;   // add option from database
        }
        else {  // no database value for that option key, use model default
            NoCaseMap::const_iterator defIt = defaultOpt.find(runOptKeyArr[nOpt]);
            if (defIt != defaultOpt.cend()) argStore.args[runOptKeyArr[nOpt]] = defIt->second;
        }
    }

    // save options in database
    for (NoCaseMap::const_iterator propIt = argStore.args.cbegin(); propIt != argStore.args.cend(); propIt++) {

        // skip run id and connection string: it is already in database
        if (propIt->first == RunOptionsKey::runId || propIt->first == RunOptionsKey::dbConnStr) continue;

        i_dbExec->update(
            "DELETE FROM run_option WHERE run_id = " + to_string(runId) + " AND option_key = " + toQuoted(propIt->first)
            );

        // remove subsample count from run_option, it is stored in run_lst
        if (propIt->first == RunOptionsKey::subSampleCount) continue;
                
        i_dbExec->update(
            "INSERT INTO run_option (run_id, option_key, option_value) VALUES (" + 
            to_string(runId) + ", " + toQuoted(propIt->first) + ", " + toQuoted(propIt->second) + ")"
            );
    }
}

/** load metadata tables from db and broadcast to all modeling processes. */
MetaRunHolder * RunController::loadMetaTables(bool i_isMpiUsed, IDbExec * i_dbExec, IMsgExec * i_msgExec)
{
    if (i_msgExec == NULL) throw ModelException("invalid (NULL) message passing interface");

    // load from db by model id and run id
    unique_ptr<MetaRunHolder> mh(new MetaRunHolder);

    if (!i_isMpiUsed || i_msgExec->isRoot()) {

        if (i_dbExec == NULL) throw ModelException("invalid (NULL) database connection");

        mh->modelDic.reset(IModelDicTable::create(i_dbExec, OM_MODEL_NAME, OM_MODEL_TIMESTAMP));

        const ModelDicRow * mdRow = mh->modelDic->byNameTimeStamp(OM_MODEL_NAME, OM_MODEL_TIMESTAMP);
        if (mdRow == NULL) throw DbException("model %s not found in the database", OM_MODEL_NAME);

        mh->runOption.reset(IRunOptionTable::create(i_dbExec, runId));
        mh->typeDic.reset(ITypeDicTable::create(i_dbExec, mdRow->modelId));
        mh->typeEnumLst.reset(ITypeEnumLstTable::create(i_dbExec, mdRow->modelId));
        mh->paramDic.reset(IParamDicTable::create(i_dbExec, mdRow->modelId));
        mh->paramDims.reset(IParamDimsTable::create(i_dbExec, mdRow->modelId));
        mh->tableDic.reset(ITableDicTable::create(i_dbExec, mdRow->modelId));
        mh->tableDims.reset(ITableDimsTable::create(i_dbExec, mdRow->modelId));
        mh->tableAcc.reset(ITableAccTable::create(i_dbExec, mdRow->modelId));
        mh->tableUnit.reset(ITableUnitTable::create(i_dbExec, mdRow->modelId));
    }

    // if no db connection from child process then broadcast from root to all other processes
    if (i_isMpiUsed) {

        // broadcast run id
        i_msgExec->bcast(i_msgExec->rootRank, typeid(int), 1, &runId);
    
        // broadcast metadata tables
        broadcastTable<IModelDicTable>(mh->modelDic, i_msgExec, MsgTag::modelDic);
        broadcastTable<IRunOptionTable>(mh->runOption, i_msgExec, MsgTag::runOption);
        broadcastTable<ITypeDicTable>(mh->typeDic, i_msgExec, MsgTag::typeDic);
        broadcastTable<ITypeEnumLstTable>(mh->typeEnumLst, i_msgExec, MsgTag::typeEnumLst);
        broadcastTable<IParamDicTable>(mh->paramDic, i_msgExec, MsgTag::parameterDic);
        broadcastTable<IParamDimsTable>(mh->paramDims, i_msgExec, MsgTag::parameterDims);
        broadcastTable<ITableDicTable>(mh->tableDic, i_msgExec, MsgTag::tableDic);
        broadcastTable<ITableDimsTable>(mh->tableDims, i_msgExec, MsgTag::tableDims);
        broadcastTable<ITableAccTable>(mh->tableAcc, i_msgExec, MsgTag::tableAcc);
        broadcastTable<ITableUnitTable>(mh->tableUnit, i_msgExec, MsgTag::tableUnit);
    }

    // set run id: it is broadcasted by MPI or initialized for root process
    mh->runId = runId;

    return mh.release();
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

