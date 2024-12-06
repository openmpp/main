/**
* @file    AttributeGroupSymbol.h
* Declarations for the AttributeGroupSymbol class.
*/
// Copyright (c) 2013-2024 OpenM++ Contributors
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include "GroupSymbol.h"
#include "AttributeSymbol.h"

/**
* AttributeGroupSymbol.
*
*/
class AttributeGroupSymbol : public GroupSymbol
{
private:
    typedef GroupSymbol super;

public:
    bool is_base_symbol() const { return false; }

    /**
    * Morphing constructor.
    *
    * @param [in,out]  sym The symbol to be morphed.
    */
    AttributeGroupSymbol(Symbol *sym, const Symbol* ent, omc::location decl_loc = omc::location())
        : GroupSymbol(sym, decl_loc)
        , entity(ent->stable_rp())
        , pp_entity(nullptr)
    {
    }

    void post_parse(int pass);

    void populate_metadata(openm::MetaModelHolder& metaRows);

    /**
     * Reference to pointer to entity.
     * 
     * Stable to symbol morphing during parse phase.
     */
    Symbol*& entity;

    /**
     * Direct pointer to entity.
     * 
     * For use post-parse.
     */
    EntitySymbol *pp_entity;
};
