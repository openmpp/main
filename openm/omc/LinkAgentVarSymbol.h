/**
 * @file    AgentLinkSymbol.h
 * Declarations for the AgentLinkSymbol class.
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include "AgentVarSymbol.h"

class AgentMultilinkSymbol;

using namespace std;

/**
* Symbol for a link member of an agent.
*/
class LinkAgentVarSymbol : public AgentVarSymbol
{
private:
    typedef AgentVarSymbol super;

public:
    bool is_base_symbol() const { return false; }

    /**
    * Constructor.
    *
    * @param [in,out]  sym The symbol to be morphed.
    */
    LinkAgentVarSymbol(Symbol *sym, const Symbol *agent, const Symbol *type, yy::location decl_loc = yy::location())
        : AgentVarSymbol(sym, agent, type, decl_loc)
        , reciprocal_link(nullptr)
        , reciprocal_multilink(nullptr)
    {
    }

    void post_parse(int pass);

    CodeBlock cxx_declaration_agent();

    /**
     * The reciprocal link if one-to-one
     */
    LinkAgentVarSymbol *reciprocal_link;

    /**
     * The reciprocal multilink if one-to-many
     */
    AgentMultilinkSymbol *reciprocal_multilink;

};

