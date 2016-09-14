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
        ParameterWriter(const char * i_name, IDbExec * i_dbExec, const MetaRunHolder * i_metaStore);

        // Parameter writer cleanup
        virtual ~ParameterWriter(void) throw() { }

    protected:
        int paramId;                        // parameter id
        int dimCount;                       // number of dimensions
        size_t totalSize;                   // total number of values in the table
        vector<int> dimSizeVec;             // size of each parameter dimension
        const ParamDicRow * paramRow;       // parameter metadata row
        const TypeDicRow * paramTypeRow;    // parameter type
        vector<ParamDimsRow> paramDims;     // parameter dimensions

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
            const MetaRunHolder * i_metaStore,
            const MetaSetHolder * i_metaSet
            );

        // Parameter writer cleanup
        ~ParameterSetWriter(void) throw() override { }

        // return total number of values
        virtual size_t sizeOf(void) const throw() override { return totalSize; }

        // write workset parameter values
        void writeParameter(
            IDbExec * i_dbExec, const type_info & i_type, size_t i_size, void * i_valueArr
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
            const MetaRunHolder * i_metaStore,
            const char * i_doubleFormat = "",
            const char * i_longDoubleFormat = ""
        );

        // Parameter writer cleanup
        ~ParameterRunWriter(void) throw() override { }

        // return total number of values
        virtual size_t sizeOf(void) const throw() override { return totalSize; }

        // load parameter values from csv file into run table
        virtual void loadCsvParameter(IDbExec * i_dbExec, const char * i_filePath) override;

        // calculate run parameter values digest and store only single copy of parameter values
        void digestParameter(IDbExec * i_dbExec, const type_info & i_type) override;

    private:
        int runId;              // model run id
        string doubleFmt;       // printf format for float and double
        string longDoubleFmt;   // printf format for long double
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
    const MetaRunHolder * i_metaStore,
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
    const MetaRunHolder * i_metaStore,
    const char * i_doubleFormat,
    const char * i_longDoubleFormat
    )
{
    return new ParameterRunWriter(i_runId, i_name, i_dbExec, i_metaStore, i_doubleFormat, i_longDoubleFormat);
}

