/**
* @file    EntitySetSymbol.cpp
* Definitions for the EntitySetSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <cassert>
#include "EntitySetSymbol.h"
#include "EntitySymbol.h"
#include "EntityInternalSymbol.h"
#include "EntityFuncSymbol.h"
#include "IdentityAttributeSymbol.h"
#include "RangeSymbol.h"
#include "DimensionSymbol.h"
#include "AttributeSymbol.h"
#include "NumericSymbol.h"
#include "EnumerationSymbol.h"
#include "CodeBlock.h"

using namespace std;

void EntitySetSymbol::create_auxiliary_symbols()
{
    {
        assert(!cell); // initialization guarantee
        // Set storage type to int. Can be changed in a subsequent pass to optimize storage based on array size.
        auto *typ = NumericSymbol::find(token::TK_int);
        assert(typ); // initialization guarantee
        cell = new EntityInternalSymbol("om_" + name + "_cell", agent, typ);
    }

    {
        assert(!update_cell_fn); // initialization guarantee
        update_cell_fn = new EntityFuncSymbol("om_" + name + "_update_cell", agent);
        assert(update_cell_fn); // out of memory check
        update_cell_fn->doc_block = doxygen_short("Update the active cell index of table " + name + " using attributes in the " + agent->name + " entity.");
    }

    {
        assert(!insert_fn); // initialization guarantee
        insert_fn = new EntityFuncSymbol("om_" + name + "_insert", agent);
        assert(insert_fn); // out of memory check
        insert_fn->doc_block = doxygen_short("Insert the entity into the active cell in " + name + ".");
    }

    {
        assert(!erase_fn); // initialization guarantee
        erase_fn = new EntityFuncSymbol("om_" + name + "_erase", agent);
        assert(erase_fn); // out of memory check
        erase_fn->doc_block = doxygen_short("Erase the entity from the active cell in " + name + ".");
    }
}

void EntitySetSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case eAssignMembers:
    {
        // assign direct pointer to agent for use post-parse
        pp_agent = dynamic_cast<EntitySymbol *> (pp_symbol(agent));
        assert(pp_agent); // parser guarantee

        break;
    }
    case ePopulateCollections:
    {
        // add this entity set to the complete list of entity sets
        pp_all_entity_sets.push_back(this);

        // Add this entity set to the agent's list of entity sets
        pp_agent->pp_agent_entity_sets.push_back(this);

        break;
    }
    case ePopulateDependencies:
    {
        // The following block of code is almost identical in EntitySetSymbol and EntityTableSymbol
        // construct function bodies
        build_body_update_cell();
        build_body_insert();
        build_body_erase();

        // Dependency on change in index attributes
        for (auto dim : dimension_list) {
            auto av = dim->pp_attribute;
            CodeBlock& c = av->side_effects_fn->func_body;
            c += injection_description();
            c += "// cell change in " + name;
            c += "if (om_active) {";
            if (filter) {
                c += "if (" + filter->name + ") {";
            }
            c += erase_fn->name + "();";
            c += update_cell_fn->name + "();";
            c += insert_fn->name + "();";
            if (filter) {
                c += "}";
            }
            c += "}";
        }

        // Dependency on filter
        if (filter) {
            CodeBlock& c = filter->side_effects_fn->func_body;
            c += injection_description();
            c += "// filter change in " + name;
            c += "if (om_active) {";
            c += "if (om_new) {";
            c += "// filter changed from false to true";
            c += update_cell_fn->name + "();";
            c += insert_fn->name + "();";
            c += "}";
            c += "else {";
            c += "// filter changed from true to false";
            c += erase_fn->name + "();";
            c += "}";
            c += "}";
        }
        break;
    }
    default:
        break;
    }
}

CodeBlock EntitySetSymbol::cxx_declaration_global()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_global();

    // Perform operations specific to this level in the Symbol hierarchy.
    string dims = "";
    for (auto dim : dimension_list) {
        auto es = dim->pp_enumeration;
        assert(es); // integrity check guarantee
        dims += "[" + to_string(es->pp_size()) + "]";
    }
    h += "extern thread_local EntitySet<" + pp_agent->name + "> * "+ name + dims + ";";
    h += "";

    return h;
}

CodeBlock EntitySetSymbol::cxx_definition_global()
{
    // Hook into the hierarchical calling chain
    CodeBlock c = super::cxx_definition_global();

    // Perform operations specific to this level in the Symbol hierarchy.
    string dims = "";
    for (auto dim : dimension_list) {
        auto es = dim->pp_enumeration;
        assert(es); // integrity check guarantee
        dims += "[" + to_string(es->pp_size()) + "]";
    }
    c += "thread_local EntitySet<" + pp_agent->name + "> * "+ name + dims + ";";
    c += "";

    return c;
}

// The following function definition is identical in EntitySetSymbol and EntityTableSymbol
void EntitySetSymbol::build_body_update_cell()
{
    CodeBlock& c = update_cell_fn->func_body;

    int rank = dimension_list.size();

    if (rank == 0) {
        // short version for rank 0
        c += "// only a single cell if rank 0";
        c += cell->name + " = 0;" ;
        return;
    }

    c += "int cell = 0;" ;
    c += "int index = 0;" ;

    // build an unwound loop of code
    for (auto dim : dimension_list ) {
        auto av = dim->pp_attribute;
        auto es = dim->pp_enumeration;
        // bail if dimension erroneous (error already reported)
        if (!av || !es) {
            break;
        }
        assert(es); // integrity check guarantee
        c += "";
        c += "// dimension=" + to_string(dim->index) + " attribute=" + av->name + " type=" + es->name + " size=" + to_string(es->pp_size());
        if (dim->index > 0) {
            c += "cell *= " + to_string(es->pp_size()) + ";";
        }
        c += "index = " + av->name + ";";
        auto rs = dynamic_cast<RangeSymbol *>(es);
        if (rs) {
            c += "// adjust range to zero-based" ;
            c += "index -= " + to_string(rs->lower_bound) + ";";
        }
        c += "cell += index;";
    }
    c += "";
    c += "assert(cell >= 0 && cell < " + to_string(cell_count()) + "); // logic guarantee";
    c += "";
    c += cell->name + " = cell;" ;
}

void EntitySetSymbol::build_body_insert()
{
    CodeBlock& c = insert_fn->func_body;

    if (dimension_count() == 0) {
        c += name + "->insert(this);" ;
    }
    else {
        c += "int cell = " + cell->name + ";" ;
        c += "EntitySet<" + pp_agent->name + "> ** flattened_array = reinterpret_cast<EntitySet<" + pp_agent->name + "> **>(" + name + ");";
        c += "assert(flattened_array[cell]);";
        c += "flattened_array[cell]->insert(this);";
    }
}

void EntitySetSymbol::build_body_erase()
{
    CodeBlock& c = erase_fn->func_body;

    if (dimension_count() == 0) {
        c += name + "->erase(this);" ;
    }
    else {
        c += "int cell = " + cell->name + ";";
        c += "EntitySet<" + pp_agent->name + "> ** flattened_array = reinterpret_cast<EntitySet<" + pp_agent->name + "> **>(" + name + ");";
        c += "assert(flattened_array[cell]);";
        c += "flattened_array[cell]->erase(this);";
    }
}

// The following function definition is identical in EntitySetSymbol and EntityTableSymbol
size_t EntitySetSymbol::cell_count() const
{
    size_t cells = 1;
    for (auto dim : dimension_list) {
        auto es = dim->pp_enumeration;
        assert(es); // integrity check guarantee
        cells *= es->pp_size();
    }
    return cells;
}
