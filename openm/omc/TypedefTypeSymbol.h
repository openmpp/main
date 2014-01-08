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
* e.g. TK_Time for the time_type statement, TK_index to the index_type statement.
* The one exception is the model_type statement which has an associated TypedefTypeSymbol with 'type' TK_model_type.
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
    * @param   type    The token for the keyword of the type, e.g. token::KW_Time
    * @param   value   The token for the associated type, e.g. token::KW_double
    */
    TypedefTypeSymbol(token_type type, token_type value)
        : TypeSymbol(token_to_string(type))
        , value(value)
    {
    }

    void post_parse(int pass);

    CodeBlock cxx_declaration_global();

    CodeBlock cxx_definition_global();

    /** The C++ type of the given openM++ type */
    token_type value;
};
