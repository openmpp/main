/**
* @file    AgentCallbackMemberSymbol.cpp
* Definitions for the AgentCallbackMemberSymbol class.
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "AgentCallbackMemberSymbol.h"
#include "CodeBlock.h"

using namespace std;

CodeBlock AgentCallbackMemberSymbol::cxx_declaration_agent_scope()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_agent_scope();

    // Perform operations specific to this level in the Symbol hierarchy.

    return h;
}

CodeBlock AgentCallbackMemberSymbol::cxx_definition()
{
    // Hook into the hierarchical calling chain
    CodeBlock c = super::cxx_definition();

    // Perform operations specific to this level in the Symbol hierarchy.

    return c;
}


