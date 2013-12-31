/**
* @file    LanguageSymbol.h
* Declarations for the LanguageSymbol class.
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "Symbol.h"

/**
* Symbol for a supported human language in the model.
*
* Each of the comma-separated language codes in the 'languages' statement
* becomes a LanguageSymbol in the symbol table.  The first language in the
* languages statement becomes the default language for the model.
*/
class LanguageSymbol : public Symbol
{
private:
    typedef Symbol super;

public:

    /**
    * Constructor.
    *
    * @param [in,out]  sym Symbol to be morphed
    * @param   deflang     true if this is the model's default language.
    */
    LanguageSymbol(Symbol *sym, bool deflang)
        : Symbol(sym)
        , default_language(deflang)
    {
    }

    /**
    * Flag indicating if this is the default language.
    */

    bool default_language;
};

