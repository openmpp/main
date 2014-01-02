/**
* @file    AgentDataMemberSymbol.h
* Declarations for the AgentDataMemberSymbol class.
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "AgentMemberSymbol.h"


class CodeBlock;

using namespace std;

class AgentDataMemberSymbol : public AgentMemberSymbol
{
private:
    typedef AgentMemberSymbol super;

public:
    AgentDataMemberSymbol(Symbol *sym, const Symbol *agent, token_type type)
        : AgentMemberSymbol(sym, agent)
        , type(type)
    {
    }

    AgentDataMemberSymbol(const string member_name, const Symbol *agent, token_type type)
        : AgentMemberSymbol(member_name, agent)
        , type(type)
    {
    }


    /**
    * Gets the initial value for the data member.
    *
    * @return The initial value as a string.
    */

    virtual string initial_value() const;

    virtual CodeBlock cxx_initialize_expression() const;


    CodeBlock cxx_declaration_agent_scope();

    CodeBlock cxx_definition();


    void post_parse(int pass);


    /**
    * C++ fundamental type.
    *
    * Recorded as a token value, e.g. int_KW.
    */

    token_type type;

};
