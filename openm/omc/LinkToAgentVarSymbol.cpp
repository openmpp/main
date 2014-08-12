/**
* @file    LinkToAgentVarSymbol.cpp
* Definitions for the LinkToAgentVarSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include "LinkToAgentVarSymbol.h"
#include "LinkAgentVarSymbol.h"
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
        assert(pp_link); // TODO: possible model source code error

        // assign direct pointer to agentvar in agent at other end of link
        assert(pp_link->reciprocal_link); // TODO: possible model coding error?
        assert(pp_link->reciprocal_link->agent); // TODO: possible model coding error?
        auto sym = Symbol::get_symbol(agentvar, pp_link->reciprocal_link->agent);
        assert(sym); // parser / scanner guarantee
        auto av = dynamic_cast<AgentVarSymbol *> (sym);
        assert(av); // TODO possible model source code error
        pp_agentvar = av;
        break;
    }
    default:
        break;
    }
}
