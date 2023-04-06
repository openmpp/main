/**
* @file
* OpenM++ modeling library: run controller class to create new model run(s) and support data exchange
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <filesystem>
#include "model.h"
#include "libopenm/common/omOS.h"

using namespace std;
using namespace openm;
namespace fs = std::filesystem;

// initialize microdata entity writing
void RunController::initMicrodata(void)
{
    if (!modelRunOptions().isMicrodata()) return;

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
            if (modelRunOptions().isTextMicrodata()) {
                eLast->second.csvHdr = "key";
                if (OM_USE_MICRODATA_EVENTS && isCsvEventColumn) eLast->second.csvHdr += ",event";
            }
        }

        // add attribute to this entity
        eLast->second.attrs.push_back(EntityAttrItem(EntityNameSizeArr[idx].attributeId, idx));
        eLast->second.attrs.back().fmtValue.reset(new ShortFormatter(EntityNameSizeArr[idx].typeOf, dblFmt.c_str()));
        eLast->second.attrs.back().sqlFmtValue.reset(new ShortFormatter(EntityNameSizeArr[idx].typeOf, true, dblFmt.c_str()));

        if (modelRunOptions().isTextMicrodata()) {   // make csv header line for the entity
            eLast->second.csvHdr += ",";
            eLast->second.csvHdr += EntityNameSizeArr[idx].attribute;
        }
    }

    // setup database row layout, prepare statement helpers and buffers
    for (auto & ent : entityMap)
    {
        ptrdiff_t off = sizeof(int) + sizeof(uint64_t); // add size of run id and microdata key

        for (auto & attr : ent.second.attrs)
        {
            attr.rowOffset = off;
            off += EntityNameSizeArr[attr.idxOf].size;
        }
        ent.second.rowSize = off;   // database row size

        entityDbMap[ent.first].entityId = ent.first;    // add database rows buffer

        // create db values setters and types array for insert statement
        ent.second.rowSetter.push_back(DbValueSetter(typeid(int)));         // run id
        ent.second.rowSetter.push_back(DbValueSetter(typeid(uint64_t)));    // microdata key
        ent.second.rowTypes.push_back(&typeid(int));                        // run id
        ent.second.rowTypes.push_back(&typeid(uint64_t));                   // microdata key

        for (const EntityAttrItem & attr : ent.second.attrs)
        {
            ent.second.rowSetter.push_back(DbValueSetter(EntityNameSizeArr[attr.idxOf].typeOf));
            ent.second.rowTypes.push_back(&ent.second.rowSetter.back().valueType());
        }

        entityIds.push_back(ent.first); // add to array of entity id's for microdata child exchange
    }

    std::sort(entityIds.begin(), entityIds.end());  // sorted array of entity id's
}

// check if any microdata write required for this entity kind
tuple<bool, const RunController::EntityItem &> RunController::findEntityItem(int i_entityKind) const
{
    const auto eIt = entityMap.find(i_entityKind);
    if (eIt == entityMap.cend()) return { false, EntityItem(i_entityKind) };   // not found

    return { true, eIt->second };   // found
}

// insert run entity metadata and create run entity database tables
void RunController::createRunEntityTables(IDbExec * i_dbExec)
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
        for (int idx : entityIdxArr)
        {
            int eId = EntityNameSizeArr[idx].entityId;

            auto eIt = find_if(
                entVec.begin(),
                entVec.end(),
                [eId](EntAttrs & ea) -> bool { return ea.entityId == eId; }
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
            const EntityAttrRow * a = metaStore->entityAttr->byKey(modelId, ent->entityId, aId);
            if (a == nullptr) throw DbException("Microdata attribute not found, entity: %s attribute id: %d", ent->entityName.c_str(), aId);

            attrs.push_back(*a);
        }

        // calculate entity table digest (generation digest)
        const string entMd5 = makeEntityGenDigest(ent, attrs);
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
            dbTableName = p + "_g" + s;

            i_dbExec->update(
                "UPDATE id_lst SET id_value = id_value + 1 WHERE id_key = 'entity_hid'"
            );
            genHid = i_dbExec->selectToInt(
                "SELECT id_value FROM id_lst WHERE id_key = 'entity_hid'",
                -1);
            if (genHid <= 0) throw DbException(LT("invalid (not positive) entity Hid of: %s"), ent->entityName.c_str());

            i_dbExec->update(
                "INSERT INTO entity_gen (entity_gen_hid, entity_hid, db_entity_table, gen_digest)" \
                " VALUES (" +
                to_string(genHid) + ", " +
                to_string(ent->entityHid) + ", " +
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

            // create microdata values table
            // 
            // CREATE TABLE Person_g87abcdef
            // (
            //   run_id     INT    NOT NULL,
            //   entity_key BIGINT NOT NULL,
            //   attr4      INT    NOT NULL,
            //   attr7      FLOAT,          -- float attribute value NaN is NULL
            //   PRIMARY KEY (run_id, entity_key)
            // )
            const string prv = i_dbExec->provider();

            string bodySql = "(" \
                "run_id INT NOT NULL, " \
                " entity_key " + IDbExec::bigIntTypeName(prv) + " NOT NULL,";

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

        // create sql prefix to insert microdata into db
        // attributes column order is the same as attributes in model run options, ex: attr7, attr4
        //
        // INSERT INTO Person_g87abcdef (run_id, entity_key, attr7, attr4) VALUES (
        //
        entIt->second.sqlInsPrefix = "INSERT INTO " + dbTableName + "(run_id, entity_key";

        for (const EntityAttrItem & a : entIt->second.attrs)
        {
            const EntityAttrRow * aIt = metaStore->entityAttr->byKey(modelId, ent->entityId, a.attrId);
            if (aIt == nullptr) throw DbException("Microdata attribute not found, entity: %s attribute id: %d", ent->entityName.c_str(), a.attrId);

            entIt->second.sqlInsPrefix += ", " + aIt->columnName();
        }
        entIt->second.sqlInsPrefix += ") VALUES (";

        entIt->second.genDigest = entMd5;       // this entity is initialzed now
        entIt->second.genHid = genHid;
        entIt->second.dbMicrodataTable = dbTableName;
    }
}

// insert run entity rows for all entities in that model run
void RunController::insertRunEntity(int i_runId, IDbExec * i_dbExec)
{
    if (!modelRunOptions().isDbMicrodata) return;   // exit: disabled microdata writing into database

    // insert run entity row: base run is current run until value digest calculated
    for (const auto & e : entityMap)
    {
        i_dbExec->update(
            "INSERT INTO run_entity (run_id, entity_gen_hid, base_run_id, row_count, value_digest)" \
            " VALUES (" +
            to_string(i_runId) + ", " +
            to_string(e.second.genHid) + ", " +
            to_string(i_runId) + ", " +
            "0, " +
            "NULL)");
    }
}

// calculate entity generation digest: based on entity digest, attribute name, type digest
const string RunController::makeEntityGenDigest(const EntityDicRow * i_entRow, const vector<EntityAttrRow> i_attrRows) const
{
    // make digest header as entity name
    MD5 md5Full;
    md5Full.add("entity_digest\n", strlen("entity_digest\n"));
    string sLine = i_entRow->digest + "\n";
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

/** push microdata database rows into the buffer. */
void RunController::pushDbMicrodata(int i_runId, int i_entityKind, uint64_t i_microdataKey, const void * i_entityThis)
{
    if (!modelRunOptions().isDbMicrodata) return;

    // check if any microdata write required for this entity kind
    auto [isFound, entItem] = findEntityItem(i_entityKind);
    if (!isFound) return;

    if (i_entityThis == nullptr) throw ModelException("invalid (NULL) entity this pointer, entity kind: %d microdata key: %llu", i_entityKind, i_microdataKey);

    // pack entity attribute values into database row buffer
    auto ed = entityDbMap.find(i_entityKind);
    if (ed == entityDbMap.end()) throw ModelException("Not found entity in entity DB map, entity kind: %d microdata key: %llu", i_entityKind, i_microdataKey);
    auto & eDb = ed->second;

    lock_guard<recursive_mutex> lck(eDb.theMutex); // lock the database buffer

    unique_ptr<uint8_t[]> btArr(new uint8_t[entItem.rowSize]);
    uint8_t * pVal = btArr.get();

    size_t nOff = 0;
    nOff = memCopyTo(pVal, nOff, &i_runId, sizeof(int));                // first column: run id
    nOff = memCopyTo(pVal, nOff, &i_microdataKey, sizeof(uint64_t));    // second column: microdata key

    // copy attributes value
    for (const auto & attr : entItem.attrs)
    {
        nOff = memCopyTo(pVal, nOff, (reinterpret_cast<const uint8_t *>(i_entityThis) + EntityNameSizeArr[attr.idxOf].offset), EntityNameSizeArr[attr.idxOf].size);
    }

    eDb.rowLst.push_back(move(btArr));  // append to database microdata row buffer
}

