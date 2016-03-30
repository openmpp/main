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
            int i_modelId, 
            int i_runId,
            const char * i_name, 
            IDbExec * i_dbExec, 
            const MetaRunHolder * i_metaStore,
            bool i_isBaseRunIdSelected = false,
            int i_baseRunId = 0
            );

        // input parameter reader cleanup
        ~ParameterReader(void) throw() { }

        // return input parameter size: total number of values in the table
        long long sizeOf(void) const throw() { return totalSize; }

        // read input parameter values
        void readParameter(
            IDbExec * i_dbExec, const type_info & i_type, long long i_size, void * io_valueArr
            );

    private:
        int runId;              // source run id
        int paramId;            // parameter id
        int dimCount;           // number of dimensions
        long long totalSize;    // total number of values in the table
        string selectValueSql;  // sql to select value

    private:
        ParameterReader(const ParameterReader & i_reader) = delete;
        ParameterReader & operator=(const ParameterReader & i_reader) = delete;
    };
}

// input parameter reader cleanup
IParameterReader::~IParameterReader(void) throw() { }

// input parameter reader factory: create new reader
IParameterReader * IParameterReader::create(
    int i_modelId,  int i_runId, const char * i_name,  IDbExec * i_dbExec, const MetaRunHolder * i_metaStore
    )
{
    return new ParameterReader(i_modelId, i_runId, i_name, i_dbExec, i_metaStore);
}

// input parameter reader factory: create new reader using preloaded base run id rows
IParameterReader * IParameterReader::create(
    int i_modelId,  
    int i_runId, 
    const char * i_name,  
    IDbExec * i_dbExec, 
    const MetaRunHolder * i_metaStore, 
    const vector<RunParamRow> & i_runParamVec
    )
{
    // check parameters
    if (i_dbExec == NULL) throw DbException("invalid (NULL) database connection");
    if (i_metaStore == NULL) throw DbException("invalid (NULL) model metadata");
    if (i_name == NULL || i_name[0] == '\0') throw DbException("Invalid (empty) input parameter name");

    // find parameter id
    const ParamDicRow * paramRow = i_metaStore->paramDic->byModelIdName(i_modelId, i_name);
    if (paramRow == NULL) throw DbException("parameter not found in parameters dictionary: %s", i_name);

    // find base run id, if exist
    int baseRunId = RunParamRow::findBaseRunId(i_runId, paramRow->paramId, i_runParamVec);

    // create new parameter reader
    return new ParameterReader(i_modelId, i_runId, i_name, i_dbExec, i_metaStore, true, baseRunId);
}

// new input parameter reader
ParameterReader::ParameterReader(
    int i_modelId,  
    int i_runId, 
    const char * i_name,  
    IDbExec * i_dbExec, 
    const MetaRunHolder * i_metaStore,
    bool i_isBaseRunIdSelected,
    int i_baseRunId
    ) :
    runId(i_runId),
    paramId(0),
    dimCount(0),
    totalSize(0)
{ 
    // check parameters
    if (i_dbExec == NULL) throw DbException("invalid (NULL) database connection");
    if (i_metaStore == NULL) throw DbException("invalid (NULL) model metadata");
    if (i_name == NULL || i_name[0] == '\0') throw DbException("Invalid (empty) input parameter name");

    // find model in metadata tables
    const ModelDicRow * mdRow = i_metaStore->modelDic->byKey(i_modelId);
    if (mdRow == NULL) throw DbException("model not found in the database");

    // get parameter, parameter type and dimensions list
    const ParamDicRow * paramRow = i_metaStore->paramDic->byModelIdName(i_modelId, i_name);
    if (paramRow == NULL) throw DbException("parameter not found in parameters dictionary: %s", i_name);

    paramId = paramRow->paramId;
    dimCount = paramRow->rank;

    vector<ParamDimsRow> dimVec = i_metaStore->paramDims->byModelIdParamId(i_modelId, paramId);
    if (dimCount < 0 || dimCount != (int)dimVec.size()) throw DbException("invalid parameter rank or dimensions not found for parameter: %s", i_name);

    const TypeDicRow * paramTypeRow = i_metaStore->typeDic->byKey(i_modelId, paramRow->typeId);
    if (paramTypeRow == NULL) throw DbException("type not found for parameter: %s", i_name);

    // get dimensions type and size, calculate total size
    totalSize = 1;
    for (const ParamDimsRow & dim : dimVec) {

        const TypeDicRow * typeRow = i_metaStore->typeDic->byKey(i_modelId, dim.typeId);
        if (typeRow == NULL) throw DbException("type not found for dimension %s of parameter: %s", dim.name.c_str(), i_name);

        int dimSize = (int)i_metaStore->typeEnumLst->byModelIdTypeId(i_modelId, dim.typeId).size();

        if (dimSize > 0) totalSize *= dimSize;  // can be simple type, without enums in enum list
    }

    if (totalSize <= 0) throw DbException("invalid size of the parameter: %s", i_name);

    // if parameter from base run then select actual run id
    int actualRunId = 0;
    if (i_isBaseRunIdSelected) {
        actualRunId = i_baseRunId;
    }
    else {
        actualRunId = IRunParamTable::selectBaseRunId(i_dbExec, i_runId, paramId);
    }

    if (actualRunId <= 0) actualRunId = i_runId;    // no base run id: parameter value stored under current run id

    // make sql to select parameter value
    // SELECT dim0, dim1, value FROM model_1_paramName WHERE run_id = 4 ORDER BY 1, 2
    string sDimLst;
    string sOrder;

    for (int k = 0; k < dimCount; k++) {
        sDimLst += dimVec[k].name + ", ";
        sOrder += to_string(k + 1) + ((k < dimCount - 1) ? ", " : "");
    }

    selectValueSql = "SELECT " + sDimLst + " value" + 
        " FROM "  + mdRow->modelPrefix + mdRow->paramPrefix + paramRow->dbNameSuffix + 
        " WHERE run_id = " + to_string(actualRunId) +
        ((dimCount > 0) ? " ORDER BY " + sOrder : "");
}

// read input parameter values
void ParameterReader::readParameter(IDbExec * i_dbExec, const type_info & i_type, long long i_size, void * io_valueArr)
{
    // validate parameters
    if (i_dbExec == NULL) throw DbException("invalid (NULL) database connection");
    if (i_size <= 0 || totalSize != i_size) throw DbException("invalid value array size: %ld for output table id: %d", i_size, paramId);
    if (io_valueArr == NULL) throw DbException("invalid value array: it can not be NULL for parameter, id: %d", paramId);

    // select parameter and return value column
    i_dbExec->selectColumn(selectValueSql, dimCount, i_type,  i_size,  io_valueArr);
}
