/**
* @file    ModelSymbol.h
* Declarations for the ModelSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

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

    ModelSymbol(string nm)
        : Symbol(nm)
    {
    }

    CodeBlock cxx_definition_global();

    void populate_metadata(openm::MetaModelHolder & metaRows);

    string time_stamp;
};

