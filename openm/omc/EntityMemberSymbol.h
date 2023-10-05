/**
* @file    EntityMemberSymbol.h
* Declarations for the EntityMemberSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include "Symbol.h"

using namespace std;

class EntityMemberSymbol : public Symbol
{
private:
    typedef Symbol super;

public:
    bool is_base_symbol() const { return false; }

    EntityMemberSymbol(Symbol *sym, const Symbol * entity, omc::location decl_loc = omc::location())
        : Symbol(sym, decl_loc)
        , entity(entity->stable_rp())
        , pp_entity(nullptr)
    {
    }

    EntityMemberSymbol(const string name, const Symbol * entity, omc::location decl_loc = omc::location())
        : Symbol(name, entity, decl_loc)
        , entity(entity->stable_rp())
        , pp_entity(nullptr)
    {
    }


    void post_parse(int pass);

    /**
    * Reference to pointer to entity.
    *
    * Stable to symbol morphing during parse phase.
    */

    Symbol*& entity;

    /**
    * Direct pointer to entity.
    *
    * Set post-parse for convenience.
    */

    EntitySymbol *pp_entity;
};


