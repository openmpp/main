/**
* @file    LinkToAttributeSymbol.h
* Declarations for the LinkToAttributeSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include "Symbol.h"

class EntitySymbol;
class AttributeSymbol;
class LinkAttributeSymbol;

/**
 * A symbol for a link to an agent variable symbol.
 * 
 * A LinkToAttributeSmbol records the use of an attribute in a different agent across a link
 * attribute.  An example might be “my_thing->making_trouble” in the declaration of an identity
 * attribute in the "Person" agent.  The example is a reference to the attribute named
 * "making_trouble" in the agent "my_thing", which is an agent of type "Thing".
 */
class LinkToAttributeSymbol : public Symbol
{
private:
    typedef Symbol super;

public:
    bool is_base_symbol() const { return false; }

    LinkToAttributeSymbol(const Symbol *agent_context, const Symbol *link, const string attribute)
        : Symbol(LinkToAttributeSymbol::member_name(link, attribute), agent_context)
        , agent_context(agent_context->stable_rp())
        , pp_agent_context(nullptr)
        , link(link->stable_rp())
        , pp_link(nullptr)
        , attribute(attribute)
        , pp_attribute(nullptr)
    {
    }

    static string member_name(const Symbol* link, const string attribute);

    static string symbol_name(const Symbol* agent_context, const Symbol* link, const string attribute);

    static Symbol *create_symbol(const Symbol* agent_context, const Symbol* link, const string attribute);

    void post_parse(int pass);

    /**
     * The agent context in code using the link (lhs of link).
     */
    Symbol*& agent_context;
    EntitySymbol *pp_agent_context;

    /**
     * The link being used (a link agent var in agent_context)
     */
    Symbol*& link;
    LinkAttributeSymbol *pp_link;

    /**
     * The attribute being referenced across the link.
     */
    const string attribute;
    AttributeSymbol *pp_attribute;
};

