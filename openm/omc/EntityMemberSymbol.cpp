/**
* @file    EntityMemberSymbol.cpp
* Definitions for the EntityMemberSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <cassert>
#include "EntityMemberSymbol.h"
#include "EntitySymbol.h"

using namespace std;

void EntityMemberSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case eAssignMembers:
        {
            // assign direct pointer to agent for use post-parse
            pp_agent = dynamic_cast<EntitySymbol *> (pp_symbol(agent));
            assert(pp_agent); // parser guarantee
        }
        break;
    default:
        break;
    }
}



