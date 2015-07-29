/**
* @file    AgentHookSymbol.h
* Declarations for the AgentHookSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include "Symbol.h"
#include "AgentFuncSymbol.h"

using namespace std;

class AgentHookSymbol : public Symbol
{
private:
    typedef Symbol super;

public:
    bool is_base_symbol() const { return false; }

    explicit AgentHookSymbol(const Symbol *agent, const Symbol *from, const Symbol *to, int order = 0, yy::location decl_loc = yy::location())
        : Symbol(symbol_name(from, to), agent, decl_loc)
        , agent(agent->stable_rp())
        , pp_agent(nullptr)
        , from(from->stable_rp())
        , pp_from(nullptr)
        , to(to->stable_rp())
        , pp_to(nullptr)
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
    static bool exists(const Symbol* agent, const Symbol* from, const Symbol* to);

    /**
     * Create auxiliary symbols associated with this symbol
     */
    void create_auxiliary_symbols();

    void post_parse(int pass);

    /**
     * Reference to pointer to agent.
     * 
     * Stable to symbol morhing during parse phase.
     */
    Symbol*& agent;

    /**
     * Direct pointer to agent.
     * 
     * Set post-parse for convenience.
     */
    AgentSymbol *pp_agent;

    /**
     * The 'from' function for the hook (stable to morphing)
     */
    Symbol*& from;

    /**
     * The 'from' function for the hook (typed pointer)
     */
    AgentFuncSymbol *pp_from;

    /**
     * The 'to' function for the hook (stable to morphing)
     */
    Symbol*& to;

    /**
     * The hook order.
     * 
     * This is the optional third argument of the hook declaration in model code.  It specifies an
     * optional calling order to handle the situation where more than one function is hooked to
     * another.
     */
    int order;

    /**
     * The 'from' function for the hook (typed pointer)
     */
    AgentFuncSymbol *pp_to;

    /**
     * The hook implement function for the 'to' function
     */
    AgentFuncSymbol *hook_fn;
};


