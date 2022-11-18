/**
* @file
* OpenM++ modeling library: run controller class to create new model run(s) and support data exchange (create new run part)
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "model.h"
#include "modelHelper.h"
#include "runControllerImpl.h"

using namespace std;
using namespace openm;

/** create task run entry in database */
int RunController::createTaskRun(int i_taskId, IDbExec * i_dbExec)
{
    // update in transaction scope
    unique_lock<recursive_mutex> lck = i_dbExec->beginTransactionThreaded();

    // get next task run id
    i_dbExec->update("UPDATE id_lst SET id_value = id_value + 1 WHERE id_key = 'run_id_set_id'");

    int taskRunId = i_dbExec->selectToInt("SELECT id_value FROM id_lst WHERE id_key = 'run_id_set_id'", 0);
    if (taskRunId <= 0)
        throw DbException("invalid task run id: %d", taskRunId);

    string dtStr = toDateTimeString(theLog->timeStamp()); // get log date-time as string

    // make new task run name or use name specified by model run options
    string trn = cleanPathChars(argOpts().strOption(RunOptionsKey::taskRunName), OM_NAME_DB_MAX);
    if (trn.empty()) {
        trn = cleanPathChars(OM_MODEL_NAME + string("_") + argOpts().strOption(RunOptionsKey::taskName), OM_NAME_DB_MAX);
    }

    // create new task run
    i_dbExec->update(
        "INSERT INTO task_run_lst (task_run_id, task_id, run_name, sub_count, create_dt, status, update_dt, run_stamp)" \
        " VALUES (" +
        to_string(taskRunId) + ", " +
        to_string(i_taskId) + ", " +
        toQuoted(trn) + ", " +
        to_string(subValueCount) + ", " +
        toQuoted(dtStr) + ", " +
        toQuoted(RunStatus::init) + ", " +
        toQuoted(dtStr) + ", " +
        toQuoted(argOpts().strOption(ArgKey::runStamp)) + ")"
    );

    // completed: commit the changes
    i_dbExec->commit();

    setArgOpt(RunOptionsKey::taskRunId, to_string(taskRunId));
    return taskRunId;
}

/** find modeling task, if specified */
int RunController::findTask(IDbExec * i_dbExec)
{
    // find task id or name if specified as run options
    int taskId = argOpts().intOption(RunOptionsKey::taskId, 0);
    string taskName = argOpts().strOption(RunOptionsKey::taskName);

    if (taskId > 0) {
        int cnt = i_dbExec->selectToInt(
            "SELECT COUNT(*) FROM task_lst WHERE task_id = " + to_string(taskId), 0
        );
        if (cnt <= 0) throw DbException("task id not found in database: %d", taskId);
    }
    else {  // find task id by name, if taskName option specified 

        if (!taskName.empty()) {
            taskId = i_dbExec->selectToInt(
                "SELECT MIN(task_id) FROM task_lst WHERE model_id = " + to_string(modelId) +
                " AND task_name = " + toQuoted(taskName),
                0);
            if (taskId <= 0)
                throw DbException("model %s, id: %d does not contain task with name: %s", metaStore->modelRow->name.c_str(), modelId, taskName.c_str());
        }
    }

    // if task found then add it to run options
    if (taskId > 0) {

        if (taskName.empty()) {
            taskName = i_dbExec->selectToStr(
                "SELECT task_name FROM task_lst WHERE task_id = " + to_string(taskId)
            );
        }
        setArgOpt(RunOptionsKey::taskId, to_string(taskId));
        setArgOpt(RunOptionsKey::taskName, taskName);
    }

    return taskId;
}

