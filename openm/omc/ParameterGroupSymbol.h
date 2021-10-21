/**
* @file    ParameterGroupSymbol.h
* Declarations for the ParameterGroupSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include "GroupSymbol.h"

/**
* ParameterGroupSymbol.
*
*/
class ParameterGroupSymbol : public GroupSymbol
{
private:
    typedef GroupSymbol super;

public:
    bool is_base_symbol() const { return false; }

    /**
    * Morphing constructor.
    *
    * @param [in,out]  sym The symbol to be morphed.
    */
    ParameterGroupSymbol(Symbol *sym, omc::location decl_loc = omc::location())
        : GroupSymbol(sym, decl_loc)
    {
    }

    void post_parse(int pass);

    void populate_metadata(openm::MetaModelHolder& metaRows);

    /**
     * Determine if group contains a scenario parameter
     *
     * @return true if group contains a scenario parameter, otherwise false
     */
    const bool contains_scenario_parameter();
};