/** pull microdata database rows from the buffer.
* if i_isNow is true then return current microdata rows for all entities
* else return non empty results only
*   if microdata row count is more than upper bound
*   or
*   if microdata row count is more than lower bound
*   time since last save is moe than microdata save time interval
*/
map<int, list<unique_ptr<uint8_t[]>>> RunController::pullDbMicrodata(bool i_isNow)
{
    map<int, list<unique_ptr<uint8_t[]>>> entMdRows;

    if (!modelRunOptions().isDbMicrodata) return entMdRows;   // return empty values

    // check interval since last save and microdata total row count
    bool isSave = i_isNow;
    chrono::system_clock::time_point nowTime = chrono::system_clock::now();

    if (!isSave)
    {
        size_t nRows = 0;
        bool isTime = false;

        for (auto & ed : entityDbMap)
        {
            EntityDbItem & eDb = ed.second;
            auto [isT, n] = statusDbMicrodata(nowTime, eDb);
            isTime |= isT;
            nRows += n;
            isSave = nRows > MaxSizeToSaveMicrodata || (isTime && nRows > MinSizeToSaveMicrodata);
            if (isSave) break;
        }
    }

    // move current microdata rows for each entity into return map
    if (isSave)
    {
        for (auto & ed : entityDbMap)
        {
            EntityDbItem & eDb = ed.second;
            entMdRows.insert(pair<int, list<unique_ptr<uint8_t[]>>>({ed.first, moveDbMicrodata(nowTime, eDb)}));
        }
    }

    return entMdRows;
}

