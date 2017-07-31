/**
* @file    EntityDataMemberSymbol.cpp
* Definitions for the EntityDataMemberSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <cassert>
#include <string>
#include "EntityDataMemberSymbol.h"
#include "EntitySymbol.h"
#include "TypeSymbol.h"
#include "RangeSymbol.h"
#include "ForeignTypeSymbol.h"
#include "CodeBlock.h"

using namespace std;

void EntityDataMemberSymbol::change_data_type(TypeSymbol *new_type)
{
    assert(new_type);
    if (pp_data_type != new_type) {
        pp_data_type = new_type;
        // maintain global counter of type changes
        ++Symbol::type_changes;
    }
}

void EntityDataMemberSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case eCreateMissingSymbols:
    {
        // Identify any unknown type, and create a corresponding global ForeignTypeSymbol
        // for subsequent type resolution.

        // The logic below is similar to Symbol::pp_symbol(),
        // which fixes up misidentifications of members and globals.
        // See the definition of that function for additional comments.
        assert(data_type);
        Symbol *sym = *&data_type;
        assert(sym);
        if (
            sym->is_base_symbol() // type was never declared
            && symbols.end() == symbols.find(sym->name) // no corresponding global type in the symbol table
            ) {
            // Create ForeignTypeSymbol for type resolution in subsequent post-parse passes.
            auto ut = new ForeignTypeSymbol(sym->name);
            assert(ut);
            // push its name into the pass #1 ignore hash
            pp_ignore_pass1.insert(ut->unique_name);
        }
        break;
    }
    case eAssignMembers:
    {
        // assign direct pointer to type symbol for use post-parse
        pp_data_type = dynamic_cast<TypeSymbol *> (pp_symbol(data_type));
        assert(pp_data_type); // parser guarantee
        // assign direct pointer to parent symbol (if used)
        if (parent) {
            pp_parent = dynamic_cast<EntityDataMemberSymbol *> (pp_symbol(parent));
            assert(pp_parent); // parser guarantee
        }
        break;
    }
    case eResolveDataTypes:
    {
        // Resolve datatype if unknown.
        if (pp_data_type->is_foreign()) {
            // data type of data member is unknown
            if (pp_parent) {
                auto typ = pp_parent->pp_data_type;
                assert(typ);
                // Set the type to the parent's type
                change_data_type(typ);
            }
        }
        break;
    }
    case ePopulateCollections:
    {
        // Add this agent data symbol to the agent's list of all such symbols
        pp_agent->pp_agent_data_members.push_back(this);
        break;
    }
    default:
        break;
    }
}

string EntityDataMemberSymbol::initialization_value(bool type_default) const
{
    assert(pp_data_type);

    // return the default initial value for a type of this kind
    return pp_data_type->default_initial_value();
}

CodeBlock EntityDataMemberSymbol::cxx_initialization_expression(bool type_default) const
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

CodeBlock EntityDataMemberSymbol::cxx_declaration_agent()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_agent();

    // Perform operations specific to this level in the Symbol hierarchy.
    // example:         /// time
    h += "";
    h += doxygen_short(pretty_name());
    return h;
}

CodeBlock EntityDataMemberSymbol::cxx_definition_agent()
{
    // Hook into the hierarchical calling chain
    CodeBlock c = super::cxx_definition_agent();

    // Perform operations specific to this level in the Symbol hierarchy.
 
    return c;
}

