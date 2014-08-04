/**
* @file    ConstantSymbol.h
* Declarations for the ConstantSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "Symbol.h"

class EnumeratorSymbol;


/**
* ConstantSymbol - Refers to either a Literal or an EnumeratorSymbol
*
*/
class ConstantSymbol : public Symbol
{
private:
    typedef Symbol super;

public:
    bool is_base_symbol() const { return false; }

    /**
    * Constructor for a ConstantSymbol representing an enumerator
    *
    * @param [in,out]  sym The symbol to be morphed.
    */
    ConstantSymbol(Symbol *enumerator, yy::location decl_loc = yy::location())
        : Symbol(next_symbol_name(), decl_loc)
        , is_enumerator(true)
        , enumerator(enumerator->stable_pp())
        , pp_enumerator(nullptr)
        , is_literal(false)
        , literal(nullptr)
    {
    }

    /**
    * Constructor for a ConstantSymbol representing a Literal
    *
    * @param [in,out]  sym The symbol to be morphed.
    */
    ConstantSymbol(Literal *literal, yy::location decl_loc = yy::location())
        : Symbol(next_symbol_name(), decl_loc)
        , is_enumerator(false)
        , enumerator(nullptr)
        , pp_enumerator(nullptr)
        , is_literal(true)
        , literal(literal)
    {
    }

    /**
     * true if this object represents an Enumerator.
     */
    const bool is_enumerator;

    /**
     * Pointer to pointer to enumerator
     * 
     * Stable to symbol morphing during parse phase.
     */
    Symbol** enumerator;

    /**
     * Direct pointer to enumerator
     * 
     * For use post-parse.
     */
    EnumeratorSymbol *pp_enumerator;

    /**
     * true if this object represents a Literal.
     */
    const bool is_literal;

    /**
     * The literal.
     */
    Literal *literal;

    /**
     * Gets the constant value as a string
     *
     * @return A string.
     */
    const string value() const;

    /**
     * Generated name for the next instance of a ConstantSymbol
     *
     * @return A string.
     */
    static string next_symbol_name();

    void post_parse(int pass);

    /**
     * Counter of instances used to generate unique names
     */
    static int instance_counter;
};
