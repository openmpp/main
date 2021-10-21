/**
* @file    ExtendParameterSymbol.h
* Declarations for the ExtendParameterSymbol class.
*/
// Copyright (c) 2013-2018 OpenM++ Contributors
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include "Symbol.h"

/**
 * Symbol associated with an extend_parameter statement
 */

class ExtendParameterSymbol : public Symbol
{
private:
    typedef Symbol super;

public:
    bool is_base_symbol() const { return false; }

    explicit ExtendParameterSymbol(Symbol *targ_param, Symbol *index_param, omc::location decl_loc = omc::location())
        : Symbol(symbol_name(targ_param), decl_loc)
        , target_parameter(targ_param->stable_rp())
        , pp_target_parameter(nullptr)
        , index_parameter(index_param ? index_param->stable_pp() : nullptr)
        , pp_index_parameter(nullptr)
    {
    }

    // Construct symbol name for the extend_parameter symbol.
    static string symbol_name(const Symbol *param);

    void post_parse(int pass);

    /**
     * The parameter to be extended
     *
     * Stable to symbol morphing during parse phase.
     */
    Symbol*& target_parameter;

    /**
     * The parameter to be extended
     *
     * Only valid after post-parse phase 1.
     */
    ParameterSymbol* pp_target_parameter;

    /**
     * The parameter holding the index series for growth (pointer to pointer)
     *
     * Stable to symbol morphing during parse phase.
     */
    Symbol** index_parameter;

    /**
     * The parameter holding the index series for growth
     *
     * Only valid after post-parse phase 1.
     */
    ParameterSymbol* pp_index_parameter;
};