// create new run, create input parameters and run options for input working sets
// find working set to run the model, it can be:
//  next set of current modeling task
//  set id or set name specified by run options (i.e. command line)
//  default working set for that model
tuple<int, int, ModelStatus> RunController::createNewRun(int i_taskRunId, bool i_isWaitTaskRun, IDbExec * i_dbExec)
{
    // if this is not a part of task then and model status not "initial"
    // then exit with "no more" data to signal all input completed because it was single input set
    ModelStatus mStatus = theModelRunState->status();
    if (i_taskRunId <= 0 && mStatus != ModelStatus::init) return { 0, 0, ModelStatus::shutdown };

    // update in transaction scope
    unique_lock<recursive_mutex> lck = i_dbExec->beginTransactionThreaded();

    // get next run id
    i_dbExec->update("UPDATE id_lst SET id_value = id_value + 1 WHERE id_key = 'run_id_set_id'");

    int nRunId = i_dbExec->selectToInt("SELECT id_value FROM id_lst WHERE id_key = 'run_id_set_id'", 0);
    if (nRunId <= 0)
        throw DbException("invalid run id: %d", nRunId);

    // if this is a part of modeling task then find next input set from that task
    int nSetId = 0;
    string runTs = makeTimeStamp(chrono::system_clock::now());
    string dtStr = toDateTimeString(runTs);

    if (i_taskRunId > 0) {
        i_dbExec->update(
            "UPDATE task_run_lst SET status =" \
            " CASE" \
            " WHEN status = 'i' THEN " + toQuoted(i_isWaitTaskRun ? RunStatus::waitProgress : RunStatus::progress) + 
            " ELSE status" \
            " END," +
            " update_dt = " + toQuoted(dtStr) +
            " WHERE task_run_id = " + to_string(i_taskRunId) + 
            " AND status IN ('i', 'p', 'w')"
            );
        i_dbExec->update(
            "INSERT INTO task_run_set (task_run_id, set_id, run_id, task_id)" \
            " SELECT " +
            to_string(i_taskRunId) + ", M.set_id, 0, M.task_id" +
            " FROM" \
            " (" \
            " SELECT MIN(S.set_id) AS \"set_id\", S.task_id" \
            " FROM task_set S" \
            " INNER JOIN task_run_lst RL ON (RL.task_id = S.task_id)" \
            " WHERE RL.task_run_id = " + to_string(i_taskRunId) + 
            " AND RL.status IN  ('i', 'p', 'w')" \
            " AND NOT EXISTS" \
            " (" \
            " SELECT RS.set_id FROM task_run_set RS" \
            " WHERE RS.task_run_id = " + to_string(i_taskRunId) + 
            " AND RS.set_id = S.set_id" +
            " )" \
            " ) AS M" \
            " WHERE NOT M.set_id IS NULL"
            );
        string statCode = i_dbExec->selectToStr(
            "SELECT status FROM task_run_lst WHERE task_run_id = " + to_string(i_taskRunId)
            );
        nSetId = i_dbExec->selectToInt(
            "SELECT set_id FROM task_run_set" \
            " WHERE task_run_id = " + to_string(i_taskRunId) +
            " AND run_id = 0",
            -1);

        // model status (task run status): progress, wait, shutdown or exit
        mStatus = RunState::fromRunStatus(statCode);
        if (mStatus <= ModelStatus::undefined) throw MsgException("invalid (undefined) model run status");

        // no input sets exist: task completed or wait for additional input
        if (nSetId <= 0) {                          

            if (mStatus == ModelStatus::progress || (!i_isWaitTaskRun && mStatus == ModelStatus::init)) {
                mStatus = ModelStatus::shutdown;    // task completed
            }
            i_dbExec->rollback();                   // rollback all database changes
            return { 0, 0, mStatus };               // return empty data set and current model status: wait or exit
        }
    }

    // if this is a task then input set is found at this point and model status is "in progress"
    // else (not a task) then move run status from init to "in progress"
    if (mStatus == ModelStatus::init) mStatus = ModelStatus::progress;

    // find set id and make sure this set exist in database:
    // if this is a task then check is next set of modeling task exist in database
    // else (not a task) then set specified by model run options or it is model default set
    auto[nId, isWsDefault, isReadonlyWsDefault] = findWorkset(nSetId, i_dbExec);
    nSetId = nId;

    // find base run id if base run options specified or if workset based on run
    int nBaseRunId = findBaseRun(i_dbExec);

    // make new run name or use name specified by model run options
    string rn = cleanPathChars(argOpts().strOption(RunOptionsKey::runName), OM_NAME_DB_MAX);
    if (rn.empty()) {

        rn = OM_MODEL_NAME;
        if (i_taskRunId > 0) {
            string trn = argOpts().strOption(RunOptionsKey::taskRunName);
            if (!trn.empty()) {
                rn = trn;       // if task run name specified then use it as prefix of run name
            }
            else {
                rn += "_" + argOpts().strOption(RunOptionsKey::taskName);   // task run name prefix: modelName_taskName
            }
        }
        rn += "_" + argOpts().strOption(RunOptionsKey::setName);

        rn = cleanPathChars(rn, OM_NAME_DB_MAX);
    }
    setArgOpt(RunOptionsKey::runId, to_string(nRunId));
    setArgOpt(RunOptionsKey::runName, rn);
    setArgOpt(RunOptionsKey::runCreated, runTs);

    // calculate run metadata digest and create new run entry
    RunLstRow rr(nRunId);
    rr.name = rn;
    rr.subCount = subValueCount;
    rr.createDateTime = dtStr;
    rr.runStamp = argOpts().strOption(ArgKey::runStamp);

    string metaDigest = IRunLstTable::digestRunMeta(metaStore->modelRow->digest, rr);

    i_dbExec->update(
        "INSERT INTO run_lst" \
        " (run_id, model_id, run_name, sub_count, sub_started, sub_completed, sub_restart, create_dt, status, update_dt, run_digest, value_digest, run_stamp)" \
        " VALUES (" +
        to_string(nRunId) + ", " +
        to_string(modelId) + ", " +
        toQuoted(rn) + ", " +
        to_string(subValueCount) + ", " +
        to_string(subValueCount) + ", " +
        "0, " +
        "0, " +
        toQuoted(dtStr) + ", " +
        toQuoted(RunStatus::progress) + ", " +
        toQuoted(dtStr) + ", " +
        toQuoted(metaDigest) + ", " +
        "NULL, " +
        toQuoted(rr.runStamp) + ")"
    );

    // if this is a next set of the modeling task then update task progress
    if (i_taskRunId > 0) {
        i_dbExec->update(
            "UPDATE task_run_set SET run_id = " + to_string(nRunId) +
            " WHERE task_run_id = " + to_string(i_taskRunId) + 
            " AND set_id = " + to_string(nSetId) 
            );
    }

    // save run options in run_option table
    createRunOptions(nRunId, i_dbExec);

    // copy input parameters from "base" run and working set into new run id
    createRunParameters(nRunId, nSetId, isWsDefault, isReadonlyWsDefault, nBaseRunId, i_dbExec);

    // insert run description and text
    createRunText(nRunId, nSetId, i_dbExec);

    // insert run entity metadata and create run entity tables
    if (modelRunOptions().isDbMicrodata) {
        createRunEntityTables(i_dbExec);
        insertRunEntity(nRunId, i_dbExec);
    }

    // completed: commit the changes
    i_dbExec->commit();

    return { nSetId, nRunId, mStatus };
}

