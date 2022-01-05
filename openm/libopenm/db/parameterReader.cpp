// OpenM++ data library: input parameter reader
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "dbParameter.h"

using namespace openm;

namespace openm
{
    // input parameter reader implementation
    class ParameterReader : public IParameterReader
    {
    public:
        ParameterReader(
            int i_runId,
            const char * i_name, 
            IDbExec * i_dbExec, 
            const MetaHolder * i_metaStore
            );

        // input parameter reader cleanup
        ~ParameterReader(void) noexcept { }

        // return input parameter id
        int parameterId(void) const noexcept override { return paramId; }

        // return input parameter size: total number of values in the table
        size_t sizeOf(void) const noexcept override { return totalSize; }

        // read input parameter single sub value
        void readParameter(
            IDbExec * i_dbExec, int i_subId, const type_info & i_type, size_t i_size, void * io_valueArr
            ) override;

        // read all sub values of input parameter
        void readParameter(
            IDbExec * i_dbExec, const type_info & i_type, int i_subCount, size_t i_size, void * io_valueArr
        ) override;

        // read input parameter single selected sub values
        void readParameter(
            IDbExec * i_dbExec, const vector<int> & i_subIdArr, const type_info & i_type, size_t i_size, void * io_valueArr
        ) override;

    private:
        int runId;                      // source run id
        int paramId;                    // parameter id
        int dimCount;                   // number of dimensions
        size_t totalSize;               // total number of values in the table
        string paramDbTable;            // db table name for run values of parameter
        const ParamDicRow * paramRow;   // parameter metadata row
        vector<ParamDimsRow> paramDims; // parameter dimensions

    private:
        ParameterReader(const ParameterReader & i_reader) = delete;
        ParameterReader & operator=(const ParameterReader & i_reader) = delete;
    };
}

// input parameter reader cleanup
IParameterReader::~IParameterReader(void) noexcept { }

// input parameter reader factory: create new reader
IParameterReader * IParameterReader::create(
     int i_runId, const char * i_name,  IDbExec * i_dbExec, const MetaHolder * i_metaStore
    )
{
    return new ParameterReader(i_runId, i_name, i_dbExec, i_metaStore);
}

// new input parameter reader
ParameterReader::ParameterReader(
    int i_runId, 
    const char * i_name,  
    IDbExec * i_dbExec, 
    const MetaHolder * i_metaStore
    ) :
    runId(i_runId),
    paramId(0),
    dimCount(0),
    totalSize(0)
{ 
    // check parameters
    if (i_dbExec == nullptr) throw DbException("invalid (NULL) database connection");
    if (i_metaStore == nullptr) throw DbException("invalid (NULL) model metadata");
    if (i_name == nullptr || i_name[0] == '\0') throw DbException("Invalid (empty) input parameter name");

    // get parameter, parameter type and dimensions list
    int mId = i_metaStore->modelRow->modelId;

    paramRow = i_metaStore->paramDic->byModelIdName(mId, i_name);
    if (paramRow == nullptr) throw DbException("parameter not found in parameters dictionary: %s", i_name);

    paramId = paramRow->paramId;
    dimCount = paramRow->rank;
    paramDbTable = paramRow->dbRunTable;

    paramDims = i_metaStore->paramDims->byModelIdParamId(mId, paramId);
    if (dimCount < 0 || dimCount != (int)paramDims.size()) throw DbException("invalid parameter rank or dimensions not found for parameter: %s", i_name);

    totalSize = i_metaStore->parameterSize(*paramRow);  // parameter size: product of dimensions size
}

// read input parameter single sub value
void ParameterReader::readParameter(IDbExec * i_dbExec, int i_subId, const type_info & i_type, size_t i_size, void * io_valueArr)
{
    // validate parameters
    if (i_dbExec == nullptr) throw DbException("invalid (NULL) database connection");
    if (i_size <= 0 || totalSize != i_size) throw DbException("invalid value array size: %zd for parameter id: %d", i_size, paramId);
    if (io_valueArr == nullptr) throw DbException("invalid value array: it can not be NULL for parameter, id: %d", paramId);

    const vector<int> subIdArr = { i_subId };
    readParameter(i_dbExec, subIdArr, i_type, i_size, io_valueArr);
}

