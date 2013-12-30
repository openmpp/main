/**
* @file    AgentMemberSymbol.h
* Declarations for the AgentMemberSymbol class.
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "Symbol.h"

using namespace std;

class AgentMemberSymbol : public Symbol
{
private:
    typedef Symbol super;

public:
    AgentMemberSymbol(Symbol *sym, const Symbol *agent)
        : Symbol(sym)
        , agent(agent->get_rpSymbol())
        , pp_agent(nullptr)
    {
    }

    AgentMemberSymbol(const string name, const Symbol *agent)
        : Symbol(name, agent)
        , agent(agent->get_rpSymbol())
        , pp_agent(nullptr)
    {
    }

    void post_parse(int pass);

    /**
    * Reference to pointer to agent.
    *
    * Stable to symbol morhing during parse phase.
    */

    Symbol*& agent;

    /**
    * Direct pointer to agent.
    *
    * Set post-parse for convenience.
    */

    AgentSymbol *pp_agent;
};