// find id and name of source working set for input parameters:
//   if set id specified as run option then use such set id
//   if set name specified as run option then find set id by name
//   else use min(set id) as default set of model parameters
tuple<int, bool, bool> RunController::findWorkset(int i_setId, IDbExec * i_dbExec)
{
    // find set id of parameters workset, default is first set id for that model
    int setId = (i_setId > 0) ? i_setId : argOpts().intOption(RunOptionsKey::setId, 0);
    string setName = argOpts().strOption(RunOptionsKey::setName);
    bool isWsDefault = false;
    bool isReadonlyWsDefault = false;

    if (setId > 0) {
        int cnt = i_dbExec->selectToInt(
            "SELECT COUNT(*) FROM workset_lst WHERE set_id = " + to_string(setId), 0
        );
        if (cnt <= 0) throw DbException("working set id not found in database: %d", setId);
    }
    else {  // find set id by name, if setName option specified 

        if (!setName.empty()) {
            setId = i_dbExec->selectToInt(
                "SELECT MIN(set_id) FROM workset_lst WHERE model_id = " + to_string(modelId) +
                " AND set_name = " + toQuoted(setName),
                0);
            if (setId <= 0)
                throw DbException("model %s, id: %d does not contain working set with name: %s", metaStore->modelRow->name.c_str(), modelId, setName.c_str());
        }
    }

    // if set id not defined then use default working set for the model
    if (setId <= 0) {
        setId = i_dbExec->selectToInt(
            "SELECT MIN(set_id) FROM workset_lst WHERE model_id = " + to_string(modelId), 0
        );
        isWsDefault = true;
        isReadonlyWsDefault = 0 < i_dbExec->selectToInt(
            "SELECT is_readonly FROM workset_lst WHERE set_id = " + to_string(setId),
            0);
    }
    if (setId <= 0)
        throw DbException("model %s, id: %d must have at least one working set", metaStore->modelRow->name.c_str(), modelId);

    // update run options: actual set id and set name can be different
    setName = i_dbExec->selectToStr("SELECT set_name FROM workset_lst WHERE set_id = " + to_string(setId));
    setArgOpt(RunOptionsKey::setId, to_string(setId));
    setArgOpt(RunOptionsKey::setName, setName);

    return { setId, isWsDefault, isReadonlyWsDefault };
}

