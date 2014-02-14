/**
* @file    AgentInternalSymbol.h
* Declarations for the AgentInternalSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
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
    bool is_base_symbol() const { return false; }

    AgentInternalSymbol(const string member_name, const Symbol *agent, const Symbol *type)
        : AgentDataMemberSymbol(member_name, agent, type)
    {
    }

    CodeBlock cxx_initialization_expression(bool type_default) const;

    CodeBlock cxx_declaration_agent();

};

