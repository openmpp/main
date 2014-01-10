/**
* @file    TableAnalysisAgentVarSymbol.cpp
* Definitions for the TableAnalysisAgentVarSymbol class.
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include "TableAnalysisAgentVarSymbol.h"
#include "AgentVarSymbol.h"
#include "AgentInternalSymbol.h"
#include "TableSymbol.h"
#include "CodeBlock.h"
#include "libopenm/common/omHelper.h"

using namespace std;
using namespace openm;

// static
string TableAnalysisAgentVarSymbol::symbol_name(const Symbol *table, const Symbol *agentvar)
{
    string result;
    result = "om_taav_" + table->name + "_" + agentvar->name;
    return result;
}

// static
bool TableAnalysisAgentVarSymbol::exists(const Symbol *table, const Symbol *agentvar)
{
    string unm = symbol_name(table, agentvar);
    return symbols.count(unm) == 0 ? false : true;
}

void TableAnalysisAgentVarSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case eCreateMissingSymbols:
        {
            if ( need_value_in ) {
                // Create symbol for the data member which will hold the 'in' value of the increment.
                auto av = dynamic_cast<AgentVarSymbol *>(agentvar);
                if (av == nullptr) {
                    throw HelperException("Error: agentvar %s used in table %s but not declared in agent", agentvar->name.c_str(), table->name.c_str());
                }
                string member_name = in_agentvar_name();
                auto sym = new AgentInternalSymbol(member_name, av->agent, av->type);
            }
        }
        break;
    case ePopulateCollections:
        {
            // assign direct pointer to table for post-parse use
            pp_table = dynamic_cast<TableSymbol *> (table);
            assert(pp_table); // parser guarantee

            // assign direct pointer to agentvar for post-parse use
            pp_agentvar = dynamic_cast<AgentVarSymbol *> (agentvar);
            assert(pp_agentvar); // parser guarantee

            // Add this TableAnalysisAgentVarSymbol to the table's list of agentvars
            pp_table->pp_table_agentvars.push_back(this);
        }
        break;
    default:
        break;
    }
}

string TableAnalysisAgentVarSymbol::in_agentvar_name() const
{
    string result;
    result = "om_in_" + table->name + "_" + agentvar->name;
    return result;
}

CodeBlock TableAnalysisAgentVarSymbol::cxx_prepare_increment() const
{
    assert(pp_table);  // only call post-parse
    assert(pp_agentvar);  // only call post-parse
    CodeBlock c;
    // example:              DurationOfLife.value_in.alive = alive;\n
    c += in_agentvar_name() + " = " + pp_agentvar->name + ";";
    return c;
}

