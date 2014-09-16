/**
* @file    TableSymbol.cpp
* Definitions for the TableSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
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
#include "TableAccumulatorSymbol.h"
#include "TableAnalysisAgentVarSymbol.h"
#include "TableExpressionSymbol.h"
#include "EnumerationSymbol.h"
#include "CodeBlock.h"
#include "libopenm/db/metaModelHolder.h"

using namespace std;
using namespace openm;

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

        // The following block of code is identical in EntitySetSymbol and TableSymbol.
        // Validate dimension list and populate the post-parse version.
        for (auto psym : dimension_list) {
            assert(psym); // logic guarantee
            auto sym = *psym; // remove one level of indirection
            assert(sym); // grammar guarantee
            auto avs = dynamic_cast<AgentVarSymbol *>(sym);
            if (!avs) {
                pp_error("'" + sym->name + "' is not an agentvar in dimension of '" + name + "'");
                continue; // don't insert invalid type in dimension list
            }
            auto es = dynamic_cast<EnumerationSymbol *>(avs->pp_data_type);
            if (!es) {
                pp_error("The datatype of '" + avs->name + "' must be an enumeration type in dimension of '" + name + "'");
                continue; // don't insert invalid type in dimension list
            }
            pp_dimension_list_agentvar.push_back(avs);
            pp_dimension_list_enum.push_back(es);
        }
        // clear the parse version to avoid inadvertant use post-parse
        dimension_list.clear();
        break;
    }

    case ePopulateDependencies:
    {
        // The following block of code is almost identical in EntitySetSymbol and TableSymbol
        // construct function bodies
        build_body_update_cell();
        build_body_prepare_increments();
        build_body_process_increments();

        // Dependency on change in index agentvars
        for (auto av : pp_dimension_list_agentvar) {
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

        // Dependency on ilter
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
        for (auto es : pp_dimension_list_enum) {
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

    h += "";
    h += "class " + name + " {";
    h += "public:";
    h += "void initialize_accumulators();";
    h += "void compute_expressions();";
    // constructor
    // E.g. DurationOfLife() { initialize_accumulators();}
    h += name + "() { initialize_accumulators();}";
    h += "";
    h += "// constants";
    h += "static const int n_cells = " + to_string(n_cells) + ";";
    h += "static const int n_accumulators = " + to_string(n_accumulators) + ";";
    h += "static const int n_expressions = " + to_string(n_expressions) + ";";

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

    h += "";
    h += "// accumulator array of pointers";
    h += "double *accumulators[n_accumulators];";
    h += "";
    h += "// expression array of pointers";
    h += "double *expressions[n_expressions];";

    h += "};";
    h += "extern " + name + " the" + name + ";";

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
    c += name + " the" + name + ";";

    // definition of initialize_accumulators()
    c += "void " + name + "::initialize_accumulators()";
    c += "{";
    for (auto acc : pp_accumulators) {
        string initial_value = "";
        switch (acc->accumulator) {
        case token::TK_sum:
            initial_value = "  0.0";
            break;
        case token::TK_minimum:
            initial_value = " DBL_MAX";
            break;
        case token::TK_maximum:
            initial_value = "-DBL_MAX";
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
        //c += "index = " + av->unique_name + ".get();";
        c += "index = " + av->name + ";";
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
        default:
            assert(0); // parser guarantee
        }
        c += "}";
    }
}

// The following function definition is identical in EntitySetSymbol and TableSymbol
int TableSymbol::rank()
{
    return pp_dimension_list_agentvar.size();
}

// The following function definition is identical in EntitySetSymbol and TableSymbol
int TableSymbol::cell_count()
{
    int cells = 1;
    for (auto es : pp_dimension_list_enum) {
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
    tableDic.unitPos = expr_dim_position;
    metaRows.tableDic.push_back(tableDic);

    for (auto lang : Symbol::pp_all_languages) {
        TableDicTxtLangRow tableTxt;
        tableTxt.tableId = pp_table_id;
        tableTxt.langName = lang->name;
        tableTxt.descr = label(*lang);
        tableTxt.note = note(*lang);
        tableTxt.unitDescr = "Expressions (" + lang->name + ")"; // TODO
        tableTxt.unitNote = "Expressions Note (" + lang->name + ")"; // TODO
        metaRows.tableTxt.push_back(tableTxt);
    }

    // dimensions for table
    int dim = 0;
    for (auto av : pp_dimension_list_agentvar ) {
        auto es = dynamic_cast<EnumerationSymbol *>(av->pp_data_type);
        assert(es); // integrity check guarantee
        TableDimsRow tableDims;
        tableDims.tableId = pp_table_id;
        tableDims.name = "dim" + to_string(dim);
        tableDims.pos = dim;
        tableDims.typeId = es->type_id;
        tableDims.isTotal = false;
        tableDims.dimSize = es->pp_size();
        metaRows.tableDims.push_back(tableDims);

        for (auto lang : Symbol::pp_all_languages) {
            TableDimsTxtLangRow tableDimsTxt;
            tableDimsTxt.tableId = pp_table_id;
            tableDimsTxt.name = "dim" + to_string(dim);
            tableDimsTxt.langName = lang->name;
            tableDimsTxt.descr = av->label(*lang);
            tableDimsTxt.note = av->note(*lang);
            metaRows.tableDimsTxt.push_back(tableDimsTxt);
        }
        ++dim;
    }


    // accumulators for table
    for (auto acc : pp_accumulators) {
        TableAccRow tableAcc;

        tableAcc.tableId = pp_table_id;
        tableAcc.accId = acc->index;
        tableAcc.name = "acc" + to_string(acc->index);
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
        TableUnitRow tableUnit;
        tableUnit.tableId = pp_table_id;
        tableUnit.unitId = expr->index;
        tableUnit.name = "expr" + to_string(expr->index);
        tableUnit.src = expr->get_expression(expr->root, TableExpressionSymbol::expression_style::sql);
        metaRows.tableUnit.push_back(tableUnit);

        for (auto lang : Symbol::pp_all_languages) {
            TableUnitTxtLangRow tableUnitTxt;
            tableUnitTxt.tableId = pp_table_id;
            tableUnitTxt.unitId = expr->index;

            tableUnitTxt.langName = lang->name;
            tableUnitTxt.descr = expr->label(*lang);
            tableUnitTxt.note = expr->note(*lang);
            metaRows.tableUnitTxt.push_back(tableUnitTxt);
        }
    }
}




