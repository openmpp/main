// OpenM++ data library: parameter table writer
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "dbParameter.h"
using namespace openm;

namespace openm
{
    // input parameter writer implementation
    class ParameterWriter : public IParameterWriter
    {
    public:
        ParameterWriter(
            int i_modelId,
            int i_setId,
            const char * i_name,
            IDbExec * i_dbExec,
            const MetaRunHolder * i_metaStore,
            const MetaSetHolder * i_metaSet
            );

        // Parameter writer cleanup
        ~ParameterWriter(void) throw() { }

        // return total number of values
        long long sizeOf(void) const throw() { return totalSize; }

        // write parameter values
        void writeParameter(IDbExec * i_dbExec, const type_info & i_type, long long i_size, void * i_valueArr);

    private:
        int setId;                  // destination workset id
        int paramId;                // parameter id
        int dimCount;               // number of dimensions
        long long totalSize;        // total number of values in the table
        vector<int> dimSizeVec;     // parameter dimensions
        string insertValueSql;      // sql to insert parameter value
        string deleteValueSql;      // sql to delete parameter value

    private:
        ParameterWriter(const ParameterWriter & i_writer);              // = delete;
        ParameterWriter & operator=(const ParameterWriter & i_writer);  // = delete;
    };
}

// Parameter writer cleanup
IParameterWriter::~IParameterWriter(void) throw() { }

// Parameter writer factory: create new writer
IParameterWriter * IParameterWriter::create(
    int i_modelId,
    int i_setId,
    const char * i_name,
    IDbExec * i_dbExec,
    const MetaRunHolder * i_metaStore,
    const MetaSetHolder * i_metaSet
    )
{
    return new ParameterWriter(i_modelId, i_setId, i_name, i_dbExec, i_metaStore, i_metaSet);
}

// New parameter writer
ParameterWriter::ParameterWriter(
    int i_modelId,
    int i_setId,
    const char * i_name,
    IDbExec * i_dbExec,
    const MetaRunHolder * i_metaStore,
    const MetaSetHolder * i_metaSet
    ) :
    setId(i_setId),
    paramId(0),
    dimCount(0),
    totalSize(0)
{
    // check parameters
    if (i_dbExec == NULL) throw DbException("invalid (NULL) database connection");
    if (i_metaStore == NULL) throw DbException("invalid (NULL) model metadata");
    if (i_metaSet == NULL) throw DbException("invalid (NULL) workset parameters metadata");
    if (i_name == NULL || i_name[0] == '\0') throw DbException("Invalid (empty) input parameter name");

    // find model, parameter and parameter type in metadata tables
    const ModelDicRow * mdRow = i_metaStore->modelDic->byKey(i_modelId);
    if (mdRow == NULL) throw DbException("model not found in the database, id: %d", i_modelId);

    const ParamDicRow * paramRow = i_metaStore->paramDic->byModelIdName(i_modelId, i_name);
    if (paramRow == NULL) throw DbException("parameter not found in parameters dictionary: %s", i_name);

    paramId = paramRow->paramId;
    dimCount = paramRow->rank;

    // check if workset belong to model and parameter is part of the workset
    if (i_metaSet->worksetRow.setId != setId) throw DbException("invalid workset id: %d, expected: %d", i_metaSet->worksetRow.setId, setId);
    if (i_metaSet->worksetRow.modelId != i_modelId) throw DbException("workset %d does not belong to model: %s", setId, mdRow->name.c_str());
    if (i_metaSet->worksetRow.isReadonly) throw DbException("workset %d is read-only", setId);

    vector<WorksetParamRow>::const_iterator wsParamIt = WorksetParamRow::byKey(setId, paramId, i_metaSet->worksetParam);
    if (wsParamIt == i_metaSet->worksetParam.cend()) throw DbException("workset %d does not contain parameter %s", setId, i_name);

    // get dimensions list
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

        dimSizeVec.push_back(dimSize);
    }

    if (totalSize <= 0) throw DbException("invalid size of the parameter: %s", i_name);

    // make sql to insert parameter value
    // INSERT INTO modelone_201208171604590148_w0_ageSex (set_id, dim0, dim1, value) VALUES (2, ?, ?, ?)
    string tblName = mdRow->modelPrefix + mdRow->setPrefix + paramRow->dbNameSuffix;

    insertValueSql = "INSERT INTO " + tblName + " (set_id";

    for (const ParamDimsRow & dim : dimVec) {
        insertValueSql += ", " + dim.name;
    }

    insertValueSql += ", value) VALUES (" + to_string(setId);

    for (int nDim = 0; nDim < dimCount; nDim++) {
        insertValueSql += ", ?";
    }

    insertValueSql += ", ?)";

    // make sql to delete parameter value
    deleteValueSql = "DELETE FROM " + tblName + " WHERE set_id = " + to_string(setId);
}

