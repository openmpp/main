/**
* @file    ScenarioSymbol.h
* Declarations for the ScenarioSymbol class.
*/
// Copyright (c) 2013-2016 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include "Symbol.h"

/**
* Symbol for the published scenario.
*
* A single instance of ScenarioSymbol exists in the symbol table.
*
*/
class ScenarioSymbol : public Symbol
{
private:
    typedef Symbol super;

public:
    bool is_base_symbol() const { return false; }

    ScenarioSymbol(string nm, string enm)
        : Symbol(nm)
        , external_name(enm)
    {
    }

    void post_parse(int pass);

    /**
     * The external name of the published scenario.
     * 
     * The external name of the published scenario
     * is given by the -s option when omc is invoked.
     */
    string external_name;
};

