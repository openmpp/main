/**
* @file    ModuleSymbol.h
* Declarations for the ModuleSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "Symbol.h"

/**
* Symbol for a source code module.
*
* The name of a ModuleSymbol is the name of the model source code file,
* including the .mpp or .ompp file extension.
*/
class ModuleSymbol : public Symbol
{
private:
    typedef Symbol super;

public:
    bool is_base_symbol() const { return false; }

    /**
    * Constructor.
    *
    * Unlike most symbols, a ModuleSymbol is created outside of
    * the parser and scanner.
    *
    * @param   nm  The name of the module.
    *
    */
    ModuleSymbol(string nm)
        : Symbol(nm)
    {
    }
};

