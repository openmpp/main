// OpenM++ data library: parameter table writer
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "dbValue.h"
#include "dbParameter.h"

using namespace openm;

namespace openm
{
    // input parameter writer base class
    class ParameterWriter
    {
    public:
        ParameterWriter(const char * i_name, IDbExec * i_dbExec, const MetaHolder * i_metaStore);

        // Parameter writer cleanup
        virtual ~ParameterWriter(void) throw() { }

    protected:
        int modelId;                            // model id in database
        int paramId;                            // parameter id
        int dimCount;                           // number of dimensions
        size_t totalSize;                       // total number of values in the table
        vector<int> dimSizeVec;                 // size of each parameter dimension
        const ParamDicRow * paramRow;           // parameter metadata row
        const TypeDicRow * paramTypeRow;        // parameter type
        vector<TypeEnumLstRow> paramEnums;      // enums for parameter value, if parameter value type is enum based
        vector<ParamDimsRow> paramDims;         // parameter dimensions
        vector<vector<TypeEnumLstRow>> dimEnums;    // enums for each dimension

    private:
        ParameterWriter(const ParameterWriter & i_writer) = delete;
        ParameterWriter & operator=(const ParameterWriter & i_writer) = delete;
    };

    // input parameter writer into workset
    class ParameterSetWriter : public ParameterWriter, public IParameterSetWriter
    {
    public:
        ParameterSetWriter(
            int i_setId,
            const char * i_name,
            IDbExec * i_dbExec,
            const MetaHolder * i_metaStore,
            const MetaSetHolder * i_metaSet
            );

        // Parameter writer cleanup
        ~ParameterSetWriter(void) throw() override { }

        // return total number of values
        virtual size_t sizeOf(void) const throw() override { return totalSize; }

        // write workset parameter values
        void writeParameter(
            IDbExec * i_dbExec, int i_subId, const type_info & i_type, size_t i_size, void * i_valueArr
            ) override;

    private:
        int setId;              // destination workset id
        string paramSetDbTable; // db table name for workset values of parameter

    private:
        ParameterSetWriter(const ParameterSetWriter & i_writer) = delete;
        ParameterSetWriter & operator=(const ParameterSetWriter & i_writer) = delete;
    };

    // input parameter writer for model run
    class ParameterRunWriter : public ParameterWriter, public IParameterRunWriter
    {
    public:
        ParameterRunWriter(
            int i_runId, 
            const char * i_name, 
            IDbExec * i_dbExec, 
            const MetaHolder * i_metaStore,
            const char * i_doubleFormat = ""
        );

        // Parameter writer cleanup
        ~ParameterRunWriter(void) throw() override { }

        // return total number of values
        virtual size_t sizeOf(void) const throw() override { return totalSize; }

        // load parameter values from csv file into run table
        virtual void loadCsvParameter(IDbExec * i_dbExec, int i_paramSubCount, const char * i_filePath, bool i_isIdCsv = false) override;
    
        // calculate run parameter values digest and store only single copy of parameter values
        void digestParameter(IDbExec * i_dbExec, int i_paramSubCount, const type_info & i_type) override;

    private:
        int runId;              // model run id
        string doubleFmt;       // printf format for float and double
        string paramRunDbTable; // db table name for run values of parameter

    private:
        ParameterRunWriter(const ParameterRunWriter & i_writer) = delete;
        ParameterRunWriter & operator=(const ParameterRunWriter & i_writer) = delete;
    };
}

// Parameter writer cleanup
IParameterSetWriter::~IParameterSetWriter(void) throw() { }
IParameterRunWriter::~IParameterRunWriter(void) throw() { }

// Parameter writer factory: create new writer for workset parameter
IParameterSetWriter * IParameterSetWriter::create(
    int i_setId,
    const char * i_name,
    IDbExec * i_dbExec,
    const MetaHolder * i_metaStore,
    const MetaSetHolder * i_metaSet
    )
{
    return new ParameterSetWriter(i_setId, i_name, i_dbExec, i_metaStore, i_metaSet);
}

// Parameter writer factory: create new writer for model run parameter
IParameterRunWriter * IParameterRunWriter::create(
    int i_runId,
    const char * i_name,
    IDbExec * i_dbExec,
    const MetaHolder * i_metaStore,
    const char * i_doubleFormat
    )
{
    return new ParameterRunWriter(i_runId, i_name, i_dbExec, i_metaStore, i_doubleFormat);
}

