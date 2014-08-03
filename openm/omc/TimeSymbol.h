/**
* @file    TimeSymbol.h
* Declarations for the TimeSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "NumericSymbol.h"

class TimeSymbol : public NumericSymbol
{
private:
    typedef NumericSymbol super;

public:
    bool is_base_symbol() const { return false; }

    /**
     * Constructor.
     *
     * @param kw1 Keyword #1 for the associated typedef, e.g. token::KW_double.
     */
    TimeSymbol(token_type wrapped_type)
        : NumericSymbol(token::TK_Time, "0")
        , time_type(time_type)
    {
    }

    /**
     * Query if Time is wrapped, i.e. is floating point.
     *
     * @return true if wrapped, false if not.
     */
    bool is_wrapped();

    CodeBlock cxx_declaration_global();

    CodeBlock cxx_definition_global();

    /**
     * Type used for time, ex. TK_double.
     */
    token_type time_type;

    /**
     * Gets the unique TimeSymbol.
     *
     * @return null if it fails, else the TimeSymbol.
     */
    static TimeSymbol * find();
};
