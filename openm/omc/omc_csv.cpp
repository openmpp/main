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

// read parameters data from .csv or .tsv files from fixed directory and parameter directory
extern void omc::readParameterCsvFiles(
    bool i_isFixed,
    const string & i_srcDir,
    const map<string, ParameterSymbol *> & i_paramNameMap,
    forward_list<unique_ptr<Constant> > & io_cpLst
)
{
    // get list of parameterName.csv files
    list<string> csv_files = listSourceFiles(i_srcDir, { ".csv", ".tsv" });
    int csvFilesCount = 0;

    // read csv files if file name is parameterName.csv, ignore case of parameterName
    // each line of csv file can contain multiple columns separated by comma or tab
    // any csv value can be "quoted by double quote", quoutes are removed
    for (string & pathCsv : csv_files) {

        // find parameter name, case-insensitive
        string nameLc = getFileNameStem(pathCsv);
        toLower(nameLc);

        auto paramIt = i_paramNameMap.find(nameLc);
        if (paramIt == i_paramNameMap.end()) continue;    // skip: it is not a parameter

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
            theLog->logFormatted("error : invalid(empty) parameter.csv file : % s", pathCsv.c_str());
            continue;   // skip this csv file
        }

        // re-initialize parameter: clear previous values from .dat file
        param->source = i_isFixed ? ParameterSymbol::fixed_parameter : ParameterSymbol::scenario_parameter;
        param->initializer_list.clear();
        TypeSymbol * pt = param->pp_datatype;
        size_t nParamSize = param->size();
        param->post_parse_mark_enumerations();
        Symbol::all_source_files.push_back(pathCsv);
        csvFilesCount++;

        // create initalizer list from csv values
        bool isCsv = equalNoCase(getFileNameExt(pathCsv).c_str(), ".csv");
        unsigned int nLine = 0;
        bool isValid = true;

        for (const string & line : csvLines) {
            nLine++;
            list<string> cols = splitCsv(line, (isCsv ? "," : "\t"), true, '"');   // split csv columns, separated by comma or tab, "unquoute" column values

            // for each value: check parameter value type, create constant and append to initializer list
            unsigned int nCol = 0;
            for (const string & val : cols) {
                nCol++;

                // if this a line is past parameter size then all values must be empty
                // empty lines at the end of csv can produce excessive empty values, which must be ignored
                if (param->initializer_list.size() >= nParamSize) {
                    isValid = val.empty();
                    if (isValid) {
                        continue;   // skip all empty values after end of parameter
                    }
                    else {          // error: non-empty value after end of parameter
                        Symbol::pp_error(
                            yy::location(yy::position(&pathCsv, nLine, nCol)),
                            LT("error : initializer for parameter '") + param->name + LT("' has size ") + to_string(param->initializer_list.size() + 1) + LT(", exceeds ") + to_string(nParamSize));
                        break;
                    }
                }

                // validate value literal and make constant for parameter initializer list
                // if it is empty value then use default initial value
                string v = !val.empty() ? val : pt->default_initial_value();

                Constant * c = pt->make_constant(v);
                isValid = c != nullptr;
                if (!isValid) {
                    Symbol::pp_error(
                        yy::location(yy::position(&pathCsv, nLine, nCol)),
                        LT("error : '") + v + LT("' is not a valid '") + pt->name + LT("' in initializer for parameter '") + param->name + LT("'"));
                    break;
                }

                io_cpLst.push_front(unique_ptr<Constant>(c));
                param->initializer_list.push_back(c);
            }
            if (!isValid) break;    // invalid parameter value
        }
    }

    if (i_isFixed && csvFilesCount) theLog->logFormatted("Compiled fixed parameters: %d .csv file(s) from: %s", csvFilesCount, i_srcDir.c_str());
    if (!i_isFixed && csvFilesCount) theLog->logFormatted("Compiled scenario parameters: %d .csv file(s) from: %s", csvFilesCount, i_srcDir.c_str());
}
