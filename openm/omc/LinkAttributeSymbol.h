/**
 * @file    LinkAttributeSymbol.h
 * Declarations for the LinkAttributeSymbol class.
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include "AttributeSymbol.h"

class EntityMultilinkSymbol;

using namespace std;

/**
* Symbol for a link member of an entity.
* 
* Note that this symbol is for the link itself, not for an attribute
* pointed to by a link.
* A link can be used in an expression without being followed by
* the -> operator, in which case it returns a bool
* just like C++ does with a pointer.
* 
* During parsing, a link which points to an attribute
* is held in a LinkToAttributeSymbol object.
* 
*/
class LinkAttributeSymbol : public AttributeSymbol
{
private:
    typedef AttributeSymbol super;

public:
    bool is_base_symbol() const { return false; }

    /**
    * Constructor.
    *
    * @param [in,out]  sym The symbol to be morphed.
    */
    LinkAttributeSymbol(Symbol *sym, const Symbol *agent, const Symbol *type, yy::location decl_loc = yy::location())
        : AttributeSymbol(sym, agent, type, decl_loc)
        , reciprocal_link(nullptr)
        , reciprocal_multilink(nullptr)
    {
    }

    void post_parse(int pass);

    CodeBlock cxx_declaration_agent();

    /**
     * The reciprocal link if one-to-one
     */
    LinkAttributeSymbol *reciprocal_link;

    /**
     * The reciprocal multilink if one-to-many
     */
    EntityMultilinkSymbol *reciprocal_multilink;

};

