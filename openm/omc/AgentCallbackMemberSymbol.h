/**
* @file    AgentCallbackMemberSymbol.h
* Declarations for the AgentCallbackMemberSymbol class.
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "AgentDataMemberSymbol.h"

using namespace std;

/**
* Agent callback member symbol.
* Contains functionality to support callback functions at containing agent scope.
*/

class AgentCallbackMemberSymbol : public AgentDataMemberSymbol
{
private:
    typedef AgentDataMemberSymbol super;

public:
    AgentCallbackMemberSymbol(Symbol *sym, const Symbol *agent, token_type type)
        : AgentDataMemberSymbol(sym, agent, type)
    {
    }

    AgentCallbackMemberSymbol(const string member_name, const Symbol *agent, token_type type)
        : AgentDataMemberSymbol(member_name, agent, type)
    {
    }

    /**
    * Get the c++ declaration code associated with the symbol.
    */

    virtual CodeBlock cxx_declaration_agent_scope();

    /**
    * Get the c++ definition code associated with the symbol.
    */

    virtual CodeBlock cxx_definition();

};

