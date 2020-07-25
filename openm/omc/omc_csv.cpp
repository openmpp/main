/**
 * @file    omc_csv.cpp
 * omc functions to read parameters data from .csv or .tsv files.
*/
// Copyright (c) 2013-2016 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "omc_file.h"
#include "omc_csv.h"

using namespace std;
using namespace openm;

// parameter dimension
struct PDim {
    string name;                        // dimension name
    const EnumerationSymbol * eSym;     // dimension symbol
    int size;                           // dimension size (item count)
    int shift;                          // multiplier to convert from dimension item index to cell index of parameter value 
    map <string, int> nameIdxMap;       // map enum names to index in dimension
};

static const vector< pair< int, vector<string> > > parseSimpleCsv(const ParameterSymbol * i_param, const list<string> & i_csvLines, const char * i_separator, string & i_pathCsv);
static const vector< pair< int, vector<string> > > parseDimCsv(const ParameterSymbol * i_param, const list<string> & i_csvLines, const char * i_separator, string & i_pathCsv);
static const vector<PDim> getDims(const ParameterSymbol * i_param);
static const map<string, int> getParamEnums(const ParameterSymbol * i_param);

// read parameters data from .csv or .tsv files from fixed directory and parameter directory
extern void omc::readParameterCsvFiles(
    bool i_isFixed,
    const string & i_srcDir,
    forward_list<unique_ptr<Constant> > & io_cpLst)
{
    // to filter csv files make map of parameter name in lower case to parameter symbol
    map<string, ParameterSymbol *> paramNameMap;
    for (const auto param : Symbol::pp_all_parameters) {
        string nameLc = param->name;
        toLower(nameLc);
        paramNameMap[nameLc] = param;
    }

    // get list of parameterName.csv files
    list<string> csv_files = listSourceFiles(i_srcDir, { ".csv", ".tsv", ".value.csv", ".value.tsv" });
    int nFiles = 0;

    // read csv file if file name is parameterName.csv, ignore case of parameterName
    // each line of csv file can contain multiple columns separated by comma or tab
    // any csv value can be "quoted by double quote", quoutes are removed
    for (string & pathCsv : csv_files) {

        // get parameter name from file path
        // parameter values separator: comma .csv or tab .tsv
        // file content: dimensions and values or parameter values only
        string nameLc = getFileNameStem(pathCsv);
        toLower(nameLc);
        const char * separator = equalNoCase(getFileNameExt(pathCsv).c_str(), ".csv") ? "," : "\t";

        bool isValueOnly = endWithNoCase(nameLc, ".value");
        if (isValueOnly) nameLc = getFileNameStem(nameLc);

        // find parameter name in lower case
        auto paramIt = paramNameMap.find(nameLc);
        if (paramIt == paramNameMap.end()) continue;    // skip: it is not a parameter

        ParameterSymbol * param = paramIt->second;      // this is a parameter to initilize from csv

        if (param->source == ParameterSymbol::derived_parameter) {
            theLog->logFormatted("warning : ignoring initializer for derived parameter %s", param->name.c_str());
            continue;   // skip derived parameters
        }
        // if parameter is a missing then it can can be initialized as either scenario or fixed parameter by reading csv file
        // it is an error to change parameter from fixed to scenario (or other way around: scenario to fixed) using csv file
        // if parameter already initialized as fixed parameter then csv file must be in fixed directory
        if (param->source == ParameterSymbol::scenario_parameter) {
            if (i_isFixed) {
                theLog->logFormatted("error : scenario parameter %s cannot be re-initialized as fixed parameter by csv file: %s", param->name.c_str(), pathCsv.c_str());
                Symbol::post_parse_errors++;
                continue;   // skip this csv file
            }
            else {
                theLog->logFormatted("re-initialize scenario parameter %s from: %s", param->name.c_str(), pathCsv.c_str());
            }
        }
        // if parameter already initialized as scenario parameter then csv file must be in parameter directory
        if (param->source == ParameterSymbol::fixed_parameter) {
            if (!i_isFixed) {
                theLog->logFormatted("error : fixed parameter %s cannot be re-initialized as scenario parameter by csv file: %s", param->name.c_str(), pathCsv.c_str());
                Symbol::post_parse_errors++;
                continue;   // skip this csv file
            }
            else {
                theLog->logFormatted("re-initialize fixed parameter %s from: %s", param->name.c_str(), pathCsv.c_str());
            }
        }

        // read csv file, it must not be empty
        list<string> csvLines = fileToUtf8Lines(pathCsv.c_str(), Symbol::code_page.c_str());
        if (csvLines.size() <= 0) {
            theLog->logFormatted("error : invalid (empty) parameter file : %s", pathCsv.c_str());
            Symbol::post_parse_errors++;
            continue;   // skip this csv file
        }
        Symbol::all_source_files.push_back(pathCsv);    // add csv file path to list of source files
        nFiles++;

        // re-initialize parameter: clear previous values from .dat file
        param->source = i_isFixed ? ParameterSymbol::fixed_parameter : ParameterSymbol::scenario_parameter;
        param->sub_initial_list.resize(1);
        param->sub_initial_list[0].first = param->default_sub_id;
        param->sub_initial_list[0].second.clear();
        param->post_parse_mark_enumerations();  // mark all dimensions and dimension enum items as dependency of parameter

        // create list of string literal parameter values from csv lines
        vector< pair< int, vector<string> > > subValArr;
        if (isValueOnly) {
            subValArr = parseSimpleCsv(param, csvLines, separator, pathCsv);
        }
        else {
            subValArr = parseDimCsv(param, csvLines, separator, pathCsv);
        }
        if (subValArr.size() <= 0) {
            theLog->logFormatted("error : invalid or empty parameter file : %s", pathCsv.c_str());
            Symbol::post_parse_errors++;
            continue;     // skip invalid parameter csv
        }

        param->default_sub_id = subValArr[0].first; // default parameter sub value is a first sub value from csv file

        // convert string literal values to parameter type constants and append to initializer list
        TypeSymbol * pt = param->pp_datatype;

        for (size_t nSub = 0; nSub < subValArr.size(); nSub++) {

            // clear first sub-value (default sub-value) or append additional sub-values
            if (nSub < param->sub_initial_list.size()) {
                param->sub_initial_list[nSub].first = subValArr[nSub].first;
                param->sub_initial_list[nSub].second.clear();
            }
            else {
                param->sub_initial_list.push_back({ subValArr[nSub].first, list<Constant *>() });
            }

            // make parameter value constants from string literals
            for (const string & val : subValArr[nSub].second) {

                // if it is empty value then use default initial value
                string v = !val.empty() ? val : pt->default_initial_value();

                // validate value literal and make constant for parameter initializer list
                Constant * c = pt->make_constant(v);
                if (c == nullptr) {
                    Symbol::pp_error(
                        yy::location(yy::position(&pathCsv)),
                        LT("error : '") + v + LT("' is not a valid '") + pt->name + LT("' in initializer for parameter '") + param->name + LT("'"));

                    param->sub_initial_list[nSub].second.clear();   // parameter value invalid
                    break;
                }

                io_cpLst.push_front(unique_ptr<Constant>(c));
                param->sub_initial_list[nSub].second.push_back(c);
            }
        }
    }

    if (nFiles && i_isFixed) theLog->logFormatted("Processed %d fixed parameter .csv file(s) from: %s", nFiles, i_srcDir.c_str());
    if (nFiles && !i_isFixed) theLog->logFormatted("Processed %d scenario parameter .csv file(s) from: %s", nFiles, i_srcDir.c_str());
}

