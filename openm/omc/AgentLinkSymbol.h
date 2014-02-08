/**
 * @file    AgentLinkSymbol.h
 * Declarations for the AgentLinkSymbol class.
 */
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "AgentMemberSymbol.h"

using namespace std;

/**
* Symbol for a (single) link member of an agent.
*/
class AgentLinkSymbol : public AgentMemberSymbol
{
private:
    typedef AgentMemberSymbol super;

public:
    bool is_base_symbol() const { return false; }

    /**
    * Constructor.
    *
    * @param [in,out]  sym The symbol to be morphed.
    */
    AgentLinkSymbol(Symbol *sym, const Symbol *agent, yy::location decl_loc = yy::location())
        : AgentMemberSymbol(sym, agent, decl_loc)
    {
    }

    void post_parse(int pass);

    CodeBlock cxx_declaration_agent();

    CodeBlock cxx_definition_agent();

};

