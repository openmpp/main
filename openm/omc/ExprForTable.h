/**
 * @file    ExprForTable.h
 * Declares and implements the class ExprForTable which represents elements in an openM++ table expression
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include "Symbol.h"

class EntityTableAccumulatorSymbol;
class Literal;

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

class ExprForTableAccumulator : public ExprForTable {
public:

    /**
     * Constructor for an accumulator leaf (terminal) in a table expression tree.
     *
     * @param   accumulator The accumulator being tabulated.
     */

	ExprForTableAccumulator(const Symbol *accumulator)
	    : accumulator ( accumulator->stable_rp() )
        , pp_accumulator ( nullptr )
    {
    }

	Symbol*& accumulator;
	EntityTableAccumulatorSymbol *pp_accumulator;
};

class ExprForTableLiteral : public ExprForTable {
public:

    /**
     * Constructor for an accumulator literal (terminal) in a table expression tree.
     *
     * @param   accumulator The accumulator being tabulated.
     */

	ExprForTableLiteral(const Literal *constant)
	    : constant (constant)
    {
    }

    const Literal *constant;
};
