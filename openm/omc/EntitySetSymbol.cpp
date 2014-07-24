/**
* @file    EntitySetSymbol.cpp
* Definitions for the EntitySetSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include "EntitySetSymbol.h"
#include "AgentSymbol.h"
#include "AgentInternalSymbol.h"
#include "AgentFuncSymbol.h"
#include "IdentityAgentVarSymbol.h"
#include "RangeSymbol.h"
#include "AgentVarSymbol.h"
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
        cell = new AgentInternalSymbol("om_" + name + "_cell", agent, typ);
    }

    {
        assert(!update_cell_fn); // initialization guarantee
        update_cell_fn = new AgentFuncSymbol("om_" + name + "_update_cell", agent);
        assert(update_cell_fn); // out of memory check
        update_cell_fn->doc_block = doxygen_short("Update the active cell index of table " + name + " using agentvars in the " + agent->name + " agent.");
    }

    {
        assert(!insert_fn); // initialization guarantee
        insert_fn = new AgentFuncSymbol("om_" + name + "_insert", agent);
        assert(insert_fn); // out of memory check
        insert_fn->doc_block = doxygen_short("Insert the entity into the active cell in " + name + ".");
    }

    {
        assert(!erase_fn); // initialization guarantee
        erase_fn = new AgentFuncSymbol("om_" + name + "_erase", agent);
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
    case ePopulateCollections:
        {
            // assign direct pointer to agent for use post-parse
            pp_agent = dynamic_cast<AgentSymbol *> (pp_symbol(agent));
            assert(pp_agent); // parser guarantee

            // add this entity set to the complete list of entity sets
            pp_all_entity_sets.push_back(this);

            // Add this entity set to the agent's list of entity sets
            pp_agent->pp_agent_entity_sets.push_back(this);

            // The following block of code is identical in EntitySetSymbol and TableSymbol
            // validate dimension list
            // and populate the post-parse version
            for (auto psym : dimension_list) {
                assert(psym); // logic guarantee
                auto sym = *psym; // remove one level of indirection
                assert(sym); // grammar guarantee
                auto avs = dynamic_cast<AgentVarSymbol *>(sym);
                if (!avs) {
                    pp_error("'" + sym->name + "' is not an agentvar in dimension of '" + name + "'");
                    continue; // don't insert invalid type in dimension list
                }
                auto es = dynamic_cast<EnumerationSymbol *>(pp_symbol(avs->data_type));
                if (!es) {
                    pp_error("The datatype of '" + avs->name + "' must be an enumeration type in dimension of '" + name + "'");
                    continue; // don't insert invalid type in dimension list
                }
                pp_dimension_list_agentvar.push_back(avs);
                pp_dimension_list_enum.push_back(es);
            }
            // clear the parse version to avoid inadvertant use post-parse
            dimension_list.clear();
        }
        break;
    case ePopulateDependencies:
        {
            // The following block of code is almost identical in EntitySetSymbol and TableSymbol
            // construct function bodies
            build_body_update_cell();
            build_body_insert();
            build_body_erase();

            // Dependency on change in index agentvars
            for (auto av : pp_dimension_list_agentvar) {
                CodeBlock& c = av->side_effects_fn->func_body;
                c += "// cell change in table " + name;
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
                c += "";
            }

            // Dependency on table filter
            if (filter) {
                CodeBlock& c = filter->side_effects_fn->func_body;
                c += "// filter change in " + name;
                c += "if (om_active) {";
                c += "if (new_value) {";
                c += "// filter changed from false to true";
                c += update_cell_fn->name + "();";
                c += insert_fn->name + "();";
                c += "}";
                c += "else {";
                c += "// filter changed from true to false";
                c += erase_fn->name + "();";
                c += "}";
                c += "}";
                c += "";
            }
        }
        break;
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
    for (auto es : pp_dimension_list_enum) {
        assert(es); // integrity check guarantee
        dims += "[" + to_string(es->pp_size()) + "]";
    }
    h += "extern EntitySet<" + pp_agent->name + "> "+ name + dims + ";";
    h += "";

    return h;
}

CodeBlock EntitySetSymbol::cxx_definition_global()
{
    // Hook into the hierarchical calling chain
    CodeBlock c = super::cxx_definition_global();

    // Perform operations specific to this level in the Symbol hierarchy.
    string dims = "";
    for (auto es : pp_dimension_list_enum) {
        assert(es); // integrity check guarantee
        dims += "[" + to_string(es->pp_size()) + "]";
    }
    c += "EntitySet<" + pp_agent->name + "> "+ name + dims + ";";
    c += "";

    return c;
}

// The following function definition is identical in EntitySetSymbol and TableSymbol
void EntitySetSymbol::build_body_update_cell()
{
    CodeBlock& c = update_cell_fn->func_body;

    int rank = pp_dimension_list_enum.size();

    if (rank == 0) {
        // short version for rank 0
        c += "// only a single cell if rank 0";
        c += cell->name + " = 0;" ;
        return;
    }

    c += "int cell = 0;" ;
    c += "int index = 0;" ;

    // build an unwound loop of code
    int dim = 0;
    for (auto av : pp_dimension_list_agentvar ) {
        auto es = dynamic_cast<EnumerationSymbol *>(av->pp_data_type);
        assert(es); // integrity check guarantee
        c += "";
        c += "// dimension=" + to_string(dim) + " agentvar=" + av->name + " type=" + es->name + " size=" + to_string(es->pp_size());
        if (dim > 0) {
            c += "cell *= " + to_string(es->pp_size()) + ";";
        }
        c += "index = " + av->unique_name + ".get();";
        auto rs = dynamic_cast<RangeSymbol *>(es);
        if (rs) {
            c += "// adjust range to zero-based" ;
            c += "index -= " + to_string(rs->lower_bound) + ";";
        }
        c += "cell += index;";
        ++dim;
    }
    c += "";
    c += "assert(cell >= 0 && cell < " + name + "::n_cells ); // logic guarantee";
    c += "";
    c += cell->name + " = cell;" ;
}

void EntitySetSymbol::build_body_insert()
{
    CodeBlock& c = insert_fn->func_body;

    c += "int cell = " + cell->name + ";" ;
    c += "EntitySet<" + pp_agent->name + "> * flattened_array = &" + name + ";";
    c += "flattened_array[cell]->insert(this);";
}

void EntitySetSymbol::build_body_erase()
{
    CodeBlock& c = erase_fn->func_body;

    c += "int cell = " + cell->name + ";" ;
    c += "EntitySet<" + pp_agent->name + "> * flattened_array = &" + name + ";";
    c += "flattened_array[cell]->erase(this);";
}

// The following function definition is identical in EntitySetSymbol and TableSymbol
int EntitySetSymbol::rank()
{
    return pp_dimension_list_agentvar.size();
}

// The following function definition is identical in EntitySetSymbol and TableSymbol
int EntitySetSymbol::cell_count()
{
    int cells = 1;
    for (auto es : pp_dimension_list_enum) {
        assert(es); // integrity check guarantee
        cells *= es->pp_size();
    }
    return cells;
}
