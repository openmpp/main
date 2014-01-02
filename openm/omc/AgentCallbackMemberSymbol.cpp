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
    // First get declaration code at next level up in the Symbol hierarchy
    CodeBlock h = super::cxx_declaration_agent_scope();

    // Now add declaration code specific to this level in the Symbol hierarchy

    return h;
}

CodeBlock AgentCallbackMemberSymbol::cxx_definition()
{
    // First get definition code at next level up in the Symbol hierarchy
    CodeBlock c = super::cxx_definition();

    // Now add definition code specific to this level in the Symbol hierarchy

    return c;
}


