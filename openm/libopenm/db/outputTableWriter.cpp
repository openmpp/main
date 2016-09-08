// OpenM++ data library: output table writer
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "dbValue.h"
#include "dbOutputTable.h"

using namespace openm;

namespace openm
{
    // output table writer implementation
    class OutputTableWriter : public IOutputTableWriter
    {
    public:
        OutputTableWriter(
            int i_runId,
            const char * i_name, 
            IDbExec * i_dbExec, 
            const MetaRunHolder * i_metaStore, 
            int i_numSubSamples,
            bool i_isSparse = false, 
            double i_nullValue = DBL_EPSILON
            );

        // Output table writer cleanup
        ~OutputTableWriter(void) throw() { }

        // return total number of values for each accumulator
        size_t sizeOf(void) const throw() override { return totalSize; }

        // return number of output aggregated expressions
        int expressionCount(void) const throw() override { return exprCount; }

        // write output table accumulator values
        void writeAccumulator(IDbExec * i_dbExec, int i_nSubSample, int i_accId, size_t i_size, const double * i_valueArr) override;

        // write all output table values: aggregate subsamples using table expressions
        void writeAllExpressions(IDbExec * i_dbExec) override;

        // write output table value: aggregated output expression value
        void writeExpression(IDbExec * i_dbExec, int i_nExpression) override;

        // calculate output table values digest and store only single copy of output values
        void digestOutput(IDbExec * i_dbExec) override;

    private:
        int runId;                      // destination run id
        int modelId;                    // model id in database
        int tableId;                    // output table id
        int numSubSamples;              // number of subsamples in model run
        int dimCount;                   // number of dimensions
        int accCount;                   // number of accumulators
        int exprCount;                  // number of aggregated expressions
        size_t totalSize;               // total number of values in the table
        bool isSparseTable;             // if true then use sparse output to database
        double nullValue;               // if is sparse and abs(value) <= nullValue then value not stored
        string accDbTable;              // db table name for accumulators
        string valueDbTable;            // db table name for aggregated expressions
        const TableDicRow * tableRow;   // table db row
        vector<TableDimsRow> tableDims; // table dimensions
        vector<TableAccRow> tableAcc;   // table accumulators
        vector<TableExprRow> tableExpr; // table aggregation expressions

    private:
        OutputTableWriter(const OutputTableWriter & i_writer) = delete;
        OutputTableWriter & operator=(const OutputTableWriter & i_writer) = delete;
    };
}

// Output table writer cleanup
IOutputTableWriter::~IOutputTableWriter(void) throw() { }

// Output table writer factory: create new writer
IOutputTableWriter * IOutputTableWriter::create(
    int i_runId,
    const char * i_name, 
    IDbExec * i_dbExec, 
    const MetaRunHolder * i_metaStore, 
    int i_numSubSamples, 
    bool i_isSparseGlobal, 
    double i_nullValue
    )
{
    return new OutputTableWriter(
        i_runId, i_name, i_dbExec, i_metaStore, i_numSubSamples, i_isSparseGlobal, i_nullValue
        );
}

// New output table writer
OutputTableWriter::OutputTableWriter(
    int i_runId,
    const char * i_name, 
    IDbExec * i_dbExec, 
    const MetaRunHolder * i_metaStore, 
    int i_numSubSamples, 
    bool i_isSparseGlobal, 
    double i_nullValue
    ) :
    runId(i_runId),
    modelId(0),
    tableId(0),
    numSubSamples(i_numSubSamples),
    dimCount(0),
    accCount(0),
    exprCount(0),
    totalSize(0),
    isSparseTable(i_isSparseGlobal),
    nullValue(i_nullValue >= 0.0 ? i_nullValue : DBL_EPSILON),
    tableRow(nullptr)
{ 
    // check parameters
    if (i_dbExec == nullptr) throw DbException("invalid (NULL) database connection");
    if (i_metaStore == nullptr) throw DbException("invalid (NULL) model metadata");
    if (i_name == nullptr || i_name[0] == '\0') throw DbException("Invalid (empty) output table name");

    // get table dimensions, accumulators and aggregation expressions
    modelId = i_metaStore->modelRow->modelId;

    tableRow = i_metaStore->tableDic->byModelIdName(modelId, i_name);
    if (tableRow == nullptr) throw DbException("output table not found in table dictionary: %s", i_name);

    tableId = tableRow->tableId;
    dimCount = tableRow->rank;

    accDbTable = tableRow->dbAccTable;
    valueDbTable = tableRow->dbExprTable;

    tableDims = i_metaStore->tableDims->byModelIdTableId(modelId, tableId);
    if (dimCount < 0 || dimCount != (int)tableDims.size()) throw DbException("invalid table rank or dimensions not found for table: %s", i_name);

    tableAcc = i_metaStore->tableAcc->byModelIdTableId(modelId, tableId);
    accCount = (int)tableAcc.size();
    if (accCount <= 0) throw DbException("output table accumulators not found for table: %s", i_name);

    tableExpr = i_metaStore->tableExpr->byModelIdTableId(modelId, tableId);
    exprCount = (int)tableExpr.size();

    // calculate total number of values for each accumulator
    totalSize = 1;
    for (const TableDimsRow & dim : tableDims) {
        if (dim.dimSize <= 0) throw DbException("invalid size of dimension %s for table: %s", dim.name.c_str(), i_name);
        totalSize *= dim.dimSize;
    }
    if (totalSize <= 0) throw DbException("invalid size of the table: %s", i_name);
}

