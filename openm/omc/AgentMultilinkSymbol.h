/**
* @file    AgentMultilinkSymbol.h
* Declarations for the AgentMultilinkSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "AgentDataMemberSymbol.h"

class LinkAgentVarSymbol;

using namespace std;

/**
* Symbol for a multilink member of an agent.
*/
class AgentMultilinkSymbol : public AgentDataMemberSymbol
{
private:
    typedef AgentDataMemberSymbol super;

public:
    bool is_base_symbol() const { return false; }

    AgentMultilinkSymbol(Symbol *sym, const Symbol *agent, const Symbol *type, yy::location decl_loc = yy::location())
        : AgentDataMemberSymbol(sym, agent, type, decl_loc)
        , reciprocal_link(nullptr)
        , reciprocal_multilink(nullptr)
    {
    }

    void post_parse(int pass);

    CodeBlock cxx_declaration_agent();

    /**
     * The reciprocal link if many-to-one
     */
    LinkAgentVarSymbol *reciprocal_link;

    /**
     * The reciprocal multilink if many-to-many
     */
    AgentMultilinkSymbol *reciprocal_multilink;
};

