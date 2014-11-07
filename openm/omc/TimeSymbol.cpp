/**
* @file    TimeSymbol.cpp
* Definitions for the TimeSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "TimeSymbol.h"
#include "CodeBlock.h"

using namespace std;

bool TimeSymbol::is_wrapped()
{
    return time_type == token::TK_float || time_type == token::TK_double || time_type == token::TK_ldouble;
}

CodeBlock TimeSymbol::cxx_declaration_global()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_global();

    // Perform operations specific to this level in the Symbol hierarchy.

    // The NumericSymbol upwards in the hierarchy does not generated a 
    // typedef for Time, because it requires special treatment.
    // 
    string typedef_string;
    if (is_wrapped()) {
        // wrap the floating point type in the typedef
        typedef_string = "fixed_precision_float<" + token_to_string(time_type) + ">";
    }
    else {
        typedef_string = token_to_string(time_type);
    }

    h += "typedef " + typedef_string + " Time;";
    h += "typedef " + typedef_string + " TIME; // For Modgen source compatibility";
    // Time 'literals'
    h += "extern const Time time_infinite;";
    h += "extern const Time TIME_INFINITE; // For Modgen source compatibility";
    h += "extern const Time time_undef;";
    h += "extern const Time TIME_UNDEF; // For Modgen source compatibility";

    return h;
}

CodeBlock TimeSymbol::cxx_definition_global()
{
    // Hook into the hierarchical calling chain
    CodeBlock c = super::cxx_definition_global();

    // Perform operations specific to this level in the Symbol hierarchy.

    // Time 'literals'
    c += "const Time time_infinite = 32767;";
    c += "const Time TIME_INFINITE = 32767; // for Modgen models";
    c += "const Time time_undef = -1;";
    c += "const Time TIME_UNDEF = -1; // for Modgen models";
    c += "";

    return c;
}

// static
TimeSymbol * TimeSymbol::find()
{
    auto ts = dynamic_cast<TimeSymbol *>(get_symbol(token_to_string(token::TK_Time)));
    assert(ts); // only called when valid
    return ts;
}
