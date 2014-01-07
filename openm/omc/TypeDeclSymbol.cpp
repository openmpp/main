/**
* @file    TypeDeclSymbol.cpp
* Definitions for the TypeDeclSymbol class.
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "TypeDeclSymbol.h"
#include "CodeBlock.h"

using namespace std;

void TypeDeclSymbol::post_parse(int pass)
{
    // Hook into the hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case 1:
        // add this type to the complete list of types
        pp_all_types.push_back(this);
        break;
    default:
        break;
    }
}

CodeBlock TypeDeclSymbol::cxx_declaration_global()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_global();

    // Perform operations specific to this level in the Symbol hierarchy.

    string typ = Symbol::token_to_string(value);
    // E.g. typedef double real;
    h += "typedef " + typ + " " + name + ";";
    if (name == Symbol::token_to_string(token::TK_Time)) {
        h += "typedef " + typ + " TIME; // for Modgen models";
        // Time 'literals'
        h += "extern const Time time_infinite;";
        h += "extern const Time TIME_INFINITE; // for Modgen models";
        h += "";
    }
    return h;
}

CodeBlock TypeDeclSymbol::cxx_definition_global()
{
    // Hook into the hierarchical calling chain
    CodeBlock c = super::cxx_definition_global();

    // Perform operations specific to this level in the Symbol hierarchy.
    if (name == Symbol::token_to_string(token::TK_Time)) {
        // Time 'literals'
        c += "const Time time_infinite = 32767;";
        c += "const Time TIME_INFINITE = 32767; // for Modgen models";
        c += "";
    }
    return c;
}

