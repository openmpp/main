/**
* @file    TableSymbol.cpp
* Implements the TableSymbol derived class of the Symbol class.
* Also implements related classes.
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include <algorithm>
#include <typeinfo>
#include <set>
#include "ast.h"
#include "parser_helper.h"

/**
* Post-parse operations for TableExpressionSymbol
*/

void TableExpressionSymbol::post_parse(int pass)
{
    // First perform post-parse operations at next level up in Symbol hierarchy
    super::post_parse(pass);

    switch (pass) {
    case 1:
        // assign direct pointer to table for post-parse use
        pp_table = dynamic_cast<TableSymbol *> (table);
        // Add this table expression to the table's list of expressions
        pp_table->pp_expressions.push_back(this);
        // Perform post-parse operation to each element of the expression
        post_parse_traverse(root);

        break;
    default:
        break;
    }
}

/**
* Perform post-parse operations on nodes of a ExprForTable tree
*
* @param   node    The root of the expression tree.
*
* @return  Result as a \a CodeBlock
*/

void TableExpressionSymbol::post_parse_traverse(ExprForTable *node)
{
    auto leaf = dynamic_cast<ExprForTableLeaf *>(node);
    if (leaf != nullptr) {
        (leaf->pp_accumulator) = dynamic_cast<TableAccumulatorSymbol *>(leaf->accumulator);
    }
    else {
        auto op = dynamic_cast<ExprForTableOp *>(node);
        assert(op);
        post_parse_traverse(op->left);
        post_parse_traverse(op->right);
    }
}

/**
* Get C++ expression using accumulators
*
* @param   node    The root of the expression tree.
*
* @return  Result as a \a CodeBlock
*/

string TableExpressionSymbol::get_expression(const ExprForTable *node, expression_style style)
{
    const ExprForTableLeaf* leaf = dynamic_cast<const ExprForTableLeaf *>(node);
    if (leaf != nullptr) {
        const TableAccumulatorSymbol *accumulator = leaf->pp_accumulator;
        string result;
        switch (style) {
        case cxx:
            result = "accumulators[" + to_string(accumulator->index) + "][cell]";
            break;
        case sql:
        {
                    string agg_func = "";
                    switch (accumulator->accumulator) {
                    case token::TK_sum:
                        agg_func = "OM_SUM";
                        break;
                    case token::TK_min:
                        agg_func = "OM_MIN";
                        break;
                    case token::TK_max:
                        agg_func = "OM_MAX";
                        break;
                    default:
                        assert(0);
                    }
                    result = agg_func + "( acc" + to_string(accumulator->index) + " )";
                    break;
        }
        default:
            assert(0);
        }
        return result;
    }
    else {
        auto binary_node = dynamic_cast<const ExprForTableOp *>(node);
        assert(binary_node);
        string expr_left = get_expression(binary_node->left, style);
        string expr_right = get_expression(binary_node->right, style);
        return "( " + expr_left + " " + token_to_string(binary_node->op) + " " + expr_right + " )";
    }
}

/**
* Post-parse operations for TableExpressionSymbol
*/

void TableAccumulatorSymbol::post_parse(int pass)
{
    // First perform post-parse operations at next level up in Symbol hierarchy
    super::post_parse(pass);

    switch (pass) {
    case 1:
        // assign direct pointer to table for post-parse use
        pp_table = dynamic_cast<TableSymbol *> (table);
        assert(pp_table); // grammar guarantee

        // assign direct pointer to agentvar for post-parse use
        pp_agentvar = dynamic_cast<AgentVarSymbol *> (agentvar);
        assert(pp_agentvar); // grammar guarantee

        // assign direct pointer to TableAnalysisAgentVarSymbol for post-parse use
        pp_analysis_agentvar = dynamic_cast<TableAnalysisAgentVarSymbol *> (analysis_agentvar);
        assert(pp_analysis_agentvar); // grammar guarantee

        // Add this table accumulator to the table's list of accumulators
        pp_table->pp_accumulators.push_back(this);

        break;
    default:
        break;
    }
}

/**
* Post-parse operations for TableAnalysisAgentVarSymbol
*/

void TableAnalysisAgentVarSymbol::post_parse(int pass)
{
    // First perform post-parse operations at next level up in Symbol hierarchy
    super::post_parse(pass);

    switch (pass) {
    case 0:
        if (need_value_in) {
            // Create symbol for the data member which will hold the 'in' value of the increment.
            auto av = dynamic_cast<AgentVarSymbol *>(agentvar);
            assert(av); // TODO: catch developer code error - table analysis agentvar not declared
            string member_name = in_agentvar_name();
            auto sym = new AgentInternalSymbol(member_name, av->agent, av->type);
        }
        break;
    case 1:
        // assign direct pointer to table for post-parse use
        pp_table = dynamic_cast<TableSymbol *> (table);
        assert(pp_table); // grammar guarantee

        // assign direct pointer to agentvar for post-parse use
        pp_agentvar = dynamic_cast<AgentVarSymbol *> (agentvar);
        assert(pp_agentvar); // grammar guarantee

        // Add this TableAnalysisAgentVarSymbol to the table's list of agentvars
        pp_table->pp_table_agentvars.push_back(this);

        break;
    default:
        break;
    }
}

CodeBlock TableAnalysisAgentVarSymbol::cxx_prepare_increment() const
{
    assert(pp_table);  // only call post-parse
    assert(pp_agentvar);  // only call post-parse
    CodeBlock c;
    // example:              DurationOfLife.value_in.alive = alive;\n
    c += in_agentvar_name() + " = " + pp_agentvar->name + ";";
    return c;
}


