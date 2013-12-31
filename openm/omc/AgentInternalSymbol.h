/**
* @file    AgentInternalSymbol.h
* Declarations for the AgentInternalSymbol class.
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "AgentDataMemberSymbol.h"

using namespace std;

/**
* Internal data symbol.
* Abstracts a member of an agent class which stores information
* used internally by om.  These members are not accessible to
* developer code, and have no side-effects.
*/

class AgentInternalSymbol : public AgentDataMemberSymbol
{
private:
    typedef AgentDataMemberSymbol super;

public:
    AgentInternalSymbol(Symbol *sym, const Symbol *agent, token_type type)
        : AgentDataMemberSymbol(sym, agent, type)
    {
    }

    AgentInternalSymbol(const string member_name, const Symbol *agent, token_type type)
        : AgentDataMemberSymbol(member_name, agent, type)
    {
    }

    void post_parse(int pass);

    virtual CodeBlock cxx_initialize_expression() const;

    /**
    * Get the c++ declaration code associated with the symbol.
    */

    virtual CodeBlock cxx_declaration_agent_scope();

    /**
    * Get the c++ definition code associated with the symbol.
    */

    virtual CodeBlock cxx_definition();

};