// parse "simple" parameter csv file: simple file contains only parameter values, no dimensions
const vector< pair< int, vector<string> > > parseSimpleCsv(const ParameterSymbol * i_param, const list<string> & i_csvLines, const char * i_separator, string & i_pathCsv)
{
    // if parameter type is partition then get a map from database enum name to c++ enum id
    TypeSymbol * pt = i_param->pp_datatype;
    bool isPartition = false;
    map<string, int> enumValueMap;

    if (auto pe = dynamic_cast<const PartitionSymbol *>(pt); pe) {
        isPartition = true;
        enumValueMap = getParamEnums(i_param);
    }

    // append csv string values to output list
    // skip empty lines from csv file and skip empty values after parameter size
    size_t nParamSize = i_param->size();
    vector<string> valArr;
    valArr.reserve(nParamSize);

    unsigned int nLine = 0;
    for (const string & line : i_csvLines) {
        nLine++;

        list<string> cols = splitCsv(line, i_separator, true, '"'); // split csv columns, separated by comma or tab, "unquoute" column values

        // append string literal values to initializer list
        unsigned int nCol = 0;
        for (const string & val : cols) {
            nCol++;

            // if this a line is past parameter size then all values must be empty
            // empty lines at the end of csv can produce excessive empty values, which must be ignored
            if (valArr.size() >= nParamSize) {
                if (val.empty()) {
                    continue;   // skip all empty values after end of parameter
                }
                // else error: non-empty value after end of parameter
                Symbol::pp_error(
                    yy::location(yy::position(&i_pathCsv, nLine, nCol)),
                    LT("error : initializer for parameter '") + i_param->name + LT("' has size ") + to_string(valArr.size() + 1) + LT(", exceeds ") + to_string(nParamSize));

                return vector< pair< int, vector<string> > >(); // return error: invalid parameter value
            }

            if (!isPartition || val.empty() || equalNoCase(val.c_str(), "NULL")) {
                valArr.emplace_back(val);
            }
            else {  // parameter value is a partion type, convert from database name to c++ enum id: [-10, 0) => 7

                const auto eIt = enumValueMap.find(val);
                if (eIt == enumValueMap.cend()) {
                    Symbol::pp_error(
                        yy::location(yy::position(&i_pathCsv, nLine, nCol + 1)),
                        LT("error : '") + val + LT("' is not a valid '") + pt->name + LT("' in initializer for parameter '") + i_param->name + LT("'"));

                    return vector< pair< int, vector<string> > >(); // return error: invalid parameter value
                }

                valArr.emplace_back(val) = to_string(eIt->second);  // store enum id as parameter value
            }

        }
    }

    return { { i_param->default_sub_id, valArr } };
}