// New parameter writer
ParameterWriter::ParameterWriter(
    const char * i_name, IDbExec * i_dbExec, const MetaRunHolder * i_metaStore
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
    int mId = i_metaStore->modelRow->modelId;

    paramRow = i_metaStore->paramDic->byModelIdName(mId, i_name);
    if (paramRow == nullptr) throw DbException("parameter not found in parameters dictionary: %s", i_name);

    paramId = paramRow->paramId;
    dimCount = paramRow->rank;

    // get dimensions list
    paramDims = i_metaStore->paramDims->byModelIdParamId(mId, paramId);
    if (dimCount < 0 || dimCount != (int)paramDims.size()) throw DbException("invalid parameter rank or dimensions not found for parameter: %s", i_name);

    paramTypeRow = i_metaStore->typeDic->byKey(mId, paramRow->typeId);
    if (paramTypeRow == nullptr) throw DbException("type not found for parameter: %s", i_name);

    // get dimensions type and size, calculate total size
    totalSize = 1;
    for (const ParamDimsRow & dim : paramDims) {

        const TypeDicRow * typeRow = i_metaStore->typeDic->byKey(mId, dim.typeId);
        if (typeRow == nullptr) throw DbException("type not found for dimension %s of parameter: %s", dim.name.c_str(), i_name);

        int dimSize = (int)i_metaStore->typeEnumLst->byModelIdTypeId(mId, dim.typeId).size();

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
    const MetaRunHolder * i_metaStore,
    const MetaSetHolder * i_metaSet
    ) :
    ParameterWriter(i_name, i_dbExec, i_metaStore),
    setId(i_setId)
{
    if (i_metaSet == nullptr) throw DbException("invalid (NULL) workset parameters metadata");

    // check if workset belong to model and parameter is part of the workset
    if (i_metaSet->worksetRow.setId != i_setId) throw DbException("invalid workset id: %d, expected: %d", i_metaSet->worksetRow.setId, i_setId);
    if (i_metaSet->worksetRow.modelId != i_metaStore->modelRow->modelId) throw DbException("workset %d does not belong to model %s", i_setId, i_metaStore->modelRow->name.c_str());
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
    const MetaRunHolder * i_metaStore,
    const char * i_doubleFormat,
    const char * i_longDoubleFormat
) :
    ParameterWriter(i_name, i_dbExec, i_metaStore),
    runId(i_runId)
{
    paramRunDbTable = paramRow->dbRunTable;
    doubleFmt = (i_doubleFormat != nullptr) ? i_doubleFormat : "";
    longDoubleFmt = (i_longDoubleFormat != nullptr) ? i_longDoubleFormat : "";
}

// parameter value casting from array cell
template<typename TValue> void setDbValue(size_t i_cellOffset, const void * i_valueArr, DbValue & o_dbVal)
{
    TValue val = static_cast<const TValue *>(i_valueArr)[i_cellOffset];
    o_dbVal = DbValue(val);
}

// write workset parameter values
void ParameterSetWriter::writeParameter(IDbExec * i_dbExec, const type_info & i_type, size_t i_size, void * i_valueArr)
{
    if (i_dbExec == nullptr) throw DbException("invalid (NULL) database connection");

    if (i_size <= 0 || totalSize != i_size) throw DbException("invalid value array size: %zd for parameter, id: %d", i_size, paramId);
    if (i_valueArr == nullptr) throw DbException("invalid value array: it can not be NULL for parameter, id: %d", paramId);

    if (!i_dbExec->isTransaction()) throw DbException("workset update must be in transaction scope");

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
    // INSERT INTO ageSex_w201208171604590148 (set_id, dim0, dim1, param_value) VALUES (2, ?, ?, ?)
    //
    string insSql = "INSERT INTO " + paramSetDbTable + " (set_id";

    for (const ParamDimsRow & dim : paramDims) {
        insSql += ", " + dim.name;
    }

    insSql += ", param_value) VALUES (" + to_string(setId);

    for (int nDim = 0; nDim < dimCount; nDim++) {
        insSql += ", ?";
    }
    insSql += ", ?)";

    // delete existing parameter values
    i_dbExec->update("DELETE FROM " + paramSetDbTable + " WHERE set_id = " + to_string(setId));

    // do insert values
    {
        // prepare insert statement
        exit_guard<IDbExec> onExit(i_dbExec, &IDbExec::releaseStatement);
        i_dbExec->createStatement(insSql, (int)typeArr.size(), typeArr.data());

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

// load parameter values from csv file into run table
void ParameterRunWriter::loadCsvParameter(IDbExec * i_dbExec, const char * i_filePath)
{
    if (i_dbExec == nullptr) throw DbException("invalid (NULL) database connection");
    if (i_filePath == nullptr) throw DbException("invalid (empty) parameter.csv file path for parameter: %d %s", paramId, paramRow->paramName.c_str());
    if (!i_dbExec->isTransaction()) throw DbException("parameter update must be in transaction scope for parameter: %d %s", paramId, paramRow->paramName.c_str());

    // check if special parameter value handling required: quoted for string or bool conversion to 0/1
    bool isBool = paramTypeRow->isBool();
    bool isStr = paramTypeRow->isString();

    // make prefix of sql to insert parameter value:
    //
    // INSERT INTO ageSex_p201208171604590148 (run_id, dim0,dim1,param_value) VALUES (2,
    //
    string nameLst;
    for (const ParamDimsRow & dim : paramDims) {
        nameLst += dim.name + ",";
    }
    nameLst += "param_value";

    string insPrefix = "INSERT INTO " + paramRunDbTable + " (run_id, " + nameLst + ") VALUES (" + to_string(runId) + ", ";

    // read csv file into list of strings
    list<string> csvLines = fileToUtf8Lines(i_filePath);
    if (csvLines.size() <= 0)
        throw DbException("invalid (empty) parameter.csv file path for parameter: %d %s", paramId, paramRow->paramName.c_str());

    // do insert values
    size_t rowCount = 0;
    int rowSize = dimCount + 1;
    string insSql;

    for (const string & line : csvLines) {

        // check header, expected: dim0,dim1,param_value
        if (rowCount == 0) {
            
            if (!startWithNoCase(csvLines.front(), nameLst.c_str()))
                throw DbException("invalid parameter.csv file header, expected: %s for parameter: %d %s", nameLst.c_str(), paramId, paramRow->paramName.c_str());
            
            rowCount++;
            continue;       // done with csv file header
        }

        if (line.empty()) continue;     // skip empty lines

        // split to columns, unquote and check row size
        list<string> csvCols = splitCsv(line, ",", true, '"');
        if (csvCols.size() < (size_t)rowSize)
            throw DbException("invalid parameter.csv file at line %zd, expected: %d columns for parameter: %d %s", rowCount, rowSize, paramId, paramRow->paramName.c_str());

        // make insert sql: append dimensions, it cannot be empty
        insSql = insPrefix;

        list<string>::const_iterator col = csvCols.cbegin();
        for (int k = 0; k < dimCount; k++) {

            if (col->empty())
                throw DbException("invalid parameter.csv file at line %zd dimension is empty at column %d for parameter: %d %s", rowCount, k, paramId, paramRow->paramName.c_str());

            insSql += *col + ", ";
            ++col;
        }

        // make insert sql: append parameter value, it cannot be empty
        if (col->empty())
            throw DbException("invalid parameter.csv file at line %zd, value is empty at column %d for parameter: %d %s", rowCount, dimCount, paramId, paramRow->paramName.c_str());

        if (!isStr && !isBool) {
            insSql += *col + ")";
        }
        if (isStr) {
            insSql += toQuoted(*col) + ")";
        }
        if (isBool) {
            bool isOk = false;
            if (equalNoCase(col->c_str(), "1") || equalNoCase(col->c_str(), "true") ||
                equalNoCase(col->c_str(), "t") || equalNoCase(col->c_str(), "-1")) {
                insSql += "1)";
                isOk = true;
            }
            if (equalNoCase(col->c_str(), "0") || equalNoCase(col->c_str(), "false") || equalNoCase(col->c_str(), "f")) {
                insSql += "0)";
                isOk = true;
            }
            if (!isOk) throw DbException("invalid parameter.csv file at line %zd, invalid logical value at column %d for parameter: %d %s", rowCount, dimCount, paramId, paramRow->paramName.c_str());
        }

        // do insert into parameter value table
        i_dbExec->update(insSql);
        rowCount++;
    }

    // done with insert
    if (rowCount <= 0 || totalSize + 1 != rowCount) throw DbException("invalid parameter.csv size: %zd, expected: %zd for parameter: %d %s ", rowCount, totalSize, paramId, paramRow->paramName.c_str());
}

// calculate parameter values digest and store only single copy of parameter values
void ParameterRunWriter::digestParameter(IDbExec * i_dbExec, const type_info & i_type)
{
    if (i_dbExec == nullptr) throw DbException("invalid (NULL) database connection");
    if (!i_dbExec->isTransaction()) throw DbException("parameter update must be in transaction scope");

    // insert run_parameter dictionary record
    string sHid = to_string(paramRow->paramHid);
    string sRunId = to_string(runId);

    i_dbExec->update(
        "INSERT INTO run_parameter (run_id, parameter_hid, base_run_id, run_digest)" \
        " VALUES (" + sRunId + ", " + sHid + ", " + sRunId + ", " + toQuoted(paramRow->digest) + ")"
        );

    // build sql to select parameter values:
    //
    // SELECT dim0,dim1,param_value FROM ageSex_p20120817 WHERE run_id = 11 ORDER BY 1, 2
    //
    string nameCsv;
    for (const ParamDimsRow & dim : paramDims) {
        nameCsv += dim.name + ",";
    }
    nameCsv += "param_value";

    string sql = "SELECT " + nameCsv +" FROM " + paramRunDbTable + " WHERE run_id = " + sRunId;

    if (dimCount > 0) {
        sql += " ORDER BY ";
        for (int nDim = 0; nDim < dimCount; nDim++) {
            sql += ((nDim > 0) ? ", " : "") + to_string(nDim + 1);
        }
    }

    // select parameter values and calculate digest
    MD5 md5;

    md5.add("parameter_name,parameter_digest\n", sizeof("parameter_name,parameter_digest\n"));  // start from metadata digest
    string sLine = paramRow->paramName + "," + paramRow->digest + "\n";
    md5.add(sLine.c_str(), sLine.length());

    sLine = nameCsv + "\n";
    md5.add(sLine.c_str(), sLine.length()); // append values header

    ValueRowDigester md5Rd(dimCount, i_type, &md5, doubleFmt.c_str(), longDoubleFmt.c_str());
    ValueRowAdapter adp(dimCount, i_type);

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

