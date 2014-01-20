/**
* @file    TypedefTypeSymbol.h
* Declarations for the TypedefTypeSymbol class.
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "TypeSymbol.h"

/**
* TypedefTypeSymbol.
*
* The symbol table is initialized with one TypedefTypeSymbol
* corresponding to each kind of type declaration statement in openM++, e.g. time_type, real_type.
* The symbol type (first argument of constructor) is the token associated with the corresponding type name,
* e.g. TK_Time for the time_type statement, TK_real to the real_type statement.
* These default symbols are morphed if the corresponding statement is found
* in the model source code.
*/
class TypedefTypeSymbol : public TypeSymbol
{
private:
    typedef TypeSymbol super;

public:

    /**
    * Constructor.
    *
    * @param type The token for the keyword of the type, e.g. token::KW_Time.
    */

    TypedefTypeSymbol(token_type type, const string & initial_value)
        : TypeSymbol(token_to_string(type))
        , type(type)
        , initial_value(initial_value)
    {
        Set_keywords();
    }

    /**
    * Constructor.
    *
    * @param type The token for the keyword of the type, e.g. token::KW_Time.
    * @param kw1  Keyword #1 for the associated typedef, e.g. token::KW_double.
    */

    TypedefTypeSymbol(token_type type, token_type kw1, const string & initial_value)
        : TypeSymbol(token_to_string(type))
        , type(type)
        , initial_value(initial_value)
    {
        Set_keywords(kw1);
    }

    /**
    * Constructor.
    *
    * @param type The token for the keyword of the type, e.g. token::KW_uint.
    * @param kw1  Keyword #1 for the associated typedef, e.g. token::KW_unsigned.
    * @param kw2  Keyword #2 for the associated typedef, e.g. token::KW_int.
    */

    TypedefTypeSymbol(token_type type, token_type kw1, token_type kw2, const string & initial_value)
        : TypeSymbol(token_to_string(type))
        , type(type)
        , initial_value(initial_value)
    {
        Set_keywords(kw1, kw2);
    }

    void Set_keywords() {
        keywords.clear();
    }

    void Set_keywords( token_type kw1 ) {
        Set_keywords();
        keywords.push_back(kw1);
    }

    void Set_keywords(token_type kw1, token_type kw2) {
        Set_keywords(kw1);
        keywords.push_back(kw2);
    }

    const string get_initial_value() const {
        return initial_value;
    }

    CodeBlock cxx_declaration_global();

    void populate_metadata(openm::MetaModelHolder & metaRows);


    /**
     * The type, e.g. TK_uint
     */

    token_type type;

    /**
    * The initial value for a quantity of this type.
    */

    const string initial_value;

    /**
     * A list of keywords used to construct the typedef statement, e.g. TK_unsigned, TK_int
     */

    list<token_type> keywords;


    /**
     * Gets the TypedefTypeSymbol for a given type
     *
     * @param type The type.
     *
     * @return null if it fails, else the TypedefTypeymbol.
     */

    static TypedefTypeSymbol *get_typedef_symbol(token_type type);
};
