/**
* @file    LanguageSymbol.h
* Declarations for the LanguageSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

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
    * Morphing constructor.
    *
    * @param [in,out]  sym Symbol to be morphed
    * @param   deflang     true if this is the model's default language.
    */
    explicit LanguageSymbol(Symbol *sym, yy::location decl_loc = yy::location())
        : Symbol(sym, decl_loc)
    {
        language_id = next_language_id;
        next_language_id++;
        name_to_id.emplace(name, language_id);
        id_to_sym.push_back(this);
    }

    void post_parse(int pass);

    /**
     * Number of languages in the model
     *
     * @return An int.
     */
    static int number_of_languages()
    {
        return next_language_id;
    }

    /**
     * Identifier for the language.
     */
    int language_id;

    /**
     * Gets the default language for hte model
     *
     * @return The default language.
     */
    static string default_language()
    {
        assert(id_to_sym.size() > 0);
        return id_to_sym[0]->name;
    }

    /**
     * language_id for the next LanguageSymbol
     * 
     * A shared counter used to assign sequentially increasing identifier to each LanguageSymbol.
     */
    static int next_language_id;

    /**
     * Map from language name to language id
     */
    static map<string, int> name_to_id;

    /**
     * Vector from language id to language symbol
     */
    static vector<LanguageSymbol *> id_to_sym;
};