// parse "dimension" parameter csv file
// each file line contain sub-value id (optional), dimensions enum values and parameter value
// first file line must be a header: sub_id,dimName0,dimName1,param_value
// sub-value id column is optional
// if header starts with "sub_id" then first column is sub-value id 
// else file contains only dimensions and parameter values.
const vector< pair< int, vector<string> > > parseDimCsv(const ParameterSymbol * i_param, const list<string> & i_csvLines, const char * i_separator, string & i_pathCsv)
{
    if (i_csvLines.size() <= 0) {
        theLog->logFormatted("error : invalid (empty) parameter file : %s", i_pathCsv.c_str());
        return vector< pair< int, vector<string> > >();    // return error: parameter file must have header line
    }
    // get dimensions and dimension items
    vector<PDim> dims = getDims(i_param);

    size_t nRank = i_param->rank();
    if (dims.size() != nRank) {
        throw HelperException(LT("error : parameter %s must have %zd dimension(s), but found only %zd"), i_param->name.c_str(), dims.size(), nRank);
    }

    // verify file header
    list<string> hCols = splitCsv(i_csvLines.front(), i_separator, true, '"');  // split csv columns, separated by comma or tab, "unquoute" column values

    bool isSubIdFile = hCols.size() == nRank + 2 && equalNoCase(hCols.front().c_str(), "sub_id");

    size_t colCount = isSubIdFile ? nRank + 2 : nRank + 1;
   if (hCols.size() != colCount) {
        theLog->logFormatted("error : invalid header (first line) in parameter file : %s", i_pathCsv.c_str());
        return vector< pair< int, vector<string> > >();     // return error: parameter header line must have rank + 1 columns
    }

    bool isSkip = isSubIdFile;
    size_t k = 0;
    for (auto const & h : hCols) {
        if (isSkip && k == 0) {
            isSkip = false;
            continue;       // skip first column if it is "sub_id"
        }
        if (k < nRank && !equalNoCase(h.c_str(), dims[k].name.c_str())) {
            theLog->logFormatted("error : invalid header (first line) in parameter file : %s, expected: %s, found: %s", i_pathCsv.c_str(), dims[k].name.c_str(), h.c_str());
            return vector< pair< int, vector<string> > >();     // return error: parameter header line must have rank + 1 columns
        }
        k++;
    }
    if (!equalNoCase(hCols.back().c_str(), "param_value")) {
        theLog->logFormatted("error : invalid header (first line) in parameter file : %s, expected: %s, found: %s", i_pathCsv.c_str(), "param_value", hCols.back().c_str());
        return vector< pair< int, vector<string> > >();     // return error: parameter header line must have rank + 1 columns
    }

    // if parameter type is partition then get a map from database enum name to c++ enum id
    TypeSymbol * pt = i_param->pp_datatype;
    bool isPartition = false;
    map<string, int> enumValueMap;

    if (auto pe = dynamic_cast<const PartitionSymbol *>(pt); pe) {
        isPartition = true;
        enumValueMap = getParamEnums(i_param);
    }

    // append csv string values to output list, skip empty lines from csv file
    size_t nParamSize = i_param->size();
    map<int, int> subIdIndexMap;        // map sub-value id to index in result arrays

    vector< pair< int, vector<string> > > subValArr;    // parameter sub values (parameter cells) for each sub value id
    vector< vector<bool> > useArr;                      // if true then parameter cell value already found in csv

    unsigned int nLine = 0;
    for (const string & line : i_csvLines) {

        if (nLine++ == 0) continue; // skip header line

        list<string> cols = splitCsv(line, i_separator, true, '"'); // split csv columns, separated by comma or tab, "unquoute" column values

        // skip empty lines
        if (cols.size() <= 0 || (cols.size() == 1 && cols.front().empty())) {
            continue;   // skip all empty values after end of parameter
        }

        // each non-empty line in csv file must have rank + 1 or rank + 2 columns
        if (cols.size() != colCount) {
            Symbol::pp_error(
                yy::location(yy::position(&i_pathCsv, nLine)),
                LT("error : line of parameter file '") + i_param->name + LT("' has ") + to_string(cols.size()) + LT(" columns, expected: ") + to_string(colCount));

            return vector< pair< int, vector<string> > >(); // return error: invalid parameter value
        }

        // for each line: check dimension enums and append string literal parameter value to initializer list
        int nCol = 0;
        int nDim = 0;
        int nCell = 0;
        int nSubId = i_param->default_sub_id;
        int nSubIndex = 0;
        for (const string & val : cols) {

            if (isSubIdFile && nCol == 0) {

                if (!IntegerLiteral::is_valid_literal(val.c_str())) {
                    Symbol::pp_error(
                        yy::location(yy::position(&i_pathCsv, nLine, nCol + 1)),
                        LT("error : '") + val + LT("' is not a valid sub value id '") + val.c_str() + LT("' in initializer for parameter '") + i_param->name + LT("'"));

                        return vector< pair< int, vector<string> > >(); // return error: sub value id must be integer
                }

                long lv = stol(val);
                if (lv < INT_MIN || lv > INT_MAX) {
                    Symbol::pp_error(
                        yy::location(yy::position(&i_pathCsv, nLine, nCol + 1)),
                        LT("error : '") + val + LT("' is not a valid sub value id '") + val.c_str() + LT("' in initializer for parameter '") + i_param->name + LT("'"));

                    return vector< pair< int, vector<string> > >(); // return error: sub value id out of range
                }

                nSubId = (int)lv;
                nCol++;
                continue;   // done with sub id
            }
            // else not a sub id column, it is dimension enum item or parameter value

            // find sub value index by sub value id
            const auto subIt = subIdIndexMap.find(nSubId);
            if (subIt != subIdIndexMap.cend()) {
                nSubIndex = subIt->second;
            }
            else {  // sub value id not found: add new sub value
                subValArr.emplace_back(pair< int, vector<string> >(nSubId, vector<string>(nParamSize)));
                nSubIndex = (int)subValArr.size() - 1;
                subIdIndexMap[nSubId] = nSubIndex;
                useArr.emplace_back(vector<bool>(nParamSize, false));
            }

            // find csv dimension item in dimension enum item array
            if (nDim < (int)nRank) {

                const auto eIt = dims[nDim].nameIdxMap.find(val);
                if (eIt == dims[nDim].nameIdxMap.cend()) {
                    Symbol::pp_error(
                        yy::location(yy::position(&i_pathCsv, nLine, nCol + 1)),
                        LT("error : '") + val + LT("' is not a valid item of dimension '") + dims[nDim].eSym->name + LT("' in initializer for parameter '") + i_param->name + LT("'"));

                    return vector< pair< int, vector<string> > >(); // return error: invalid dimension item
                }

                nCell += eIt->second * dims[nDim].shift;    // cell index of parameter value
                nDim++;
                nCol++;
                continue;   // done with dimension
            }
            // else parameter value column

            // validate cell index: must be in range of parameter size
            if (nCell < 0 || nCell >= (int)nParamSize) {
                throw HelperException(LT("error : parameter %s cell index invalid: %d"), i_param->name.c_str(), nCell);
            }

            // check if this is duplicate row: if that cell already found by sub value id and dimension(s) items
            if (useArr[nSubIndex][nCell]) {
                Symbol::pp_error(
                    yy::location(yy::position(&i_pathCsv, nLine)),
                    LT("error : duplicate dimension items: '") + line + LT("' in initializer for parameter '") + i_param->name + LT("'"));

                return vector< pair< int, vector<string> > >(); // return error: invalid dimension item
            }

            // convert cell value, if required and store it
            if (!val.empty() && !equalNoCase(val.c_str(), "NULL")) {

                if (!isPartition) {
                    subValArr[nSubIndex].second[nCell] = val;   // store cell value
                }
                else {  // parameter value is a partion type, convert from database name to c++ enum id: [-10, 0) => 7

                    const auto eIt = enumValueMap.find(val);
                    if (eIt == enumValueMap.cend()) {
                        Symbol::pp_error(
                            yy::location(yy::position(&i_pathCsv, nLine, nCol + 1)),
                            LT("error : '") + val + LT("' is not a valid '") + pt->name + LT("' in initializer for parameter '") + i_param->name + LT("'"));

                        return vector< pair< int, vector<string> > >(); // return error: invalid parameter value
                    }
                    subValArr[nSubIndex].second[nCell] = to_string(eIt->second);    // store cell value
                }
            }

            useArr[nSubIndex][nCell] = true;            // mark parameter cell of this sub value as "found" to avoid duplicates
            nCol++;
        }
    }

    return subValArr;
}

