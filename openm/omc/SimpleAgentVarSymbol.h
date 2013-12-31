/**
* @file    SimpleAgentVarSymbol.h
* Declarations for the SimpleAgentVarSymbol class.
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "AgentVarSymbol.h"

using namespace std;

/**
* SimpleAgentVarSymbol.
*/
class SimpleAgentVarSymbol : public AgentVarSymbol
{
private:
    typedef AgentVarSymbol super;

public:
    SimpleAgentVarSymbol(Symbol *sym, const Symbol *agent, token_type type, Literal *initializer)
        : AgentVarSymbol(sym, agent, type)
        , initializer(initializer)
    {
    }

    ~SimpleAgentVarSymbol()
    {
    }

    /**
    * Gets the initial value for the agentvar
    *
    * @return  The initial value as a string.
    */
    string initial_value() const;

    void post_parse(int pass);

    /**
    * Gets the c++ declaration for the simple agentvar.
    */

    CodeBlock cxx_declaration_agent_scope();

    // members

    /** C++ literal to initialize simple agentvar at agent creation */
    Literal *initializer;

};

