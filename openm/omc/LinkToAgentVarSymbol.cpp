/**
* @file    LinkToAgentVarSymbol.cpp
* Definitions for the LinkToAgentVarSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include "LinkToAgentVarSymbol.h"
#include "LinkAgentVarSymbol.h"
#include "AgentMultilinkSymbol.h"
#include "AgentSymbol.h"
#include "CodeBlock.h"

using namespace std;
using namespace openm;

// static
string LinkToAgentVarSymbol::member_name(const Symbol* link, const string agentvar)
{
    string result = link->name + "->" + agentvar;
    return result;
}

// static
string LinkToAgentVarSymbol::symbol_name(const Symbol* agent_context, const Symbol* link, const string agentvar)
{
    string member = LinkToAgentVarSymbol::member_name(link, agentvar);
    string result = Symbol::symbol_name(member, agent_context);
    return result;
}

// static
Symbol *LinkToAgentVarSymbol::create_symbol(const Symbol* agent_context, const Symbol* link, const string agentvar)
{
    Symbol *sym = nullptr;
    string nm = LinkToAgentVarSymbol::symbol_name(agent_context, link, agentvar);
    auto it = symbols.find(nm);
    if (it != symbols.end())
        sym = it->second;
    else
        sym = new LinkToAgentVarSymbol(agent_context, link, agentvar);

    return sym;
}

void LinkToAgentVarSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case eAssignMembers:
    {
        // assign direct pointer to agent for use post-parse
        pp_agent_context = dynamic_cast<AgentSymbol *> (pp_symbol(agent_context));
        assert(pp_agent_context); // parser guarantee

        // assign direct pointer to link for use post-parse
        pp_link = dynamic_cast<LinkAgentVarSymbol *> (pp_symbol(link));
        assert(pp_link); // parser guarantee (link statement)

        // assign direct pointer to agentvar in agent at other end of link
        Symbol *agent = nullptr;
        if (pp_link->reciprocal_link) {
            // is a one-to-one link
            auto reciprocal = pp_link->reciprocal_link;
            agent = reciprocal->agent;
            assert(agent); // grammar guarantee (link statement)
        }
        else {
            // is a one-to-many link
            assert(pp_link->reciprocal_multilink); // grammar guarantee (must be either link or multilink)
            auto reciprocal = pp_link->reciprocal_multilink;
            agent = reciprocal->agent;
            assert(agent); // grammar guarantee (link statement)
        }
        auto sym = Symbol::get_symbol(agentvar, agent);
        auto av = dynamic_cast<AgentVarSymbol *> (sym);
        if (!av) {
            pp_error("error - invalid member '" + agentvar + "' of link '" + pp_link->name + "'");
        }
        pp_agentvar = av;
        break;
    }
    default:
        break;
    }
}