// New parameter writer
ParameterWriter::ParameterWriter(
    const char * i_name, IDbExec * i_dbExec, const MetaHolder * i_metaStore
    ) :
    paramId(0),
    dimCount(0),
    totalSize(0)
{
    // check parameters
    if (i_dbExec == nullptr) throw DbException("invalid (NULL) database connection");
    if (i_metaStore == nullptr) throw DbException("invalid (NULL) model metadata");
    if (i_name == nullptr || i_name[0] == '\0') throw DbException("Invalid (empty) input parameter name");

    // find model, parameter and parameter type in metadata tables
    modelId = i_metaStore->modelRow->modelId;

    paramRow = i_metaStore->paramDic->byModelIdName(modelId, i_name);
    if (paramRow == nullptr) throw DbException("parameter not found in parameters dictionary: %s", i_name);

    paramId = paramRow->paramId;
    dimCount = paramRow->rank;

    // parameter type and enums for parameter values, if parameter is enum-based
    paramTypeRow = i_metaStore->typeDic->byKey(modelId, paramRow->typeId);
    if (paramTypeRow == nullptr) throw DbException("type not found for parameter: %s", i_name);

    if (!paramTypeRow->isBuiltIn()) paramEnums = i_metaStore->typeEnumLst->byModelIdTypeId(modelId, paramTypeRow->typeId);

    // get dimensions list
    paramDims = i_metaStore->paramDims->byModelIdParamId(modelId, paramId);
    if (dimCount < 0 || dimCount != (int)paramDims.size()) throw DbException("invalid parameter rank or dimensions not found for parameter: %s", i_name);

    // get dimensions type and size, calculate total size
    totalSize = 1;
    for (const ParamDimsRow & dim : paramDims) {

        const TypeDicRow * typeRow = i_metaStore->typeDic->byKey(modelId, dim.typeId);
        if (typeRow == nullptr) throw DbException("type not found for dimension %s of parameter: %s", dim.name.c_str(), i_name);

        vector<TypeEnumLstRow> de = i_metaStore->typeEnumLst->byModelIdTypeId(modelId, dim.typeId);
        int dimSize = (int)de.size();
        dimEnums.push_back(de);

        if (dimSize > 0) totalSize *= dimSize;  // can be simple type, without enums in enum list

        dimSizeVec.push_back(dimSize);
    }

    if (totalSize <= 0) throw DbException("invalid size of the parameter: %s", i_name);
}

// create new writer for workset parameter
ParameterSetWriter::ParameterSetWriter(
    int i_setId,
    const char * i_name,
    IDbExec * i_dbExec,
    const MetaHolder * i_metaStore,
    const MetaSetHolder * i_metaSet
    ) :
    ParameterWriter(i_name, i_dbExec, i_metaStore),
    setId(i_setId)
{
    if (i_metaSet == nullptr) throw DbException("invalid (NULL) workset parameters metadata");

    // check if workset belong to model and parameter is part of the workset
    if (i_metaSet->worksetRow.setId != i_setId) throw DbException("invalid workset id: %d, expected: %d", i_metaSet->worksetRow.setId, i_setId);
    if (i_metaSet->worksetRow.modelId != modelId) throw DbException("workset %d does not belong to model %s", i_setId, i_metaStore->modelRow->name.c_str());
    if (i_metaSet->worksetRow.isReadonly) throw DbException("workset %d is read-only", i_setId);

    vector<WorksetParamRow>::const_iterator wsParamIt = WorksetParamRow::byKey(i_setId, paramId, i_metaSet->worksetParam);
    if (wsParamIt == i_metaSet->worksetParam.cend()) throw DbException("workset %d does not contain parameter %s", i_setId, paramRow->paramName.c_str());

    // name of workset parameter value table
    paramSetDbTable = paramRow->dbSetTable;
}

// create new writer for model run parameter
ParameterRunWriter::ParameterRunWriter(
    int i_runId,
    const char * i_name,
    IDbExec * i_dbExec,
    const MetaHolder * i_metaStore,
    const char * i_doubleFormat
) :
    ParameterWriter(i_name, i_dbExec, i_metaStore),
    runId(i_runId)
{
    paramRunDbTable = paramRow->dbRunTable;
    doubleFmt = (i_doubleFormat != nullptr) ? i_doubleFormat : "";
}

// parameter value casting from array cell
template<typename TValue> void setDbValue(size_t i_cellOffset, const void * i_valueArr, DbValue & o_dbVal)
{
    TValue val = static_cast<const TValue *>(i_valueArr)[i_cellOffset];
    o_dbVal = DbValue(val);
}

