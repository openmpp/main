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

#include "libopenm/omError.h"

#include "Symbol.h"
#include "ParameterSymbol.h"

using namespace std;
using namespace openm;

void do_markup(const string& om_developer_cpp_path, const string& om_definitions_cpp_path)
{
    if (!Symbol::option_index_errors) {
        // no markup needed
        return;
    }

    theLog->logMsg("Model code markup - start");

    /// vector of pattern and replacement pairs
    std::vector<std::pair<std::regex, std::string>> patterns;
    {
        // populate regex pattern and replacement pairs for parameters
        for (auto p : Symbol::pp_all_parameters) {
            if (p->rank() == 0) {
                // no index checking for scalars!
                continue;
            }

            //if (p->name != "UnionDurationBaseline") continue; // testing

            /// Regex matching a symbol name followed by indices (one bracketed index expression for each dimension)
            string srch = "";
            // ex: (\bAgeBaselinePreg1\b)(\s*)\[([^\[\]]+)\]

            /// The replacement string for regex
            string repl = "";
            // ex: $1$2[om_check_index($3,99,0,"$1")]

            srch += "(\\b" + p->name + "\\b)";  // group 1: the symbol name. \b is 0-length positional identifying begin or end boundary of a 'word'
            // ex: (\bAgeBaselinePreg1\b)

            repl += "$1";
            // ex: $1

            int dim = 0;
            int grp_white = 2;
            int grp_index = 3;
            for (auto dim_size : p->pp_shape) {
                srch +=
                    "(\\s*)"        // group $grp_white: optional white space
                    // ex: (\s*)
                    "\\["           // left bracket
                    "([^\\[\\]]+)"  // group $grp_index: everything inside brackets (except [, to not match nested constructs)
                    "\\]"           // right bracket
                    // ex: \[([^\[\]]+)\]
                    ;

                repl += 
                    "$" + to_string(grp_white) +
                    // ex. $2
                    "[om_check_index($" + to_string(grp_index) + "," +
                    // ex. [om_check_index($3,
                    to_string(dim_size) + "," + 
                    // ex. 42,
                    to_string(dim) + ","
                    // ex. 0,
                    "\"$1\","
                    // ex. "$1",
                    "__FILE__,__LINE__)]"
                    // ex. __FILE__, __LINE__)]
                    ;
                // ex. $2[om_check_index($3,42,0,"$1",__FILE__,__LINE__)]
                // $2 is possible whitespace preceding [...]
                // $3 is the index expression
                // 42 is the size of this dimension
                // 0 is the 0-based dimension number
                // $1 is the symbol (array) name
                // advance dimension for next dimenion
                ++dim;
                // advance group counters for next dimension
                grp_white += 2;
                grp_index += 2;
            }
            patterns.push_back({ std::regex(srch), repl });
        }
    }

    // wait 1 second for previous close of om_developer_cpp to settle
    this_thread::sleep_for(chrono::milliseconds(1000));

    // TODO: use u8string for code_in and code_out when available on all target platforms.

    string path = om_developer_cpp_path;
    {
        theLog->logFormatted(" Marking up %s", path.c_str());

        /// Original model code
        string code_in;

        /// Modified model code
        string code_out;

        {
            // Slurp om_developer.cpp to string code_in
            // Note that om_developer_cpp is a UTF-8 file, so no conversion is needed.
            ifstream ifs(path, ios::in | ios::binary);
            if (ifs.fail()) throw HelperException(LT("error : unable to open %s"), path);
            std::ostringstream sstr;
            sstr << ifs.rdbuf();
            ifs.close();
            code_in = sstr.str();
        }

        // create code_out from code_in
        code_out = code_in;
        int counter = 0;
        for (auto& it : patterns) {
            // apply all patterns to model code
            auto srch = it.first;
            auto repl = it.second;
            code_out = std::regex_replace(code_out, srch, repl);
            ++counter;
            if (0 == counter % 100 || counter == 1 || counter == patterns.size()) {
                // progress indicator if 100 or more patterns
                theLog->logFormatted("   Pattern %d of %d", counter, patterns.size());
            }
        }

        {
            // Dump string code_out to om_developer.cpp
            // wait 1 second for close of om_developer_cpp to settle
            this_thread::sleep_for(chrono::milliseconds(1000));
            ofstream ofs(path, ios::out | ios::trunc | ios::binary);
            if (ofs.fail()) throw HelperException(LT("error : unable to open %s"), path);
            ofs << code_out;
            ofs.close();
        }
    }

    theLog->logMsg("Model code markup - finish");
}