// find base run to get model parameters, it can be any existing run, including not completed run (in progress or failed).
//   if base run id specified then check is it exist and use it
//   else if run digest specified then use it to find base run id
//   else if base run name specified then select first run with that name to find base run id
int RunController::findBaseRun(IDbExec * i_dbExec)
{
    // find base run by id if specified as run options
    int baseId = argOpts().intOption(RunOptionsKey::baseRunId, 0);
    string dg;

    if (baseId > 0) {
        dg = i_dbExec->selectToStr(
            "SELECT run_digest FROM run_lst WHERE run_id = " + to_string(baseId)
        );
        if (dg.empty()) throw DbException("base run id not found for the model where id: %d", baseId, modelId);
    }

    // if base run digest specified then find base run id by run digest
    if (baseId <= 0) {
        dg = argOpts().strOption(RunOptionsKey::baseRunDigest);
        if (!dg.empty()) {
            baseId = i_dbExec->selectToInt(
                "SELECT MIN(run_id) FROM run_lst WHERE run_digest = " + toQuoted(dg) + " AND model_id = " + to_string(modelId),
                0);
            if (baseId <= 0) throw DbException("base run digest: %s not found for the model where id: %d", dg.c_str(), modelId);
        }
    }

    // if base run name specified then find base run id as first run with that name
    if (baseId <= 0) {
        if (string rn = argOpts().strOption(RunOptionsKey::baseRunName); !rn.empty()) {
            baseId = i_dbExec->selectToInt(
                "SELECT MIN(run_id) FROM run_lst WHERE run_name = " + toQuoted(rn) + " AND model_id = " + to_string(modelId),
                0);
            if (baseId <= 0) throw DbException("base run name %s not found for the model where id: %d", rn.c_str(), modelId);
        }
    }

    // if base run found then add it to run options
    if (baseId > 0) {
        setArgOpt(RunOptionsKey::baseRunId, to_string(baseId));
        if (dg.empty()) {
            dg = i_dbExec->selectToStr(
                "SELECT run_digest FROM run_lst WHERE run_id = " + to_string(baseId)
            );
        }
        if (!dg.empty()) setArgOpt(RunOptionsKey::baseRunDigest, dg);
    }

    return baseId;
}

/** return a copy of all run options as [key, value] pairs, ordered by key. */
vector<pair<string, string>> RunController::allOptions(void) const noexcept
{
    try {
        vector<pair<string, string>> kvLst;
        if (!metaStore->runOptionTable) return kvLst;   // run option table not yet loaded

        // select all run_option table rows where run id is a current run
        int nRunId = currentRunId();
        size_t n = metaStore->runOptionTable->rowCount();

        for (size_t k = 0; k < n; k++) {
            const RunOptionRow * r = metaStore->runOptionTable->byIndex(k);

            if (r != nullptr && r->runId == nRunId) kvLst.push_back(pair(r->key, r->value));
        }
        return kvLst;
    }
    catch (...) {
        return vector<pair<string, string>>();
    }
}

