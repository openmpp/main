/**
* @file    RealSymbol.h
* Declarations for the RealSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "NumericSymbol.h"

class RealSymbol : public NumericSymbol
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
    RealSymbol(token_type kw1)
        : NumericSymbol(token::TK_real, kw1, "0")
    {
    }

    /**
     * Query if this object is float.
     *
     * @return true if float, false if not.
     */
    bool is_float();

    CodeBlock cxx_declaration_global();

    /**
     * Gets the unique RealSymbol.
     *
     * @return the RealSymbol.
     */
    static RealSymbol * find();
};
