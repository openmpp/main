// OpenM++ data library: output table reader
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "dbValue.h"
#include "dbOutputTable.h"
#include "libopenm/omModelRunState.h"

using namespace openm;

namespace openm
{
    /** output table reader base class */
    class OutputTableReader
    {
    public:
        OutputTableReader() :
            runId(0), isSparse(false), readSize(0), totalSize(0), idCount(0)
        { }

        // output table reader cleanup
        virtual ~OutputTableReader(void) noexcept { }

        /** read output table values from sparse output table into result array */
        void readTable(IDbExec * i_dbExec, bool i_isNanFill, size_t i_size, double * io_valueArr);

    protected:
        int runId;                  // source run id
        bool isSparse;              // if true the table is sparse
        size_t readSize;            // number of values to select from the table
        size_t totalSize;           // total number of values in the table
        string tableName;           // output table name
        string sqlQuery;            // sql to select expression values from output table
        int idCount;                // number of id columns
        vector<int> idSizeVec;      // size of each id column
        vector<vector<int>> colIds; // id's for each sql column: expression ids or accumulator ids, sub value ids, enum ids for each dimension

        // basic validation and member initalization
        tuple<int, int, int, const TableDicRow *, const vector<TableDimsRow>> startup(
            const char * i_name,
            IDbExec * i_dbExec,
            const MetaHolder * i_metaStore);

        // find model run and base table run id
        tuple<int, int> findBaseRunId(int i_runId, int i_tableHid, const char * i_name, IDbExec * i_dbExec);

        // for each dimension get size and enum id's including total enum id, calculate total size
        void processDims(int i_modelId, const char * i_name, const vector<TableDimsRow> & i_tableDims, const MetaHolder * i_metaStore);

    private:
        OutputTableReader(const OutputTableReader & i_reader) = delete;
        OutputTableReader & operator=(const OutputTableReader & i_reader) = delete;
    };

    // output table expressions reader implementation
    class OutputTableExprReader : public OutputTableReader, public IOutputTableExprReader
    {
    public:
        OutputTableExprReader(
            int i_runId,
            const char * i_name,
            IDbExec * i_dbExec,
            const MetaHolder * i_metaStore,
            const char * i_expr = ""
            );

        // output table reader cleanup
        ~OutputTableExprReader(void) noexcept { }

        /** return number of values to select from the table */
        size_t sizeOf(void) const noexcept override { return readSize; }

        /** return output table size: total number of values in the table */
        size_t totalSizeOf(void) const noexcept override { return totalSize; }

        /** read output table values from sparse output table into result array */
        void readTable(IDbExec * i_dbExec, bool i_isNanFill, size_t i_size, double * io_valueArr) override {
            OutputTableReader::readTable(i_dbExec, i_isNanFill, i_size, io_valueArr);
        };

    private:
        OutputTableExprReader(const OutputTableExprReader & i_reader) = delete;
        OutputTableExprReader & operator=(const OutputTableExprReader & i_reader) = delete;
    };

    // output table accumulators reader implementation
    class OutputTableAccReader : public OutputTableReader, public IOutputTableAccReader
    {
    public:
        OutputTableAccReader(
            int i_runId,
            IDbExec * i_dbExec,
            const char * i_name,
            const MetaHolder * i_metaStore,
            const char * i_acc = "",
            const vector<int> & i_subIdArr = {}
            );

        // output table reader cleanup
        ~OutputTableAccReader(void) noexcept { }

        /** return number of values to select from the table */
        size_t sizeOf(void) const noexcept override { return readSize; }

        /** return output table size: total number of values in the table */
        size_t totalSizeOf(void) const noexcept override { return totalSize; }

        /** read output table values from sparse output table into result array */
        void readTable(IDbExec * i_dbExec, bool i_isNanFill, size_t i_size, double * io_valueArr) override {
            OutputTableReader::readTable(i_dbExec, i_isNanFill, i_size, io_valueArr);
        };

    private:
        OutputTableAccReader(const OutputTableAccReader & i_reader) = delete;
        OutputTableAccReader & operator=(const OutputTableAccReader & i_reader) = delete;
    };

    /** row processor to put values into result array from sparse db table select results */
    class ValueRowSparseSetter : public IRowProcessor
    {
    public:
        ValueRowSparseSetter(size_t i_size, const vector<int> & i_idSizeVec, const vector<vector<int>> & i_colIds, double * io_valueArr);