/** by entity id return status of microdata db rows:
*   true of false if last save time more than save time intreval
*   and microdata rows count
*/
tuple<bool, size_t> RunController::statusDbMicrodata(chrono::system_clock::time_point i_nowTime, EntityDbItem & i_entityDbItem)
{
    lock_guard<recursive_mutex> lck(i_entityDbItem.theMutex);   // lock the database buffer

    return {
        i_entityDbItem.lastSaveTime + chrono::milliseconds(OM_MICRODATA_SAVE_TIME) < i_nowTime,
        i_entityDbItem.rowLst.size()
    };
}

/** return microdata db rows and clear row list */
list<unique_ptr<uint8_t[]>> && RunController::moveDbMicrodata(chrono::system_clock::time_point i_nowTime, EntityDbItem & io_entityDbItem)
{
    lock_guard<recursive_mutex> lck(io_entityDbItem.theMutex);  // lock the database buffer

    io_entityDbItem.lastSaveTime = i_nowTime;
    return move(io_entityDbItem.rowLst);
}

/** write entity microdata rows into database. */
size_t RunController::doDbMicrodata(IDbExec * i_dbExec, int i_entityId, IRowsFirstNext & i_entityMdRows)
{
    const auto emIt = entityMap.find(i_entityId);
    if (emIt == entityMap.cend()) throw ModelException("Microdata entity map entry not found by id: %d", i_entityId);

    auto & ent = emIt->second;
    size_t rowCount = 0;

    string msg;
    size_t msgSize = 200;
    msg.reserve(msgSize + 55);

    // make insert sql statement:
    //
    // INSERT INTO Person_g87abcdef (run_id, entity_key, attr0, attr3) VALUES (?, ?, ?, ?)
    //
    int nAttrs = (int)ent.attrs.size();

    string insSql = ent.sqlInsPrefix + "?, ?";  // INSERT INTO Person_g87abcdef (....) VALUES (?, ?

    for (int k = 0; k < nAttrs; k++)
    {
        insSql += ", ?";
    }
    insSql += ")";

    // storage for sub value id, dimension items and db row values
    unique_ptr<DbValue[]> valVecUptr(new DbValue[2 + nAttrs]);        // run id, microdata key, attributes
    DbValue * valVec = valVecUptr.get();

    // insert entity microdata rows in transaction scope
    uint8_t * pRow = i_entityMdRows.toFirst();
    uint64_t mKey = 0;

    if (pRow == nullptr) return 0;  // no db rows for that entity

    try {
        unique_lock<recursive_mutex> lck = i_dbExec->beginTransactionThreaded();
        {
            // prepare insert statement
            exit_guard<IDbExec> onExit(i_dbExec, &IDbExec::releaseStatement);
            i_dbExec->createStatement(insSql, (int)ent.rowTypes.size(), ent.rowTypes.data());

            do {
                // add row key: run id and microdata key
                int rId = *static_cast<const int *>(static_cast<const void *>(pRow));
                mKey = *static_cast<const uint64_t *>(static_cast<const void *>(pRow + sizeof(int)));
                valVec[0] = DbValue(rId);
                valVec[1] = DbValue(mKey);

                // add microdata attribute values
                for (size_t k = 0; k < ent.attrs.size(); k++)
                {
                    ent.rowSetter[k + 2].set(static_cast<const void *>(pRow + ent.attrs[k].rowOffset), valVec[k + 2]);
                }

                // insert cell value into parameter table
                i_dbExec->executeStatement(2 + nAttrs, valVec);

                rowCount++;
                pRow = i_entityMdRows.toNext();     // move to the next row

            } while (pRow != nullptr);
        }
        i_dbExec->commit();
    }
    catch (...) {   // log microdata key and attributes to identify the row
        try {
            size_t k = 0;
            msg = to_string(mKey);

            if (pRow != nullptr) {
                for (; k < ent.attrs.size() && msg.length() < msgSize; k++)
                {
                    msg += ", ";
                    msg += ent.attrs[k].fmtValue->formatValue(pRow + ent.attrs[k].rowOffset);
                }
            }
            if (k < ent.attrs.size()) msg += ",...";

            theLog->logMsg("Error at microdata", msg.c_str());

            i_dbExec->rollback();
        }
        catch (...) {}  // suppress error during error reporting
        //
        throw;
    }

    return rowCount;
}

