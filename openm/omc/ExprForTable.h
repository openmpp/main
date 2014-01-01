/**
 * @file    ExprForTable.h
 * Declares and implements the class ExprForTable which represents elements in an openM++ table expression
 */
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "Symbol.h"

class TableAccumulatorSymbol;

using namespace std;

/**
 * Table expression tree element.
 * 
 * An expression in an openM++ table is parsed into a tree whose nodes and leaves are 
 * instances of ExprForTable.  Each such instance represents a binary operator
 * or a terminal leaf of the tree.
 */

class ExprForTable {
public:

	virtual ~ExprForTable()
    {
    	//TODO traverse tree recursively and destroy (but why bother?)
    }
};

class ExprForTableOp : public ExprForTable {
public:

    /**
     * Constructor for a binary operator in a table expression tree.
     *
     * @param   op      The token for the binary operator, e.g. token::TK_PLUS.
     * @param   left    The left argument.
     * @param   right   The right argument.
     */

	ExprForTableOp(token_type op, ExprForTable *left, ExprForTable *right)
	    : op ( op )
	    , left ( left )
	    , right ( right )
    {
    }

	const token_type op;
	ExprForTable *left;
	ExprForTable *right;
};

class ExprForTableLeaf : public ExprForTable {
public:

    /**
     * Constructor for a leaf (terminal) in a table expression tree.
     *
     * @param   accumulator The accumulator being tabulated.
     */

	ExprForTableLeaf(const Symbol *accumulator)
	    : accumulator ( accumulator->get_rpSymbol() )
        , pp_accumulator ( nullptr )
    {
    }

	Symbol*& accumulator;
	TableAccumulatorSymbol *pp_accumulator;
};