        /** IRowProcessor implementation: put values into result array from sparse output table */
        void processRow(IRowBaseUptr & i_row) override;

    private:
        size_t readSize = 0;                // number of values to select from the table
        const vector<int> & idSizeVec;      // size of each id column
        const vector<vector<int>> & colIds; // id's for each sql column: expression ids or accumulator ids, sub value ids, enum ids for each dimension
        double * valueArr = nullptr;        // result array to set values
        int idCount = 0;                    // number of id columns
        unique_ptr<int[]> idxArrUptr;         // storage for current row indices of id's
        int * idxArr = nullptr;             // current row indices for each id from id column

    private:
        ValueRowSparseSetter(const ValueRowSparseSetter & i_reader) = delete;
        ValueRowSparseSetter & operator=(const ValueRowSparseSetter & i_reader) = delete;
    };
}

// output table reader cleanup
IOutputTableReader::~IOutputTableReader(void) noexcept { }
IOutputTableExprReader::~IOutputTableExprReader(void) noexcept { }
IOutputTableAccReader::~IOutputTableAccReader(void) noexcept { }

/** output table reader factory: create new reader table expressions reader */
IOutputTableExprReader * IOutputTableExprReader::create(
    int i_runId,
    const char * i_name,
    IDbExec * i_dbExec,
    const MetaHolder * i_metaStore,
    const char * i_expr)
{
    return new OutputTableExprReader(i_runId, i_name, i_dbExec, i_metaStore, i_expr);
}

/** output table reader factory: create new reader table accumulators reader */
IOutputTableAccReader * IOutputTableAccReader::create(
    int i_runId,
    const char * i_name,
    IDbExec * i_dbExec,
    const MetaHolder * i_metaStore,
    const char * i_acc,
    const vector<int> & i_subIdArr
)
{
    return new OutputTableAccReader(i_runId, i_dbExec, i_name, i_metaStore, i_acc, i_subIdArr);
}

// basic validation and member initalization
tuple<int, int, int, const TableDicRow *, const vector<TableDimsRow>> OutputTableReader::startup(
    const char * i_name,
    IDbExec * i_dbExec,
    const MetaHolder * i_metaStore)
{
    // check parameters
    if (i_dbExec == nullptr) throw DbException("invalid (NULL) database connection");
    if (i_name == nullptr) throw DbException("invalid (NULL) output table name");
    if (i_metaStore == nullptr) throw DbException("invalid (NULL) model metadata");

    if (i_metaStore->tableDic == nullptr) throw DbException("invalid (NULL) table dictionary model metadata");
    if (i_metaStore->tableDims == nullptr) throw DbException("invalid (NULL) table dimensions model metadata");
    if (i_metaStore->typeDic == nullptr) throw DbException("invalid (NULL) type dictionary model metadata");
    if (i_metaStore->typeEnumLst == nullptr) throw DbException("invalid (NULL) type enums list model metadata");

    // get table row table dimensions
    int mId = i_metaStore->modelRow->modelId;

    const TableDicRow * tableRow = i_metaStore->tableDic->byModelIdName(mId, i_name);
    if (tableRow == nullptr) throw DbException("output table not found in tables dictionary: %s", i_name);

    int tableId = tableRow->tableId;
    int dimCount = tableRow->rank;
    isSparse = tableRow->isSparse;

    const vector<TableDimsRow> tableDims = i_metaStore->tableDims->byModelIdTableId(mId, tableId);
    if (dimCount < 0 || dimCount != (int)tableDims.size()) throw DbException("invalid table rank or dimensions not found for table: %s", i_name);

    return { mId, tableId, dimCount, tableRow, tableDims };
}

// find model run and base table run id
tuple<int, int> OutputTableReader::findBaseRunId(int i_runId, int i_tableHid, const char * i_name, IDbExec * i_dbExec)
{
    vector<RunLstRow> runVec = IRunLstTable::byKey(i_dbExec, i_runId);
    if (runVec.empty()) throw DbException("run id: %d not found in the database", i_runId);

    const RunLstRow & runRow = runVec[0];
    if (runRow.status != RunStatus::done) throw DbException("run id: %d not completed successfully", i_runId);

    int baseRunId = i_dbExec->selectToInt(
        "SELECT base_run_id FROM run_table WHERE run_id = " + to_string(i_runId) + " AND table_hid = " + to_string(i_tableHid),
        0);
    if (baseRunId <= 0) throw DbException("output table base run id not found in the database, run id: %d, table:", i_runId, i_name);

    return { baseRunId, runRow.subCount };
}