// read all sub values of input parameter
void ParameterReader::readParameter(IDbExec * i_dbExec, const type_info & i_type, int i_subCount, size_t i_size, void * io_valueArr)
{
    // validate parameters
    if (i_dbExec == nullptr) throw DbException("invalid (NULL) database connection");
    if (i_size <= 0 || totalSize != i_size) throw DbException("invalid value array size: %zd for parameter id: %d", i_size, paramId);
    if (io_valueArr == nullptr) throw DbException("invalid value array: it can not be NULL for parameter, id: %d", paramId);

    // validate sub_id: must be between 0 and sub_count
    int nSub = i_dbExec->selectToInt(
        "SELECT sub_count FROM run_parameter WHERE run_id = " + to_string(runId) + " AND parameter_hid = " + to_string(paramRow->paramHid),
        -1);
    if (i_subCount != nSub) throw DbException("invalid sub value count: %d, expected: %d for parameter id: %d", i_subCount, nSub, paramId);

    const vector<int> subIdArr;
    readParameter(i_dbExec, subIdArr, i_type, i_size, io_valueArr);
}

// read input parameter single selected sub values or all sub values if i_subIdArr is empty
void ParameterReader::readParameter(IDbExec * i_dbExec, const vector<int> & i_subIdArr, const type_info & i_type, size_t i_size, void * io_valueArr)
{
    // validate parameters
    if (i_dbExec == nullptr) throw DbException("invalid (NULL) database connection");
    if (i_size <= 0 || totalSize != i_size) throw DbException("invalid value array size: %zd for parameter id: %d", i_size, paramId);
    if (io_valueArr == nullptr) throw DbException("invalid value array: it can not be NULL for parameter, id: %d", paramId);

    // validate sub_id: must be between 0 and sub_count
    int nDstSubCount = (int)i_subIdArr.size();

    int nSub = i_dbExec->selectToInt(
        "SELECT sub_count FROM run_parameter WHERE run_id = " + to_string(runId) + " AND parameter_hid = " + to_string(paramRow->paramHid),
        -1);
    if (nDstSubCount > 0) {
        for (int nId : i_subIdArr) {
            if (nId < 0 || nId >= nSub) throw DbException("invalid sub value id: %d for parameter id: %d", nId, paramId);
        }
    }

    // make sql to select parameter value
    //
    // SELECT sub_id, dim0, dim1, param_value 
    // FROM ageSex_p201208171604590148 
    // WHERE run_id = 
    // (
    //   SELECT R.base_run_id FROM run_parameter R WHERE R.run_id = 4 AND R.parameter_hid = 654
    // )
    // AND sub_id IN (2, 4, 6, 8)
    // ORDER BY 1, 2, 3
    //
    string sColLst;
    string sOrder = "1";

    for (int k = 0; k < dimCount; k++) {
        sColLst += paramDims[k].columnName() + ", ";
        sOrder += ", " + to_string(k + 2);
    }

    string sIdLst;
    for (int k = 0; k < nDstSubCount; k++) {
        sIdLst += (k > 0 ? ", " : "") + to_string(i_subIdArr[k]);
    }

    string sql = "SELECT sub_id, " + sColLst + " param_value" +
        " FROM " + paramDbTable +
        " WHERE run_id = " \
        " (" \
        " SELECT R.base_run_id FROM run_parameter R" \
        " WHERE R.run_id = " + to_string(runId) + " AND R.parameter_hid = " + to_string(paramRow->paramHid) +
        " )" +
        (nDstSubCount > 0 ? " AND sub_id IN (" + sIdLst + ")" : "") +
        " ORDER BY " + sOrder;

    // select parameter and return value column
    i_dbExec->selectColumn(sql, 1 + dimCount, i_type, (nDstSubCount > 0 ? nDstSubCount * i_size : nSub * i_size), io_valueArr);
}
