/**
* @file    LinkToAttributeSymbol.cpp
* Definitions for the LinkToAttributeSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <cassert>
#include "LinkToAttributeSymbol.h"
#include "LinkAttributeSymbol.h"
#include "EntityMultilinkSymbol.h"
#include "EntitySymbol.h"
#include "CodeBlock.h"

using namespace std;
using namespace openm;

// static
string LinkToAttributeSymbol::member_name(const Symbol* link, const string agentvar)
{
    string result = link->name + "->" + agentvar;
    return result;
}

// static
string LinkToAttributeSymbol::symbol_name(const Symbol* agent_context, const Symbol* link, const string agentvar)
{
    string member = LinkToAttributeSymbol::member_name(link, agentvar);
    string result = Symbol::symbol_name(member, agent_context);
    return result;
}

// static
Symbol *LinkToAttributeSymbol::create_symbol(const Symbol* agent_context, const Symbol* link, const string agentvar)
{
    Symbol *sym = nullptr;
    string nm = LinkToAttributeSymbol::symbol_name(agent_context, link, agentvar);
    auto it = symbols.find(nm);
    if (it != symbols.end())
        sym = it->second;
    else
        sym = new LinkToAttributeSymbol(agent_context, link, agentvar);

    return sym;
}

void LinkToAttributeSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case eAssignMembers:
    {
        // assign direct pointer to agent for use post-parse
        pp_agent_context = dynamic_cast<EntitySymbol *> (pp_symbol(agent_context));
        assert(pp_agent_context); // parser guarantee

        // assign direct pointer to link for use post-parse
        pp_link = dynamic_cast<LinkAttributeSymbol *> (pp_symbol(link));
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
        auto av = dynamic_cast<AttributeSymbol *> (sym);
        if (!av) {
            pp_error(LT("error : invalid member '") + agentvar + LT("' of link '") + pp_link->name + LT("'"));
        }
        pp_agentvar = av;
        break;
    }
    default:
        break;
    }
}