// for each dimension get size and enum id's including total enum id, calculate total size
void OutputTableReader::processDims(int i_modelId, const char * i_name, const vector<TableDimsRow> & i_tableDims, const MetaHolder * i_metaStore)
{
    for (int nDim = 0; nDim < (int)i_tableDims.size(); nDim++) {

        const TableDimsRow & dim = i_tableDims[nDim];
        if (dim.dimSize <= 0) throw DbException("invalid size: %d of dimension %s for table: %s", dim.dimSize, dim.name.c_str(), i_name);

        const vector<TypeEnumLstRow> de = i_metaStore->typeEnumLst->byModelIdTypeId(i_modelId, dim.typeId);

        size_t n = de.size() + (dim.isTotal ? 1 : 0);
        if (n != (size_t)dim.dimSize) throw DbException("invalid size: %d of dimension %s for table: %s, expected: %zd", dim.dimSize, dim.name.c_str(), i_name, n);

        vector<int> eIds(n);
        for (size_t k = 0; k < de.size(); k++) {
            eIds[k] = de[k].enumId;
        }
        if (dim.isTotal) {
            const TypeDicRow * t = i_metaStore->typeDic->byKey(i_modelId, dim.typeId);
            if (t == nullptr) throw DbException("type not found for output table dimension %s %s", i_name, dim.name.c_str());

            eIds[dim.dimSize - 1] = t->totalEnumId;
            sort(eIds.begin(), eIds.end());
        }

        colIds[nDim].swap(eIds);
        idSizeVec[nDim] = dim.dimSize;
        readSize *= dim.dimSize;
        totalSize *= dim.dimSize;
    }

    if (readSize <= 0 || totalSize > INT_MAX) throw DbException("invalid size of the table: %s", i_name);
}

// new output table expression reader
OutputTableExprReader::OutputTableExprReader(
    int i_runId,
    const char * i_name,
    IDbExec * i_dbExec,
    const MetaHolder * i_metaStore,
    const char * i_expr)
{
    // basic validation and member initalization
    auto[mId, tableId, dimCount, tableRow, tableDims] = startup(i_name, i_dbExec, i_metaStore);

    // get table expressions rows
    if (i_metaStore->tableExpr == nullptr) throw DbException("invalid (NULL) table expressions model metadata");

    const vector<TableExprRow> tableExpr = i_metaStore->tableExpr->byModelIdTableId(mId, tableId);
    int exprCount = (int)tableExpr.size();
    if (exprCount <= 0) throw DbException("output table expression rows not found in database: %s", i_name);

    // if we need to read only single expression then find expression row
    bool isAll = i_expr == nullptr || i_expr[0] == '\0';

    int nExprId = 0;
    if (!isAll) {
        auto tr = i_metaStore->tableExpr->findFirst(
            [i_expr](TableExprRow i_row) -> bool { return i_row.name == i_expr; }
        );
        if (tr == nullptr) throw DbException("output table expression not found: %s %s", i_name, i_expr);

        nExprId = tr->exprId;
    }

    // add all expression ids into column ids array
    idCount = 1 + dimCount;
    colIds.resize(idCount);
    idSizeVec.resize(idCount);

    if (!isAll) {
        colIds[0] = { nExprId };
        idSizeVec[0] = 1;
        readSize = 1;
    }
    else {
        vector<int> nIds(exprCount);
        for (int k = 0; k < exprCount; k++) {
            nIds[k] = tableExpr[k].exprId;
        }
        colIds[0].swap(nIds);

        idSizeVec[0] = exprCount;
        readSize = exprCount;
    }
    totalSize = exprCount;

    // for each dimension get size and enum id's including total enum id, calculate total size
    processDims(mId, i_name, tableDims, i_metaStore);

    // find model run and base table run id
    auto[baseRunId, subCount] = findBaseRunId(i_runId, tableRow->tableHid, i_name, i_dbExec);

    // build sql to select expressions values:
    //
    // SELECT expr_id, dim0, dim1, expr_value
    // FROM salarySex_v201208171604590148
    // WHERE run_id = 11
    // AND expr_id = 0
    // ORDER BY 1, 2, 3
    //
    sqlQuery = "SELECT expr_id, ";
    string orderBy = " ORDER BY 1";

    for (int k = 0; k < dimCount; k++) {
        sqlQuery += tableDims[k].columnName() + ", ";
        orderBy += ", " + to_string(k + 2);
    }

    sqlQuery += "expr_value FROM " + tableRow->dbExprTable +
        " WHERE run_id = " + to_string(baseRunId) +
        (!isAll ? " AND expr_id = " + to_string(nExprId) : "") +
        orderBy;
}

