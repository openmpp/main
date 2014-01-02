/**
* @file    AgentCallbackMemberSymbol.cpp
* Definitions for the AgentCallbackMemberSymbol class.
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "AgentCallbackMemberSymbol.h"
#include "CodeBlock.h"

using namespace std;

void AgentCallbackMemberSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Now do operations specific to this level in the Symbol hierarchy
    switch (pass) {
    case 1:
        // Nothing
        break;
    default:
        break;
    }
}

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


