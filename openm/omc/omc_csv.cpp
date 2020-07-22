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

// dimension item
struct EItem {
    int id;         // enum id
    string name;    // enum name
};

// parameter dimension
struct Dim {
    string name;                        // dimension name
    const EnumerationSymbol * eSym;     // dimension symbol
    int size;                           // dimension size (item count)
    int shift;                          // multiplier to convert from dimension item index to cell index of parameter value 
    vector <EItem> enums;               // dimension items
    map <string, int> nameIdxMap;       // map enum names to index in dimension
};

static vector<Dim> getDims(const ParameterSymbol * i_param);
static const vector<string> parseSimpleCsv(const ParameterSymbol * i_param, const list<string> & i_csvLines, bool i_isCsvFile, string & i_pathCsv);
static const vector<string> parseDimCsv(const ParameterSymbol * i_param, const list<string> & i_csvLines, bool i_isCsvFile, string & i_pathCsv);

// read parameters data from .csv or .tsv files from fixed directory and parameter directory
extern void omc::readParameterCsvFiles(
    bool i_isFixed,
    const string & i_srcDir,
    forward_list<unique_ptr<Constant> > & io_cpLst
)
{
    // to filter csv files make map of parameter name in lower case to parameter symbol
    map<string, ParameterSymbol *> paramNameMap;
    for (const auto param : Symbol::pp_all_parameters) {
        string nameLc = param->name;
        toLower(nameLc);
        paramNameMap[nameLc] = param;
    }

    // get list of parameterName.csv files
    list<string> csv_files = listSourceFiles(i_srcDir, { ".csv", ".tsv", ".dim.csv", ".dim.tsv" });
    int nFiles = 0;

    // read csv files if file name is parameterName.csv, ignore case of parameterName
    // each line of csv file can contain multiple columns separated by comma or tab
    // any csv value can be "quoted by double quote", quoutes are removed
    for (string & pathCsv : csv_files) {

        // get parameter name from file path
        // get file type: CVS or TSV, with dimensions or simple file with parameter data only
        string nameLc = getFileNameStem(pathCsv);
        toLower(nameLc);
        bool isCsvFile = equalNoCase(getFileNameExt(pathCsv).c_str(), ".csv");

        bool isDimFile = endWithNoCase(nameLc, ".dim");
        if (isDimFile) nameLc = getFileNameStem(nameLc);

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
                continue;   // skip this csv file
            }
            else {
                theLog->logFormatted("scenario parameter %s re-initialized from: %s", param->name.c_str(), pathCsv.c_str());
            }
        }
        // if parameter already initialized as scenario parameter then csv file must be in parameter directory
        if (param->source == ParameterSymbol::fixed_parameter) {
            if (!i_isFixed) {
                theLog->logFormatted("error : fixed parameter %s cannot be re-initialized as scenario parameter by csv file: %s", param->name.c_str(), pathCsv.c_str());
                continue;   // skip this csv file
            }
            else {
                theLog->logFormatted("fixed parameter %s re-initialized from: %s", param->name.c_str(), pathCsv.c_str());
            }
        }

        // read csv file, it must not be empty
        list<string> csvLines = fileToUtf8Lines(pathCsv.c_str(), Symbol::code_page.c_str());
        if (csvLines.size() <= 0) {
            theLog->logFormatted("error : invalid (empty) parameter file : %s", pathCsv.c_str());
            continue;   // skip this csv file
        }
        Symbol::all_source_files.push_back(pathCsv);    // add csv file path to list of source files
        nFiles++;

        // re-initialize parameter: clear previous values from .dat file
        param->source = i_isFixed ? ParameterSymbol::fixed_parameter : ParameterSymbol::scenario_parameter;
        param->initializer_list.clear();
        param->post_parse_mark_enumerations();  // mark all dimensions and dimension enum items as dependency of parameter

        // create list of string literal parameter values from csv lines
        vector<string> valLst;
        if (!isDimFile) {
            valLst = parseSimpleCsv(param, csvLines, isCsvFile, pathCsv);
        }
        else {
            valLst = parseDimCsv(param, csvLines, isCsvFile, pathCsv);
        }

        if (valLst.size() <= 0) {
            theLog->logFormatted("error : invalid or empty parameter file : %s", pathCsv.c_str());
            continue;     // skip invalid parameter csv
        }

        // convert string literal values to parameter type constants and append to initializer list
        TypeSymbol * pt = param->pp_datatype;

        unsigned int nLine = 0;
        for (const string & val : valLst) {
            nLine++;

            // if it is empty value then use default initial value
            string v = !val.empty() ? val : pt->default_initial_value();

            // validate value literal and make constant for parameter initializer list
            Constant * c = pt->make_constant(v);
            if (c == nullptr) {
                Symbol::pp_error(
                    yy::location(yy::position(&pathCsv, nLine)),
                    LT("error : '") + v + LT("' is not a valid '") + pt->name + LT("' in initializer for parameter '") + param->name + LT("'"));

                param->initializer_list.clear();    // parameter value invalid
                break;
            }

            io_cpLst.push_front(unique_ptr<Constant>(c));
            param->initializer_list.push_back(c);
        }
    }

    if (i_isFixed && nFiles) theLog->logFormatted("Compiled fixed parameters: %d .csv/.tsv file(s) from: %s", nFiles, i_srcDir.c_str());
    if (!i_isFixed && nFiles) theLog->logFormatted("Compiled scenario parameters: %d .csv/.tsv file(s) from: %s", nFiles, i_srcDir.c_str());
}

