/**
* @file    AgentInternalSymbol.cpp
* Definitions for the AgentInternalSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "AgentInternalSymbol.h"
#include "TypeSymbol.h"
#include "CodeBlock.h"

using namespace std;

CodeBlock AgentInternalSymbol::cxx_initialization_expression(bool type_default) const
{
    CodeBlock c;
    // example:              om_in_DurationOfLife_alive = false;\n
    c += name + " = " + initialization_value(type_default) + ";";
    return c;
}

string AgentInternalSymbol::initialization_value(bool type_default) const
{
    string result;

    if (!type_default && initializer != "") {
        result = initializer;
    }
    else {
        result = super::initialization_value(type_default);
    }

    return result;
}

CodeBlock AgentInternalSymbol::cxx_declaration_agent()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_agent();

    // Perform operations specific to this level in the Symbol hierarchy.
    h += pp_data_type->name + " " + name + ";";
    return h;
}