// write workset parameter values
void ParameterSetWriter::writeParameter(IDbExec * i_dbExec, int i_subId, const type_info & i_type, size_t i_size, void * i_valueArr)
{
    if (i_dbExec == nullptr) throw DbException("invalid (NULL) database connection");
    if (i_size <= 0 || totalSize != i_size) throw DbException("invalid value array size: %zd for parameter, id: %d", i_size, paramId);
    if (i_valueArr == nullptr) throw DbException("invalid value array: it can not be NULL for parameter, id: %d", paramId);

    if (!i_dbExec->isTransaction()) throw DbException("workset update must be in transaction scope");

    // validate sub_id: must be between 0 and sub_count
    int nSub = i_dbExec->selectToInt(
        "SELECT sub_count FROM workset_parameter WHERE set_id = " + to_string(setId) + " AND parameter_hid = " + to_string(paramRow->paramHid),
        -1);
    if (i_subId < 0 || i_subId >= nSub) throw DbException("invalid sub value id: %d for parameter id: %d", i_subId, paramId);

    // set parameter columns type: dimensions and value
    vector<const type_info *> typeArr;
    for (int nDim = 0; nDim < dimCount; nDim++) {
        typeArr.push_back(&typeid(int));
    }
    typeArr.push_back(&i_type);         // type of value

    // set parameter column conversion handler
    function<void (size_t i_cellOffset, const void * i_valueArr, DbValue & o_dbVal)> doSetValue = nullptr;

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

    if (doSetValue == nullptr) throw DbException("invalid type to use as input parameter");    // conversion to target parameter type is not supported

    // make sql to insert parameter value
    // INSERT INTO ageSex_w201208171604590148 (set_id, sub_id, dim0, dim1, param_value) VALUES (2, 31, ?, ?, ?)
    //
    string insSql = "INSERT INTO " + paramSetDbTable + " (set_id, sub_id";

    for (const ParamDimsRow & dim : paramDims) {
        insSql += ", " + dim.name;
    }

    insSql += ", param_value) VALUES (" + to_string(setId) + ", " + to_string(i_subId);

    for (int nDim = 0; nDim < dimCount; nDim++) {
        insSql += ", ?";
    }
    insSql += ", ?)";

    // delete existing parameter values
    i_dbExec->update(
        "DELETE FROM " + paramSetDbTable + " WHERE set_id = " + to_string(setId) + " AND sub_id = " + to_string(i_subId)
    );

    // do insert values
    {
        // prepare insert statement
        exit_guard<IDbExec> onExit(i_dbExec, &IDbExec::releaseStatement);
        i_dbExec->createStatement(insSql, (int)typeArr.size(), typeArr.data());

        // storage for dimension enum indexes
        unique_ptr<int> cellArrUptr(new int[dimCount]);
        int * cellArr = cellArrUptr.get();

        for (int k = 0; k < dimCount; k++) {
            cellArr[k] = 0;
        }

        // storage for dimension items and db row values
        int rowSize = dimCount + 1;
        unique_ptr<DbValue> valVecUptr(new DbValue[rowSize]);
        DbValue * valVec = valVecUptr.get();

        // loop through all dimensions and store cell values
        for (size_t cellOffset = 0; cellOffset < i_size; cellOffset++) {

            // set sql parameter values: dimension enum id by enum index
            for (int nDim = 0; nDim < dimCount; nDim++) {

                int eId = cellArr[nDim];
                if (dimSizeVec[nDim] > 0) eId = dimEnums[nDim][cellArr[nDim]].enumId;

                valVec[nDim] = DbValue(eId);
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

// load parameter values from csv file into run table
void ParameterRunWriter::loadCsvParameter(IDbExec * i_dbExec, int i_paramSubCount, const char * i_filePath, bool i_isIdCsv)
{
    if (i_dbExec == nullptr) throw DbException("invalid (NULL) database connection");
    if (i_filePath == nullptr) throw DbException("invalid (empty) parameter.csv file path for parameter: %d %s", paramId, paramRow->paramName.c_str());
    if (i_paramSubCount <= 0) throw DbException("invalid sub-value count %d for parameter: %d %s", i_paramSubCount, paramId, paramRow->paramName.c_str());
    if (!i_dbExec->isTransaction()) throw DbException("parameter update must be in transaction scope for parameter: %d %s", paramId, paramRow->paramName.c_str());

    // check if special parameter value handling required: 
    // bool conversion to 0/1 or quoted for string or enum code to id
    bool isBool = paramTypeRow->isBool();
    bool isStr = paramTypeRow->isString();
    bool isEnum = !paramTypeRow->isBuiltIn();
    bool isNullable = paramRow->isExtendable;

    // if csv file contain enum codes then sort type enum by unique key: model id, type id, enum name
    auto enumNameCmp = [](const TypeEnumLstRow & i_left, const TypeEnumLstRow & i_right) -> bool {
        return
            (i_left.modelId < i_right.modelId) ||
            (i_left.modelId == i_right.modelId && i_left.typeId < i_right.typeId) ||
            (i_left.modelId == i_right.modelId && i_left.typeId == i_right.typeId && i_left.name < i_right.name);
    };

    if (!i_isIdCsv) {
        for (auto & eRows : dimEnums) {
            sort(eRows.begin(), eRows.end(), enumNameCmp);
        }
        sort(paramEnums.begin(), paramEnums.end(), enumNameCmp);
    }

    // read csv file into list of strings
    list<string> csvLines = fileToUtf8Lines(i_filePath);
    if (csvLines.size() <= 0)
        throw DbException("invalid (empty) parameter.csv file path for parameter: %d %s", paramId, paramRow->paramName.c_str());

    // pre-allocate csv columns space
    size_t rowCount = 0;
    int rowSize = 1 + dimCount + 1;

    list<string> csvCols;
    for (int k = 0; k < rowSize; k++) {
        string s;
        s.reserve(255);
        csvCols.push_back(std::move(s));
    }

    // make prefix of sql to insert parameter value:
    //
    // INSERT INTO ageSex_p201208171604590148 (run_id, sub_id,dim0,dim1,param_value) VALUES (2,
    //
    string nameLst = "sub_id,";
    for (const ParamDimsRow & dim : paramDims) {
        nameLst += dim.name + ",";
    }
    nameLst += "param_value";

    string insPrefix = "INSERT INTO " + paramRunDbTable + " (run_id, " + nameLst + ") VALUES (" + to_string(runId) + ", ";

    // do insert values
    string insSql;
    const locale csvLocale("");     // user default locale for csv

    for (const string & line : csvLines) {

        // check header, expected: sub_id,dim0,dim1,param_value
        if (rowCount == 0) {
            
            if (!startWithNoCase(csvLines.front(), nameLst.c_str()))
                throw DbException("invalid parameter.csv file header, expected: %s for parameter: %d %s", nameLst.c_str(), paramId, paramRow->paramName.c_str());
            
            rowCount++;
            continue;       // done with csv file header
        }

        if (line.empty()) continue;     // skip empty lines

        // split to columns, unquote and check row size
        splitCsv(line, csvCols, ",", true, '"', csvLocale);
        if (csvCols.size() < (size_t)rowSize)
            throw DbException("invalid parameter.csv file at line %zd, expected: %d columns for parameter: %d %s", rowCount, rowSize, paramId, paramRow->paramName.c_str());

        // make insert sql: append sub_id and dimensions, it cannot be empty
        insSql.clear();
        insSql += insPrefix;

        // append sub_id it must be integer value between 0 and sub count
        list<string>::const_iterator col = csvCols.cbegin();
        if (col->empty())
            throw DbException("invalid parameter.csv file at line %zd, sub-value index is empty at column zero for parameter: %d %s", rowCount, paramId, paramRow->paramName.c_str());

        int nSub = -1;
        try {
            nSub = std::stoi(*col);
        }
        catch (...) {
            nSub = -1;
        }
        if (nSub < 0) throw DbException("invalid sub-value index %s in csv file at line %zd, column zero, parameter: %d %s, value: %s", col->c_str(), rowCount, paramId, paramRow->paramName.c_str(), col->c_str());
        
        if (nSub >= i_paramSubCount) continue;  // skip csv line if csv file has more sub-values than model want to use now

        insSql += *col + ", ";  // append sub_id
        ++col;                  // next column

        // dimension columns as code or enum id or value
        for (int k = 0; k < dimCount; k++) {

            if (col->empty())
                throw DbException("invalid parameter.csv file at line %zd dimension is empty at column %d for parameter: %d %s", rowCount, k + 1, paramId, paramRow->paramName.c_str());

            // if dimension is a simple type (integer) then use column value to insert
            // if csv contains enum id's then validate enum id and use column value to insert
            // else dimension is enum-based and contains enum code: find enum id by code
            if (dimSizeVec[k] <= 0) {
                insSql += *col + ", ";  // simple dimension type: insert csv value
            }
            else { // csv contains code or enum id

                if (i_isIdCsv) {    // if csv contain enum id the validate id and insert csv value

                    int eId = 0;
                    try {
                        eId = std::stoi(*col);
                    }
                    catch (...) {
                        throw DbException("invalid value in csv file at line %zd, column %d, parameter: %d %s, value: %s", rowCount, k + 1, paramId, paramRow->paramName.c_str(), col->c_str());
                    }

                    TypeEnumLstRow eRow(modelId, paramDims[k].typeId, eId);
                    if (!binary_search(dimEnums[k].cbegin(), dimEnums[k].cend(), eRow, TypeEnumLstRow::isKeyLess))
                        throw DbException("invalid value in csv file at line %zd, column %d, parameter: %d %s, value: %s", rowCount, k + 1, paramId, paramRow->paramName.c_str(), col->c_str());

                    insSql += *col + ", ";  // enum id: insert csv value
                }
                else { // csv contains code, find enum id by code

                    TypeEnumLstRow eRow(modelId, paramDims[k].typeId, 0);
                    eRow.name = *col;
                    auto eIt = lower_bound(dimEnums[k].cbegin(), dimEnums[k].cend(), eRow, enumNameCmp);

                    if (eIt == dimEnums[k].cend() || eIt->modelId != modelId || eIt->typeId != paramDims[k].typeId || eIt->name != *col)
                        throw DbException("invalid value in csv file at line %zd, column %d, parameter: %d %s, value: %s", rowCount, k + 1, paramId, paramRow->paramName.c_str(), col->c_str());

                    insSql += to_string(eIt->enumId) + ", ";  // insert enum id
                }
            }

            ++col;  // next column
        }

        // make insert sql: append parameter value, it can be empty or null for extendable parameters
        if (!isNullable && (col->empty() || *col == "null" || *col == "NULL"))
            throw DbException("invalid parameter.csv file at line %zd, value is empty or NULL at column %d for parameter: %d %s", rowCount, dimCount, paramId, paramRow->paramName.c_str());

        if (!isStr && !isBool && !isEnum) {     // default: no conversion required, insert value as is or NULL if value column empty
            insSql += (!col->empty() ? *col : "NULL") + ")";
        }

        if (isStr) {
            insSql += toQuoted(*col) + ")";     // string parameter: use 'sql quotes'
        }

        if (isBool) {           // boolean make 0/1 from false/true
            if (!TypeDicRow::isBoolValid(col->c_str())) throw DbException("invalid parameter.csv file at line %zd, invalid logical value at column %d for parameter: %d %s", rowCount, dimCount, paramId, paramRow->paramName.c_str());

            insSql += TypeDicRow::isBoolTrue(col->c_str()) ? "1)" : "0)";
        }

        if (isEnum) {   // enum-based parameter value, csv contains code or enum id

            if (i_isIdCsv) {    // if csv contain enum id the validate id and insert csv value

                int eId = 0;
                try {
                    eId = std::stoi(*col);
                }
                catch (...) {
                    throw DbException("invalid value in csv file at line %zd, column %d, parameter: %d %s, value: %s", rowCount, dimCount, paramId, paramRow->paramName.c_str(), col->c_str());
                }

                TypeEnumLstRow eRow(modelId, paramTypeRow->typeId, eId);
                if (!binary_search(paramEnums.cbegin(), paramEnums.cend(), eRow, TypeEnumLstRow::isKeyLess))
                    throw DbException("invalid value in csv file at line %zd, column %d, parameter: %d %s, value: %s", rowCount, dimCount, paramId, paramRow->paramName.c_str(), col->c_str());

                insSql += *col + ")";
            }
            else { // csv contains code, find enum id by code

                TypeEnumLstRow eRow(modelId, paramTypeRow->typeId, 0);
                eRow.name = *col;
                auto eIt = lower_bound(paramEnums.cbegin(), paramEnums.cend(), eRow, enumNameCmp);

                if (eIt == paramEnums.cend() || eIt->modelId != modelId || eIt->typeId != paramTypeRow->typeId || eIt->name != *col)
                    throw DbException("invalid value in csv file at line %zd, column %d, parameter: %d %s, value: %s", rowCount, dimCount, paramId, paramRow->paramName.c_str(), col->c_str());

                insSql += to_string(eIt->enumId) + ")";  // insert enum id
            }
        }

        // do insert into parameter value table
        i_dbExec->update(insSql);
        rowCount++;
    }

    // done with insert
    if (rowCount <= 0 || (totalSize * i_paramSubCount) + 1 != rowCount) 
        throw DbException("invalid parameter.csv size: %zd, expected: %zd rows for parameter: %d %s ", rowCount, (totalSize * i_paramSubCount), paramId, paramRow->paramName.c_str());
}

// calculate parameter values digest and store only single copy of parameter values
void ParameterRunWriter::digestParameter(IDbExec * i_dbExec, int i_paramSubCount, const type_info & i_type)
{
    if (i_dbExec == nullptr) throw DbException("invalid (NULL) database connection");
    if (i_paramSubCount <= 0) throw DbException("invalid sub-value count %d for parameter: %d %s", i_paramSubCount, paramId, paramRow->paramName.c_str());
    if (!i_dbExec->isTransaction()) throw DbException("parameter update must be in transaction scope");

    // insert run_parameter dictionary record
    string sHid = to_string(paramRow->paramHid);
    string sRunId = to_string(runId);

    i_dbExec->update(
        "INSERT INTO run_parameter (run_id, parameter_hid, base_run_id, sub_count, run_digest)" \
        " VALUES (" + sRunId + ", " + sHid + ", " + sRunId + ", " + to_string(i_paramSubCount) + ", NULL)"
        );

    // build sql to select parameter values:
    //
    // SELECT sub_id,dim0,dim1,param_value FROM ageSex_p20120817 WHERE run_id = 11 ORDER BY 1, 2, 3
    //
    string nameCsv = "sub_id,";
    for (const ParamDimsRow & dim : paramDims) {
        nameCsv += dim.name + ",";
    }
    nameCsv += "param_value";

    string sql = "SELECT " + nameCsv + " FROM " + paramRunDbTable + " WHERE run_id = " + sRunId;

    sql += " ORDER BY 1";
    for (int nDim = 0; nDim < dimCount; nDim++) {
        sql += ", " + to_string(nDim + 2);
    }

    // select parameter values and calculate digest
    MD5 md5;

    // start from metadata digest
    string sLine = "parameter_name,parameter_digest\n" + paramRow->paramName + "," + paramRow->digest + "\n";
    md5.add(sLine.c_str(), sLine.length());

    // append values header
    sLine = nameCsv + "\n";
    md5.add(sLine.c_str(), sLine.length());

    // append parameter values digest
    // +1 column: sub_id
    ValueRowDigester md5Rd(1 + dimCount, i_type, &md5, doubleFmt.c_str());
    ValueRowAdapter adp(1 + dimCount, i_type);

    i_dbExec->selectToRowProcessor(sql, adp, md5Rd);

    string sDigest = md5.getHash();     // digest of parameter metadata and values

    // update digest and base run id
    //
    // UPDATE run_parameter SET run_digest = '22ee44cc' WHERE run_id = 11 parameter_hid = 456
    //
    // UPDATE run_parameter SET 
    //   base_run_id =
    //   (
    //     SELECT MIN(E.run_id) FROM run_parameter E
    //     WHERE E.parameter_hid = 456
    //     AND E.run_digest = '22ee44cc'
    //   )
    // WHERE run_id = 11 AND parameter_hid = 456
    //
    i_dbExec->update(
        "UPDATE run_parameter SET run_digest = " + toQuoted(sDigest) + 
        " WHERE run_id = " + sRunId + " AND parameter_hid = " + sHid
        );

    i_dbExec->update(
        "UPDATE run_parameter SET base_run_id =" \
        " (" \
        " SELECT MIN(E.run_id) FROM run_parameter E" \
        " WHERE E.parameter_hid = " + sHid +
        " AND E.run_digest = " + toQuoted(sDigest) +
        " )" \
        " WHERE run_id = " + sRunId + " AND parameter_hid = " + sHid
        );

    // if same digest already exists then delete current run value rows
    int nBase = i_dbExec->selectToInt(
        "SELECT base_run_id FROM run_parameter WHERE run_id = " + sRunId + " AND parameter_hid = " + sHid,
        0);

    if (nBase > 0 && nBase != runId) {
        i_dbExec->update("DELETE FROM " + paramRunDbTable + " WHERE run_id = " + sRunId);
    }
}

