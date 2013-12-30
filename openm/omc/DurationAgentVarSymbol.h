/**
* @file    DurationAgentVarSymbol.h
* Declarations for the DurationAgentVarSymbol class.
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "Symbol.h"

using namespace std;

/**
* DurationAgentVarSymbol.
*
* Symbol for derived agentvars of the form duration().
*/

class DurationAgentVarSymbol : public AgentVarSymbol
{
private:
    typedef AgentVarSymbol super;

public:
    // constructor for 0-argument derived agentvars, e.g. duration()
    DurationAgentVarSymbol(const Symbol *agent)
        : AgentVarSymbol(DurationAgentVarSymbol::member_name(), agent, token::TK_Time)
    {
    }

    static string member_name();

    static string symbol_name(const Symbol *agent);

    /**
    * Get a symbol for a derived agentvar of form duration()
    *
    * @param   agent   The agent.
    *
    * @return  The symbol.
    */

    static Symbol * get_symbol(const Symbol *agent);

    void post_parse(int pass);

    /**
    * Generates the c++ declaration for the derived agentvar.
    */

    CodeBlock cxx_declaration_agent_scope();

};

