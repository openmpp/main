/**
* @file    AgentFuncSymbol.h
* Declarations for the AgentFuncSymbol class.
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "Symbol.h"

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

    /**
    * Constructor.
    *
    * @param [in,out]  sym The symbol to be morphed.
    */
    AgentFuncSymbol(Symbol *sym, const Symbol *agent)
        : AgentMemberSymbol(sym, agent)
    {
    }

    void post_parse(int pass);

};

