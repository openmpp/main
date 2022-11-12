/**
* @file
* OpenM++ modeling library: run controller class to create new model run(s) and support data exchange
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <filesystem>
#include "model.h"
#include "modelHelper.h"
#include "runControllerImpl.h"

using namespace std;
using namespace openm;
namespace fs = std::filesystem;

// model run basic support public interface
IRunOptions::~IRunOptions() noexcept { }
IRunBase::~IRunBase() noexcept { }

/** last cleanup */
RunController::~RunController(void) noexcept { }

/** create run controller, load metadata tables and broadcast it to all modeling processes. */
RunController * RunController::create(const ArgReader & i_argOpts, bool i_isMpiUsed, IDbExec * i_dbExec, IMsgExec * i_msgExec)
{
    int nProcess = i_isMpiUsed ? i_msgExec->worldSize() : 1;    // number of processes: MPI world size

    bool isTask = i_argOpts.intOption(RunOptionsKey::taskId, 0) > 0 || i_argOpts.strOption(RunOptionsKey::taskName) != "";
    bool isRestartId = i_argOpts.intOption(RunOptionsKey::restartRunId, 0) > 0;

    // init crc-64 static table
    // it must be done in main thread only and before any crc-64 calls
    xz_crc64_init();

    // multiple processes: create run controller for root or child process
    // single process run: create single process run controller 
    // "restart run": if no modeling task and run id explicitly specified
    unique_ptr<RunController> ctrl;

    if (nProcess > 1) {
        if (i_msgExec->isRoot()) {
            ctrl.reset(new RootController(nProcess, i_argOpts, i_dbExec, i_msgExec));
        }
        else {
            ctrl.reset(new ChildController(nProcess, i_argOpts, i_msgExec));
        }
    }
    else {
        if (!isTask && isRestartId) {
            ctrl.reset(new RestartController(i_argOpts, i_dbExec));
        }
        else {
            ctrl.reset(new SingleController(i_argOpts, i_dbExec));
        }
        i_msgExec->setCleanExit(true);  // clean exit flag applicable only to multiple processes model run, only to root or child process
    }

    // load metadata tables and broadcast it to all modeling processes
    ctrl->init();
    ctrl->initMicrodata();

    return ctrl.release();
}

/** impelementation of model process shutdown if exiting without completion. */
void RunController::doShutdownOnExit(ModelStatus i_status, int i_runId, int i_taskRunId, IDbExec * i_dbExec)
{
    updateRunState(i_dbExec, runStateStore().saveUpdated(true)); // update run status for all sub-values

    ModelStatus mStatus = theModelRunState->updateStatus(i_status);     // set model status

    // update run status and task status in database
    {
        unique_lock<recursive_mutex> lck = i_dbExec->beginTransactionThreaded();

        string sDt = makeDateTime(chrono::system_clock::now());

        // if error then update run status only else update run status and run digest
        if (RunState::isError(mStatus)) {
            i_dbExec->update(
                "UPDATE run_lst SET status = " + toQuoted(RunStatus::error) + "," +
                " update_dt = " + toQuoted(sDt) +
                " WHERE run_id = " + to_string(i_runId)
            );
        }
        else {
            i_dbExec->update(
                "UPDATE run_lst SET status = " + toQuoted(RunStatus::exit) + "," +
                " update_dt = " + toQuoted(sDt) +
                " WHERE run_id = " + to_string(i_runId)
            );

            string sDigest = IRunLstTable::digestRunValue(i_dbExec, modelId, i_runId);

            i_dbExec->update(
                "UPDATE run_lst SET value_digest = " + ((!sDigest.empty()) ? toQuoted(sDigest) : "NULL") +
                " WHERE run_id = " + to_string(i_runId)
            );
        }

        // for all sub-values where status is in-progress set to final (error) status
        i_dbExec->update(
            "UPDATE run_progress SET"
            " status = " + (!RunState::isError(mStatus) ? toQuoted(RunStatus::exit) : toQuoted(RunStatus::error)) + "," +
            " update_dt = " + toQuoted(sDt) +
            " WHERE run_id = " + to_string(i_runId) +
            " AND status IN (" + toQuoted(RunStatus::init) + ", " + toQuoted(RunStatus::progress) + ")"
        );

        // update task run progress
        if (i_taskRunId > 0) {
            i_dbExec->update(
                "UPDATE task_run_lst SET status = " +
                (!RunState::isError(mStatus) ? toQuoted(RunStatus::exit) : toQuoted(RunStatus::error)) + "," +
                " update_dt = " + toQuoted(sDt) +
                " WHERE task_run_id = " + to_string(i_taskRunId)
                );
        }
        i_dbExec->commit();
    }

    // close microdata csv files
    closeCsvMicrodata();
}