/** write microdata into database using sql insert literal. */
size_t RunController::doDbMicrodataSql(IDbExec * i_dbExec, const map<int, list<unique_ptr<uint8_t[]>>> & i_entityMdRows)
{
    size_t rowCount = 0;    // total rows inserted
    string sql;
    sql.reserve(OM_STR_DB_MAX);

    string msg;
    size_t msgSize = 200;
    msg.reserve(msgSize + 55);

    for (const auto & e : entityMap)
    {
        const auto & ent = e.second;

        const auto mdIt = i_entityMdRows.find(ent.entityId);
        if (mdIt == i_entityMdRows.cend()) continue;            // no db rows for that entity
        if (mdIt->second.empty()) continue;                     // no db rows for that entity

        const auto & mdRows = mdIt->second;

        // insert entity microdata rows in transaction scope
        uint64_t mKey = 0;
        const uint8_t * pVal = nullptr;

        try {
            unique_lock<recursive_mutex> lck = i_dbExec->beginTransactionThreaded();

            for (const auto & md : mdRows)
            {
                // build sql insert
                sql.clear();
                sql += ent.sqlInsPrefix;   // INSERT INTO Person_g87abcdef (....) VALUES (

                // add row key: run id and microdata key
                pVal = md.get();

                int rId = *static_cast<const int *>(static_cast<const void *>(pVal));
                mKey = *static_cast<const uint64_t *>(static_cast<const void *>(pVal + sizeof(int)));

                sql += to_string(rId) + ", " + to_string(mKey);

                // add microdata values
                for (const EntityAttrItem & attr : ent.attrs)
                {
                    sql += ", ";
                    sql += attr.sqlFmtValue->formatValue(pVal + attr.rowOffset);
                }
                sql += ")";

                i_dbExec->update(sql);  // insert microdata row
                rowCount++;
            }
            i_dbExec->commit();
        }
        catch (...) {   // log microdata key and attributes to identify the row
            try {
                size_t k = 0;
                msg = to_string(mKey);

                if (pVal != nullptr) {
                    for (; k < ent.attrs.size() && msg.length() < msgSize; k++)
                    {
                        msg += ", ";
                        msg += ent.attrs[k].fmtValue->formatValue(pVal + ent.attrs[k].rowOffset);
                    }
                }
                if (k < ent.attrs.size()) msg += ",...";

                theLog->logMsg("Error at microdata", msg.c_str());

                i_dbExec->rollback();

            } catch (...) { }  // suppress error during error reporting
            //
            throw;
        }
    }

    return rowCount;
}