/** save run options by inserting into run_option table */
void RunController::createRunOptions(int i_runId, IDbExec * i_dbExec) const
{
    // save options in database
    string dbImportPrefix = string(RunOptionsKey::importDbPrefix) + ".";

    for (NoCaseMap::const_iterator optIt = argOpts().args.cbegin(); optIt != argOpts().args.cend(); optIt++) {

        // hide connection string: it can contain passwords
        if (equalNoCase(optIt->first.c_str(), RunOptionsKey::dbConnStr)) continue;
        if (equalNoCase(optIt->first.c_str(), dbImportPrefix.c_str(), dbImportPrefix.length())) continue;

        i_dbExec->update(
            "INSERT INTO run_option (run_id, option_key, option_value) VALUES (" +
            to_string(i_runId) + ", " + toQuoted(optIt->first) + ", " + toQuoted(optIt->second.substr(0, OM_OPTION_DB_MAX)) + ")"
        );
    }

    // append "last" log file path, if not explictly specified and path is not empty
    string fp = theLog->lastLogPath();
    if (!argOpts().isOptionExist(ArgKey::logFilePath) && !fp.empty()) {
        i_dbExec->update(
            "INSERT INTO run_option (run_id, option_key, option_value)" \
            " VALUES (" + to_string(i_runId) + ", " + toQuoted(ArgKey::logFilePath) + ", " + toQuoted(fp.substr(0, OM_OPTION_DB_MAX)) + ")"
        );
    }

    // append "stamped" log file path, if path is not empty
    fp = theLog->stampedLogPath();
    if (!fp.empty()) {
        i_dbExec->update(
            "INSERT INTO run_option (run_id, option_key, option_value)" \
            " VALUES (" + to_string(i_runId) + ", 'OpenM.LogStampedFilePath', " + toQuoted(fp.substr(0, OM_OPTION_DB_MAX)) + ")"
        );
    }
}

// cretate run description and notes using run options or by copy it from workset text
void RunController::createRunText(int i_runId, int i_setId, IDbExec * i_dbExec) const
{
    // insert run description using run options
    for (const auto & lOpt : langOptsMap) {
        i_dbExec->update(
            "INSERT INTO run_txt (run_id, lang_id, descr, note)" \
            " VALUES (" +
            to_string(i_runId) + ", " +
            to_string(lOpt.first) + ", " +
            toQuoted(lOpt.second.first) + ", " +
            (!lOpt.second.second.empty() ? toQuoted(lOpt.second.second) : "NULL") + ")");
    }

    // copy workset text into run text where run description is not specified by run options
    vector<WorksetTxtRow> wtRows = IWorksetTxtTable::select(i_dbExec, i_setId);

    for (const auto & row : wtRows) {
        if (langOptsMap.find(row.langId) != langOptsMap.end()) continue;    // skip: run text specified through options

        i_dbExec->update(
            "INSERT INTO run_txt (run_id, lang_id, descr, note)" \
            " VALUES (" +
            to_string(i_runId) + ", " +
            to_string(row.langId) + ", " +
            toQuoted(row.descr) + ", " +
            (!row.note.empty() ? toQuoted(row.note) : "NULL") + ")");
    }
}

