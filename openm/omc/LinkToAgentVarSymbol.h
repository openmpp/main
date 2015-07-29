/**
* @file    LinkToAgentVarSymbol.h
* Declarations for the LinkToAgentVarSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include "Symbol.h"

class AgentSymbol;
class AgentVarSymbol;
class LinkAgentVarSymbol;

/**
 * A symbol for a link to an agent variable symbol.
 * 
 * A LinkToAgentVarSmbol records the use of an agentvar in a different agent across a link
 * agentvar.  An example might be “my_thing->making_trouble” in the declaration of an identity
 * agentvar in the "Person" agent.  The example is a reference to the agentvar named
 * "making_trouble" in the agent "my_thing", which is an agent of type "Thing".
 */
class LinkToAgentVarSymbol : public Symbol
{
private:
    typedef Symbol super;

public:
    bool is_base_symbol() const { return false; }

    LinkToAgentVarSymbol(const Symbol *agent_context, const Symbol *link, const string agentvar)
        : Symbol(LinkToAgentVarSymbol::member_name(link, agentvar), agent_context)
        , agent_context(agent_context->stable_rp())
        , pp_agent_context(nullptr)
        , link(link->stable_rp())
        , pp_link(nullptr)
        , agentvar(agentvar)
        , pp_agentvar(nullptr)
    {
    }

    static string member_name(const Symbol* link, const string agentvar);

    static string symbol_name(const Symbol* agent_context, const Symbol* link, const string agentvar);

    static Symbol *create_symbol(const Symbol* agent_context, const Symbol* link, const string agentvar);

    void post_parse(int pass);

    /**
     * The agent context in code using the link (lhs of link).
     */
    Symbol*& agent_context;
    AgentSymbol *pp_agent_context;

    /**
     * The link being used (a link agent var in agent_context)
     */
    Symbol*& link;
    LinkAgentVarSymbol *pp_link;

    /**
     * The agentvar being referenced across the link.
     */
    const string agentvar;
    AgentVarSymbol *pp_agentvar;
};

