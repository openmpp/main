/**
* @file    omc_markup.cpp
*
* Function to markup model code file
*
*/
// Copyright (c) 2024-2024 OpenM++ Contributors (see AUTHORS.txt)
// This code is licensed under the MIT license (see LICENSE.txt)

#include <iostream>
#include <fstream>
#include <regex>
#include <chrono>
#include <thread>

//#include "omc_file.h"
#include "omc_markup.h"

#include "Symbol.h"
#include "ParameterSymbol.h"

void do_markup(const string& om_developer_cpp_path)
{
    if (!Symbol::option_index_errors) {
        // no markup needed
        return;
    }

    theLog->logMsg("Model code markup - start");

    /// patterns and replacements
    std::vector<std::pair<std::regex, std::string>> patterns;
    {
        // populate patterns and replacements

        // Tests using RiskPaths
        //patterns.push_back({ std::regex("AgeBaselinePreg1\\["), "AgeBaselinePreg1[0+" });
        //patterns.push_back({ std::regex("AgeBaselinePreg1\\[([a-z_]+)\\]"), "AgeBaselinePreg1[0+$1]" });
        //patterns.push_back({ std::regex("(AgeBaselinePreg1)\\[([^\\]]+)\\]"), "$1[($2)]" });
        //patterns.push_back({ std::regex("(\\bAgeBaselinePreg1\\b)\\[([^\\]]+)\\]"), "$1[($2)]" });
        //patterns.push_back({ std::regex("(\\bAgeBaselinePreg1\\b\\s*)\\[([^\\]]+)\\]"), "$1[($2)]" });
        //patterns.push_back({ std::regex("(\\bAgeBaselinePreg1\\b)(\\s*)\\[([^\\]]+)\\]"), "$1$2[om_check_index($3,99,0,\"$1\")]" });
        for (auto p : Symbol::pp_all_parameters) {
            if (p->rank() == 1) {  // TODO extend to multiple dimensions
                // ex: (\bAgeBaselinePreg1\b)(\s*)\[([^\[\]]+)\]
                /// Regex matching a symbol name followed by indices (one for each dimension)
                string srch;
                srch += "(\\b" + p->name + "\\b)";  // group 1: the symbol name. \b is 0-length positional identifying begin or end boundary of a 'word'
                srch += "(\\s*)";                   // group 2: optional white space following the symbol name
                srch += "\\[";                      // left bracket
                srch += "([^\\[\\]]+)";             // group 3: everything inside brackets (except [, to ignore nested constructs)
                srch += "\\]";                      // right bracket
                // ex: $1$2[om_check_index($3,99,0,"$1")]
                /// The replacement string for regex
                string repl;
                /// The 0-based dimension number of this index
                size_t dim = 0;
                /// The size of this dimension
                size_t size = p->pp_shape.front();
                repl += "$1$2[om_check_index($3, " + to_string(size) + ", " + to_string(dim) + ", \"$1\", __FILE__, __LINE__)]";
                patterns.push_back({ std::regex(srch), repl });
            }
        }
    }

    // wait 1 second for previous close of om_developer_cpp to settle
    this_thread::sleep_for(chrono::milliseconds(1000));

    // TODO: use u8string for code_in and code_out when available on all target platforms.

    /// Original model code
    string code_in;

    /// Modified model code
    string code_out;

    {
        // Slurp om_developer.cpp to string code_in
        // Note that om_developer_cpp is a UTF-8 file, so no conversion is needed.
        ifstream ifs(om_developer_cpp_path, ios::in | ios::binary);
        std::ostringstream sstr;
        sstr << ifs.rdbuf();
        ifs.close();
        code_in = sstr.str();
    }

    // create code_out from code_in
    code_out = code_in;
    for (auto& it : patterns) {
        // apply all patterns to model code
        auto srch = it.first;
        auto repl = it.second;
        code_out = std::regex_replace(code_out, srch, repl);
    }

    {
        // Dump string code_out to om_developer.cpp
        // wait 1 second for close of om_developer_cpp to settle
        this_thread::sleep_for(chrono::milliseconds(1000));
        ofstream ofs(om_developer_cpp_path, ios::out | ios::trunc | ios::binary);
        ofs << code_out;
        ofs.close();
    }

    theLog->logMsg("Model code markup - finish");
}