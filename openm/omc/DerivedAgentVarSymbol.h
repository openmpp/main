/**
* @file    DerivedAgentVarSymbol.h
* Declarations for the DerivedAgentVarSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "AgentVarSymbol.h"
#include "NumericSymbol.h"
#include "ConstantSymbol.h"

class PartitionSymbol;

using namespace std;

/**
* DerivedAgentVarSymbol
*
* Symbol for derived agentvars with various signatures
* E.g. duration(happy, true)
*/

class DerivedAgentVarSymbol : public AgentVarSymbol
{
private:
    typedef AgentVarSymbol super;

public:
    bool is_base_symbol() const { return false; }

    /**
     * Constructor for literal second argument
     *
     * @param agent    The agent.
     */
    DerivedAgentVarSymbol(const Symbol *agent,
                          token_type tk1,
                          token_type tk2,
                          const Symbol *av1,
                          const Symbol *av2,
                          const Symbol *prt,
                          const ConstantSymbol *k1,
                          const ConstantSymbol *k2,
                          const ConstantSymbol *k3,
                          yy::location decl_loc = yy::location())
        : AgentVarSymbol(DerivedAgentVarSymbol::member_name(tk1, tk2, av1, av2, prt, k1, k2, k3),
                        agent,
                        NumericSymbol::find(token::TK_double),
                        decl_loc
                        )
        , tk1(tk1)
        , tk2(tk2)
        , av1(av1 ? av1->stable_pp() : nullptr)
        , av2(av2 ? av2->stable_pp() : nullptr)
        , prt(prt ? prt->stable_pp() : nullptr)
        , k1(k1)
        , k2(k2)
        , k3(k3)
    {
    }

    /**
     * The member name for a specific symbol of this kind.
     *
     * @return The member namne as a string.
     */
    static string member_name(token_type tk1,
                              token_type tk2,
                              const Symbol *av1,
                              const Symbol *av2,
                              const Symbol *prt,
                              const ConstantSymbol *k1,
                              const ConstantSymbol *k2,
                              const ConstantSymbol *k3);

    static Symbol * create_symbol(const Symbol* agent,
                                  token_type tk1,
                                  token_type tk2,
                                  const Symbol *av1,
                                  const Symbol *av2,
                                  const Symbol *prt,
                                  const ConstantSymbol *k1,
                                  const ConstantSymbol *k2,
                                  const ConstantSymbol *k3,
                                  yy::location decl_loc);

    void post_parse(int pass);

    CodeBlock cxx_declaration_agent();

    /**
     * Token #1.
     */
    const token_type tk1;

    /**
     * Token #2.
     */
    const token_type tk2;

    /**
     * agentvar #1
     * 
     * Stable to symbol morphing during parse phase.
     */
    Symbol** av1;

    /**
     * agentvar #1
     * 
     * Only valid after post-parse phase 1.
     */
    AgentVarSymbol *pp_av1;

    /**
     * agentvar #2
     * 
     * Stable to symbol morphing during parse phase.
     */
    Symbol** av2;

    /**
     * agentvar #2
     * 
     * Only valid after post-parse phase 1.
     */
    AgentVarSymbol *pp_av2;

    /**
     * partition
     * 
     * Stable to symbol morphing during parse phase.
     */
    Symbol** prt;

    /**
     * partition
     * 
     * Only valid after post-parse phase 1.
     */
    PartitionSymbol *pp_prt;

    /**
     * Constant #1.
     */
    const ConstantSymbol *k1;

    /**
     * Constant #2.
     */
    const ConstantSymbol *k2;

    /**
     * Constant #3.
     */
    const ConstantSymbol *k3;
};

