/**
* @file    AgentMemberSymbol.cpp
* Definitions for the AgentMemberSymbol class.
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include <algorithm>
#include <typeinfo>
#include <set>
#include "ast.h"
#include "parser_helper.h"

using namespace std;

void AgentMemberSymbol::post_parse(int pass)
{
    // First perform post-parse operations at next level up in Symbol hierarchy
    super::post_parse(pass);

    switch (pass) {
    case 1:
        // assign direct pointer to agent for use post-parse
        pp_agent = dynamic_cast<AgentSymbol *> (agent);
        break;
    default:
        break;
    }
}



