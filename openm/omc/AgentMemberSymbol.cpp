/**
* @file    AgentMemberSymbol.cpp
* Definitions for the AgentMemberSymbol class.
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include "AgentMemberSymbol.h"
#include "AgentSymbol.h"

using namespace std;

void AgentMemberSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case ePopulateCollections:
        {
            // assign direct pointer to agent for use post-parse
            pp_agent = dynamic_cast<AgentSymbol *> (agent);
            assert(pp_agent); // parser guarantee
        }
        break;
    default:
        break;
    }
}



