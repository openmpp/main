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
            pp_type_symbol = dynamic_cast<TypeSymbol *> (pp_symbol(type_symbol));
            assert(pp_type_symbol); // parser guarantee

            // Add this agent data symbol to the agent's list of all such symbols
            pp_agent->pp_agent_data_members.push_back(this);
        }
        break;
    default:
        break;
    }
}

string AgentDataMemberSymbol::initial_value() const
{
    assert(pp_type_symbol);

    // return the default initial value for a type of this kind
    return pp_type_symbol->default_initial_value();
}

CodeBlock AgentDataMemberSymbol::cxx_initialize_expression() const
{
    // example:              time.initialize(0);\n
    CodeBlock c;
    c += name + ".initialize( " + initial_value() + " );";
    return c;
}

CodeBlock AgentDataMemberSymbol::cxx_declaration_agent()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_agent();

    // Perform operations specific to this level in the Symbol hierarchy.
    h += "";
    // example:         //* time */
    h += doxygen(name);
    return h;
}

CodeBlock AgentDataMemberSymbol::cxx_definition_agent()
{
    // Hook into the hierarchical calling chain
    CodeBlock c = super::cxx_definition_agent();

    // Perform operations specific to this level in the Symbol hierarchy.
    c += "";
    // example:         //* time */
    c += doxygen(name);
    return c;
}

