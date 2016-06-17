/**
* @file    EntityTableSymbol.cpp
* Definitions for the EntityTableSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <cassert>
#include "EntityTableSymbol.h"
#include "LanguageSymbol.h"
#include "EntitySymbol.h"
#include "EntityIncrementSymbol.h"
#include "EntityInternalSymbol.h"
#include "AttributeSymbol.h"
#include "BuiltinAttributeSymbol.h"
#include "EntityFuncSymbol.h"
#include "IdentityAttributeSymbol.h"
#include "NumericSymbol.h"
#include "BoolSymbol.h"
#include "RangeSymbol.h"
#include "DimensionSymbol.h"
#include "EntityTableAccumulatorSymbol.h"
#include "EntityTableMeasureAttributeSymbol.h"
#include "EntityTableMeasureSymbol.h"
#include "EnumerationSymbol.h"
#include "CodeBlock.h"
#include "libopenm/db/metaModelHolder.h"

using namespace std;
using namespace openm;

void EntityTableSymbol::create_auxiliary_symbols()
{
    {
        assert(!increment); // initialization guarantee
        increment = new EntityIncrementSymbol("om_" + name + "_incr", agent, this);
    }

    {
        assert(!current_cell_fn); // initialization guarantee
        current_cell_fn = new EntityFuncSymbol("om_" + name + "_current_cell", agent, "int", "");
        assert(current_cell_fn); // out of memory check
        current_cell_fn->doc_block = doxygen_short("Compute the current cell index of table " + name + " using attributes in the " + agent->name + " entity.");
    }

    {
        assert(!init_increment_fn); // initialization guarantee
        init_increment_fn = new EntityFuncSymbol("om_" + name + "_init_increment", agent, "void", "int pending, big_counter pending_event_counter");
        assert(init_increment_fn); // out of memory check
        init_increment_fn->doc_block = doxygen_short("Initialize the increment for the active table cell in " + name + ".");
    }

    {
        assert(!push_increment_fn); // initialization guarantee
        push_increment_fn = new EntityFuncSymbol("om_" + name + "_push_increment", agent, "void", "int cell_in, int pending, big_counter pending_event_counter");
        assert(push_increment_fn); // out of memory check
        push_increment_fn->doc_block = doxygen_short("Finalize the increment and push it to the accumulators in " + name + ".");
    }
}

void EntityTableSymbol::post_parse(int pass)
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
        // add this table to the complete list of entity tables
        pp_all_entity_tables.push_back(this);

        // Add this table to the agent's list of entity tables
        pp_agent->pp_entity_tables.push_back(this);

        // Add the increment for this table to the agent's list of all callback members
        pp_agent->pp_callback_members.push_back(increment);

        break;
    }

    case ePopulateDependencies:
    {
        // Process collections of observations required by accumulators
        for (auto acc : pp_accumulators) {
            if (acc->has_obs_collection) {
                // Search for an accumulator which is already marked to update the same collection.
                EntityTableAccumulatorSymbol *acc_found = nullptr;
                for (auto acc2 : pp_accumulators) {
                    if (acc2->increment == acc->increment
                     && acc2->table_op == acc->table_op
                     && acc2->updates_obs_collection) {
                        // Accumulator found with the same increment, and handling the collection.
                        acc_found = acc2;
                        break;
                    }
                }
                if (acc_found) {
                    // Another accumulator is already updating the collection of observations.
                    // Share the same collection.
                    acc->updates_obs_collection = false;
                    acc->obs_collection_index = acc_found->obs_collection_index;
                }
                else {
                    // This collection of observations is not handled by another accumulator.
                    // This accumulator will update the collection.
                    acc->updates_obs_collection = true;
                    acc->obs_collection_index = n_collections;
                    ++n_collections;
                }
            }
        }

        // construct function bodies
        build_body_current_cell();
        build_body_init_increment();
        build_body_push_increment();

        // Dependency on change in attributes used as dimensions
        for (auto dim : dimension_list) {
            auto av = dim->pp_attribute;
            CodeBlock& c = av->side_effects_fn->func_body;
            c += injection_description();
            c += "// Cell change in " + name;
            c += "if (om_active) {";
            c += "// Check and start pending increment in entity table " + name;
            c += "auto cell = " + current_cell_fn->name + "();";
            c += "auto & incr = " + increment->name + ";";
            c += "incr.set_cell(cell);";
            c += "incr.start_pending();";
            c += "}";
        }

        // Dependency on filter
        if (filter) {
            CodeBlock& c = filter->side_effects_fn->func_body;
            c += injection_description();
            c += "// filter change in " + name;
            c += "if (om_active) {";
            c += "// Check and start pending increment in entity table " + name;
            c += "auto & incr = " + increment->name + ";";
            c += "incr.set_filter(om_new);";
            c += "incr.start_pending();";
            c += "}";
        }

        // Dependency on change in any attribute in the table
        // to trigger processing of "lazy increments"
        {
            // All attributes used in the table.
            list<AttributeSymbol *> all_attributes;

            // Attributes used as dimensions
            for (auto dim : dimension_list) {
                all_attributes.push_back(dim->pp_attribute);
            }
            // Attributes used in measures
            for (auto ma : pp_measure_attributes) {
                all_attributes.push_back(ma->pp_agentvar);
            }
            // Attribute of filter (if present)
            if (filter) {
                all_attributes.push_back(filter);
            }
            
            for (auto attr : all_attributes) {
                CodeBlock& c = attr->notify_fn->func_body;
                c += injection_description();
                c += "// Check for and finish pending increment in entity table " + name;
                c += "if (om_active) {";
                c += "auto & incr = " + increment->name + ";";
                c += "incr.finish_pending();";
                c += "}";
            }
        }

        break;
    }

    default:
    break;
    }
}

CodeBlock EntityTableSymbol::cxx_declaration_global()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_global();

    // Perform operations specific to this level in the Symbol hierarchy.
    int n_dimensions = dimension_count();
    int n_cells = cell_count();
    int n_measures = measure_count();
    int n_accumulators = accumulator_count();

    string cxx_template;
    if (n_collections == 0) {
        cxx_template = 
            "EntityTable<"
            + to_string(n_dimensions) + ", "
            + to_string(n_cells) + ", "
            + to_string(n_measures) + ", "
            + to_string(n_accumulators)
            + ">";
    }
    else {
        cxx_template = 
            "EntityTableWithObs<"
            + to_string(n_dimensions) + ", "
            + to_string(n_cells) + ", "
            + to_string(n_measures) + ", "
            + to_string(n_accumulators) + ", "
            + to_string(n_collections)
            + ">";
    }
    h += "class " + cxx_class + " final : public " + cxx_template;
    h += "{";
    h += "public:";
    // constructor
    h += cxx_class + "(initializer_list<int> shape) : " + cxx_template + "(shape)";
    h += "{";
    h += "}";
    h += "void initialize_accumulators();";
    h += "void extract_accumulators();";
    h += "void scale_accumulators();";
    h += "void compute_expressions();";
    h += "};";

    h += "typedef " + cxx_class + " " + cxx_type + ";";
    h += "extern thread_local " + cxx_type + " * " + cxx_instance + ";";
    h += "";

    return h;
}

CodeBlock EntityTableSymbol::cxx_definition_global()
{
    // Hook into the hierarchical calling chain
    CodeBlock c = super::cxx_definition_global();

    // Perform operations specific to this level in the Symbol hierarchy.
    c += "";
    // table definition
    // E.g. DurationOfLife theDurationOfLife;
    c += "thread_local " + name + " * the" + name + " = nullptr;";

    // definition of initialize_accumulators()
    c += "void " + name + "::initialize_accumulators()";
    c += "{";

    for (auto acc : pp_accumulators) {
        string initial_value = "";
        switch (acc->accumulator) {
        case token::TK_unit:
        case token::TK_sum:
            initial_value = "  0.0";
            break;
        case token::TK_minimum:
            initial_value = " std::numeric_limits<double>::infinity()";
            break;
        case token::TK_maximum:
            initial_value = "-std::numeric_limits<double>::infinity()";
            break;
        case token::TK_gini:
        case token::TK_P1:
        case token::TK_P2:
        case token::TK_P5:
        case token::TK_P10:
        case token::TK_P20:
        case token::TK_P25:
        case token::TK_P30:
        case token::TK_P40:
        case token::TK_P50:
        case token::TK_P60:
        case token::TK_P70:
        case token::TK_P75:
        case token::TK_P80:
        case token::TK_P90:
        case token::TK_P95:
        case token::TK_P98:
        case token::TK_P99:
            initial_value = "std::numeric_limits<double>::quiet_NaN()";
            break;
        default:
            // not reached
            assert(0);
        }
        // e.g.  sum(value_in(alive))
        if (acc->accumulator == token::TK_unit) {
            c += "// " + Symbol::token_to_string(acc->accumulator);
        }
        else {
            assert(acc->pp_agentvar);
            c += "// " + Symbol::token_to_string(acc->accumulator) + "(" + Symbol::token_to_string(acc->increment) + "(" + acc->pp_agentvar->name + "))";
        }
        // e.g. for ( int cell = 0; cell < n_cells; cell++ ) acc[0][cell] =   0.0;
        c += "for ( int cell = 0; cell < n_cells; cell++ ) acc[" + to_string(acc->index) + "][cell] = " + initial_value + ";";
        c += "";

    }
    c += "}";
    c += "";

    // definition of extract_accumulators()
    c += "void " + name + "::extract_accumulators()";
    c += "{";

    if (n_collections > 0) {
        c += "// process each cell";
        c += "for (size_t cell = 0; cell < n_cells; ++cell) {";

        c += "";
        c += "// Compute statistics for each observation collection in the cell.";
        c += "double P1[n_collections];";
        c += "double P2[n_collections];";
        c += "double P5[n_collections];";
        c += "double P10[n_collections];";
        c += "double P20[n_collections];";
        c += "double P25[n_collections];";
        c += "double P30[n_collections];";
        c += "double P40[n_collections];";
        c += "double P50[n_collections];";
        c += "double P60[n_collections];";
        c += "double P70[n_collections];";
        c += "double P75[n_collections];";
        c += "double P80[n_collections];";
        c += "double P90[n_collections];";
        c += "double P95[n_collections];";
        c += "double P98[n_collections];";
        c += "double P99[n_collections];";
        c += "double gini[n_collections];";
        c += "for (size_t j = 0; j < n_collections; ++j) {";
        c += "P1[j] = UNDEF_VALUE;";
        c += "P2[j] = UNDEF_VALUE;";
        c += "P5[j] = UNDEF_VALUE;";
        c += "P10[j] = UNDEF_VALUE;";
        c += "P20[j] = UNDEF_VALUE;";
        c += "P25[j] = UNDEF_VALUE;";
        c += "P30[j] = UNDEF_VALUE;";
        c += "P40[j] = UNDEF_VALUE;";
        c += "P50[j] = UNDEF_VALUE;";
        c += "P60[j] = UNDEF_VALUE;";
        c += "P70[j] = UNDEF_VALUE;";
        c += "P75[j] = UNDEF_VALUE;";
        c += "P80[j] = UNDEF_VALUE;";
        c += "P90[j] = UNDEF_VALUE;";
        c += "P95[j] = UNDEF_VALUE;";
        c += "P98[j] = UNDEF_VALUE;";
        c += "P99[j] = UNDEF_VALUE;";
        c += "gini[j] = UNDEF_VALUE;";
        c += "auto &lst = coll[cell][j];";
        c += "lst.sort();";
        c += "double total_count = distance(lst.begin(), lst.end());";
        c += "if (total_count > 0) {";
        c += "double cum_count = 0.0;";
        c += "double cum_count_prev = 0.0;";
        c += "double value_prev = 0.0;";
        c += "double cum_value = 0.0;";
        c += "double cum_value_i = 0.0; // sum of rank*value";
        c += "bool P1_done = false;";
        c += "bool P2_done = false;";
        c += "bool P5_done = false;";
        c += "bool P10_done = false;";
        c += "bool P20_done = false;";
        c += "bool P25_done = false;";
        c += "bool P30_done = false;";
        c += "bool P40_done = false;";
        c += "bool P50_done = false;";
        c += "bool P60_done = false;";
        c += "bool P70_done = false;";
        c += "bool P75_done = false;";
        c += "bool P80_done = false;";
        c += "bool P90_done = false;";
        c += "bool P95_done = false;";
        c += "bool P98_done = false;";
        c += "bool P99_done = false;";
        c += "for (auto &value : lst ) {";
        c += "cum_count += 1.0;";
        c += "cum_value += value;";
        c += "cum_value_i += cum_count * value;";
        c += "if (!P1_done && cum_count >= total_count * 0.01) {";
        c += "P1[j] = value;";
        c += "P1_done = true;";
        c += "}";
        c += "if (!P2_done && cum_count >= total_count * 0.02) {";
        c += "P2[j] = value;";
        c += "P2_done = true;";
        c += "}";
        c += "if (!P5_done && cum_count >= total_count * 0.05) {";
        c += "P5[j] = value;";
        c += "P5_done = true;";
        c += "}";
        c += "if (!P10_done && cum_count >= total_count * 0.10) {";
        c += "P10[j] = value;";
        c += "P10_done = true;";
        c += "}";
        c += "if (!P20_done && cum_count >= total_count * 0.20) {";
        c += "P20[j] = value;";
        c += "P20_done = true;";
        c += "}";
        c += "if (!P25_done && cum_count >= total_count * 0.25) {";
        c += "P25[j] = value;";
        c += "P25_done = true;";
        c += "}";
        c += "if (!P30_done && cum_count >= total_count * 0.30) {";
        c += "P30[j] = value;";
        c += "P30_done = true;";
        c += "}";
        c += "if (!P40_done && cum_count >= total_count * 0.40) {";
        c += "P40[j] = value;";
        c += "P40_done = true;";
        c += "}";
        c += "if (!P50_done && cum_count >= total_count * 0.50) {";
        c += "P50[j] = value;";
        c += "P50_done = true;";
        c += "}";
        c += "if (!P60_done && cum_count >= total_count * 0.60) {";
        c += "P60[j] = value;";
        c += "P60_done = true;";
        c += "}";
        c += "if (!P70_done && cum_count >= total_count * 0.70) {";
        c += "P70[j] = value;";
        c += "P70_done = true;";
        c += "}";
        c += "if (!P75_done && cum_count >= total_count * 0.75) {";
        c += "P75[j] = value;";
        c += "P75_done = true;";
        c += "}";
        c += "if (!P80_done && cum_count >= total_count * 0.80) {";
        c += "P80[j] = value;";
        c += "P80_done = true;";
        c += "}";
        c += "if (!P90_done && cum_count >= total_count * 0.90) {";
        c += "P90[j] = value;";
        c += "P90_done = true;";
        c += "}";
        c += "if (!P95_done && cum_count >= total_count * 0.95) {";
        c += "P95[j] = value;";
        c += "P95_done = true;";
        c += "}";
        c += "if (!P98_done && cum_count >= total_count * 0.98) {";
        c += "P98[j] = value;";
        c += "P98_done = true;";
        c += "}";
        c += "if (!P99_done && cum_count >= total_count * 0.99) {";
        c += "P99[j] = value;";
        c += "P99_done = true;";
        c += "}";
        c += "cum_count_prev = cum_count;";
        c += "value_prev = value;";
        c += "}";
        c += "gini[j] = (2.0 * cum_value_i ) / (total_count * cum_value) - (total_count + 1.0) / total_count;";
        c += "}";
        c += "}";
        c += "";

        for (auto acc : pp_accumulators) {
            if (acc->has_obs_collection) {
                c += "// Assign " + acc->pretty_name();
                string acc_index = to_string(acc->index);
                string obs_index = to_string(acc->obs_collection_index);
                string stat_name = token_to_string(acc->accumulator);
                c += "acc[" + acc_index + "][cell] = " + stat_name + "[" + obs_index + "];";
                c += "";
            }
        }

        c += "}";
        c += "";
    }

    c += "}";
    c += "";

    // definition of scale_accumulators()
    c += "void " + name + "::scale_accumulators()";
    c += "{";
    c += "double scale_factor = population_scaling_factor();";
    c += "if (scale_factor != 1.0) {";
    for (auto acc : pp_accumulators) {
        if (acc->accumulator == token::TK_sum || acc->accumulator == token::TK_unit ) {
            // e.g.  sum(value_in(alive))
            if (acc->accumulator == token::TK_unit) {
                c += "// " + Symbol::token_to_string(acc->accumulator);
            }
            else {
                assert(acc->pp_agentvar);
                c += "// " + Symbol::token_to_string(acc->accumulator) + "(" + Symbol::token_to_string(acc->increment) + "(" + acc->pp_agentvar->name + "))";
            }
            // e.g. for ( int cell = 0; cell < n_cells; cell++ ) acc[0][cell] *= scale_factor;
            c += "for (int cell = 0; cell < n_cells; cell++) acc[" + to_string(acc->index) + "][cell] *= scale_factor;";
        }
    }
    c += "}";
    c += "}";
    c += "";

    // definition of compute_expressions()
    // E.g. void DurationOfLife::compute_expressions()
    c += "void " + name + "::compute_expressions()";
    c += "{";
    for (auto measure : pp_measures) {
        auto etm = dynamic_cast<EntityTableMeasureSymbol *>(measure);
        assert(etm); // logic guarantee
        // construct scale part, e.g. "1.0E-3 * "
        string scale_part;
        if (measure->scale != 0) {
            scale_part = measure->scale_as_factor() + " * ";
        }
        // E.g.  // SUM_BEFORE( acc0 )
        c += "// " + etm->get_expression(etm->root, EntityTableMeasureSymbol::expression_style::sql);
        // E.g. for ( int cell = 0; cell < n_cells; cell++ ) expr[0][cell] = acc[0][cell] ;
        c += "for (int cell = 0; cell < n_cells; cell++ ) "
            "measure[" + to_string(etm->index) + "][cell] = " + scale_part + etm->get_expression(etm->root, EntityTableMeasureSymbol::expression_style::cxx) + " ;";
        c += "";
    }
    c += "}";
    c += "";
    return c;
}

string EntityTableSymbol::cxx_initializer() const
{
    string cxx;
    cxx = cxx_shape_initializer_list();
    return cxx;
}

void EntityTableSymbol::build_body_current_cell()
{
    CodeBlock& c = current_cell_fn->func_body;

    int rank = dimension_list.size();

    if (rank == 0) {
        // short version for rank 0
        c += "// only a single cell if rank 0";
        c += "return 0;" ;
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
        if (dim > 0) {
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
    c += "return cell;" ;
}

void EntityTableSymbol::build_body_init_increment()
{
    CodeBlock& c = init_increment_fn->func_body;

    for (auto ma : pp_measure_attributes) {
        if (!ma->need_value_in && !ma->need_value_in_event) {
            // value at start of increment not needed for this attribute
            continue;
        }

        if (ma->need_value_in) {
            auto attr = ma->pp_agentvar;
            assert(attr); // logic guarantee
            c += "{";
            c += "// interval(" + attr->name +")";
            c += "auto & value_in = " + ma->in_member_name() + ";";
            c += attr->data_type->name + " value_curr = " + attr->name + ";";
            c += "";
            c += "value_in = value_curr;";
            c += "}";
        }
        if (ma->need_value_in_event) {
            auto attr = ma->pp_agentvar;
            assert(attr); // logic guarantee
            assert(attr->lagged);
            assert(attr->lagged_event_counter);

            c += "{";
            c += "// event(" + attr->name +")";
            c += "auto & value_in = " + ma->in_event_member_name() + ";";
            c += attr->data_type->name + " value_curr = " + attr->name + ";";
            c += "auto & value_lagged = " + attr->lagged->name + ";";
            c += "auto & value_lagged_counter = " + attr->lagged_event_counter->name + ";";
            c += "";
            c += "if (pending && pending_event_counter == value_lagged_counter) {";
            c += "value_curr = value_lagged;";
            c += "}";
            c += "";
            c += "value_in = value_curr;";
            c += "}";
        }
    }
}

void EntityTableSymbol::build_body_push_increment()
{
    CodeBlock& c = push_increment_fn->func_body;

    c += "auto & table = the" + name + ";";
    for (auto acc : pp_accumulators) {
        c += "{";
        c += "// " + acc->pretty_name();
        c += "const int acc_index = " + to_string(acc->index) + "; // accumulator index";
        if (acc->accumulator != token::TK_unit) {
            auto attr = acc->pp_agentvar;
            assert(attr);
            if (acc->uses_value_out()) {
                c += attr->data_type->name + " value_out = " + attr->name + ";";
            }
            if (acc->uses_value_in()) {
                if (acc->table_op == token::TK_interval) {
                    c += "auto & value_in = " + acc->pp_analysis_agentvar->in_member_name() + ";";
                }
                else if (acc->table_op == token::TK_event) {
                    c += "auto & value_in = " + acc->pp_analysis_agentvar->in_event_member_name() + ";";
                    assert(attr->lagged);
                    assert(attr->lagged_event_counter);
                    c += "auto & value_lagged = " + attr->lagged->name + ";";
                    c += "auto & value_lagged_counter = " + attr->lagged_event_counter->name + ";";
                    c += "";
                    c += "if (pending && pending_event_counter == value_lagged_counter) {";
                    c += "value_out = value_lagged;";
                    c += "}";
                }
            }
        }
        c += "";

        c += "double dIncrement;";
        switch (acc->increment) {
        case token::TK_value_in:
            c += "dIncrement = value_in;";
            break;
        case token::TK_nz_value_in:
            c += "dIncrement = value_in != 0 ? 1.0 : 0.0;";
            break;
        case token::TK_value_in2:
            c += "dIncrement = value_in * value_in;";
            break;
        case token::TK_value_out:
            c += "dIncrement = value_out;";
            break;
        case token::TK_nz_value_out:
            c += "dIncrement = value_out != 0 ? 1.0 : 0.0;";
            break;
        case token::TK_value_out2:
            c += "dIncrement = value_out * value_out;";
            break;
        case token::TK_delta:
            c += "dIncrement = value_out - value_in;";
            break;
        case token::TK_nz_delta:
            c += "dIncrement = (value_out - value_in) != 0 ? 1.0 : 0.0;";
            break;
        case token::TK_delta2:
            c += "dIncrement = (value_out - value_in) * (value_out - value_in);";
            break;
        case token::TK_unused:
            assert(acc->accumulator == token::TK_unit);
            c += "dIncrement = 1.0;";
            break;
        default:
            assert(false); // parser guarantee
        }
        c += "";

        c += "auto & dAccumulator = table->acc[acc_index][cell_in];";
        switch (acc->accumulator) {
        case token::TK_unit:
            c += "dAccumulator += dIncrement;";
            break;
        case token::TK_sum:
            c += "dAccumulator += dIncrement;";
            break;
        case token::TK_minimum:
            c += "if ( dIncrement < dAccumulator ) dAccumulator = dIncrement;";
            break;
        case token::TK_maximum:
            c += "if ( dIncrement > dAccumulator ) dAccumulator = dIncrement;";
            break;
        case token::TK_gini:
        case token::TK_P1:
        case token::TK_P2:
        case token::TK_P5:
        case token::TK_P10:
        case token::TK_P20:
        case token::TK_P25:
        case token::TK_P30:
        case token::TK_P40:
        case token::TK_P50:
        case token::TK_P60:
        case token::TK_P70:
        case token::TK_P75:
        case token::TK_P80:
        case token::TK_P90:
        case token::TK_P95:
        case token::TK_P98:
        case token::TK_P99:
        {
            c += "const int obs_index = " + to_string(acc->obs_collection_index) + "; // observation collection index";
            if (acc->updates_obs_collection) {
                c += "auto &obs_coll = table->coll[cell_in][obs_index];";
                c += "obs_coll.push_front(dIncrement);";
            }
            else {
                c += "// Same increment is being pushed by another accumulator";
            }
        }
        break;
        default:
            assert(0); // parser guarantee
        }
        c += "}";
    }
}

void EntityTableSymbol::populate_metadata(openm::MetaModelHolder & metaRows)
{
    // Hook into the hierarchical calling chain
    super::populate_metadata(metaRows);

    // Perform operations specific to this level in the Symbol hierarchy.

    // accumulators for table
    for (auto acc : pp_accumulators) {

        TableAccRow tableAcc;

        tableAcc.tableId = pp_table_id;
        tableAcc.accId = acc->index;
        // tableAcc.name = "acc" + to_string(acc->index);
        tableAcc.expr = acc->pretty_name();
        metaRows.tableAcc.push_back(tableAcc);

        for (auto lang : Symbol::pp_all_languages) {
            TableAccTxtLangRow tableAccTxt;
            tableAccTxt.tableId = pp_table_id;
            tableAccTxt.accId = acc->index;
            tableAccTxt.langName = lang->name;
            tableAccTxt.descr = acc->label(*lang);
            tableAccTxt.note = acc->note(*lang);
            metaRows.tableAccTxt.push_back(tableAccTxt);
        }
    }

    // expressions for table
    for (auto measure : pp_measures) {

        TableExprRow tableExpr;

        auto expr = dynamic_cast<EntityTableMeasureSymbol *>(measure);
        assert(expr); // logic guarantee
        tableExpr.tableId = pp_table_id;
        tableExpr.exprId = expr->index;
        tableExpr.name = "expr" + to_string(expr->index);
        tableExpr.decimals = expr->decimals;
        // construct scale part, e.g. "1.0E-3 * "
        string scale_part;
        if (measure->scale != 0) {
            scale_part = measure->scale_as_factor() + " * ";
        }
        tableExpr.srcExpr = scale_part + expr->get_expression(expr->root, EntityTableMeasureSymbol::expression_style::sql);
        metaRows.tableExpr.push_back(tableExpr);

        for (auto lang : Symbol::pp_all_languages) {
            TableExprTxtLangRow tableExprTxt;
            tableExprTxt.tableId = pp_table_id;
            tableExprTxt.exprId = expr->index;

            tableExprTxt.langName = lang->name;

            tableExprTxt.descr = expr->label(*lang);
            
            tableExprTxt.note = expr->note(*lang);
            metaRows.tableExprTxt.push_back(tableExprTxt);
        }
    }
}