// insert run entity metadata and create run entity database tables
void RunController::createRunEntityTables(IDbExec* i_dbExec)
{
    if (!modelRunOptions().isDbMicrodata) return;   // exit: disabled microdata writing into database

    // make list of microdata entity id's and attributes
    struct EntAttrs
    {
        int entityId;           // entity id
        vector<int> attrIds;    // ordered list of entity attribute id's
    };
    vector<EntAttrs> entVec;

    {
        auto eIt = entVec.end();

        for (int idx : entityIdxArr)
        {
            int eId = EntityNameSizeArr[idx].entityId;

            eIt = find_if(
                entVec.begin(),
                entVec.end(),
                [eId](EntAttrs & ea) -> bool { return ea.entityId = eId; }
            );
            if (eIt == entVec.end()) {  // new entity

                // sort attribute id's of previous entity
                if (entVec.size() > 0) {
                    EntAttrs & eLast = entVec.back();
                    if (eLast.attrIds.empty()) throw ModelException("Microdata attribute id's array is empty for entity kind: %d", eLast.entityId);

                    std::sort(eLast.attrIds.begin(), eLast.attrIds.end());
                }

                // add new entity
                eIt = entVec.emplace(entVec.end(), EntAttrs{ eId, {} });
            }

            // add new attribute id to current entity
            eIt->attrIds.push_back(EntityNameSizeArr[idx].attributeId);
        }
        if (entVec.empty()) throw ModelException("Microdata entity index array is empty");

        // sort attribute id's of last entity
        EntAttrs & eLast = entVec.back();
        if (eLast.attrIds.empty()) throw ModelException("Microdata attribute id's array is empty for entity kind: %d", eLast.entityId);

        std::sort(eLast.attrIds.begin(), eLast.attrIds.end());
    }

    // use entity generation digest (based on entity digest, attributes name, type digest)
    // to find existing entity generation db table name or create new database table
    for (const auto & ea : entVec)
    {
        // check:
        // if entity generation digest is not empty
        // then enity run tables already created and enityMap already initalized
        auto entIt = entityMap.find(ea.entityId);
        if (entIt == entityMap.end()) throw ModelException("Microdata entity map entry not found by id: %d", ea.entityId);

        if (!entIt->second.genDigest.empty()) continue; // this entity already initialzed

        // find entity and attributes metadata
        const EntityDicRow * ent = metaStore->entityDic->byKey(modelId, ea.entityId);
        if (ent == nullptr) throw DbException("Microdata entity id not found: %d", ea.entityId);

        vector<EntityAttrRow> attrs;
        for (int aId : ea.attrIds)
        { 
            const EntityAttrRow * a = metaStore->entityAttr->byKey(modelId, ea.entityId, aId);
            if (a == nullptr) throw DbException("Microdata attribute not found, entity: %s attribute id: %d", ent->entityName.c_str(), aId);

            attrs.push_back(*a);
        }

        // calculate entity table digest (generation digest)
        const string entMd5 = makeEntityGenDigest(ent, attrs, modelRunOptions().isMicrodataEvents);
        string dbTableName;

        // check if entity generation digest already exists
        int genHid = i_dbExec->selectToInt(
            "SELECT entity_gen_hid FROM entity_gen WHERE gen_digest = " + toQuoted(entMd5),
            -1);

        if (genHid > 0) // if entity generation already exist
        {
            // validate: it must be the same entity
            int eHid = i_dbExec->selectToInt(
                "SELECT entity_hid FROM entity_gen WHERE gen_digest = " + toQuoted(entMd5),
                -1);
            if (eHid != ent->entityHid) throw DbException("Internal error: entity: %s, digest: %s, entity Hid: %d, must be: %d", ent->entityName.c_str(), entMd5.c_str(), eHid, ent->entityHid);

            // select existing db table name
            dbTableName = i_dbExec->selectToStr("SELECT db_entity_table FROM entity_gen WHERE entity_gen_hid = " + to_string(genHid));
        }
        else // make new entity generation db table name and insert new entity generation record
        {
            string p = IDbExec::makeDbNamePrefix(ent->entityId, ent->entityName);
            string s = IDbExec::makeDbNameSuffix(ent->entityId, ent->entityName, entMd5);
            dbTableName = p + (modelRunOptions().isMicrodataEvents ? "_e" : "_g") + s;

            i_dbExec->update(
                "UPDATE id_lst SET id_value = id_value + 1 WHERE id_key = 'entity_hid'"
            );
            genHid = i_dbExec->selectToInt(
                "SELECT id_value FROM id_lst WHERE id_key = 'entity_hid'",
                -1);
            if (genHid <= 0) throw DbException(LT("invalid (not positive) entity Hid of: %s"), ent->entityName.c_str());

            i_dbExec->update(
                "INSERT INTO entity_gen (entity_gen_hid, entity_hid, is_events, db_entity_table, gen_digest)" \
                " VALUES (" +
                to_string(genHid) + ", " +
                to_string(ent->entityHid) + ", " +
                (modelRunOptions().isMicrodataEvents ? "1, " : "0, ") +
                toQuoted(dbTableName) + ", " +
                toQuoted(entMd5) + ")");

            for (const EntityAttrRow & at : attrs)
            {
                i_dbExec->update(
                    "INSERT INTO entity_gen_attr (entity_gen_hid, attr_id, entity_hid)" \
                    " VALUES (" +
                    to_string(genHid) + ", " +
                    to_string(at.attrId) + ", " +
                    to_string(ent->entityHid) + ")");
            }

            // create entity values table
            // 
            // CREATE TABLE Person_g87abcdef
            // (
            //   run_id     INT    NOT NULL,
            //   entity_key BIGINT NOT NULL,
            //   attr4      INT    NOT NULL,
            //   attr7      FLOAT,          -- float attribute value NaN is NULL
            //   PRIMARY KEY (run_id, entity_key)
            // )
            //
            // Or:
            // 
            // CREATE TABLE Person_e87abcdef
            // (
            //   run_id     INT    NOT NULL,
            //   entity_key BIGINT NOT NULL,
            //   event_id   INT    NOT NULL,
            //   attr4      INT    NOT NULL,
            //   attr7      FLOAT,          -- float attribute value NaN is NULL
            //   PRIMARY KEY (run_id, entity_key, event_id)
            // )
            const string prv = i_dbExec->provider();

            string bodySql = "(" \
                "run_id INT NOT NULL, " \
                " entity_key " + IDbExec::bigIntTypeName(prv) + " NOT NULL,";

            if (modelRunOptions().isMicrodataEvents) bodySql += " event_id INT NOT NULL,";

            for (const EntityAttrRow & at : attrs)
            {
                const TypeDicRow * t = metaStore->typeDic->byKey(modelId, at.typeId);
                if (t == nullptr) throw DbException("type not found for entity attribute %s %s", ent->entityName.c_str(), at.name.c_str());

                bodySql += " " + at.columnName() +
                    " " + IDbExec::valueDbType(prv, t->name, t->typeId) +
                    (isFloatType(t->name.c_str()) ? "," : " NOT NULL,");
            }

            bodySql += " PRIMARY KEY (run_id, entity_key))";

            i_dbExec->update(IDbExec::makeSqlCreateTableIfNotExist(prv, dbTableName, bodySql));
        }

        // create slql prefix to insert microdata into db
        //
        // INSERT INTO Person_g87abcdef (run_id, entity_key, attr4, attr7) VALUES (
        //
        entIt->second.sqlInsPrefix = "INSERT INTO " + dbTableName + "(run_id, entity_key";

        if (modelRunOptions().isMicrodataEvents) entIt->second.sqlInsPrefix += ", event_id";

        for (const EntityAttrRow & at : attrs)
        {
            entIt->second.sqlInsPrefix += ", " + at.columnName();
        }
        entIt->second.sqlInsPrefix += ") VALUES (";

        entIt->second.genDigest = entMd5;   // this entity is initialzed now
    }
}