// calculate output table size: total number of values for each accumulator
size_t IOutputTableWriter::sizeOf(const MetaRunHolder * i_metaStore, int i_tableId)
{ 
    if (i_metaStore == nullptr) throw DbException("invalid (NULL) model metadata");

    // get dimensions size, including expr dimension
    int mId = i_metaStore->modelRow->modelId;

    const TableDicRow * tblRow = i_metaStore->tableDic->byKey(mId, i_tableId);
    if (tblRow == nullptr) throw DbException("table not found in table dictionary, id: %d", i_tableId);

    int dimCount = tblRow->rank;

    vector<TableDimsRow> tblDimVec = i_metaStore->tableDims->byModelIdTableId(mId, i_tableId);
    if (dimCount < 0 || dimCount != (int)tblDimVec.size()) throw DbException("invalid table rank or dimensions not found for table: %s", tblRow->tableName.c_str());

    // calculate table total size
    long long nTotal = 1;
    for (const TableDimsRow & dim : tblDimVec) {
        if (dim.dimSize <= 0) throw DbException("invalid size of dimension %s for table: %s", dim.name.c_str(), tblRow->tableName.c_str());
        nTotal *= dim.dimSize;
    }
    if (nTotal <= 0 || nTotal > SIZE_MAX) throw DbException("invalid size of the table: %s", tblRow->tableName.c_str());

    return (size_t)nTotal;
}