// new output table accumulator reader
OutputTableAccReader::OutputTableAccReader(
    int i_runId,
    IDbExec * i_dbExec,
    const char * i_name,
    const MetaHolder * i_metaStore,
    const char * i_acc,
    const vector<int> & i_subIdArr)
{
    // basic validation and member initalization
    auto [mId, tableId, dimCount, tableRow, tableDims] = startup(i_name, i_dbExec, i_metaStore);

    // get table accumulator rows
    if (i_metaStore->tableAcc == nullptr) throw DbException("invalid (NULL) table accumulators model metadata");

    const vector<TableAccRow> tableAcc = i_metaStore->tableAcc->byModelIdTableId(mId, tableId);
    int accCount = (int)tableAcc.size();
    if (accCount <= 0) throw DbException("output table accumulator rows not found in database: %s", i_name);

    // if we need to read only single accumulator then find accumulator row
    bool isAllAcc = i_acc == nullptr || i_acc[0] == '\0';

    int nAccId = 0;
    if (!isAllAcc) {
        auto tr = i_metaStore->tableAcc->findFirst(
            [i_acc](TableAccRow i_row) -> bool { return i_row.name == i_acc; }
        );
        if (tr == nullptr) throw DbException("output table accumulator not found: %s %s", i_name, i_acc);

        nAccId = tr->accId;
    }

    // add all accumulator ids into column ids array
    idCount = 2 + dimCount;
    colIds.resize(idCount);
    idSizeVec.resize(idCount);

    if (!isAllAcc) {
        colIds[0] = { nAccId };
        idSizeVec[0] = 1;
        readSize = 1;
    }
    else {
        vector<int> nIds(accCount);
        for (int k = 0; k < accCount; k++) {
            nIds[k] = tableAcc[k].accId;
        }
        colIds[0].swap(nIds);

        idSizeVec[0] = accCount;
        readSize = accCount;
    }
    totalSize = accCount;

    // find model run and base table run id
    auto [baseRunId, subCount] = findBaseRunId(i_runId, tableRow->tableHid, i_name, i_dbExec);

    // add sub value ids into column ids array
    bool isAllSub = i_subIdArr.size() <= 0;

    if (isAllSub) {
        vector<int> nIds(subCount);
        for (int k = 0; k < subCount; k++) {
            nIds[k] = k;
        }
        colIds[1].swap(nIds);
        idSizeVec[1] = subCount;
        readSize *= subCount;
    }
    else {
        int nSub = (int)i_subIdArr.size();
        vector<int> nIds(nSub);
        for (int k = 0; k < nSub; k++) {
            nIds[k] = i_subIdArr[k];
            if (nIds[k] <= 0 || nIds[k] >= subCount) throw DbException("output table sub value id %d out of range %s", i_acc, i_name);
        }
        colIds[1].swap(nIds);
        idSizeVec[1] = nSub;
        readSize *= nSub;
    }
    totalSize *= subCount;

    // for each dimension get size and enum id's including total enum id, calculate total size
    processDims(mId, i_name, tableDims, i_metaStore);

    // build sql to select accumulator values:
    //
    // SELECT acc_id, sub_id, dim0, dim1, acc_value
    // FROM salarySex_a201208171604590148
    // WHERE run_id = 11
    // ORDER BY 1, 2, 3, 4
    //
    sqlQuery = "SELECT acc_id, sub_id, ";
    string orderBy = " ORDER BY 1, 2";

    for (int k = 0; k < dimCount; k++) {
        sqlQuery += tableDims[k].columnName() + ", ";
        orderBy += ", " + to_string(k + 3);
    }

    sqlQuery += "acc_value FROM " + tableRow->dbAccTable +
        " WHERE run_id = " + to_string(baseRunId) +
        (!isAllAcc ? " AND acc_id = " + to_string(nAccId) : "");
 
    if (!isAllSub) {
        sqlQuery += " AND sub_id IN (";
        for (size_t n = 0; n < i_subIdArr.size(); n++) {
            sqlQuery += to_string(i_subIdArr[n]) + ((n < i_subIdArr.size() - 1) ? ", " : "");
        }
        sqlQuery += ")";
    }

    sqlQuery += orderBy;
}