namespace
{
    // microdata table row to string converter
    struct MdValueRow : public IRowBase
    {
        MdValueRow(const vector<const type_info *> i_typeVec, const string & i_doubleFmt, string * io_mdLine) :
            colCount((int)i_typeVec.size()),
            mdLine(io_mdLine),
            typeVec(i_typeVec)
        {
            for (const auto & ti : i_typeVec)
            {
                unique_ptr<ShortFormatter> f(new ShortFormatter(*ti, i_doubleFmt.c_str()));
                fmtVec.push_back(move(f));
            }
        }

        ~MdValueRow(void) noexcept { }

        int colCount = 0;                   // number of microdata table columns: run id, entity key plus attributes count
        int lastCol = 0;                    // last column set index
        string * mdLine;                    // microdata value line
        vector<const type_info *> typeVec;  // column types
        vector<unique_ptr<ShortFormatter>> fmtVec;    // converter for value column into string

        // append microdata column value to comma separated string, skip run id (skip first column)
        void append(int i_column, const void * i_value)
        {
            if (i_column < 0 || i_column < lastCol || i_column >= colCount) throw DbException("db column number out of range: %d", i_column);

            if (i_column == 0) { // skip run id (skip first column)
                lastCol++;
                return;
            }

            // append , comma(s) for NULL attribute value(s) and last comma for the new value
            for (; lastCol < i_column; lastCol++)
            {
                mdLine->append(",");
                if (lastCol < i_column - 1) mdLine->append(nullValueString);
            }

            // append column value as string or "null" if value is NULL
            mdLine->append(fmtVec[i_column]->formatValue(i_value));
        }
    };

    // Microdata row adapter to select row from microdata values table
    class MdRowAdapter : public IRowAdapter
    {
    public:
        // new row adapter for microdata values table row
        MdRowAdapter(MdValueRow * i_mdRow) : mdRow(i_mdRow) {}

        // IRowAdapter interface implementation
        IRowBase * createRow(void) const override { return mdRow; }
        int size(void) const override { return mdRow->colCount; }
        const type_info * const * columnTypes(void) const override { return mdRow->typeVec.data(); }

        /** IRowAdapter interface implementation: set column value */
        void set(IRowBase * i_row, int i_column, const void * i_value) const override
        {
            MdValueRow * row = dynamic_cast<MdValueRow *>(i_row);
            row->append(i_column, i_value);
        }

    private:
        MdValueRow * mdRow; // microdata table row to string converter

    private:
        MdRowAdapter(const MdRowAdapter &) = delete;
        MdRowAdapter & operator=(const MdRowAdapter &) = delete;
    };

    /** row processor to calculate digest of microdata values table row */
    class MdRowDigester : public IRowProcessor
    {
    public:
        MdRowDigester(int i_colCount, MD5 * io_md5, int * io_rowCount) : colCount(i_colCount), md5(io_md5), rowCount(io_rowCount) {}

