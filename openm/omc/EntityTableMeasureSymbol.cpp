/**
* @file    EntityTableMeasureSymbol.h
* Definitions for the EntityTableMeasureSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include "EntityTableAccumulatorSymbol.h"
#include "EntityTableMeasureSymbol.h"
#include "EntityTableSymbol.h"
#include "ExprForTable.h"
#include "Literal.h"

// static
string EntityTableMeasureSymbol::symbol_name(const Symbol* table, int index)
{
    assert(table);
    return table->name + ".Expr" + to_string(index);
}

/**
* Post-parse operations for EntityTableMeasureSymbol
*/

void EntityTableMeasureSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case eAssignMembers:
    {
        // assign direct pointer to table for post-parse use
        pp_table = dynamic_cast<EntityTableSymbol *> (pp_symbol(table));
        assert(pp_table); // parser guarantee

        // Add this table expression to the table's list of expressions
        pp_table->pp_expressions.push_back(this);

        // Perform post-parse operation to each element of the expression
        post_parse_traverse(root);
        break;
    }
    default:
        break;
    }
}

/**
* Perform post-parse operations on nodes of a ExprForTable tree
*
* Assigns the pp_accumulator pointer using the assumulator member from the parse phase.
* 
* @param   node    The root of the expression tree.
*
* @return  Result as a \a CodeBlock
*/

void EntityTableMeasureSymbol::post_parse_traverse(ExprForTable *node)
{
    auto acc = dynamic_cast<ExprForTableAccumulator *>(node);
    auto lit = dynamic_cast<const ExprForTableLiteral *>(node);
    if (acc != nullptr) {
        // found an accumulator leaf of the expression tree
        (acc->pp_accumulator) = dynamic_cast<EntityTableAccumulatorSymbol *>(acc->accumulator);
        assert(acc->pp_accumulator); // parser guarantee
    }
    else if (lit != nullptr) {
        // Literals can be left as-is for the post-parse processing phase.
    }
    else {
        // an operator
        auto op = dynamic_cast<ExprForTableOp *>(node);
        assert(op); // parser guarantee
        if (op->left) {
            post_parse_traverse(op->left);
        }
        else {
            // unary operators have no left argument
        }
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

string EntityTableMeasureSymbol::get_expression(const ExprForTable *node, expression_style style)
{
    const ExprForTableAccumulator* acc = dynamic_cast<const ExprForTableAccumulator *>(node);
    const ExprForTableLiteral* lit = dynamic_cast<const ExprForTableLiteral *>(node);

    if (acc != nullptr) {
        // Node is a terminal accumulator leaf of the expression tree
        const EntityTableAccumulatorSymbol *accumulator = acc->pp_accumulator;
        string result;
        switch (style) {
        case cxx:
            result = "acc[" + to_string(accumulator->index) + "][cell]";
            break;
        case sql:
            {
                string agg_func = "";
                switch (accumulator->accumulator) {
                case token::TK_sum:
                    agg_func = "OM_SUM";
                    break;
                case token::TK_minimum:
                    agg_func = "OM_MIN";
                    break;
                case token::TK_maximum:
                    agg_func = "OM_MAX";
                    break;
                default:
                    // Other accumulators cannot be composited, so average
                    agg_func = "OM_AVG";
                    break;
                }
                result = agg_func + "( acc" + to_string(accumulator->index) + " )";
                break;
            }
            break;
        default:
            assert(0);
        }
        return result;
    }
    else if (lit != nullptr) {
        // Node is a terminal numeric literal leaf of the expression tree
        return lit->constant->value();
    }
    else {
        auto binary_node = dynamic_cast<const ExprForTableOp *>(node);
        assert(binary_node); // parser guarantee
        string expr_left("");
        if (binary_node->left) {
            expr_left = get_expression(binary_node->left, style);
        }
        else
        {
            // unary operators have no left argument
        }
        string expr_right = get_expression(binary_node->right, style);
        return "( " + expr_left + " " + token_to_string(binary_node->op) + " " + expr_right + " )";
    }
}