// Parameter value casting
template<typename TValue> void setDbValue(long long i_cellOffset, void * i_valueArr, DbValue & o_dbVal)
{
    TValue val = static_cast<TValue *>(i_valueArr)[i_cellOffset];
    o_dbVal = DbValue(val);
}

// write parameter values
void ParameterWriter::writeParameter(IDbExec * i_dbExec, const type_info & i_type, long long i_size, void * i_valueArr)
{
    // validate parameters
    if (i_dbExec == NULL) throw DbException("invalid (NULL) database connection");
    if (i_size <= 0 || totalSize != i_size) throw DbException("invalid value array size: %lld for parameter, id: %d", i_size, paramId);
    if (i_valueArr == NULL) throw DbException("invalid value array: it can not be NULL for parameter, id: %d", paramId);

    // set parameter columns type: dimensions and value
    vector<const type_info *> typeArr;
    for (int nDim = 0; nDim < dimCount; nDim++) {
        typeArr.push_back(&typeid(int));
    }
    typeArr.push_back(&i_type);         // type of value

    // set parameter column conversion handler
    function<void (long long i_cellOffset, void * i_valueArr, DbValue & o_dbVal)> doSetValue = nullptr;

    if (i_type == typeid(char)) doSetValue = setDbValue<char>;
    if (i_type == typeid(unsigned char)) doSetValue = setDbValue<unsigned char>;
    if (i_type == typeid(short)) doSetValue = setDbValue<short>;
    if (i_type == typeid(unsigned short)) doSetValue = setDbValue<unsigned short>;
    if (i_type == typeid(int)) doSetValue = setDbValue<int>;
    if (i_type == typeid(unsigned int)) doSetValue = setDbValue<unsigned int>;
    if (i_type == typeid(long)) doSetValue = setDbValue<long>;
    if (i_type == typeid(unsigned long)) doSetValue = setDbValue<unsigned long>;
    if (i_type == typeid(long long)) doSetValue = setDbValue<long long>;
    if (i_type == typeid(unsigned long long)) doSetValue = setDbValue<unsigned long long>;
    if (i_type == typeid(int8_t)) doSetValue = setDbValue<int8_t>;
    if (i_type == typeid(uint8_t)) doSetValue = setDbValue<uint8_t>;
    if (i_type == typeid(int16_t)) doSetValue = setDbValue<int16_t>;
    if (i_type == typeid(uint16_t)) doSetValue = setDbValue<uint16_t>;
    if (i_type == typeid(int32_t)) doSetValue = setDbValue<int32_t>;
    if (i_type == typeid(uint32_t)) doSetValue = setDbValue<uint32_t>;
    if (i_type == typeid(int64_t)) doSetValue = setDbValue<int64_t>;
    if (i_type == typeid(uint64_t)) doSetValue = setDbValue<uint64_t>;
    if (i_type == typeid(bool)) doSetValue = setDbValue<bool>;
    if (i_type == typeid(float)) doSetValue = setDbValue<float>;
    if (i_type == typeid(double)) doSetValue = setDbValue<double>;
    if (i_type == typeid(long double)) doSetValue = setDbValue<long double>;
    if (i_type == typeid(char *)) doSetValue = setDbValue<char *>;

    if (doSetValue == NULL) throw DbException("invalid type to use as input parameter");    // conversion to target parameter type is not supported

    // check if update is in transaction scope
    if (!i_dbExec->isTransaction()) throw DbException("workset update must be in transaction scope");

    // delete existing parameter values
    i_dbExec->update(deleteValueSql);

    // do insert values
    {
        // prepare insert statement
        exit_guard<IDbExec> onExit(i_dbExec, &IDbExec::releaseStatement);
        i_dbExec->createStatement(insertValueSql, (int)typeArr.size(), typeArr.data());

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
        for (long long cellOffset = 0; cellOffset < i_size; cellOffset++) {

            // set dimension items
            for (int k = 0; k < dimCount; k++) {
                valVec[k] = DbValue(cellArr[k]);
            }

            doSetValue(cellOffset, i_valueArr, valVec[dimCount]);   // set parameter value

            // insert cell value into parameter table
            i_dbExec->executeStatement(rowSize, valVec);

            // get next cell indices
            for (int nDim = dimCount - 1; nDim >= 0; nDim--) {
                if (nDim > 0 && cellArr[nDim] >= dimSizeVec[nDim] - 1) {
                    cellArr[nDim] = 0;
                }
                else {
                    cellArr[nDim]++;
                    break;
                }
            }
            if (cellOffset + 1 < i_size && dimCount > 0 && cellArr[0] >= dimSizeVec[0]) throw DbException("Invalid value array size");
        }
    }
    // done with insert
}
