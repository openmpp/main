/**
 * @file    ExprForAgentVar.h
 * Declares and implements the class ExprForAgentVar which represents elements in an openM++ agentvar expression
 */
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "Symbol.h"

class Literal;

using namespace std;


/**
 * AgentVar expression tree element.
 * 
 * An expression in an openM++ expression agentvar specification or filter is parsed into a tree
 * whose nodes and leaves are instances of ExprForAgentVar.  Each such instance represents an
 * operator or a terminal leaf of the tree.
 */

class ExprForAgentVar {
public:

	virtual ~ExprForAgentVar()
    {
    	//TODO traverse tree recursively and destroy (but why bother?)
    }
};

class ExprForAgentVarUnaryOp : public ExprForAgentVar {
public:

    /**
     * Constructor for a unary operator in a table expression tree.
     *
     * @param   op      The token for the binary operator, e.g. token::TK_PLUS.
     * @param   left    The left argument.
     * @param   right   The right argument.
     */

	ExprForAgentVarUnaryOp(token_type op, ExprForAgentVar *right)
	    : op ( op )
	    , right ( right )
    {
    }

	const token_type op;
	ExprForAgentVar *right;
};

class ExprForAgentVarBinaryOp : public ExprForAgentVar {
public:

    /**
     * Constructor for a binary operator in an agenvar expression tree.
     *
     * @param   op      The token for the binary operator, e.g. token::TK_PLUS.
     * @param   left    The left argument.
     * @param   right   The right argument.
     */

	ExprForAgentVarBinaryOp(token_type op, ExprForAgentVar *left, ExprForAgentVar *right)
	    : op ( op )
	    , left ( left )
	    , right ( right )
    {
    }

	const token_type op;
	ExprForAgentVar *left;
	ExprForAgentVar *right;
};

class ExprForAgentVarTernaryOp : public ExprForAgentVar {
public:

    /**
     * Constructor for the ternary operator in an agentvar expression tree.
     *
     * @param op             The token for the binary operator, e.g. token::TK_PLUS.
     * @param [in,out] left  The left argument.
     * @param [in,out] right The right argument.
     */

	ExprForAgentVarTernaryOp(ExprForAgentVar *cond, ExprForAgentVar *first, ExprForAgentVar *second)
	    : cond ( cond )
	    , first ( first )
	    , second ( second )
    {
    }

	ExprForAgentVar *cond;
	ExprForAgentVar *first;
	ExprForAgentVar *second;
};

class ExprForAgentVarSymbol : public ExprForAgentVar {
public:

    /**
     * Constructor for a symbol (terminal) in an agentvar expression tree.
     *
     */

	ExprForAgentVarSymbol(const Symbol *symbol)
	    : symbol ( symbol->stable_rp() )
        , pp_symbol ( nullptr )
    {
    }

	Symbol*& symbol;
	Symbol* pp_symbol;
};

class ExprForAgentVarLiteral : public ExprForAgentVar {
public:

    /**
     * Constructor for a constant literal (terminal) in an agentvar expression tree.
     *
     * @param constant 
     */

	ExprForAgentVarLiteral(const Literal *constant)
	    : constant (constant)
    {
    }

    const Literal *constant;
};
