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
#include "TimeSymbol.h"
#include "UnknownTypeSymbol.h"
#include "CodeBlock.h"
#include "AttributeSymbol.h"
#include "EntityInternalSymbol.h"
#include "BuiltinAttributeSymbol.h"
#include "LinkAttributeSymbol.h"
#include "MaintainedAttributeSymbol.h"
#include "IdentityAttributeSymbol.h"
#include "DerivedAttributeSymbol.h"
#include "SimpleAttributeSymbol.h"
#include "MultilinkAttributeSymbol.h"
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

bool EntityDataMemberSymbol::is_visible_attribute(void) const
{
    auto attr = dynamic_cast<const AttributeSymbol*>(this);
    if (attr) {
        return Symbol::option_all_attributes_visible ? true : !attr->is_generated;
    }
    else {
        return false;
    }
}

bool EntityDataMemberSymbol::is_eligible_microdata(void) const
{
    return pp_eligible_microdata;
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

bool EntityDataMemberSymbol::is_identity_attribute(void) const
{
    return (bool) dynamic_cast<const IdentityAttributeSymbol*>(this);
}

bool EntityDataMemberSymbol::is_derived_attribute(void) const
{
    return (bool) dynamic_cast<const DerivedAttributeSymbol*>(this);
}

bool EntityDataMemberSymbol::is_simple_attribute(void) const
{
    return (bool) dynamic_cast<const SimpleAttributeSymbol*>(this);
}

bool EntityDataMemberSymbol::is_multilink_aggregate_attribute(void) const
{
    return (bool) dynamic_cast<const MultilinkAttributeSymbol*>(this);
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
        // Add this entity data symbol to the entity's list of all such symbols
        pp_entity->pp_data_members.push_back(this);

        // Determine if eligible as microdata.
        pp_eligible_microdata = false;
        if (option_microdata_output) {
            if (is_visible_attribute() && !is_link_attribute()) {
                if (pp_data_type->is_enumeration()) {
                    auto es = dynamic_cast<EnumerationSymbol*>(pp_data_type);
                    assert(es); // compiler guarantee
                    if (es->pp_size() > Symbol::option_microdata_max_enumerators) {
                        Symbol::pp_warning(formatToString(
                            LT("warning - attribute '%s' has %d enumerators making it ineligible as microdata - consider using int.")
                            , name.c_str()
                            , es->pp_size()
                        ));
                    }
                    else {
                        pp_eligible_microdata = true;
                    }
                }
                else {
                    pp_eligible_microdata = true;
                }
            }
        }
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

CodeBlock EntityDataMemberSymbol::cxx_declaration_entity()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_entity();

    // Perform operations specific to this level in the Symbol hierarchy.
    // example:         /// time
    h += "";
    h += doxygen_short(pretty_name());
    return h;
}

CodeBlock EntityDataMemberSymbol::cxx_definition_entity()
{
    // Hook into the hierarchical calling chain
    CodeBlock c = super::cxx_definition_entity();

    // Perform operations specific to this level in the Symbol hierarchy.
 
    return c;
}

// return C++ type name, raise an exception if type is a string type
const string EntityDataMemberSymbol::cxx_type_of(void) const
{
    string typ;

    if (pp_data_type->is_time()) {
        // For Time, the type is wrapped
        auto ts = dynamic_cast<TimeSymbol *>(pp_data_type);
        assert(ts); // grammar guarantee
        typ = Symbol::token_to_string(ts->time_type);
    }
    else if (pp_data_type->is_numeric_or_bool()) {
        // For fundamental types (and bool), the name of the symbol is the name of the type
        typ = pp_data_type->name;
    }
    else if (pp_data_type->is_string()) {
        // For the string type, the type is "std::string"
        throw HelperException(LT("error : string is not supported as a type for entity data member %s"), name.c_str());
    }
    else {
        // for attributes of type classification, range, or partition
        // get the underlying storage type
        auto ens = dynamic_cast<EnumerationSymbol *>(pp_data_type);
        assert(ens); // grammar guarantee
        typ = Symbol::token_to_string(ens->storage_type);
    }
    return typ;
}