/** implementation model process shutdown: update run state and cleanup resources. */
void RunController::doShutdownAll(int i_taskRunId, IDbExec * i_dbExec)
{
    updateRunState(i_dbExec, runStateStore().saveUpdated(true)); // update run status for all sub-values

    theModelRunState->updateStatus(ModelStatus::done);          // set model status as completed OK

    if (i_taskRunId > 0) {      // update task status as completed

        string sDt = makeDateTime(chrono::system_clock::now());

        i_dbExec->update(
            "UPDATE task_run_lst SET status = " + toQuoted(RunStatus::done) + ", " +
            " update_dt = " + toQuoted(sDt) +
            " WHERE task_run_id = " + to_string(i_taskRunId)
        );
        i_dbExec->update(
            "UPDATE task_run_lst SET status = " + toQuoted(RunStatus::error) + 
            " WHERE task_run_id = " + to_string(i_taskRunId) +
            " AND EXISTS" \
            " (" \
            "   SELECT * FROM task_run_set TRS " \
            "   INNER JOIN run_lst RL ON (RL.run_id = TRS.run_id)" \
            "   WHERE TRS.task_run_id = task_run_lst.task_run_id" \
            "   AND RL.status = " + toQuoted(RunStatus::error) +
            " )"
        );
    }
}

/** implementation of model run shutdown. */
void RunController::doShutdownRun(int i_runId, int i_taskRunId, IDbExec * i_dbExec)
{
    updateRunState(i_dbExec, runStateStore().saveUpdated(true)); // update run status for all sub-values

    // update run status: all sub-values completed at this point
    string sDt = makeDateTime(chrono::system_clock::now());
    bool isRunError = false;
    {
        unique_lock<recursive_mutex> lck = i_dbExec->beginTransactionThreaded();

        i_dbExec->update(
            "UPDATE run_lst" \
            " SET status = CASE WHEN status = 'i' THEN 'p' ELSE status END," \
            " sub_completed = sub_count," \
            " sub_restart = sub_count," \
            " update_dt = " + toQuoted(sDt) +
            " WHERE run_id = " + to_string(i_runId)
        );

        i_dbExec->update(
            "UPDATE run_lst SET status = " + toQuoted(RunStatus::error) +
            " WHERE run_id = " + to_string(i_runId) +
            " AND status IN ('i', 'p')" +
            " AND EXISTS" \
            " (" \
            "   SELECT * FROM run_progress RP WHERE RP.run_id = run_lst.run_id AND RP.status = " + toQuoted(RunStatus::error) +
            " )"
        );

        isRunError = 0 < i_dbExec->selectToInt(
            "SELECT COUNT(*) FROM run_lst WHERE run_id = " + to_string(i_runId) + " AND status = " + toQuoted(RunStatus::error),
            0);

        // update task progress
        if (i_taskRunId > 0) {
            i_dbExec->update(
                "UPDATE task_run_lst SET update_dt = " + toQuoted(sDt) +
                " WHERE task_run_id = " + to_string(i_taskRunId)
            );
        }
        i_dbExec->commit();
    }

    // close microdata csv files
    closeCsvMicrodata();

    if (isRunError) return;     // run completed with errors, exit without expressions calculation

    // calculate output tables aggregated values and mark this run as completed
    theLog->logFormatted("Writing into aggregated output tables, run: %d", i_runId);

    writeOutputValues(i_runId, i_dbExec);

    // update run status as completed and calculate run digest
    theLog->logFormatted("Digest output tables, run: %d", i_runId);

    sDt = makeDateTime(chrono::system_clock::now());
    {
        unique_lock<recursive_mutex> lck = i_dbExec->beginTransactionThreaded();

        i_dbExec->update(
            "UPDATE run_lst SET status = " + toQuoted(RunStatus::done) + "," +
            " update_dt = " + toQuoted(sDt) +
            " WHERE run_id = " + to_string(i_runId)
        );

        string sDigest = IRunLstTable::digestRunValue(i_dbExec, modelId, i_runId);

        i_dbExec->update(
            "UPDATE run_lst SET value_digest = " + ((!sDigest.empty()) ? toQuoted(sDigest) : "NULL") +
            " WHERE run_id = " + to_string(i_runId)
        );

        // update task progress
        if (i_taskRunId > 0) {
            i_dbExec->update(
                "UPDATE task_run_lst SET update_dt = " + toQuoted(sDt) +
                " WHERE task_run_id = " + to_string(i_taskRunId)
            );
        }
        i_dbExec->commit();
    }
}

