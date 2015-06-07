/**
* @file    EntityTableMeasureAttributeSymbol.cpp
* Definitions for the EntityTableMeasureAttributeSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include "libopenm/common/omHelper.h"
#include "EntityTableMeasureAttributeSymbol.h"
#include "AgentVarSymbol.h"
#include "AgentInternalSymbol.h"
#include "EntityTableSymbol.h"
#include "CodeBlock.h"

using namespace std;
using namespace openm;

// static
string EntityTableMeasureAttributeSymbol::symbol_name(const Symbol *table, const Symbol *agentvar)
{
    string result;
    result = "om_" + table->name + "_taav_" + agentvar->name;
    return result;
}

// static
bool EntityTableMeasureAttributeSymbol::exists(const Symbol *table, const Symbol *agentvar)
{
    string unm = symbol_name(table, agentvar);
    return symbols.count(unm) == 0 ? false : true;
}

void EntityTableMeasureAttributeSymbol::post_parse(int pass)
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
            string member_name = in_member_name();
            auto sym = new AgentInternalSymbol(member_name, av->agent, av->data_type);
        }
        break;
    }
    case eAssignMembers:
    {
        // assign direct pointer to table for post-parse use
        pp_table = dynamic_cast<EntityTableSymbol *> (pp_symbol(table));
        assert(pp_table); // parser guarantee

        // assign direct pointer to agentvar for post-parse use
        pp_agentvar = dynamic_cast<AgentVarSymbol *> (pp_symbol(agentvar));
        assert(pp_agentvar); // parser guarantee

        break;
    }
    case ePopulateCollections:
    {
        // Add this EntityTableMeasureAttributeSymbol to the table's list of agentvars
        pp_table->pp_measure_attributes.push_back(this);
        break;
    }
    default:
        break;
    }
}

string EntityTableMeasureAttributeSymbol::in_member_name() const
{
    string result;
    result = "om_" + table->name + "_in_" + agentvar->name;
    return result;
}

CodeBlock EntityTableMeasureAttributeSymbol::cxx_prepare_increment() const
{
    assert(pp_table);  // only call post-parse
    assert(pp_agentvar);  // only call post-parse
    CodeBlock c;
    // example:              DurationOfLife.value_in.alive = alive;\n
    c += in_member_name() + " = " + pp_agentvar->name + ".get();";
    return c;
}

