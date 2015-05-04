/**
* @file    TableSymbol.cpp
* Definitions for the TableSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include <regex>
#include "TableSymbol.h"
#include "LanguageSymbol.h"
#include "AgentSymbol.h"
#include "AgentInternalSymbol.h"
#include "AgentVarSymbol.h"
#include "BuiltinAgentVarSymbol.h"
#include "AgentFuncSymbol.h"
#include "IdentityAgentVarSymbol.h"
#include "NumericSymbol.h"
#include "RangeSymbol.h"
#include "DimensionSymbol.h"
#include "TableAccumulatorSymbol.h"
#include "TableAnalysisAgentVarSymbol.h"
#include "TableExpressionSymbol.h"
#include "EnumerationSymbol.h"
#include "CodeBlock.h"
#include "libopenm/db/metaModelHolder.h"

using namespace std;
using namespace openm;

// this function exist only because g++ below 4.9 does not support std::regex
static string regexReplace(const string & i_srcText, const char * i_pattern, const char * i_replaceWith);

void TableSymbol::create_auxiliary_symbols()
{
    {
        assert(!cell); // initialization guarantee
        // Set storage type to int. Can be changed in a subsequent pass to optimize storage based on array size.
        auto *typ = NumericSymbol::find(token::TK_int);
        assert(typ); // initialization guarantee
        cell = new AgentInternalSymbol("om_" + name + "_cell", agent, typ);
    }

    {
        assert(!previous_global_counter); // initialization guarantee
        // Set storage type to big_counter, which is the same type as the global event counter.
        auto *typ = NumericSymbol::find(token::TK_big_counter);
        assert(typ); // initialization guarantee
        previous_global_counter = new AgentInternalSymbol("om_" + name + "_previous_global_counter", agent, typ);
    }

    {
        assert(!update_cell_fn); // initialization guarantee
        update_cell_fn = new AgentFuncSymbol("om_" + name + "_update_cell", agent);
        assert(update_cell_fn); // out of memory check
        update_cell_fn->doc_block = doxygen_short("Update the active cell index of table " + name + " using agentvars in the " + agent->name + " agent.");
    }

    {
        assert(!prepare_increments_fn); // initialization guarantee
        prepare_increments_fn = new AgentFuncSymbol("om_" + name + "_prepare_increments", agent);
        assert(prepare_increments_fn); // out of memory check
        prepare_increments_fn->doc_block = doxygen_short("Prepare the increments for the active table cell in " + name + ".");
    }

    {
        assert(!process_increments_fn); // initialization guarantee
        process_increments_fn = new AgentFuncSymbol("om_" + name + "_process_increments", agent);
        assert(process_increments_fn); // out of memory check
        process_increments_fn->doc_block = doxygen_short("Process the increments for the active table cell in " + name + ".");
    }
}

void TableSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case eAssignMembers:
    {
        // assign direct pointer to agent for use post-parse
        pp_agent = dynamic_cast<AgentSymbol *> (pp_symbol(agent));
        assert(pp_agent); // parser guarantee

        break;
    }
    case ePopulateCollections:
    {
        // add this table to the complete list of tables
        pp_all_tables.push_back(this);

        // Add this table to the agent's list of tables
        pp_agent->pp_agent_tables.push_back(this);

        break;
    }

    case ePopulateDependencies:
    {
        // Process collections of observations required by accumulators
        for (auto acc : pp_accumulators) {
            if (acc->has_obs_collection) {
                // Search for an accumulator which is already marked to update the same collection.
                TableAccumulatorSymbol *acc_found = nullptr;
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
                    acc->obs_collection_index = obs_collections;
                    ++obs_collections;
                }
            }
        }

        // The following block of code is almost identical in EntitySetSymbol and TableSymbol
        // construct function bodies
        build_body_update_cell();
        build_body_prepare_increments();
        build_body_process_increments();

        // Dependency on change in index agentvars
        for (auto dim : dimension_list) {
            auto av = dim->pp_attribute;
            CodeBlock& c = av->side_effects_fn->func_body;
            c += injection_description();
            c += "// cell change in " + name;
            c += "if (om_active) {";
            if (filter) {
                c += "if (" + filter->name + ") {";
            }
            c += process_increments_fn->name + "();";
            c += update_cell_fn->name + "();";
            c += prepare_increments_fn->name + "();";
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
            c += prepare_increments_fn->name + "();";
            c += "}";
            c += "else {";
            c += "// filter changed from true to false";
            c += process_increments_fn->name + "();";
            c += "}";
            c += "}";
        }

        // Mark enumerations required for metadata support for this table
        // The enumeration of each dimension is required
        for (auto dim : dimension_list) {
            assert(dim->pp_attribute); // previously verified
            auto es = dynamic_cast<EnumerationSymbol *>(dim->pp_attribute->pp_data_type);
            assert(es); // previously verified
            es->metadata_needed = true;
        }

        break;
    }

    default:
    break;
    }
}

CodeBlock TableSymbol::cxx_declaration_global()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_global();

    // Perform operations specific to this level in the Symbol hierarchy.
    int n_accumulators = pp_accumulators.size();
    int n_expressions = pp_expressions.size();
    int n_cells = cell_count();
    int n_obs_collections = obs_collections;

    h += "";
    h += "class " + name + " {";
    h += "public:";
    h += "void initialize_accumulators();";
    h += "void extract_accumulators();";
    h += "void scale_accumulators();";
    h += "void compute_expressions();";
    h += "";
    h += "// constants";
    h += "static const int n_cells = " + to_string(n_cells) + ";";
    h += "static const int n_accumulators = " + to_string(n_accumulators) + ";";
    h += "static const int n_expressions = " + to_string(n_expressions) + ";";
    if (n_obs_collections > 0) {
        h += "static const int n_obs_collections = " + to_string(n_obs_collections) + ";";
    }

    h += "";
    h += "// expression storage";
    for (int j = 0; j < n_expressions; j++) {
        h += "double expr" + to_string(j) + "[n_cells];";
    }

    h += "";
    h += "// accumulator storage";
    for (int j = 0; j < n_accumulators; j++) {
        h += "double acc" + to_string(j) + "[n_cells];";
    }

    if (n_obs_collections > 0) {
        h += "";
        h += "// observation collection storage";
        h += "// TODO: Use pointers to forward_list pending VC implementation of thread_local";
        h += "forward_list<double> * obs_collections[n_cells][n_obs_collections];";
    }

    h += "";
    h += "// accumulator array of pointers";
    h += "double *accumulators[n_accumulators];";
    h += "";
    h += "// expression array of pointers";
    h += "double *expressions[n_expressions];";

    h += "};";
    h += "extern thread_local " + name + " the" + name + ";";

    return h;
}

CodeBlock TableSymbol::cxx_definition_global()
{
    // Hook into the hierarchical calling chain
    CodeBlock c = super::cxx_definition_global();

    // Perform operations specific to this level in the Symbol hierarchy.
    c += "";
    // table definition
    // E.g. DurationOfLife theDurationOfLife;
    c += "thread_local " + name + " the" + name + ";";

    // definition of initialize_accumulators()
    c += "void " + name + "::initialize_accumulators()";
    c += "{";

    if (obs_collections > 0) {
        c += "// Initialize observation collections";
        c += "// TODO: Use pointers to forward_list pending VC implementation of thread_local";
        c += "for (int cell = 0; cell < n_cells; ++cell) {";
        c += "for (int coll = 0; coll < n_obs_collections; ++coll) {";
        c += "obs_collections[cell][coll] = new forward_list<double>;";
        c += "}";
        c += "}";
        c += "";
    }

    for (auto acc : pp_accumulators) {
        string initial_value = "";
        switch (acc->accumulator) {
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
        c += "// " + Symbol::token_to_string(acc->accumulator) + "(" + Symbol::token_to_string(acc->increment) + "(" + acc->agentvar->name + "))";
        // e.g. for ( int cell = 0; cell < n_cells; cell++ ) acc0[cell] =   0.0;
        c += "for ( int cell = 0; cell < n_cells; cell++ ) acc" + to_string(acc->index) + "[cell] = " + initial_value + ";";
        // e.g. accumulators[0] = acc0;
        c += "accumulators[" + to_string(acc->index) + "] = acc" + to_string(acc->index) + ";";
        c += "";

    }
    c += "}";
    c += "";

    // definition of extract_accumulators()
    c += "void " + name + "::extract_accumulators()";
    c += "{";

    if (obs_collections > 0) {
        c += "// process each cell";
        c += "for (int cell = 0; cell < n_cells; ++cell) {";

        c += "";
        c += "// Compute statistics for each observation collection in the cell.";
        c += "double P1[n_obs_collections];";
        c += "double P2[n_obs_collections];";
        c += "double P5[n_obs_collections];";
        c += "double P10[n_obs_collections];";
        c += "double P20[n_obs_collections];";
        c += "double P25[n_obs_collections];";
        c += "double P30[n_obs_collections];";
        c += "double P40[n_obs_collections];";
        c += "double P50[n_obs_collections];";
        c += "double P60[n_obs_collections];";
        c += "double P70[n_obs_collections];";
        c += "double P75[n_obs_collections];";
        c += "double P80[n_obs_collections];";
        c += "double P90[n_obs_collections];";
        c += "double P95[n_obs_collections];";
        c += "double P98[n_obs_collections];";
        c += "double P99[n_obs_collections];";
        c += "double gini[n_obs_collections];";
        c += "for (int coll = 0; coll < n_obs_collections; ++coll) {";
        c += "P1[coll] = UNDEF_VALUE;";
        c += "P2[coll] = UNDEF_VALUE;";
        c += "P5[coll] = UNDEF_VALUE;";
        c += "P10[coll] = UNDEF_VALUE;";
        c += "P20[coll] = UNDEF_VALUE;";
        c += "P25[coll] = UNDEF_VALUE;";
        c += "P30[coll] = UNDEF_VALUE;";
        c += "P40[coll] = UNDEF_VALUE;";
        c += "P50[coll] = UNDEF_VALUE;";
        c += "P60[coll] = UNDEF_VALUE;";
        c += "P70[coll] = UNDEF_VALUE;";
        c += "P75[coll] = UNDEF_VALUE;";
        c += "P80[coll] = UNDEF_VALUE;";
        c += "P90[coll] = UNDEF_VALUE;";
        c += "P95[coll] = UNDEF_VALUE;";
        c += "P98[coll] = UNDEF_VALUE;";
        c += "P99[coll] = UNDEF_VALUE;";
        c += "gini[coll] = UNDEF_VALUE;";
        c += "auto lst = obs_collections[cell][coll];";
        c += "lst->sort();";
        c += "double total_count = distance(lst->begin(), lst->end());";
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
        c += "for (auto value : *lst ) {";
        c += "cum_count += 1.0;";
        c += "cum_value += value;";
        c += "cum_value_i += cum_count * value;";
        c += "if (!P1_done && cum_count >= total_count * 0.01) {";
        c += "P1[coll] = value;";
        c += "P1_done = true;";
        c += "}";
        c += "if (!P2_done && cum_count >= total_count * 0.02) {";
        c += "P2[coll] = value;";
        c += "P2_done = true;";
        c += "}";
        c += "if (!P5_done && cum_count >= total_count * 0.05) {";
        c += "P5[coll] = value;";
        c += "P5_done = true;";
        c += "}";
        c += "if (!P10_done && cum_count >= total_count * 0.10) {";
        c += "P10[coll] = value;";
        c += "P10_done = true;";
        c += "}";
        c += "if (!P20_done && cum_count >= total_count * 0.20) {";
        c += "P20[coll] = value;";
        c += "P20_done = true;";
        c += "}";
        c += "if (!P25_done && cum_count >= total_count * 0.25) {";
        c += "P25[coll] = value;";
        c += "P25_done = true;";
        c += "}";
        c += "if (!P30_done && cum_count >= total_count * 0.30) {";
        c += "P30[coll] = value;";
        c += "P30_done = true;";
        c += "}";
        c += "if (!P40_done && cum_count >= total_count * 0.40) {";
        c += "P40[coll] = value;";
        c += "P40_done = true;";
        c += "}";
        c += "if (!P50_done && cum_count >= total_count * 0.50) {";
        c += "P50[coll] = value;";
        c += "P50_done = true;";
        c += "}";
        c += "if (!P60_done && cum_count >= total_count * 0.60) {";
        c += "P60[coll] = value;";
        c += "P60_done = true;";
        c += "}";
        c += "if (!P70_done && cum_count >= total_count * 0.70) {";
        c += "P70[coll] = value;";
        c += "P70_done = true;";
        c += "}";
        c += "if (!P75_done && cum_count >= total_count * 0.75) {";
        c += "P75[coll] = value;";
        c += "P75_done = true;";
        c += "}";
        c += "if (!P80_done && cum_count >= total_count * 0.80) {";
        c += "P80[coll] = value;";
        c += "P80_done = true;";
        c += "}";
        c += "if (!P90_done && cum_count >= total_count * 0.90) {";
        c += "P90[coll] = value;";
        c += "P90_done = true;";
        c += "}";
        c += "if (!P95_done && cum_count >= total_count * 0.95) {";
        c += "P95[coll] = value;";
        c += "P95_done = true;";
        c += "}";
        c += "if (!P98_done && cum_count >= total_count * 0.98) {";
        c += "P98[coll] = value;";
        c += "P98_done = true;";
        c += "}";
        c += "if (!P99_done && cum_count >= total_count * 0.99) {";
        c += "P99[coll] = value;";
        c += "P99_done = true;";
        c += "}";
        c += "cum_count_prev = cum_count;";
        c += "value_prev = value;";
        c += "}";
        c += "gini[coll] = (2.0 * cum_value_i ) / (total_count * cum_value) - (total_count + 1.0) / total_count;";
        c += "}";
        c += "}";
        c += "";

        for (auto acc : pp_accumulators) {
            if (acc->has_obs_collection) {
                c += "// Assign " + acc->pretty_name();
                string acc_name = "acc" + to_string(acc->index);
                string obs_index = to_string(acc->obs_collection_index);
                string stat_name = token_to_string(acc->accumulator);
                c += acc_name + "[cell] = " + stat_name + "[" + obs_index + "];";
                c += "";
            }
        }

        c += "// Free observation collections in cell";
        c += "for (int coll = 0; coll < n_obs_collections; ++coll) {";
        c += "obs_collections[cell][coll]->clear();";
        c += "delete obs_collections[cell][coll];";
        c += "obs_collections[cell][coll] = nullptr;";
        c += "}";

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
        if (acc->accumulator == token::TK_sum) {
            // e.g.  sum(value_in(alive))
            c += "// " + Symbol::token_to_string(acc->accumulator) + "(" + Symbol::token_to_string(acc->increment) + "(" + acc->agentvar->name + "))";
            // e.g. for ( int cell = 0; cell < n_cells; cell++ ) acc0[cell] *= scale_factor;
            c += "for ( int cell = 0; cell < n_cells; cell++ ) acc" + to_string(acc->index) + "[cell] *= scale_factor;";
            // e.g. accumulators[0] = acc0;
        }
    }
    c += "}";
    c += "}";
    c += "";

    // definition of compute_expressions()
    // E.g. void DurationOfLife::compute_expressions()
    c += "void " + name + "::compute_expressions()";
    c += "{";
    for (auto table_expr : pp_expressions) {
        // E.g.  // SUM_BEFORE( acc0 )
        c += "// " + table_expr->get_expression(table_expr->root, TableExpressionSymbol::expression_style::sql);
        // E.g. expressions[0] = expr0;
        c += "expressions[" + to_string(table_expr->index) + "] = expr" + to_string(table_expr->index) + ";";
        // E.g. for ( int cell = 0; cell < n_cells; cell++ ) expressions[0][cell] = accumulators[0][cell] ;
        c += "for ( int cell = 0; cell < n_cells; cell++ ) "
            "expressions[" + to_string(table_expr->index) + "][cell] = " + table_expr->get_expression(table_expr->root, TableExpressionSymbol::expression_style::cxx) + " ;";
        c += "";
    }
    c += "}";
    c += "";
    return c;
}

// The following function definition is identical in EntitySetSymbol and TableSymbol
void TableSymbol::build_body_update_cell()
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
        c += "// dimension=" + to_string(dim->index) + " agentvar=" + av->name + " type=" + es->name + " size=" + to_string(es->pp_size());
        if (dim > 0) {
            c += "cell *= " + to_string(es->pp_size()) + ";";
        }
        //c += "index = " + av->unique_name + ".get();";
        c += "index = " + av->name + ";";
        auto rs = dynamic_cast<RangeSymbol *>(es);
        if (rs) {
            c += "// adjust range to zero-based" ;
            c += "index -= " + to_string(rs->lower_bound) + ";";
        }
        c += "cell += index;";
    }
    c += "";
    c += "assert(cell >= 0 && cell < " + name + "::n_cells ); // logic guarantee";
    c += "";
    c += cell->name + " = cell;" ;
}

void TableSymbol::build_body_prepare_increments()
{
    CodeBlock& c = prepare_increments_fn->func_body;

    for (auto table_agentvar : pp_table_agentvars) {
        if (table_agentvar->need_value_in)
            c += table_agentvar->cxx_prepare_increment();
    }
    if (unit) {
        c += "";
        c += "// Increment table unit";
        c += unit->name + ".set(" + unit->name + ".get() + 1);";
    }
}

void TableSymbol::build_body_process_increments()
{
    CodeBlock& c = process_increments_fn->func_body;

    c += "int cell = " + cell->name + ";" ;
    c += "";

    for (auto acc : pp_accumulators) {
        // name of agentvar
        string agentvar_name = acc->agentvar->name;
        // name of 'in' for agentvar
        string in_agentvar_name = acc->pp_analysis_agentvar->in_member_name();
        // index of accumulator as string
        string accumulator_index = to_string(acc->index);
        // expression for the accumulator as string
        string accumulator_expr = "the" + name + ".accumulators[" + accumulator_index + "][cell]";

        // expression which evaluates to the value of the increment
        string increment_expr;
        switch (acc->increment) {
        case token::TK_value_in:
            increment_expr = in_agentvar_name;
            break;
        case token::TK_nz_value_in:
            increment_expr = in_agentvar_name + " != 0 ? 1.0 : 0.0";
            break;
        case token::TK_value_in2:
            increment_expr = in_agentvar_name + " * " + in_agentvar_name;
            break;
        case token::TK_value_out:
            increment_expr = agentvar_name;
            break;
        case token::TK_nz_value_out:
            increment_expr = agentvar_name + " != 0 ? 1.0 : 0.0";
            break;
        case token::TK_value_out2:
            increment_expr = agentvar_name + " * " + agentvar_name;
            break;
        case token::TK_delta:
            increment_expr = agentvar_name + " - " + in_agentvar_name;
            break;
        case token::TK_nz_delta:
            increment_expr = "(" + agentvar_name + " - " + in_agentvar_name + ") != 0 ? 1.0 : 0.0";
            break;
        case token::TK_delta2:
            increment_expr = "(" + agentvar_name + " - " + in_agentvar_name + ") * (" + agentvar_name + " - " + in_agentvar_name + ")";
            break;
        default:
            assert(0); // parser guarantee
        }

        c += "{";
        c += "// " + acc->pretty_name();
        c += "double dIncrement = " + increment_expr + ";";
        switch (acc->accumulator) {
        case token::TK_sum:
            c += accumulator_expr + " += dIncrement;";
            break;
        case token::TK_minimum:
            c += "double dAccumulator = " + accumulator_expr + ";";
            c += "if ( dIncrement < dAccumulator ) " + accumulator_expr + " = dIncrement;";
            break;
        case token::TK_maximum:
            c += "double dAccumulator = " + accumulator_expr + ";";
            c += "if ( dIncrement > dAccumulator ) " + accumulator_expr + " = dIncrement;";
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
            if (acc->updates_obs_collection) {
                string obs_index = to_string(acc->obs_collection_index);
                c += "auto lst = the" + name + ".obs_collections[cell][" + obs_index + "];";
                c += "lst->push_front(dIncrement);";
            }
            break;
        default:
            assert(0); // parser guarantee
        }
        c += "}";
    }
}

// The following function definition is identical in EntitySetSymbol and TableSymbol
int TableSymbol::rank()
{
    return dimension_list.size();
}

// The following function definition is identical in EntitySetSymbol and TableSymbol
int TableSymbol::cell_count()
{
    int cells = 1;
    for (auto dim : dimension_list) {
        auto es = dim->pp_enumeration;
        assert(es); // integrity check guarantee
        cells *= es->pp_size();
    }
    return cells;
}

void TableSymbol::populate_metadata(openm::MetaModelHolder & metaRows)
{
    // Hook into the hierarchical calling chain
    super::populate_metadata(metaRows);

    // Perform operations specific to this level in the Symbol hierarchy.
    TableDicRow tableDic;

    tableDic.tableId = pp_table_id;
    tableDic.tableName = name;
    tableDic.isUser = false;
    tableDic.rank = rank();
    tableDic.isSparse = true;   // do not store NULLs
    tableDic.isHidden = false;
    tableDic.exprPos = expr_dim_position;
    metaRows.tableDic.push_back(tableDic);

    for (auto lang : Symbol::pp_all_languages) {
        TableDicTxtLangRow tableTxt;
        tableTxt.tableId = pp_table_id;
        tableTxt.langName = lang->name;
        tableTxt.descr = label(*lang);
        tableTxt.note = note(*lang);
        tableTxt.exprDescr = "Expressions (" + lang->name + ")"; // TODO
        tableTxt.exprNote = "Expressions Note (" + lang->name + ")"; // TODO
        metaRows.tableTxt.push_back(tableTxt);
    }

    // dimensions for table
    for (auto dim : dimension_list ) {
        auto av = dim->pp_attribute;
        assert(av); // logic guarantee
        auto es = dim->pp_enumeration;
        assert(es); // logic guarantee
        TableDimsRow tableDims;
        tableDims.tableId = pp_table_id;
        tableDims.dimId = dim->index;
        //tableDims.name = "dim" + to_string(dim->index);
        tableDims.typeId = es->type_id;
        tableDims.isTotal = false;
        tableDims.dimSize = es->pp_size();
        metaRows.tableDims.push_back(tableDims);

        for (auto lang : Symbol::pp_all_languages) {
            TableDimsTxtLangRow tableDimsTxt;
            tableDimsTxt.tableId = pp_table_id;
            tableDimsTxt.dimId = dim->index;
            tableDimsTxt.langName = lang->name;
            tableDimsTxt.descr = dim->label(*lang);
            tableDimsTxt.note = dim->note(*lang);
            metaRows.tableDimsTxt.push_back(tableDimsTxt);
        }
    }

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
    for (auto expr : pp_expressions) {
        TableExprRow tableExpr;
        tableExpr.tableId = pp_table_id;
        tableExpr.exprId = expr->index;
        tableExpr.name = "expr" + to_string(expr->index);
        tableExpr.src = expr->get_expression(expr->root, TableExpressionSymbol::expression_style::sql);
        metaRows.tableExpr.push_back(tableExpr);

        for (auto lang : Symbol::pp_all_languages) {
            TableExprTxtLangRow tableExprTxt;
            tableExprTxt.tableId = pp_table_id;
            tableExprTxt.exprId = expr->index;

            tableExprTxt.langName = lang->name;

            // construct label by removing decimals=nnn from string if present
            tableExprTxt.descr = regexReplace(expr->label(*lang), "[[:space:]]+decimals=[[:digit:]]+", "");
            
            tableExprTxt.note = expr->note(*lang);
            metaRows.tableExprTxt.push_back(tableExprTxt);
        }
    }
}

// this function exist only because g++ below 4.9 does not support std::regex
#ifdef _WIN32

string regexReplace(const string & i_srcText, const char * i_pattern, const char * i_replaceWith)
{
    regex pat(i_pattern);
    return regex_replace(i_srcText, pat, i_replaceWith);
}

#else

#include <regex.h>

#define MAX_RE_ERROR_MSG    1024

string regexReplace(const string & i_srcText, const char * i_pattern, const char * i_replaceWith)
{
    // prepare regex
    regex_t re;

    int nRet = regcomp(&re, i_pattern, REG_EXTENDED | REG_NEWLINE);
    if (nRet != 0) {
        char errMsg[MAX_RE_ERROR_MSG + 1];
        regerror(nRet, &re, errMsg, sizeof(errMsg));
        throw HelperException("Regular expression error: %s", errMsg);
    }

    // replace first occurrence of pattern in source text
    string sResult;
    regmatch_t matchPos;

    nRet = regexec(&re, i_srcText.c_str(), 1, &matchPos, 0);
    if (nRet == REG_NOMATCH) {
        regfree(&re);           // cleanup
        return i_srcText;       // pattern not found - return source text as is
    }
    if (nRet != 0) {            // error 
        regfree(&re);           // cleanup
        throw HelperException("Regular expression FAILED: %s", i_pattern);
    }
    // at this point nRet == 0 and we can replace first occurrence with replacement string
    if (matchPos.rm_so >= 0 && matchPos.rm_so < (int)i_srcText.length()) {

        string sResult =
            i_srcText.substr(0, matchPos.rm_so) +
            i_replaceWith +
            ((matchPos.rm_eo >= 0 && matchPos.rm_eo < (int)i_srcText.length()) ? i_srcText.substr(matchPos.rm_eo) : "");

        regfree(&re);       // cleanup
        return sResult;
    }
    else {                  // starting offset out of range - pattern not found
        regfree(&re);       // cleanup
        return i_srcText;   // return source text as is
    }
}

#endif // _WIN32
