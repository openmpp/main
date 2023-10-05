/**
* @file    LinkToAttributeSymbol.cpp
* Definitions for the LinkToAttributeSymbol class.
*/
// Copyright (c) 2013-2021 OpenM++ Contributors
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
string LinkToAttributeSymbol::member_name(const Symbol* link, const string attribute)
{
    string result = link->name + "->" + attribute;
    return result;
}

// static
string LinkToAttributeSymbol::symbol_name(const Symbol* ent_cntxt, const Symbol* link, const string attribute)
{
    string member = LinkToAttributeSymbol::member_name(link, attribute);
    string result = Symbol::symbol_name(member, ent_cntxt);
    return result;
}

// static
Symbol *LinkToAttributeSymbol::create_symbol(const Symbol* ent_cntxt, const Symbol* link, const string attribute)
{
    Symbol *sym = nullptr;
    string nm = LinkToAttributeSymbol::symbol_name(ent_cntxt, link, attribute);
    auto it = symbols.find(nm);
    if (it != symbols.end())
        sym = it->second;
    else
        sym = new LinkToAttributeSymbol(ent_cntxt, link, attribute);

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
        // assign direct pointer to entity for use post-parse
        pp_entity_context = dynamic_cast<EntitySymbol *> (pp_symbol(entity_context));
        assert(pp_entity_context); // parser guarantee

        // assign direct pointer to link for use post-parse
        pp_link = dynamic_cast<LinkAttributeSymbol *> (pp_symbol(link));
        assert(pp_link); // parser guarantee (link statement)

        // assign direct pointer to attribute in entity at other end of link
        Symbol * ent = nullptr;
        if (pp_link->reciprocal_link) {
            // is a one-to-one link
            auto reciprocal = pp_link->reciprocal_link;
            ent = reciprocal->entity;
            assert(ent); // grammar guarantee (link statement)
        }
        else {
            // is a one-to-many link
            assert(pp_link->reciprocal_multilink); // grammar guarantee (must be either link or multilink)
            auto reciprocal = pp_link->reciprocal_multilink;
            ent = reciprocal->entity;
            assert(ent); // grammar guarantee (link statement)
        }
        auto sym = Symbol::get_symbol(attribute, ent);
        auto av = dynamic_cast<AttributeSymbol *> (sym);
        if (!av) {
            pp_error(LT("error : invalid member '") + attribute + LT("' of link '") + pp_link->name + LT("'"));
        }
        pp_attribute = av;
        break;
    }
    default:
        break;
    }
}
