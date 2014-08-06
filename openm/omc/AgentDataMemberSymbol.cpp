/**
* @file    AgentDataMemberSymbol.cpp
* Definitions for the AgentDataMemberSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include <string>
#include "AgentDataMemberSymbol.h"
#include "AgentSymbol.h"
#include "TypeSymbol.h"
#include "RangeSymbol.h"
#include "CodeBlock.h"

using namespace std;

void AgentDataMemberSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case ePopulateCollections:
        {
            // assign direct pointer to type symbol for use post-parse
            pp_data_type = dynamic_cast<TypeSymbol *> (pp_symbol(data_type));
            assert(pp_data_type); // parser guarantee

            // Add this agent data symbol to the agent's list of all such symbols
            pp_agent->pp_agent_data_members.push_back(this);
        }
        break;
    default:
        break;
    }
}

string AgentDataMemberSymbol::initialization_value(bool type_default) const
{
    assert(pp_data_type);

    // return the default initial value for a type of this kind
    return pp_data_type->default_initial_value();
}

CodeBlock AgentDataMemberSymbol::cxx_initialization_expression(bool type_default) const
{
    CodeBlock c;
    string str = initialization_value(type_default);
    // As a special case, skip initialization if initial value is an empty string.
    // Useful for members which are self-initializing collects, e.g. std::set.
    if (str.size() > 0) {
        // example:              time.initialize(0);\n
        c += name + ".initialize( " + initialization_value(type_default) + " );";
    }
    return c;
}

CodeBlock AgentDataMemberSymbol::cxx_declaration_agent()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_agent();

    // Perform operations specific to this level in the Symbol hierarchy.
    // example:         /// time
    h += "";
    h += doxygen_short(pretty_name());
    return h;
}

CodeBlock AgentDataMemberSymbol::cxx_definition_agent()
{
    // Hook into the hierarchical calling chain
    CodeBlock c = super::cxx_definition_agent();

    // Perform operations specific to this level in the Symbol hierarchy.
 
    return c;
}

