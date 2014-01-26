/**
* @file    LanguageSymbol.h
* Declarations for the LanguageSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "Symbol.h"


/**
 * Symbol for a supported human language in the model.
 * 
 * Each of the comma-separated language codes in the 'languages' statement becomes a
 * LanguageSymbol in the symbol table.  Languages are assigned unique numeric identifiers when
 * the corresponging LanguageSymbol is created.  This corresponds to the order in the languages
 * statement.  The first language in the languages statement becomes the default language for
 * the model.
 */

class LanguageSymbol : public Symbol
{
private:
    typedef Symbol super;

public:
    bool is_base_symbol() const { return false; }


    /**
    * Constructor.
    *
    * @param [in,out]  sym Symbol to be morphed
    * @param   deflang     true if this is the model's default language.
    */
    LanguageSymbol(Symbol *sym, yy::location decl_loc = yy::location())
        : Symbol(sym, decl_loc)
    {
        language_id = next_language_id;
        next_language_id++;
    }

    void post_parse(int pass);


    /**
    * Identifier for the language.
    */

    int language_id;


    /**
    * language_id for the next LanguageSymbol
    *
    * A shared counter used to assign sequentially increasing type_id to each LanguageSymbol.
    */

    static int next_language_id;
};

