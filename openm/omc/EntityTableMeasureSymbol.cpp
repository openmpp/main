/**
* @file    EntityTableMeasureSymbol.h
* Definitions for the EntityTableMeasureSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <cassert>
#include "EntityTableAccumulatorSymbol.h"
#include "EntityTableMeasureSymbol.h"
#include "EntityTableSymbol.h"
#include "ExprForTable.h"
#include "Literal.h"

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
        // Perform post-parse operation to each element of the expression
        post_parse_traverse(root);
        break;
    }
    default:
        break;
    }
}

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
        case sql_aggregated_accumulators:
            {
                string agg_func = "";
                switch (accumulator->accumulator) {
                case token::TK_sum:
                case token::TK_unit:
                    agg_func = "OM_AVG";
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
        case sql_accumulators:
            {
                result = "acc" + to_string(accumulator->index);
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
        if (binary_node->op == token::TK_sqrt) {
            return "sqrt( " + expr_right + " )";
        }
        else {
            return "( " + expr_left + " " + token_to_string(binary_node->op) + " " + expr_right + " )";
        }
    }
}

string EntityTableMeasureSymbol::heuristic_short_name(void) const
{
    return "h_" + short_name;
}