/** write output tables aggregated values into database, skip suppressed tables */
void RunController::writeOutputValues(int i_runId, IDbExec * i_dbExec) const
{
    const vector<TableDicRow> tblVec = metaStore->tableDic->byModelId(modelId);

    size_t nCount = 0;
    for (const TableDicRow & tblRow : tblVec) {
        if (!isSuppressed(tblRow.tableId)) nCount++;
    }

    int64_t lastMs = getMilliseconds();
    size_t n = 0;

    for (const TableDicRow & tblRow : tblVec) {

        if (isSuppressed(tblRow.tableId)) continue;     // skip suppressed table

        // report table aggregation progress
        int64_t now = getMilliseconds();
        n++;
        if (now > lastMs + OM_LOG_PROGRESS_TIME) {
            theLog->logFormatted("Table %zd of %zd: %s", n, nCount, tblRow.tableName.c_str());
            lastMs = now;
        }

        unique_ptr<IOutputTableWriter> writer(IOutputTableWriter::create(
            i_runId,
            tblRow.tableName.c_str(),
            i_dbExec,
            meta(),
            subValueCount,
            argOpts().strOption(RunOptionsKey::doubleFormat).c_str()
        ));
        writer->writeAllExpressions(i_dbExec);

        // calculate output table values digest and store only single copy of output values
        writer->digestOutput(i_dbExec);
    }
}

/** write output table accumulators if table is not suppressed. */
void RunController::doWriteAccumulators(
    int i_runId,
    IDbExec * i_dbExec,
    const RunOptions & i_runOpts,
    const char * i_name,
    size_t i_size,
    forward_list<unique_ptr<double[]> > & io_accValues
) const
{
    // find output table db row and accumulators
    const TableDicRow * tblRow = metaStore->tableDic->byModelIdName(modelId, i_name);
    if (tblRow == nullptr) throw DbException("output table not found in tables dictionary: %s", i_name);

    // find index of first accumulator: table rows ordered by model id, table id and accumulators id
    int nAcc = (int)metaStore->tableAcc->indexOf(
        [&](const TableAccRow & i_row) -> bool { return i_row.modelId == modelId && i_row.tableId == tblRow->tableId; }
    );
    if (nAcc < 0) throw DbException("output table accumulators not found: %s", i_name);

    // write accumulators into database
    if (!isSuppressed(tblRow->tableId)) {

        unique_ptr<IOutputTableWriter> writer(IOutputTableWriter::create(
            i_runId,
            i_name,
            i_dbExec,
            meta(),
            subValueCount,
            i_runOpts.useSparse,
            i_runOpts.nullValue
        ));

        for (const auto & apc : io_accValues) {
            writer->writeAccumulator(
                i_dbExec, i_runOpts.subValueId, metaStore->tableAcc->byIndex(nAcc)->accId, i_size, apc.get()
            );
            nAcc++;
        }
    }
}