// get parameter dimensions and items for each dimension
const vector<PDim> getDims(const ParameterSymbol * i_param)
{
    vector<PDim> dims;
    dims.reserve(i_param->rank());

    // enumeration of each dimension
    //for (const auto et : i_param->pp_enumeration_list) {
    for (const auto dim : i_param->dimension_list) {

        const EnumerationSymbol * et = dim->pp_enumeration;
        PDim d = {
            Symbol::mangle_name(dim->dim_name, dim->index),
            et,
            et->pp_size(),
            1
        };

        // dimension expected to be classification, partition or range
        if (auto ee = dynamic_cast<const EnumerationWithEnumeratorsSymbol *>(et); ee) {     // classification or partition

            const auto eLst = ee->pp_enumerators;
            int k = 0;
            for (const auto e : eLst) {
                string name = e->db_name();
                d.nameIdxMap[name] = k++;
            }
        }
        else {  // range

            auto er = dynamic_cast<const RangeSymbol *>(et);
            if (!er) {
                throw HelperException(LT("error : parameter %s dimension %s has invalid type of EnumerationSymbol"), i_param->name.c_str(), d.eSym->name.c_str());
            }
            for (int n = er->lower_bound, k = 0; n <= er->upper_bound; n++, k++) {
                string name = to_string(n);
                d.nameIdxMap[name] = k;
            }
        }

        dims.emplace_back(d);
    }

    // update index multiplier for each dimension
    for (size_t k = 1; k < dims.size(); k++) {
        for (size_t j = 0; j < k; j++) {
            dims[j].shift *= dims[k].size;
        }
    }

    return dims;
}

// get parameter values if parameter type is enumeration (classification, partition, range, boolean) 
// return map from database name to integer c++ enum value
const map<string, int> getParamEnums(const ParameterSymbol * i_param)
{
    TypeSymbol * pt = i_param->pp_datatype;
    if (!pt->is_enumeration()) return map<string, int>();   // empty result: parameter value is not a one of enumeration types

    map<string, int> eMap;

    if (auto ee = dynamic_cast<const EnumerationWithEnumeratorsSymbol *>(pt); ee) { // classification, partition or boolean

        const auto eLst = ee->pp_enumerators;
        for (const auto e : eLst) {
            eMap[e->db_name()] = e->ordinal;
        }
        return eMap;
    }
    // else must be range type

    auto er = dynamic_cast<const RangeSymbol *>(pt);
    if (!er) throw HelperException(LT("error : parameter %s has invalid type of EnumerationSymbol"), i_param->name.c_str());

    for (int n = er->lower_bound; n <= er->upper_bound; n++) {
        eMap[to_string(n)] = n;
    }
    return eMap;
}
