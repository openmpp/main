/**
* @file    ModelSymbol.h
* Declarations for the ModelSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include "Symbol.h"

/**
* Symbol for the model itself.
*
* A single instance of ModelSymbol exists in the symbol table.
*
*/
class ModelSymbol : public Symbol
{
private:
    typedef Symbol super;

public:
    bool is_base_symbol() const { return false; }

    ModelSymbol(string nm, string enm)
        : Symbol(nm)
        , external_name(enm)
    {
    }

    CodeBlock cxx_definition_global();

    void populate_metadata(openm::MetaModelHolder & metaRows);

    /**
     * The external name of the published scenario.
     * 
     * The external name of the published scenario
     * is given by the -s option when omc is invoked.
     */
    string external_name;
};

