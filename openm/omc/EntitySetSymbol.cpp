/**
* @file    EntitySetSymbol.cpp
* Definitions for the EntitySetSymbol class.
*/
// Copyright (c) 2013-2022 OpenM++ Contributors
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <cassert>
#include "EntitySetSymbol.h"
#include "EntitySymbol.h"
#include "EntityInternalSymbol.h"
#include "EntityFuncSymbol.h"
#include "GlobalFuncSymbol.h"
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
        cell = new EntityInternalSymbol("om_" + name + "_cell", entity, typ);
        cell->provenance = name + " (current cell)";
    }

    {
        assert(!update_cell_fn); // initialization guarantee
        update_cell_fn = new EntityFuncSymbol("om_" + name + "_update_cell", entity);
        assert(update_cell_fn); // out of memory check
        update_cell_fn->doc_block = doxygen_short("Update the active cell index of table " + name + " using attributes in the " + entity->name + " entity.");
    }

    {
        assert(!insert_fn); // initialization guarantee
        insert_fn = new EntityFuncSymbol("om_" + name + "_insert", entity);
        assert(insert_fn); // out of memory check
        insert_fn->doc_block = doxygen_short("Insert the entity into the active cell in " + name + ".");
    }

    {
        assert(!erase_fn); // initialization guarantee
        erase_fn = new EntityFuncSymbol("om_" + name + "_erase", entity);
        assert(erase_fn); // out of memory check
        erase_fn->doc_block = doxygen_short("Erase the entity from the active cell in " + name + ".");
    }

    {
        assert(!resource_use_gfn); // initialization guarantee
        resource_use_gfn = new GlobalFuncSymbol("om_" + name + "_resource_use", "auto", "void");
        assert(resource_use_gfn); // out of memory check
        resource_use_gfn->doc_block = doxygen_short("Report resource use of  " + name + ".");
        auto& c = resource_use_gfn->func_body;
        c += "struct result { size_t max_count; size_t total_inserts; };";
        c += "return result { " + max_gvn + ", " + inserts_gvn + " }; ";
    }

    {
        assert(!resource_use_reset_gfn); // initialization guarantee
        resource_use_reset_gfn = new GlobalFuncSymbol("om_" + name + "_resource_use_reset", "void", "void");
        assert(resource_use_reset_gfn); // out of memory check
        resource_use_reset_gfn->doc_block = doxygen_short("Reset resource use for  " + name + ".");
        auto &c = resource_use_reset_gfn->func_body;
        c += count_gvn + " = 0;";
        c += max_gvn + " = 0;";
        c += inserts_gvn + " = 0;";
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
        // assign direct pointer to entity for use post-parse
        pp_entity = dynamic_cast<EntitySymbol *> (pp_symbol(entity));
        assert(pp_entity); // parser guarantee

        // assign direct pointer to order attribute for use post-parse
        if (order) {
            pp_order_attribute = dynamic_cast<AttributeSymbol*> (pp_symbol(order));
            if (!pp_order_attribute) {
                pp_error(LT("error : '") + (*order)->name + LT("' must be an attribute of entity '") + pp_entity->name + "'");
            }
            // Note that the containing entity has an entity set using an order clause
            pp_entity->any_set_has_order_clause = true;
        }

        break;
    }
    case ePopulateCollections:
    {
        // add this entity set to the complete list of entity sets
        pp_all_entity_sets.push_back(this);

        // Add this entity set to the entity's list of entity sets
        pp_entity->pp_sets.push_back(this);

        // Add this entity set to xref of each attribute used as a dimension
        for (auto d : dimension_list) {
            auto a = d->pp_attribute;
            assert(a); // logic guarantee
            a->pp_entity_sets_using.insert(this);
        }
        // Add this entity set to xref of the attribute of its filter
        if (filter) {
            filter->pp_entity_sets_using.insert(this);
        }
        // Add this entity set to xref of the attribute used in its order clause
        if (pp_order_attribute) {
            pp_order_attribute->pp_entity_sets_using.insert(this);
        }

        break;
    }
    case ePopulateDependencies:
    {
        // The following block of code is very similar in EntitySetSymbol and EntityTableSymbol
        // construct function bodies
        build_body_update_cell();
        build_body_insert();
        build_body_erase();

        // Dependency on attribute in entity set order clause
        if (pp_order_attribute) {
            {
                CodeBlock& c = pp_order_attribute->notify_fn->func_body;
                c += injection_description();
                c += "// upcoming entity order change in entity set " + name;
                c += "if (om_active) {";
                if (filter) {
                    c += "if (" + filter->name + ") {";
                }
                c += "// Set entity set context for entity operator<";
                c += "om_entity_set_context = " + to_string(pp_entity_set_id) + "; // " + name;
                c += "// Erase from entity set, will be immediately reinserted in side_effects function after value changes";
                c += erase_fn->name + "();";
                c += "// Reset entity set context for entity operator<";
                c += "om_entity_set_context = -1;";
                if (filter) {
                    c += "}";
                }
                c += "}";
            }
            {
                CodeBlock& c = pp_order_attribute->side_effects_fn->func_body;
                c += injection_description();
                c += "// entity order change in entity set " + name;
                c += "if (om_active) {";
                if (filter) {
                    c += "if (" + filter->name + ") {";
                }
                c += "// Set entity set context for entity operator<";
                c += "om_entity_set_context = " + to_string(pp_entity_set_id) + "; // " + name;
                c += "// Was previously erased from entity set in notify function before value changed.";
                c += insert_fn->name + "();";
                c += "// Reset entity set context for entity operator<";
                c += "om_entity_set_context = -1;";
                if (filter) {
                    c += "}";
                }
                c += "}";
            }
        }

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
            if (pp_order_attribute) {
                c += "// Set entity set context for entity operator<";
                c += "om_entity_set_context = " + to_string(pp_entity_set_id) + "; // " + name;
            }
            c += erase_fn->name + "();";
            c += update_cell_fn->name + "();";
            c += insert_fn->name + "();";
            if (pp_order_attribute) {
                c += "// Reset entity set context for entity operator<";
                c += "om_entity_set_context = -1;";
            }
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
            if (pp_order_attribute) {
                c += "// Set entity set context for entity operator<";
                c += "om_entity_set_context = " + to_string(pp_entity_set_id) + "; // " + name;
            }
            c += "if (om_new) {";
            c += "// filter changed from false to true";
            c += update_cell_fn->name + "();";
            c += insert_fn->name + "();";
            c += "}";
            c += "else {";
            c += "// filter changed from true to false";
            c += erase_fn->name + "();";
            c += "}";
            if (pp_order_attribute) {
                c += "// Reset entity set context for entity operator<";
                c += "om_entity_set_context = -1;";
            }
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
    h += "";
    h += doxygen_short("entity_set: " + label());
    h += "extern thread_local EntitySet<" + pp_entity->name + "> * "+ name + dims + ";";
    h += "";

    h += "/// Count of entities in set " + name;
    h += "extern thread_local size_t "  + count_gvn + ";";
    h += "";
    h += "/// Maximum count of entities in set " + name;
    h += "extern thread_local size_t " + max_gvn + ";";
    h += "";
    h += "/// Number of moves of entities in set " + name;
    h += "extern thread_local size_t " + inserts_gvn + ";";
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
    c += "thread_local EntitySet<" + pp_entity->name + "> * "+ name + dims + ";";

    c += "thread_local size_t " + count_gvn + " = 0;";
    c += "thread_local size_t " + max_gvn + " = 0;";
    c += "thread_local size_t " + inserts_gvn + " = 0;";
    c += "";

    return c;
}