        /** IRowProcessor implementation: append row to digest */
        void processRow(IRowBaseUptr & i_row) override
        {
            MdValueRow * mdRow = dynamic_cast<MdValueRow *>(i_row.release());   // release to avoid delete

            // if microdata line has nulls at the end then add: ",null" for empty attribute values
            for (int k = mdRow->lastCol; k < colCount - 1; k++)
            {
                mdRow->mdLine->append(",").append(nullValueString);
            }

            // append current microdata line
            md5->add(mdRow->mdLine->c_str(), mdRow->mdLine->length());
            md5->add("\n", 1);      // row delimiter
            (*rowCount)++;          // count rows

            // clear buffer for the next line
            mdRow->mdLine->clear();
            mdRow->lastCol = 0;
        }

    private:
        int colCount = 0;           // number of microdata table columns: run id, entity key plus attributes count
        MD5 * md5;                  // digest calculator
        int * rowCount = nullptr;   // microdata row count

    private:
        MdRowDigester(const MdRowDigester &) = delete;
        MdRowDigester & operator=(const MdRowDigester &) = delete;
    };
}

/** calculate run value digest for all microdata entities */
void RunController::writeMicrodataDigest(int i_runId, IDbExec * i_dbExec) const
{
    if (!modelRunOptions().isDbMicrodata) return;

    theLog->logFormatted("Digest microdata values, run: %d", i_runId);

    string dblFmt = strOption(RunOptionsKey::doubleFormat);
    string mdBuf;
    mdBuf.reserve(OM_STR_DB_MAX);

    int64_t lastMs = getMilliseconds();

    for (const auto & em : entityMap)
    {
        int eId = em.first;
        const auto & ent = em.second;
        const auto eRow = metaStore->entityDic->byKey(modelId, eId);
        if (eRow == nullptr) throw ModelException("Microdata entity not found, entity kind: %d", eId);

        // report progress
        int64_t now = getMilliseconds();
        if (now > lastMs + OM_LOG_PROGRESS_TIME) {
            theLog->logFormatted("Digest entity: %s", eRow->entityName.c_str());
            lastMs = now;
        }
        string sRunId = to_string(i_runId);
        string sHid = to_string(ent.genHid);

        // sort entity attributes by id
        struct eat
        {
            int attrId;
            const type_info * attrType;
        };
        vector<eat> eatLst;

        for (const EntityAttrItem & a : ent.attrs)
        {
            eatLst.push_back({ a.attrId, &EntityNameSizeArr[a.idxOf].typeOf });
        }
        sort(
            eatLst.begin(),
            eatLst.end(),
            [](eat & i_left, eat & i_right) -> bool { return i_left.attrId < i_right.attrId; }
        );

        // column types sorted by attributes id
        vector<const type_info *> eaTypes;
        eaTypes.push_back(&typeid(int));            // run id
        eaTypes.push_back(&typeid(uint64_t));       // microdata key

        // build sql to select microdata values:
        //
        // SELECT run_id, entity_key, attr4, attr7 FROM Person_g87abcdef WHERE run_id = 11 ORDER BY 1, 2
        //
        string sql = "SELECT  run_id, entity_key";
        string hdrLine = "key";

        for (const eat & a : eatLst)
        {
            const EntityAttrRow * aIt = metaStore->entityAttr->byKey(modelId, eId, a.attrId);
            if (aIt == nullptr) throw DbException("Microdata attribute not found, entity: %s attribute id: %d", eRow->entityName.c_str(), a.attrId);

            sql += ", " + aIt->columnName();
            hdrLine += "," + aIt->name;

            eaTypes.push_back(a.attrType);
        }
        sql += " FROM " + ent.dbMicrodataTable + " WHERE run_id = " + sRunId + " ORDER BY 1, 2";

        // select microdata values and calculate digest
        MD5 md5;

        // start from metadata digest
        string sLine = "entity_name,gen_digest\n" + eRow->entityName + "," + ent.genDigest + "\n";
        md5.add(sLine.c_str(), sLine.length());

        // append values header: key,age_group,income\n
        hdrLine += "\n";
        md5.add(hdrLine.c_str(), hdrLine.length());

        // append microdata values digest
        int mdRowCount = 0;
        {
            mdBuf.clear();
            MdValueRow mdRow(eaTypes, dblFmt, &mdBuf);
            MdRowAdapter adp(&mdRow);
            MdRowDigester md5Rd(mdRow.colCount, &md5, &mdRowCount);

            i_dbExec->selectToRowProcessor(sql, adp, md5Rd);
        }

        string sDigest = md5.getHash();     // digest of microdata values

        // update digest and base run id
        //
        // UPDATE run_entity
        // SET value_digest = '22ee44cc', row_count = 87654
        // WHERE run_id = 11 entity_gen_hid = 456
        //
        // UPDATE run_entity SET 
        //   base_run_id =
        //   (
        //     SELECT MIN(E.run_id) FROM run_entity E
        //     WHERE E.entity_gen_hid = 456
        //     AND E.value_digest = '22ee44cc'
        //   )
        // WHERE run_id = 11 AND entity_gen_hid = 456
        //
        {
            unique_lock<recursive_mutex> lck = i_dbExec->beginTransactionThreaded();

            i_dbExec->update(
                "UPDATE run_entity" \
                " SET value_digest = " + toQuoted(sDigest) + ", " +
                " row_count = " + to_string(mdRowCount) +
                " WHERE run_id = " + sRunId + " AND entity_gen_hid = " + sHid
            );

            i_dbExec->update(
                "UPDATE run_entity SET base_run_id =" \
                " (" \
                " SELECT MIN(E.run_id) FROM run_entity E" \
                " WHERE E.entity_gen_hid = " + sHid +
                " AND E.value_digest = " + toQuoted(sDigest) +
                " )" \
                " WHERE run_id = " + sRunId + " AND entity_gen_hid = " + sHid
            );

            // if same digest already exists then delete current run value rows
            int nBase = i_dbExec->selectToInt(
                "SELECT base_run_id FROM run_entity WHERE run_id = " + sRunId + " AND entity_gen_hid = " + sHid,
                0);

            if (nBase > 0 && nBase != i_runId) {
                i_dbExec->update("DELETE FROM " + ent.dbMicrodataTable + " WHERE run_id = " + sRunId);
            }

            i_dbExec->commit();
        }
    }
}

