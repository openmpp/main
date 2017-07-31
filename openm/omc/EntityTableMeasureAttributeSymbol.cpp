/**
* @file    EntityTableMeasureAttributeSymbol.cpp
* Definitions for the EntityTableMeasureAttributeSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <cassert>
#include "libopenm/omError.h"
#include "EntityTableMeasureAttributeSymbol.h"
#include "AttributeSymbol.h"
#include "EntityInternalSymbol.h"
#include "EntityTableSymbol.h"
#include "CodeBlock.h"

using namespace std;
using namespace openm;

// static
string EntityTableMeasureAttributeSymbol::symbol_name(const Symbol *table, const Symbol *attribute)
{
    string result;
    result = "om_" + table->name + "_taav_" + attribute->name;
    return result;
}

// static
bool EntityTableMeasureAttributeSymbol::exists(const Symbol *table, const Symbol *attribute)
{
    string unm = symbol_name(table, attribute);
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
            auto av = dynamic_cast<AttributeSymbol *>(attribute);
            if (av == nullptr) {
                throw HelperException(LT("error : %s is not an attribute in table %s"), attribute->name.c_str(), table->name.c_str());
            }
            string member_name = in_member_name();
            auto sym = new EntityInternalSymbol(member_name, av->agent, av->data_type);
            assert(sym);
            // note parent attribute for post-parse type resolution in case data_type is unknown
            sym->parent = av->stable_pp();
			// Push the name into the post parse ignore hash for the current pass.
			pp_ignore_symbol.insert(sym->unique_name);
        }
        if ( need_value_in_event ) {
            // Create symbol for the data member which will hold the 'in' value of the increment (for 'event' tabulation operator)
            auto av = dynamic_cast<AttributeSymbol *>(attribute);
            if (av == nullptr) {
                throw HelperException(LT("error : %s is not an attribute in table %s"), attribute->name.c_str(), table->name.c_str());
            }
            string member_name = in_event_member_name();
            auto sym = new EntityInternalSymbol(member_name, av->agent, av->data_type);
            assert(sym);
            // note parent attribute for post-parse type resolution in case data_type is unknown
            sym->parent = av->stable_pp();
			// Push the name into the post parse ignore hash for the current pass.
			pp_ignore_symbol.insert(sym->unique_name);
        }
        break;
    }
    case eAssignMembers:
    {
        // assign direct pointer to table for post-parse use
        pp_table = dynamic_cast<EntityTableSymbol *> (pp_symbol(table));
        assert(pp_table); // parser guarantee

        // assign direct pointer to attribute for post-parse use
        pp_attribute = dynamic_cast<AttributeSymbol *> (pp_symbol(attribute));
        assert(pp_attribute); // parser guarantee

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
    result = "om_" + table->name + "_in_" + attribute->name;
    return result;
}

string EntityTableMeasureAttributeSymbol::in_event_member_name() const
{
    string result;
    result = "om_" + table->name + "_in_event_" + attribute->name;
    return result;
}

