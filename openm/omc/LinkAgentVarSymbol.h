/**
 * @file    AgentLinkSymbol.h
 * Declarations for the AgentLinkSymbol class.
 */
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "AgentVarSymbol.h"

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
        , single(true)
    {
    }

    void post_parse(int pass);

    CodeBlock cxx_declaration_agent();

    CodeBlock cxx_definition_agent();

    /**
     * true if single link, false if multilink
     */
    bool single;

    /**
     * The reciprocal link.
     */
    LinkAgentVarSymbol *reciprocal_link;

};