/** write microdata into CSV file. */
void RunController::writeCsvMicrodata(int i_entityKind, uint64_t i_microdataKey, int i_eventId, bool i_isSameEntity, const void * i_entityThis, string & io_line)
{
    if (!modelRunOptions().isCsvMicrodata) return;

    // if event filter is enabled then check if event id is in the filter
    if (OM_USE_MICRODATA_EVENTS && !isEntityEventInFilter(i_entityKind, i_microdataKey, i_eventId)) return;

    // check if any microdata write required for this entity kind
    auto [isFound, entItem] = findEntityItem(i_entityKind);
    if (!isFound) return;

    if (i_entityThis == nullptr) throw ModelException("invalid (NULL) entity this pointer, entity kind: %d microdata key: %llu", i_entityKind, i_microdataKey);

    // find csv output tsream by entity id
    auto ecIt = entityCsvMap.find(i_entityKind);
    if (ecIt == entityCsvMap.end()) throw ModelException("microdata CSV not found, entity kind: %d microdata key: %llu", i_entityKind, i_microdataKey);

    EntityCsvItem & eCsv = ecIt->second;

    lock_guard<recursive_mutex> lck(eCsv.theMutex); // lock csv stream

    if (!eCsv.isReady) return;  // output csv file is not ready: not open or it was write error

    // convert attribute values into string and write into csv file
    makeCsvLineMicrodata(entItem, i_microdataKey, i_eventId, i_isSameEntity, i_entityThis, io_line);
    writeCsvLineMicrodata(eCsv, io_line);
}

/** return microdata entity csv file header */
const string RunController::csvHeaderMicrodata(int i_entityKind) const
{
    const auto eIt = entityMap.find(i_entityKind);
    return (eIt != entityMap.cend()) ? eIt->second.csvHdr : "";
}

