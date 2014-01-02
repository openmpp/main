/**
* @file    ParameterSymbol.h
* Definitions for the ParameterSymbol class.
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "ParameterSymbol.h"
#include "CodeBlock.h"

using namespace std;

void ParameterSymbol::post_parse(int pass)
{
    // Hook into the hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case 1:
        // add this parameter to the complete list of parameters
        pp_parameters.push_back(this);
        break;
    default:
        break;
    }
}

CodeBlock ParameterSymbol::cxx_declaration()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration();

    // Perform operations specific to this level in the Symbol hierarchy.
    string parm_type = Symbol::token_to_string(type);
    h += "extern " + parm_type + " " + name + ";";
    return h;
}

CodeBlock ParameterSymbol::cxx_definition()
{
    // Hook into the hierarchical calling chain
    CodeBlock c = super::cxx_declaration();

    // Perform operations specific to this level in the Symbol hierarchy.
    string parm_type = Symbol::token_to_string(type);
    c += parm_type + " " + name + " = 0.0;";
    return c;
}