// write output table accumulator values
void OutputTableWriter::writeAccumulator(IDbExec * i_dbExec, int i_nSubSample, int i_accId, size_t i_size, const double * i_valueArr)
{
    // validate parameters
    if (i_dbExec == nullptr) throw DbException("invalid (NULL) database connection");
    if (i_nSubSample < 0 || i_nSubSample >= numSubSamples) throw DbException("invalid sub-sample index: %d for output table: %s", i_nSubSample, tableRow->tableName.c_str());
    if (i_accId < 0 || i_accId >= accCount) throw DbException("invalid accumulator number: %d for output table: %s", i_accId, tableRow->tableName.c_str());
    if (i_size <= 0 || totalSize != i_size) throw DbException("invalid value array size: %lld for output table: %s", i_size, tableRow->tableName.c_str());

    if (i_valueArr == nullptr) throw DbException("invalid value array: it can not be NULL for output table: %s", tableRow->tableName.c_str());

    // build sql:
    // INSERT INTO salarySex_a201208171604590148
    //   (run_id, acc_id, sub_id, dim0, dim1, acc_value) VALUES (2, 15, 4, ?, ?, ?)
    string sql = "INSERT INTO " + accDbTable + " (run_id, acc_id, sub_id";

    for (const TableDimsRow & dim : tableDims) {
        sql += ", " + dim.name;
    }

    sql += ", acc_value) VALUES (" + to_string(runId) + ", " + to_string(i_accId) + ", " + to_string(i_nSubSample);

    // build sql, append: , ?, ?, ?)
    // as dimensions parameter placeholder(s), value placeholder
    for (int nDim = 0; nDim < dimCount; nDim++) {
        sql += ", ?";
    }
    sql += ", ?)";

    // set parameters type: dimensions and accumulator value
    vector<const type_info *> tv;
    for (int nDim = 0; nDim < dimCount; nDim++) {
        tv.push_back(&typeid(int));
    }
    tv.push_back(&typeid(double));  // set parameters type: accumulator value
    
    // begin update transaction
    unique_lock<recursive_mutex> lck = i_dbExec->beginTransactionThreaded();

    // prepare statement
    {
        exit_guard<IDbExec> onExit(i_dbExec, &IDbExec::releaseStatement);
        i_dbExec->createStatement(sql, (int)tv.size(), tv.data());

        // storage for dimension items and db row values
        unique_ptr<int> cellArrUptr(new int[dimCount]);
        int * cellArr = cellArrUptr.get();

        for (int k = 0; k < dimCount; k++) {
            cellArr[k] = 0;
        }

        int rowSize = dimCount + 1;
        unique_ptr<DbValue> valVecUptr(new DbValue[rowSize]);
        DbValue * valVec = valVecUptr.get();

        // loop through all dimensions and store cell values
        for (size_t cellOffset = 0; cellOffset < i_size; cellOffset++) {

            // set parameter values: dimension items
            for (int k = 0; k < dimCount; k++) {
                valVec[k] = DbValue(cellArr[k]);
            }

            // if table is not "sparse" then store NULL value rows:
            // if no "sparse" flag set for that output table or value is finite and greater than "sparse null"
            if (!isSparseTable ||
                (isfinite(i_valueArr[cellOffset]) && fabs(i_valueArr[cellOffset]) > nullValue)) {

                // set parameter value: accumulator value
                valVec[dimCount] = DbValue(i_valueArr[cellOffset]);

                // insert cell value into output table
                i_dbExec->executeStatement(rowSize, valVec);
            }

            // get next cell indices
            for (int nDim = dimCount - 1; nDim >= 0; nDim--) {
                if (nDim > 0 && cellArr[nDim] >= tableDims[nDim].dimSize - 1) {
                    cellArr[nDim] = 0;
                }
                else {
                    cellArr[nDim]++;
                    break;
                }
            }
            if (cellOffset + 1 < i_size && dimCount > 0 && cellArr[0] >= tableDims[0].dimSize) throw DbException("Invalid value array size");
        }
    }   // done with insert

    // commit: done with subsample
    i_dbExec->commit();
}

// write all output table values: aggregate subsamples using table expressions
void OutputTableWriter::writeAllExpressions(IDbExec * i_dbExec)
{
    if (i_dbExec == nullptr) throw DbException("invalid (NULL) database connection");

    unique_lock<recursive_mutex> lck = i_dbExec->beginTransactionThreaded();

    for (int nExpr = 0; nExpr < exprCount; nExpr++) {
        writeExpression(i_dbExec, nExpr);
    }
    i_dbExec->commit();
}

// write output table value: aggregated output expression value
void OutputTableWriter::writeExpression(IDbExec * i_dbExec, int i_nExpression)
{
    // validate parameters
    if (i_dbExec == nullptr) throw DbException("invalid (NULL) database connection");
    if (i_nExpression < 0 || i_nExpression >= exprCount) throw DbException("invalid expression index: %d for output table: %s", i_nExpression, tableRow->tableName.c_str());

    // check if update is in transaction scope
    if (!i_dbExec->isTransaction()) throw DbException("output table update must be in transaction scope");
    
    // build sql:
    // INSERT INTO salarySex_v201208171604590148 
    //  (run_id, dim0, dim1, expr_id, expr_value)
    // SELECT
    //  F.run_id, F.dim0, F.dim1, 2, F.expr2
    // FROM
    // (
    //  SELECT
    //    M1.run_id, M1.dim0, M1.dim1, SUM(M1.acc0) AS expr2
    //  FROM salarySex_a201208171604590148 M1
    //  GROUP BY M1.run_id, M1.dim0, M1.dim1
    // ) F
    // WHERE F.run_id = 15
    // 
    string sql = "INSERT INTO " + valueDbTable + " (run_id";

    for (const TableDimsRow & dim : tableDims) {
        sql += ", " + dim.name;
    }

    sql += ", expr_id, expr_value) SELECT F.run_id";

    for (const TableDimsRow & dim : tableDims) {
        sql += ", F." + dim.name;
    }
    sql += ", " + to_string(i_nExpression) + ", F." + tableExpr[i_nExpression].name +
        " FROM (" +
        tableExpr[i_nExpression].sqlExpr +
        ") F WHERE F.run_id = " + to_string(runId);

    // do the insert
    i_dbExec->update(sql);
}