/** create microdata CSV files for new model run. */
void RunController::openCsvMicrodata(void)
{
    if (!modelRunOptions().isCsvMicrodata) return;   // exit: microdata csv is not enabled

    // make file stamp for csv file names: ModelName.Person.2018_11_10_22_47_46_076.0012.microdata.csv
    string stamp;

    if (isOptionExist(RunOptionsKey::taskRunId)) {
        stamp = "." + strOption(RunOptionsKey::runCreated);
    }
    if (processCount > 1) {
        size_t n = to_string(processCount).length();
        string sr = string(n, '0') + to_string(processRank);
        stamp += (stamp.empty() ? "." : "-") + sr.substr(sr.length() - n);
    }

    string dir = strOption(RunOptionsKey::microdataCsvDir);
    
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

        entityCsvMap[eId].csvSt.open(fp, ios::out | ios::trunc);
        entityCsvMap[eId].isReady = !entityCsvMap[eId].csvSt.fail();
        if (!entityCsvMap[eId].isReady) 
            throw HelperException("microdata CSV file open error, entity: %s %s", eRow->entityName.c_str(), fp.c_str());

        writeCsvLineMicrodata(entityCsvMap[eId], entIt.second.csvHdr);   // write micordata line into csv
    }
}

/** close microdata CSV files after model run completed. */
void RunController::closeCsvMicrodata(void) noexcept
{
    try {
        for (auto & e : entityCsvMap)
        {
            {
                lock_guard<recursive_mutex> lck(e.second.theMutex); // lock csv stream

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

/** make attributes csv line by converting attribute values into string */
void RunController::makeCsvLineMicrodata(const EntityItem & i_entityItem, uint64_t i_microdataKey, int i_eventId, bool i_isSameEntity, const void * i_entityThis, string & io_line) const
{
    // if event filter is enabled then check if event id is in the filter
    if (OM_USE_MICRODATA_EVENTS && !isEntityEventInFilter(i_entityItem.entityId, i_microdataKey, i_eventId)) return;

    io_line.clear();
    io_line += to_string(i_microdataKey);

    if (OM_USE_MICRODATA_EVENTS && isCsvEventColumn) {

        const char * name = EventIdNameItem::byId(i_eventId);
        if (name == nullptr) throw ModelException("entity event name not found by event id: %d, entity kind: %d microdata key: %llu", i_eventId, i_entityItem.entityId, i_microdataKey);

        io_line += ",";
        if (!i_isSameEntity) io_line += "*";
        io_line += name;
    }

    for (size_t k = 0; k < i_entityItem.attrs.size(); k++)
    {
        const EntityAttrItem & attr = i_entityItem.attrs[k];
        io_line += ",";
        io_line += attr.fmtValue->formatValue(reinterpret_cast<const uint8_t *>(i_entityThis) + EntityNameSizeArr[attr.idxOf].offset);
    }
}

/** write line into microdata CSV file, close file and raise exception on error. */
void RunController::writeCsvLineMicrodata(EntityCsvItem & io_entityCsvItem, const string & i_line)
{
    // write micordata line into csv
    io_entityCsvItem.csvSt << i_line << '\n';

    if (io_entityCsvItem.csvSt.fail()) {
        io_entityCsvItem.isReady = false;

        try { io_entityCsvItem.csvSt.close(); }
        catch (...) {}

        throw HelperException("microdata CSV write error, entity kind: %d microdata key: %llu %s", io_entityCsvItem.entityId, io_entityCsvItem.filePath.c_str());
    }
}

/** retrun true if entity event id is matching events filter or if there no filter and all events are allowed */
bool RunController::isEntityEventInFilter(int i_entityKind, uint64_t i_microdataKey, int i_eventId) const
{
    if (!EventIdNameItem::checkId(i_eventId))
        throw ModelException("invalid entity event id: %d, entity kind: %d microdata key: %llu", i_eventId, i_entityKind, i_microdataKey);

    return entityUseEvents.size() <= 0 ||
        (i_eventId >= 0 && i_eventId < (int)entityUseEvents.size() && entityUseEvents[i_eventId]);
}