/** update sub-value index to restart the run */
void RunController::updateRestartSubValueId(int i_runId, IDbExec * i_dbExec, int i_subRestart) const
{
    // update restart sub-value index
    if (i_subRestart > 0) {
        i_dbExec->update(
            "UPDATE run_lst SET status = " + toQuoted(RunStatus::progress) + "," +
            " sub_restart = " + to_string(i_subRestart) + "," +
            " update_dt = " + toQuoted(makeDateTime(chrono::system_clock::now())) +
            " WHERE run_id = " + to_string(i_runId)
            );
    }
}

/** merge updated sub-values run statue into database */
void RunController::updateRunState(IDbExec * i_dbExec, const map<pair<int, int>, RunState> i_updated) const
{
    if (i_updated.size() <= 0) return;    // no updates of run states

    // merge updated sub-values run state into run progress table
    unique_lock<recursive_mutex> lck = i_dbExec->beginTransactionThreaded();

    unordered_set<int> runErrSet;
    unordered_map<int, string> runUpdTimeMap;

    for (const auto & rst : i_updated) {

        int nRunId = rst.first.first;
        if (nRunId <= 0) continue;      // skip zero run id: it is process status

        ModelStatus mSt = rst.second.theStatus;
        string sRunId = to_string(nRunId);
        string sSubId = to_string(rst.first.second);
        string sCt = toQuoted(makeDateTime(rst.second.startTime));
        string sSt = toQuoted(RunState::toRunStatus(mSt));
        string sUpd = toQuoted(makeDateTime(rst.second.updateTime));
        string sPc = to_string(rst.second.progressCount);
        string sPv = toString(rst.second.progressValue);

        // store max updated time for each run id        
        if (auto rtIt = runUpdTimeMap.find(nRunId); rtIt != runUpdTimeMap.end()) {
            if (rtIt->second < sUpd) rtIt->second = sUpd;
        }
        else {
            runUpdTimeMap[nRunId] = sUpd;
        }

        i_dbExec->update(
            "UPDATE run_progress SET"
            " status = " + sSt + "," +
            " update_dt = " + sUpd + "," +
            " progress_count = " + sPc + "," +
            " progress_value = " + sPv +
            " WHERE run_id = " + sRunId +
            " AND sub_id = " + sSubId +
            " AND status IN('i', 'p', 'w')"
        );
        i_dbExec->update(
            "INSERT INTO run_progress" \
            " (run_id, sub_id, create_dt, status, update_dt, progress_count, progress_value)" \
            " SELECT " +
            sRunId + ", " + sSubId + ", " + sCt + ", " + sSt + ", " + sUpd + ", " + sPc + ", " + sPv +
            " FROM run_lst R" \
            " WHERE R.run_id = " + sRunId +
            " AND NOT EXISTS" \
            " (" \
            "   SELECT * FROM run_progress RP WHERE RP.run_id = R.run_id AND RP.sub_id = " + sSubId +
            ")"
        );

        if (RunState::isError(mSt)) runErrSet.insert(nRunId);
    }

    // update run status if sub-value status is error
    if (runErrSet.size() > 0) {

        // concatenate run id's where sub-value run state is error
        string sLst;
        bool isFirst = true;

        for (int n : runErrSet) {
            if (isFirst) {
                sLst = to_string(n);
                isFirst = false;
            }
            else{
                sLst += ", " + to_string(n);
            }
        }

        // set run status as error if sub-value status is error
        i_dbExec->update(
            "UPDATE run_lst SET status = " + toQuoted(RunStatus::error) +
            " WHERE run_id IN (" + sLst + ")"
            " AND status IN ('i', 'p')"
        );
    }

    // update run_lst update time for each run where status is in progress
    for (const auto & rtIt : runUpdTimeMap) {
        i_dbExec->update(
            "UPDATE run_lst SET update_dt = " + rtIt.second +
            " WHERE run_id = " + to_string(rtIt.first) + 
            " AND status IN ('i', 'p')"
        );
    }

    i_dbExec->commit();
}