// insert run entity rows for all entities in that model run
void RunController::insertRunEntity(int i_runId, IDbExec * i_dbExec)
{
    if (!modelRunOptions().isDbMicrodata) return;   // exit: disabled microdata writing into database

    // use entity generation digest (based on entity digest, attributes name, type digest)
    // to find existing entity generation db table name or create new database table
    for (const auto & e : entityMap)
    {
        const auto & ent = e.second;

        // find entity generation by digest
        int genHid = i_dbExec->selectToInt(
            "SELECT entity_gen_hid FROM entity_gen WHERE gen_digest = " + toQuoted(ent.genDigest),
            -1);
        if (genHid < 0) throw DbException("Entity %d generation not found by digest: %s", ent.entityId, ent.genDigest.c_str());

        // insert run entity row: base run is current run until value digest calculated
        i_dbExec->update(
            "INSERT INTO run_entity (run_id, entity_gen_hid, base_run_id, value_digest)" \
            " VALUES (" +
            to_string(i_runId) + ", " +
            to_string(genHid) + ", " +
            to_string(i_runId) + ", " +
            "NULL)");
    }
}

// calculate entity generation digest: based on entity digest, attribute name, type digest
const string RunController::makeEntityGenDigest(const EntityDicRow * i_entRow, const vector<EntityAttrRow> i_attrRows, bool i_isUseEvents) const
{
    // make digest header as entity name
    MD5 md5Full;
    md5Full.add("entity_digest,is_events\n", strlen("entity_digest,is_events\n"));
    string sLine = i_entRow->digest + "," + (i_isUseEvents? "1" : "0") + "\n";
    md5Full.add(sLine.c_str(), sLine.length());

    // add attributes: name and attribute type digest
    sLine = "attr_name,type_digest\n";
    md5Full.add(sLine.c_str(), sLine.length());

    for (const EntityAttrRow & attr : i_attrRows)
    {
        // find attribute type
        const TypeDicRow * tRow = metaStore->typeDic->byKey(attr.modelId, attr.typeId);
        if (tRow == nullptr)
            throw DbException(LT("in entity_attr [%s].[%s] invalid model id: %d and type id: %d: not found in type_dic"), i_entRow->entityName.c_str(), attr.name.c_str(), attr.modelId, attr.typeId);

        // add attribute to digest: name, type digest
        sLine = attr.name + "," + tRow->digest + "\n";
        md5Full.add(sLine.c_str(), sLine.length());
    }

    return md5Full.getHash();
}