// calculate output table values digest and store only single copy of output values
void OutputTableWriter::digestOutput(IDbExec * i_dbExec)
{
    if (i_dbExec == nullptr) throw DbException("invalid (NULL) database connection");

    unique_lock<recursive_mutex> lck = i_dbExec->beginTransactionThreaded();

    // insert run_table dictionary record
    string sHid = to_string(tableRow->tableHid);
    string sRunId = to_string(runId);

    i_dbExec->update(
        "INSERT INTO run_table (run_id, table_hid, base_run_id, run_digest)" \
        " VALUES (" + sRunId + ", " + sHid + ", " + sRunId + ", " + toQuoted(tableRow->digest) + ")"
        );

    // build sql to select accumulators and expressions values:
    //
    // SELECT acc_id, sub_id, dim0, dim1, acc_value
    // FROM salarySex_a201208171604590148
    // WHERE run_id = 11
    // ORDER BY 1, 2, 3, 4
    //
    string accSql = "SELECT acc_id, sub_id, ";

    for (const TableDimsRow & dim : tableDims) {
        accSql += dim.name + ", ";
    }

    accSql += "acc_value" \
        " FROM " + accDbTable +
        " WHERE run_id = " + to_string(runId) +
        " ORDER BY 1, 2";

    for (int nDim = 0; nDim < dimCount; nDim++) {
        accSql += ", " + to_string(nDim + 3);
    }

    // build sql to expressions values:
    //
    // SELECT dim0, dim1, expr_id, expr_value
    // FROM salarySex_v201208171604590148
    // WHERE run_id = 11
    // ORDER BY 1, 2, 3
    //
    string exprSql = "SELECT expr_id, ";

    for (const TableDimsRow & dim : tableDims) {
        exprSql += dim.name + ", ";
    }

    exprSql += "expr_value" \
        " FROM " + valueDbTable +
        " WHERE run_id = " + to_string(runId) +
        " ORDER BY 1";

    for (int nDim = 0; nDim < dimCount; nDim++) {
        exprSql += ", " + to_string(nDim + 2);
    }

    // select accumulator values and calculate digest
    MD5 md5;

    string sLine = tableRow->digest + "\n";
    md5.add(sLine.c_str(), sLine.length()); // start from metadata digest

    ValueRowDigester md5AccRd(dimCount + 2, typeid(double), &md5);   // +2 columns: acc_id, sub_id
    ValueRowAdapter accAdp(dimCount + 2, typeid(double));

    i_dbExec->selectToRowProcessor(accSql, accAdp, md5AccRd);

    // select expression values and append to the digest
    md5.add("exp_value\n", strlen("exp_value\n")); // expression values delimiter

    ValueRowDigester md5ExprRd(dimCount + 1, typeid(double), &md5); // +1 column: expr_id
    ValueRowAdapter exprAdp(dimCount + 1, typeid(double));

    i_dbExec->selectToRowProcessor(exprSql, exprAdp, md5ExprRd);

    string sDigest = md5.getHash();     // digest of metadata and values of accumulators and expressions

    // update digest and base run id
    //
    // UPDATE run_table SET run_digest = '22ee44cc' WHERE run_id = 11 table_hid = 456
    //
    // UPDATE run_table SET 
    //   base_run_id =
    //   (
    //     SELECT MIN(E.run_id) FROM run_table E
    //     WHERE E.table_hid = 456
    //     AND E.run_digest = '22ee44cc'
    //   )
    // WHERE run_id = 11 AND table_hid = 456
    //
    i_dbExec->update(
        "UPDATE run_table SET run_digest = " + toQuoted(sDigest) + 
        " WHERE run_id = " + sRunId + " AND table_hid = " + sHid
        );

    i_dbExec->update(
        "UPDATE run_table SET base_run_id =" \
        " (" \
        " SELECT MIN(E.run_id) FROM run_table E" \
        " WHERE E.table_hid = " + sHid +
        " AND E.run_digest = " + toQuoted(sDigest) +
        " )" \
        " WHERE run_id = " + sRunId + " AND table_hid = " + sHid
        );

    // if same digest already exists then delete current run value rows
    int nBase = i_dbExec->selectToInt(
        "SELECT base_run_id FROM run_table WHERE run_id = " + sRunId + " AND table_hid = " + sHid,
        0);

    if (nBase > 0 && nBase != runId) {
        i_dbExec->update(
            "DELETE FROM " + accDbTable + " WHERE run_id = " + sRunId
            );
        i_dbExec->update(
            "DELETE FROM " + valueDbTable + " WHERE run_id = " + sRunId
            );
    }

    i_dbExec->commit();     // completed
}