// parse "simple" parameter csv file: simple file contains only parameter values, no dimensions
const vector<string> parseSimpleCsv(const ParameterSymbol * i_param, const list<string> & i_csvLines, bool i_isCsvFile, string & i_pathCsv)
{
    // append csv string values to output list
    // skip empty lines from csv file and skip empty values after parameter size
    size_t nParamSize = i_param->size();
    vector<string> valArr;
    valArr.reserve(nParamSize);

    unsigned int nLine = 0;
    for (const string & line : i_csvLines) {
        nLine++;

        list<string> cols = splitCsv(line, (i_isCsvFile ? "," : "\t"), true, '"');   // split csv columns, separated by comma or tab, "unquoute" column values

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

                return vector<string>();    // exit on invalid parameter value
            }

            valArr.emplace_back(val);
        }
    }

    return valArr;
}

// parse "dimension" parameter csv file: each file line contain dimension enum value and parameter value
// first file line must be a header: dimName0,dimName1,param_value
const vector<string> parseDimCsv(const ParameterSymbol * i_param, const list<string> & i_csvLines, bool i_isCsvFile, string & i_pathCsv)
{
    if (i_csvLines.size() <= 0) {
        theLog->logFormatted("error : invalid (empty) parameter file : %s", i_pathCsv.c_str());
        return vector<string>();    // exit: parameter file must have header line
    }

    vector<Dim> dims = getDims(i_param);    // get dimensions and dimension items sorted by enum id's
    size_t nRank = dims.size();

    // verify file header
    list<string> hCols = splitCsv(i_csvLines.front(), (i_isCsvFile ? "," : "\t"), true, '"');   // split csv columns, separated by comma or tab, "unquoute" column values

    if (hCols.size() != nRank + 1) {
        theLog->logFormatted("error : invalid header (first line) in parameter file : %s", i_pathCsv.c_str());
        return vector<string>();    // exit: parameter header line must have rank + 1 columns
    }

    size_t k = 0;
    for (auto const & h : hCols) {
        if (k < nRank && !equalNoCase(h.c_str(), dims[k].name.c_str())) {
            theLog->logFormatted("error : invalid header (first line) in parameter file : %s, expected: %s, found: %s", i_pathCsv.c_str(), dims[k].name.c_str(), h.c_str());
            return vector<string>();    // exit: parameter header line must have rank + 1 columns
        }
        k++;
    }
    if (!equalNoCase(hCols.back().c_str(), "param_value")) {
        theLog->logFormatted("error : invalid header (first line) in parameter file : %s, expected: %s, found: %s", i_pathCsv.c_str(), "param_value", hCols.back().c_str());
        return vector<string>();    // exit: parameter header line must have rank + 1 columns
    }

    // append csv string values to output list, skip empty lines from csv file
    size_t nParamSize = i_param->size();
    vector<string> valArr(nParamSize);          // parameter values (parameter cells)
    vector<bool> useArr(nParamSize, false);     // if true then parameter cell value already found in csv

    unsigned int nLine = 0;
    for (const string & line : i_csvLines) {

        if (nLine++ == 0) continue; // skip header line

        list<string> cols = splitCsv(line, (i_isCsvFile ? "," : "\t"), true, '"');   // split csv columns, separated by comma or tab, "unquoute" column values

        // skip empty lines
        if (cols.size() <= 0 || cols.size() == 1 && cols.front().empty()) {
            continue;   // skip all empty values after end of parameter
        }

        // each non-empty line in csv file must hace rank + 1 columns
        if (cols.size() != nRank + 1) {
            Symbol::pp_error(
                yy::location(yy::position(&i_pathCsv, nLine)),
                LT("error : line in parameter '") + i_param->name + LT("' .csv/.tsv file has ") + to_string(cols.size()) + LT(" columns, expected: ") + to_string(nRank));

            return vector<string>();    // exit on invalid parameter value
        }

        // for each line: check dimension enums and append string literal parameter value to initializer list
        int nCol = 0;
        int nCell = 0;
        for (const string & val : cols) {

            // find csv dimension item in dimension enum item array
            if (nCol < (int)nRank) {

                const auto eIt = dims[nCol].nameIdxMap.find(val);
                if (eIt == dims[nCol].nameIdxMap.cend()) {
                    Symbol::pp_error(
                        yy::location(yy::position(&i_pathCsv, nLine, nCol + 1)),
                        LT("error : '") + val + LT("' is not a valid item of dimension '") + dims[nCol].eSym->name + LT("' in initializer for parameter '") + i_param->name + LT("'"));

                    return vector<string>();    // exit on invalid dimension item
                }

                nCell += eIt->second * dims[nCol].shift;    // cell index of parameter value
            }
            else {  // parameter value column

                // validate cell index: must be in range of parameter size
                if (nCell < 0 || nCell >= (int)nParamSize) {
                    throw HelperException(LT("error : parameter %s cell index invalid: %d"), i_param->name.c_str(), nCell);
                }

                // check if this is duplicate row
                if (useArr[nCell]) {
                    Symbol::pp_error(
                        yy::location(yy::position(&i_pathCsv, nLine)),
                        LT("error : duplicate dimension items: '") + line + LT("' in initializer for parameter '") + i_param->name + LT("'"));

                    return vector<string>();    // exit on invalid dimension item
                }

                valArr[nCell] = val;    // store cell value
                useArr[nCell] = true;
            }

            nCol++;
        }
    }

    return valArr;
}

// get parameter dimensions and items for each dimension
vector<Dim> getDims(const ParameterSymbol * i_param)
{
    vector<Dim> dims;
    dims.reserve(i_param->rank());

    // enumeration of each dimension
    //for (const auto et : i_param->pp_enumeration_list) {
    for (const auto dim : i_param->dimension_list) {

        const EnumerationSymbol * et = dim->pp_enumeration;
        Dim d = {
            Symbol::mangle_name(dim->dim_name, dim->index),
            et,
            et->pp_size(),
            1,
            vector<EItem>(et->pp_size())
        };

        // dimension expected to be classification, partition or range
        if (auto ee = dynamic_cast<const EnumerationWithEnumeratorsSymbol *>(et); ee) {     // classification or partition

            const auto eLst = ee->pp_enumerators;
            int k = 0;
            for (const auto e : eLst) {
                string name = e->db_name();
                d.enums[k] = { e->ordinal, name };
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
                d.enums[k] = { k,  name };
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
