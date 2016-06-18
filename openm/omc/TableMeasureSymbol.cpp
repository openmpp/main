/**
* @file    TableMeasureSymbol.cpp
* Definitions for the TableMeasureSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <cassert>
#include "DerivedTableSymbol.h"
#include "TableMeasureSymbol.h"
#include "libopenm/common/omHelper.h"

using namespace std;
using namespace openm;

// static
string TableMeasureSymbol::symbol_name(const Symbol* table, int index)
{
    assert(table);
    return table->name + ".Expr" + to_string(index);
}

string TableMeasureSymbol::pretty_name() const
{
    // example:     accumulator 0: sum(delta(interval(duration)))
    string result = " measure " + to_string(index) + ": " + measure_name;
    return result;
}

void TableMeasureSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case eAssignMembers:
    {
        // assign direct pointer to table for post-parse use
        pp_table = dynamic_cast<TableSymbol *> (pp_symbol(table));
        assert(pp_table); // parser guarantee

        // Add this measure to the table's list of measures
        pp_table->pp_measures.push_back(this);

        break;
    }
    case eResolveDataTypes:
    {
        // Process additional information in expression label
        // Symbols labels are only complete after pass eAssignMembers,
        // so do this in the next pass, which is eResolveDataTypes

        for (auto & lbl : pp_labels) {

            // process decimals= specifier
            {
                //TODO use regexp when possible
                string srch("decimals=");
                auto p = lbl.find(srch);
                if (p != std::string::npos) {
                    auto q = p + srch.length();
                    if (q < lbl.length()) {
                        string value;
                        for (auto i = q; q < lbl.length(); ++i) {
                            auto ch = lbl[i];
                            if (isdigit(ch)) value += ch;
                            else break;
                        }
                        if (value.length() > 0) {
                            decimals = stoi(value);
                        }
                    }
                    // remove decimals=nnn from label
                    lbl = regexReplace(lbl, "[[:space:]]+decimals=[[:digit:]]+", "");
                }
            }

            // process scale= specifier
            {
                //TODO use regexp when possible
                string srch("scale=");
                auto p = lbl.find(srch);
                if (p != std::string::npos) {
                    //pp_warning("warning : scale= in label of table measure is deprecated");
                    auto q = p + srch.length();
                    if (q < lbl.length()) {
                        string value;
                        for (auto i = q; q < lbl.length(); ++i) {
                            auto ch = lbl[i];
                            // handle leading - sign
                            if (i == q && ch == '-') {
                                value += ch;
                                continue;
                            }
                            if (isdigit(ch)) value += ch;
                            else break;
                        }
                        if (value.length() > 0) {
                            scale = stoi(value);
                        }
                    }
                    // remove scale=nnn from label
                    lbl = regexReplace(lbl, "[[:space:]]+scale=[[:digit:]]+", "");
                }
            }
        }
        
        break;
    }
    default:
        break;
    }
}
