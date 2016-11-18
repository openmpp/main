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
        ~ParameterReader(void) throw() { }

        // return input parameter size: total number of values in the table
        size_t sizeOf(void) const throw() { return totalSize; }

        // read input parameter values
        void readParameter(
            IDbExec * i_dbExec, const type_info & i_type, size_t i_size, void * io_valueArr
            );

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
IParameterReader::~IParameterReader(void) throw() { }

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

    const TypeDicRow * paramTypeRow = i_metaStore->typeDic->byKey(mId, paramRow->typeId);
    if (paramTypeRow == nullptr) throw DbException("type not found for parameter: %s", i_name);

    // get dimensions type and size, calculate total size
    totalSize = 1;
    for (const ParamDimsRow & dim : paramDims) {

        const TypeDicRow * typeRow = i_metaStore->typeDic->byKey(mId, dim.typeId);
        if (typeRow == nullptr) throw DbException("type not found for dimension %s of parameter: %s", dim.name.c_str(), i_name);

        int dimSize = (int)i_metaStore->typeEnumLst->byModelIdTypeId(mId, dim.typeId).size();

        if (dimSize > 0) totalSize *= dimSize;  // can be simple type, without enums in enum list
    }

    if (totalSize <= 0) throw DbException("invalid size of the parameter: %s", i_name);
}

// read input parameter values
void ParameterReader::readParameter(IDbExec * i_dbExec, const type_info & i_type, size_t i_size, void * io_valueArr)
{
    // validate parameters
    if (i_dbExec == nullptr) throw DbException("invalid (NULL) database connection");
    if (i_size <= 0 || totalSize != i_size) throw DbException("invalid value array size: %zd for parameter id: %d", i_size, paramId);
    if (io_valueArr == nullptr) throw DbException("invalid value array: it can not be NULL for parameter, id: %d", paramId);

    // make sql to select parameter value
    //
    // SELECT dim0, dim1, param_value 
    // FROM ageSex_p201208171604590148 
    // WHERE run_id = 
    // (
    //   SELECT R.base_run_id FROM run_parameter R WHERE R.run_id = 4 AND R.parameter_hid = 654
    // )
    // ORDER BY 1, 2
    //
    string sDimLst;
    string sOrder;

    for (int k = 0; k < dimCount; k++) {
        sDimLst += paramDims[k].name + ", ";
        sOrder += to_string(k + 1) + ((k < dimCount - 1) ? ", " : "");
    }

    string sql = "SELECT " + sDimLst + " param_value" + 
        " FROM "  + paramDbTable  + 
        " WHERE run_id = " \
        " (" \
        " SELECT R.base_run_id FROM run_parameter R" \
        " WHERE R.run_id = " + to_string(runId) + " AND R.parameter_hid = " + to_string(paramRow->paramHid) +
        " )" +
        ((dimCount > 0) ? " ORDER BY " + sOrder : "");

    // select parameter and return value column
    i_dbExec->selectColumn(sql, dimCount, i_type, i_size, io_valueArr);
}
