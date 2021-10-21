/**
* @file    Constant.h
* Declarations for the Constant class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include "Symbol.h"
#include "Literal.h"
#include "omc_location.hh"

/**
* Constant - Refers to either a Literal or an EnumeratorSymbol
*
*/
class Constant
{
public:

    /**
    * Constructor for a Constant representing an enumerator
    *
    * @param [in,out]  sym The symbol to be morphed.
    */
    Constant(Symbol *enumerator, omc::location decl_loc = omc::location())
        : is_enumerator(true)
        , enumerator(enumerator->stable_pp())
        , is_literal(false)
        , literal(nullptr)
        , decl_loc(decl_loc)
    {
    }

    /**
    * Constructor for a Constant representing a Literal
    *
    * @param [in,out]  sym The symbol to be morphed.
    */
    Constant(Literal *literal, omc::location decl_loc = omc::location())
        : is_enumerator(false)
        , enumerator(nullptr)
        , is_literal(true)
        , literal(literal)
        , decl_loc(decl_loc)
    {
    }

    /**
     * true if this object represents an Enumerator.
     * 
     * If true, then is_literal is false.
     */
    const bool is_enumerator;

    /**
     * Pointer to pointer to enumerator
     * 
     * Stable to symbol morphing during parse phase.
     */
    Symbol** enumerator;

    /**
     * true if this object represents a Literal.
     * 
     * If true, then is_enumerator is false.
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
     * Transform the constant value to a string which can be part of an identifier
     *
     * @return A string.
     */
    const string value_as_name() const;

    /**
     * true if the constant is valid for the given type.
     *
     * @param type The type against which the constant is checked for validity.
     *
     * @return true if valid, else false.
     */
    bool is_valid_constant(const TypeSymbol &type) const;

    /**
     * Formats the constant for the data store.
     *
     * @param type The type used to transform to a 
     *
     * @return true if valid, else false.
     */
    string format_for_storage(const TypeSymbol &type) const;

    /**
     * The declaration location.
     * 
     * Set to the source location of the constant during parsing.
     */
    omc::location decl_loc;
};
