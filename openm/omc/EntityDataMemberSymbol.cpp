/**
* @file    EntityDataMemberSymbol.cpp
* Definitions for the EntityDataMemberSymbol class.
*/
// Copyright (c) 2013-2022 OpenM++ Contributors
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <cassert>
#include <string>
#include "EntityDataMemberSymbol.h"
#include "EntitySymbol.h"
#include "TypeSymbol.h"
#include "RangeSymbol.h"
#include "UnknownTypeSymbol.h"
#include "CodeBlock.h"
#include "AttributeSymbol.h"
#include "EntityInternalSymbol.h"
#include "BuiltinAttributeSymbol.h"
#include "LinkAttributeSymbol.h"
#include "MaintainedAttributeSymbol.h"
#include "SimpleAttributeSymbol.h"
#include "EntityArrayMemberSymbol.h"
#include "EntityEventSymbol.h"
#include "EntityIncrementSymbol.h"
#include "EntityForeignMemberSymbol.h"
#include "EntityInternalSymbol.h"
#include "EntityMultilinkSymbol.h"

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

bool EntityDataMemberSymbol::is_attribute(void) const
{
    return (bool) dynamic_cast<const AttributeSymbol*>(this);
}

bool EntityDataMemberSymbol::is_builtin_attribute(void) const
{
    return (bool) dynamic_cast<const BuiltinAttributeSymbol*>(this);
}

bool EntityDataMemberSymbol::is_link_attribute(void) const
{
    return (bool) dynamic_cast<const LinkAttributeSymbol*>(this);
}

bool EntityDataMemberSymbol::is_maintained_attribute(void) const
{
    return (bool) dynamic_cast<const MaintainedAttributeSymbol*>(this);
}

bool EntityDataMemberSymbol::is_simple_attribute(void) const
{
    return (bool) dynamic_cast<const SimpleAttributeSymbol*>(this);
}

bool EntityDataMemberSymbol::is_array(void) const
{
    return (bool) dynamic_cast<const EntityArrayMemberSymbol*>(this);
}

bool EntityDataMemberSymbol::is_event(void) const
{
    return (bool) dynamic_cast<const EntityEventSymbol*>(this);
}

bool EntityDataMemberSymbol::is_increment(void) const
{
    return (bool) dynamic_cast<const EntityIncrementSymbol*>(this);
}

bool EntityDataMemberSymbol::is_foreign(void) const
{
    return (bool) dynamic_cast<const EntityForeignMemberSymbol*>(this);
}

bool EntityDataMemberSymbol::is_internal(void) const
{
    return (bool) dynamic_cast<const EntityInternalSymbol*>(this);
}

bool EntityDataMemberSymbol::is_multilink(void) const
{
    return (bool) dynamic_cast<const EntityMultilinkSymbol*>(this);
}

size_t EntityDataMemberSymbol::layout_group() const
{
    size_t result = 10; // something big for internal members, etc.

    if (name == "entity_id") {
        result = 1;
    }
    else if (name == "time") {
        result = 2;
    }
    else if (name == "age") {
        result = 3;
    }
    else if (name.rfind("om_", 0) == 0) { // name starts with om_
        result = 9; // place generated members in a group at the end
    }
    else if (is_event()) {
        result = 9; // the special self-scheduling event has a funny name starting with zzz
    }
    else {
        result = 4; // attributes declared in model code
    }
    return result;
}

size_t EntityDataMemberSymbol::alignment_size() const
{
    assert(pp_data_type);
    if (pp_data_type->is_bool()) {
        return sizeof(bool); // 1 byte
    }
    else if (pp_data_type->is_numeric()) {
        auto ntyp = dynamic_cast<const NumericSymbol*>(pp_data_type);
        assert(ntyp); // logic guarantee
        auto tok = ntyp->type;
        auto result = Symbol::storage_size(tok);
        return result;
    }
    else if (pp_data_type->is_enumeration()) {
        auto etyp = dynamic_cast<const EnumerationSymbol*>(pp_data_type);
        assert(etyp); // logic guarantee
        auto tok = etyp->storage_type;
        auto result = Symbol::storage_size(tok);
        return result;
    }
    return  8; // largest alignment for everything except C++ types, enumerations etc.
}

void EntityDataMemberSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case eCreateForeignTypes:
    {
        // Identify any unknown type, and create a corresponding global UnknownTypeSymbol
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
            // Create UnknownTypeSymbol for type resolution in subsequent post-parse passes.
            auto ft = new UnknownTypeSymbol(sym->name);
            assert(ft);
			// Push the name into the post parse ignore hash for the current pass.
			pp_symbols_ignore.insert(ft->unique_name);
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
        if (pp_data_type->is_unknown()) {
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
    // Appropriate for members which are self-initializing collections, e.g. std::set.
    if (str.size() > 0) {
        // Insert #line directive so that C++ type conversion warnings refer to model source declaration location
        if (decl_loc.begin.filename) {
            c += (no_line_directives ? "//#line " : "#line ")
                + to_string(decl_loc.begin.line)
                + " \""
                + *(decl_loc.begin.filename)
                + "\"";
        }
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

