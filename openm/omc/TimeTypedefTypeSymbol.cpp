/**
* @file    TimeTypedefTypeSymbol.cpp
* Definitions for the TimeTypedefTypeSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "TimeTypedefTypeSymbol.h"
#include "CodeBlock.h"

using namespace std;

CodeBlock TimeTypedefTypeSymbol::cxx_declaration_global()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_global();

    // Perform operations specific to this level in the Symbol hierarchy.

    // only one keyword for Time
    string typedef_string = token_to_string(keywords.front());

    h += "typedef " + typedef_string + " TIME; // For Modgen source compatibility";
    // Time 'literals'
    h += "extern const Time time_infinite;";
    h += "extern const Time TIME_INFINITE; // For Modgen source compatibility";

    return h;
}

CodeBlock TimeTypedefTypeSymbol::cxx_definition_global()
{
    // Hook into the hierarchical calling chain
    CodeBlock c = super::cxx_definition_global();

    // Perform operations specific to this level in the Symbol hierarchy.

    // Time 'literals'
    c += "const Time time_infinite = 32767;";
    c += "const Time TIME_INFINITE = 32767; // for Modgen models";
    c += "";

    return c;
}

