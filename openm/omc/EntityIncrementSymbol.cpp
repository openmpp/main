/**
* @file    EntityIncrementSymbol.cpp
* Definitions for the EntityIncrementSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include <string>
#include "EntityIncrementSymbol.h"
#include "CodeBlock.h"

using namespace std;

void EntityIncrementSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case eAssignMembers:
    {
        break;
    }
    case ePopulateCollections:
    {
        break;
    }
    default:
        break;
    }
}

CodeBlock EntityIncrementSymbol::cxx_declaration_agent()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_agent();

    // Perform operations specific to this level in the Symbol hierarchy.
    return h;
}

CodeBlock EntityIncrementSymbol::cxx_definition_agent()
{
    // Hook into the hierarchical calling chain
    CodeBlock c = super::cxx_definition_agent();

    // Perform operations specific to this level in the Symbol hierarchy.
 
    return c;
}

