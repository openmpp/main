/**
* @file    EntityMemberSymbol.h
* Declarations for the EntityMemberSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include "Symbol.h"

using namespace std;

class EntityMemberSymbol : public Symbol
{
private:
    typedef Symbol super;

public:
    bool is_base_symbol() const { return false; }

    EntityMemberSymbol(Symbol *sym, const Symbol *agent, yy::location decl_loc = yy::location())
        : Symbol(sym, decl_loc)
        , agent(agent->stable_rp())
        , pp_agent(nullptr)
    {
    }

    EntityMemberSymbol(const string name, const Symbol *agent, yy::location decl_loc = yy::location())
        : Symbol(name, agent, decl_loc)
        , agent(agent->stable_rp())
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

    EntitySymbol *pp_agent;
};


