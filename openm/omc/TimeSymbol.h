/**
* @file    TimeSymbol.h
* Declarations for the TimeSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include "NumericSymbol.h"

class TimeSymbol : public NumericSymbol
{
private:
    typedef NumericSymbol super;

public:
    bool is_base_symbol() const override { return false; }

    /**
     * Constructor.
     *
     * @param kw1 Keyword #1 for the associated typedef, e.g. token::KW_double.
     */
    TimeSymbol(token_type time_type)
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

    CodeBlock cxx_declaration_global() override;

    CodeBlock cxx_definition_global() override;

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

    /**
    * Create new Time Constant from 'i_value' literal.
    *
    * @return pointer to a new Constant or nullptr on error.
    */
    Constant * make_constant(const string & i_value) const override;

    /**
     * Formats the constant for the data store.
     *
     * @param k The constant to be formatted.
     *
     * @return The formatted for storage.
     */
    string format_for_storage(const Constant & k) const override;
};
