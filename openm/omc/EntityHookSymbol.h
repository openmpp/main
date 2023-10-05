/**
* @file    EntityHookSymbol.h
* Declarations for the EntityHookSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include "Symbol.h"
#include "EntityFuncSymbol.h"
#include "DerivedAttributeSymbol.h"

using namespace std;

class EntityHookSymbol : public Symbol
{
private:
    typedef Symbol super;

public:
    bool is_base_symbol() const { return false; }

    explicit EntityHookSymbol(const Symbol * ent, const Symbol *from, const Symbol *to, int order = 0, omc::location decl_loc = omc::location())
        : Symbol(symbol_name(from, to), ent, decl_loc)
        , entity(ent->stable_rp())
        , pp_entity(nullptr)
        , from(from->stable_rp())
        , pp_from(nullptr)
        , to(to->stable_rp())
        , to_is_self_scheduling(false)
        , pp_to_fn(nullptr)
        , pp_to_ss(nullptr)
        , hook_fn(nullptr)
        , order(order)
    {
        create_auxiliary_symbols();
    }

    /**
     * Construct symbol name for the hook
     * 
     * Example: Person::om_hook_StartClock_om_Start.
     *
     * @param from The 'from' function
     * @param to   The 'to' function
     *
     * @return A string.
     */
    static string symbol_name(const Symbol* from, const Symbol* to);

    /**
     * Check for existence of this hook symbol
     * 
     * @param from The 'from' function
     * @param to   The 'to' function
     *
     * @return true if exists, else false.
     */
    static bool exists(const Symbol* ent, const Symbol* from, const Symbol* to);

    /**
     * Create auxiliary symbols associated with this symbol
     */
    void create_auxiliary_symbols();

    void post_parse(int pass);

    /**
     * Reference to pointer to entity.
     * 
     * Stable to symbol morhing during parse phase.
     */
    Symbol*& entity;

    /**
     * Direct pointer to entity.
     * 
     * Set post-parse for convenience.
     */
    EntitySymbol *pp_entity;

    /**
     * The 'from' function for the hook (stable to morphing)
     */
    Symbol*& from;

    /**
     * The 'from' function for the hook (typed pointer)
     */
    EntityFuncSymbol *pp_from;

    /**
     * The 'to' function for the hook (stable to morphing)
     */
    Symbol*& to;

    /**
     * The 'to' target is a self-scheduling attribute
     */
    bool to_is_self_scheduling;

    /**
     * The name used for the 'to' target
     */
    string to_name;

    /**
     * The 'to' function for the hook (if function)
     */
    EntityFuncSymbol *pp_to_fn;

    /**
     * The 'to' function for the hook (if self-scheduling attribute)
     */
    DerivedAttributeSymbol *pp_to_ss;

    /**
     * The hook implement function for the 'to' function
     */
    EntityFuncSymbol *hook_fn;

    /**
     * The hook order.
     * 
     * This is the optional third argument of the hook declaration in model code.  It specifies an
     * optional calling order to handle the situation where more than one function is hooked to
     * another.
     */
    int order;
};


