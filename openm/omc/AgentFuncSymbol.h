/**
* @file    AgentFuncSymbol.h
* Declarations for the AgentFuncSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "AgentMemberSymbol.h"

using namespace std;

/**
* Symbol for a function member of an agent.
*
* The symbol table contains an AgentFuncSymbol for each agent function,
* apart from event implement functions, which are represented by AgentEventSymbol's.
*/
class AgentFuncSymbol : public AgentMemberSymbol
{
private:
    typedef AgentMemberSymbol super;

public:
    bool is_base_symbol() const { return false; }


    /**
    * Constructor.
    *
    * @param [in,out]  sym The symbol to be morphed.
    */
    AgentFuncSymbol(Symbol *sym, const Symbol *agent, yy::location decl_loc = yy::location())
        : AgentMemberSymbol(sym, agent, decl_loc)
    {
    }

    AgentFuncSymbol(const string name, const Symbol *agent, yy::location decl_loc = yy::location())
        : AgentMemberSymbol(name, agent, decl_loc)
    {
    }

    void post_parse(int pass);

};