// initialize microdata entity writing
void RunController::initMicrodata(void)
{
    bool isAnyCsv = modelRunOptions().isCsvMicrodata || modelRunOptions().isTraceMicrodata;
    if (!isAnyCsv && !modelRunOptions().isDbMicrodata) return;

    // setup attributes list for each entity and initialize converters from attribute value to string
    string dblFmt = argOpts().strOption(RunOptionsKey::doubleFormat);

    for (int idx : entityIdxArr)
    {
        // find entity if it is already exist
        // if entity does not exist then create new entity item
        int eId = EntityNameSizeArr[idx].entityId;
        auto eLast = entityMap.find(eId);

        if (eLast == entityMap.end()) {
            eLast = entityMap.insert({ eId, EntityItem(eId) }).first;
            if (isAnyCsv) {
                eLast->second.csvHdr = "key";
            }
        }

        // add attribute to this entity
        eLast->second.attrs.push_back(EntityAttrItem(EntityNameSizeArr[idx].attributeId, idx));
        eLast->second.attrs.back().fmtValue.reset(new ShortFormatter(EntityNameSizeArr[idx].typeOf, dblFmt.c_str()));

        if (isAnyCsv) {   // make csv header line for the entity
            eLast->second.csvHdr += ",";
            eLast->second.csvHdr += EntityNameSizeArr[idx].attribute;
        }
    }
}

/** write microdata into the database and/or CSV file. */
tuple<bool, const RunController::EntityItem &> RunController::writeMicrodata(int i_entityKind, uint64_t i_microdataKey, const void * i_entityThis)
{
    if (!modelRunOptions().isDbMicrodata && !modelRunOptions().isCsvMicrodata && !modelRunOptions().isTraceMicrodata) {
        return { false, EntityItem(i_entityKind) };     // microdata writing is not enabled
    }

    // check if any microdata write required for this entity kind
    const auto eIt = entityMap.find(i_entityKind);
    if (eIt == entityMap.cend()) return { false, EntityItem(i_entityKind) };

    if (i_entityThis == nullptr) throw ModelException("invalid (NULL) entity this pointer, entity kind: %d microdata key: %llu", i_entityKind, i_microdataKey);

    if (modelRunOptions().isCsvMicrodata) doCsvMicrodata(eIt->second, i_microdataKey, i_entityThis);

    return { true, eIt->second };
}

/** return microdata entity csv file header */
const string RunController::getHeaderCsvMicrodata(int i_entityKind) const
{
    const auto eIt = entityMap.find(i_entityKind);
    return (eIt != entityMap.cend()) ? eIt->second.csvHdr : "";
}