// The following function definition is identical in EntitySetSymbol and EntityTableSymbol
void EntitySetSymbol::build_body_update_cell()
{
    CodeBlock& c = update_cell_fn->func_body;

    size_t rank = dimension_list.size();

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

    c += "if constexpr (om_resource_use_on) {";
    c +=     "++" + inserts_gvn + ";";
    c +=     "++" + count_gvn + ";";
    c +=     "if (" + max_gvn + " < " + count_gvn + ") {";
    c +=         max_gvn + " = " + count_gvn + ";";
    c +=     "}";
    c += "}";

    if (dimension_count() == 0) {
        c += name + "->insert(this);" ;
    }
    else {
        c += "int cell = " + cell->name + ";" ;
        c += "EntitySet<" + pp_entity->name + "> ** flattened_array = reinterpret_cast<EntitySet<" + pp_entity->name + "> **>(" + name + ");";
        c += "assert(flattened_array[cell]);";
        c += "flattened_array[cell]->insert(this);";
    }
}

void EntitySetSymbol::build_body_erase()
{
    CodeBlock& c = erase_fn->func_body;

    c += "if constexpr (om_resource_use_on) {";
    c +=     "--" + name + "_count;";
    c += "}";

    if (dimension_count() == 0) {
        c += name + "->erase(this);" ;
    }
    else {
        c += "int cell = " + cell->name + ";";
        c += "EntitySet<" + pp_entity->name + "> ** flattened_array = reinterpret_cast<EntitySet<" + pp_entity->name + "> **>(" + name + ");";
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
