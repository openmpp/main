/**
 * @file    AgentMultiLinkSymbol.cpp
 * Definitions for the AgentMultiLinkSymbol class.
 */
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "AgentMultiLinkSymbol.h"
#include "AgentSymbol.h"
#include "CodeBlock.h"

using namespace std;

void AgentMultiLinkSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case ePopulateCollections:
        break;
    default:
        break;
    }
}

CodeBlock AgentMultiLinkSymbol::cxx_declaration_agent()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_agent();

    // Perform operations specific to this level in the Symbol hierarchy.

    return h;
}


CodeBlock AgentMultiLinkSymbol::cxx_definition_agent()
{
    // Hook into the hierarchical calling chain
    CodeBlock c = super::cxx_definition_agent();

    // Perform operations specific to this level in the Symbol hierarchy.

    return c;
}

