/**
 * @file    ExprForAttribute.h
 * Declares and implements the class ExprForAttribute which represents elements in an openM++ attribute expression
 */
// Copyright (c) 2013-2016 OpenM++ Contributors
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include "Symbol.h"

class Literal;

using namespace std;


/**
 * Expression tree element in an identity attribute.
 * 
 * An expression in an openM++ identity attribute specification or filter is parsed into a tree
 * whose nodes and leaves are instances of ExprForAttribute.  Each such instance represents an
 * operator or a terminal leaf of the tree.
 */

class ExprForAttribute {
public:

	virtual ~ExprForAttribute()
    {
    	//TODO traverse tree recursively and destroy (but why bother?)
    }

    /**
     * true if expression or subexpression contains a pointer operator
     */
    virtual bool uses_pointer() const = 0;
};

class ExprForAttributeUnaryOp : public ExprForAttribute {
public:

    /**
     * Constructor for a unary operator in an identity attribute expression tree.
     *
     * @param   op      The token for the unary operator, e.g. token::TK_MINUS.
     * @param   right   The right argument.
     */
	ExprForAttributeUnaryOp(token_type op, ExprForAttribute *right)
	    : op ( op )
	    , right ( right )
    {
    }

    bool uses_pointer() const
    {
        // TK_STAR is unary * in this context
        return (op == token_type::TK_STAR) || right->uses_pointer();
    }

	const token_type op;
	ExprForAttribute *right;
};

class ExprForAttributeBinaryOp : public ExprForAttribute {
public:

    /**
     * Constructor for a binary operator in an identity attribute expression tree.
     *
     * @param   op      The token for the binary operator, e.g. token::TK_PLUS.
     * @param   left    The left argument.
     * @param   right   The right argument.
     */
	ExprForAttributeBinaryOp(token_type op, ExprForAttribute *left, ExprForAttribute *right)
	    : op ( op )
	    , left ( left )
	    , right ( right )
    {
    }

    bool uses_pointer() const
    {
        // TK_MEMBER_OF_POINTER is binary operator ->
        return (op == token_type::TK_MEMBER_OF_POINTER) || left->uses_pointer() || right->uses_pointer();
    }

	const token_type op;
	ExprForAttribute *left;
	ExprForAttribute *right;
};

class ExprForAttributeTernaryOp : public ExprForAttribute {
public:

    /**
     * Constructor for the ternary operator in an identity attribute expression tree.
     *
     * @param [in,out] cond   The condition argumnet of the C ternary opertor.
     * @param [in,out] first  The result if true.
     * @param [in,out] second The result if false.
     */
	ExprForAttributeTernaryOp(ExprForAttribute *cond, ExprForAttribute *first, ExprForAttribute *second)
	    : cond ( cond )
	    , first ( first )
	    , second ( second )
    {
    }

    bool uses_pointer() const
    {
        return cond->uses_pointer() || first->uses_pointer() || second->uses_pointer();
    }

    ExprForAttribute *cond;
	ExprForAttribute *first;
	ExprForAttribute *second;
};

class ExprForAttributeSymbol : public ExprForAttribute {
public:

    /**
     * Constructor for a symbol (terminal) in an identity attribute expression tree.
     *
     */
	ExprForAttributeSymbol(const Symbol *symbol)
	    : symbol ( symbol->stable_rp() )
        , pp_symbol ( nullptr )
    {
    }

    bool uses_pointer() const
    {
        // detect use of omc hack which creates "A->B" as a name
        // for an anonymous identity attribute.
        if (symbol->name.find("->") != std::string::npos) {
            // symbol 'name' contains "->"
            return true;
        }
        else {
            return false;
        }
    }

	Symbol*& symbol;
	Symbol* pp_symbol;
};

class ExprForAttributeLiteral : public ExprForAttribute {
public:

    /**
     * Constructor for a constant literal (terminal) in an identity attribute expression tree.
     *
     * @param constant 
     */
	ExprForAttributeLiteral(const Literal *constant)
	    : constant (constant)
    {
    }

    bool uses_pointer() const
    {
        return false;
    }

    const Literal *constant;
};