/** read output table values from sparse output table into result array */
void OutputTableReader::readTable(IDbExec * i_dbExec, bool i_isNanFill, size_t i_size, double * io_valueArr)
{
    // check parameters
    if (i_dbExec == nullptr) throw DbException("invalid (NULL) database connection");
    if (i_size != readSize) throw DbException("invalid size: %zd, expected: %zd of output values array to read output table %s", i_size, readSize, tableName.c_str());
    if (i_size != readSize || io_valueArr == nullptr) throw DbException("invalid (NULL) values array to read output table %s", tableName.c_str());

    // fill result array with zero or NaN
    // zero is default behavior for sparse table, NaN normally indicate existing db row with NULL value
    std::fill(
        io_valueArr,
        &io_valueArr[readSize],
        (i_isNanFill ? numeric_limits<double>::quiet_NaN() : 0.0)
    );

    // if table not sparse then select value column
    if (!isSparse) {
        i_dbExec->selectColumn(sqlQuery, idCount, typeid(double), i_size, io_valueArr);
    }
    else {  // create row processor to set sparse values
        ValueRowSparseSetter setter(i_size, idSizeVec, colIds, io_valueArr);
        ValueRowAdapter adp(idCount, typeid(double));

        i_dbExec->selectToRowProcessor(sqlQuery, adp, setter);
    }
}

/** initialize row processor to read sparse table values */
ValueRowSparseSetter::ValueRowSparseSetter(
    size_t i_size, const vector<int> & i_idSizeVec, const vector<vector<int>> & i_colIds, double * io_valueArr
) :
    readSize(i_size),
    idSizeVec(i_idSizeVec),
    colIds(i_colIds),
    valueArr(io_valueArr)
{
    if (readSize <= 0 || valueArr == nullptr) throw DbException("invalid size (or NULL) output values array");

    // make array to store current row indices for column id's
    idCount = (int)idSizeVec.size();

    idxArrUptr.reset(new int[idCount]);
    idxArr = idxArrUptr.get();
    std::fill(idxArr, &idxArr[idCount], 0);
}

/** process each row selected from sparse output table and put value into result array */
void ValueRowSparseSetter::processRow(IRowBaseUptr & i_row)
{
    ValueRow * row = dynamic_cast<ValueRow *>(i_row.get());

    // for each id column find index of id and calculate value cell offset
    int prevSize = 1;
    int cellOffset = 0;

    for (int k = idCount - 1; k >= 0; k--) {

        auto nId = row->idArr[k]; // current enum id

        // expected id if table not sparse
        int eId = idxArr[k];
        if (idSizeVec[k] > 0) eId = colIds[k][idxArr[k]];

        // if table sparse then we may need to search current id
        if (eId != nId) {
            if (idSizeVec[k] <= 0) throw DbException("enum id: %d not found in column: %d", nId, k);

            auto it = lower_bound(colIds[k].cbegin(), colIds[k].cend(), nId);
            if (it == colIds[k].cend() || *it != nId) throw DbException("enum id: %d not found in column: %d", nId, k);

            idxArr[k] = (int)distance(colIds[k].cbegin(), it);   // id found: adjust index of id
        }

        cellOffset += idxArr[k] * prevSize;     // value cell index
        prevSize *= idSizeVec[k];
    }

    // set value cell
    if (cellOffset < 0 || cellOffset >= (int)readSize) throw DbException("invalid value cell offset: %d, it must be between zero and %zd", cellOffset, readSize);

    valueArr[cellOffset] = row->isNotNull ? row->dbVal.dVal : numeric_limits<double>::quiet_NaN();

    // get next cell indices
    for (int n = idCount - 1; n >= 0; n--) {
        if (n > 0 && idxArr[n] >= idSizeVec[n] - 1) {
            idxArr[n] = 0;
        }
        else {
            idxArr[n]++;
            break;
        }
    }
    if (idCount > 0 && idxArr[0] >= idSizeVec[0]) idxArr[0] = 0;    // past last row: id columns should be ordered by id's (enum ids, ex[pression id, sub value id)
}