/** create microdata CSV files for new model run. */
void RunController::openCsvMicrodata(int i_runId)
{
    if (!modelRunOptions().isCsvMicrodata) return;   // exit: microdata csv is not enabled

    // make file stamp for csv file names: ModelName.Person.2018_11_10_22_47_46_076.0012.microdata.csv
    string stamp;

    if (metaStore->runOptionTable->isExist(i_runId, RunOptionsKey::taskRunId)) {
        stamp = "." + metaStore->runOptionTable->strValue(i_runId, RunOptionsKey::runCreated);
    }
    if (processCount > 1) {
        size_t n = to_string(processCount).length();
        string sr = string(n, '0') + to_string(processRank);
        stamp += (stamp.empty() ? "." : "-") + sr.substr(sr.length() - n);
    }

    string dir = metaStore->runOptionTable->strValue(i_runId, RunOptionsKey::microdataCsvDir);
    
    // for each entity create new microdata csv file and write csv header line
    entityCsvMap.clear();

    for (const auto & entIt : entityMap)
    {
        int eId = entIt.first;
        const auto eRow = metaStore->entityDic->byKey(modelId, eId);
        if (eRow == nullptr) throw ModelException("Microdata entity not found, entity kind: %d", eId);

        fs::path p(dir);
        string fp = p.append(OM_MODEL_NAME).concat("." + eRow->entityName + stamp + ".microdata.csv").generic_string();

        entityCsvMap[eId].entityId = eId;
        entityCsvMap[eId].filePath = fp;
        entityCsvMap[eId].csvBuf.reserve(OM_STR_DB_MAX);

        entityCsvMap[eId].csvSt.open(fp, ios::out | ios::trunc);
        entityCsvMap[eId].isReady = !entityCsvMap[eId].csvSt.fail();
        if (!entityCsvMap[eId].isReady) 
            throw HelperException("microdata CSV file open error, entity: %s %s", eRow->entityName.c_str(), fp.c_str());

        doCsvLineMicrodata(entityCsvMap[eId], entIt.second.csvHdr);   // write micordata line into csv
    }
}

/** close microdata CSV files after model run completed. */
void RunController::closeCsvMicrodata(void) noexcept
{
    try {
        for (auto & e : entityCsvMap)
        {
            {
                lock_guard<recursive_mutex> lck(e.second.theMutex); // lock the log 

                e.second.isReady = false;
                try {
                    e.second.csvSt.close();
                }
                catch (...) {}
            }
        }
        entityCsvMap.clear();
    }
    catch (...) {
    }
}

/** write microdata into CSV file. */
void RunController::doCsvMicrodata(const EntityItem & i_entityItem, uint64_t i_microdataKey, const void * i_entityThis)
{
    auto ecIt = entityCsvMap.find(i_entityItem.entityId);
    if (ecIt == entityCsvMap.end()) throw ModelException("microdata CSV not found, entity kind: %d microdata key: %llu", i_entityItem.entityId, i_microdataKey);

    EntityCsvItem & eCsv = ecIt->second;

    lock_guard<recursive_mutex> lck(eCsv.theMutex); // lock the log 

    if (!eCsv.isReady) return;  // output csv file is not ready: not open or it was write error

    // converte attribute values into string and write into csv file
    makeCsvLineMicrodata(i_entityItem, i_microdataKey, i_entityThis, eCsv.csvBuf);
    doCsvLineMicrodata(eCsv, eCsv.csvBuf);
}

/** make attributes csv line by converting attribute values into string */
void RunController::makeCsvLineMicrodata(const EntityItem & i_entityItem, uint64_t i_microdataKey, const void * i_entityThis, string & io_line)
{
    io_line.clear();
    io_line += to_string(i_microdataKey);

    for (size_t k = 0; k < i_entityItem.attrs.size(); k++)
    {
        const EntityAttrItem & attr = i_entityItem.attrs[k];
        io_line += ",";
        io_line += attr.fmtValue->formatValue(reinterpret_cast<const uint8_t *>(i_entityThis) + EntityNameSizeArr[attr.idxOf].offset);
    }
}

/** write line into microdata CSV file, close file and raise exception on error. */
void RunController::doCsvLineMicrodata(EntityCsvItem & io_entityCsvItem, const string & i_line)
{
    // write micordata line into csv
    io_entityCsvItem.csvSt << i_line << '\n';

    if (io_entityCsvItem.csvSt.fail()) {
        io_entityCsvItem.isReady = false;

        try { io_entityCsvItem.csvSt.close(); }
        catch (...) {}

        throw HelperException("microdata CSV write error, entity kind: %d microdata key: %llu %s", io_entityCsvItem.entityId, io_entityCsvItem.filePath.c_str());
    }
    // theTrace->logMsg(eCsv.csvBuf.c_str());
}

