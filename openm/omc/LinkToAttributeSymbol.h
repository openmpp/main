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
 * A symbol for a link to an attribute in an entity.
 * 
 * A LinkToAttributeSymbol records the use of an attribute of a different entity across a link.
 * An example is my_thing->making_trouble in the declaration of an identity
 * attribute in the Person entity.  It is a reference to the attribute named
 * making_trouble of the entity pointed to by the link my_thing.
 * 
 * NB: A LinkToAttributeSymbol is not an attribute of an entity.
 */
class LinkToAttributeSymbol : public Symbol
{
private:
    typedef Symbol super;

public:
    bool is_base_symbol() const { return false; }

    LinkToAttributeSymbol(const Symbol *ent_cntxt, const Symbol *link, const string attribute)
        : Symbol(LinkToAttributeSymbol::member_name(link, attribute), ent_cntxt)
        , entity_context(ent_cntxt->stable_rp())
        , pp_entity_context(nullptr)
        , link(link->stable_rp())
        , pp_link(nullptr)
        , attribute(attribute)
        , pp_attribute(nullptr)
    {
    }

    static string member_name(const Symbol* link, const string attribute);

    static string symbol_name(const Symbol* ent_cntxt, const Symbol* link, const string attribute);

    static Symbol *create_symbol(const Symbol* ent_cntxt, const Symbol* link, const string attribute);

    void post_parse(int pass);

    /**
     * The entity context in code using the link (lhs of link).
     */
    Symbol*& entity_context;
    EntitySymbol *pp_entity_context;

    /**
     * The link being used (a link entity var in entity_context)
     */
    Symbol*& link;
    LinkAttributeSymbol *pp_link;

    /**
     * The attribute being referenced across the link.
     */
    const string attribute;
    AttributeSymbol *pp_attribute;
};

