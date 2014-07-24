/**
* @file    EntitySetSymbol.h
* Declarations for the EntitySetSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "Symbol.h"

class CodeBlock;

/**
* EntitySetSymbol.
*
*/
class EntitySetSymbol : public Symbol
{
private:
    typedef Symbol super;

public:
    bool is_base_symbol() const { return false; }

    /**
    * Morphing constructor.
    *
    * @param [in,out]  sym The symbol to be morphed.
    */
    EntitySetSymbol(Symbol *sym, const Symbol *agent, yy::location decl_loc = yy::location())
        : Symbol(sym, decl_loc)
        , agent(agent->stable_rp())
    {
    }

    void post_parse(int pass);

    CodeBlock cxx_declaration_global();

    CodeBlock cxx_definition_global();

    /**
     * Reference to pointer to agent.
     * 
     * Stable to symbol morphing during parse phase.
     */
    Symbol*& agent;

    /**
     * Direct pointer to agent.
     * 
     * For use post-parse.
     */
    AgentSymbol *pp_agent;
};
