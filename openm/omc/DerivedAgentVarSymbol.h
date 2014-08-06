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
class IdentityAgentVarSymbol;

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
     * Constructor with all arguments.
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
                        NumericSymbol::find(token::TK_double),// will be changed later
                        decl_loc
                        )
        , tk1(tk1)
        , tk2(tk2)
        , av1(av1 ? av1->stable_pp() : nullptr)
        , pp_av1(nullptr)
        , av2(av2 ? av2->stable_pp() : nullptr)
        , pp_av2(nullptr)
        , prt(prt ? prt->stable_pp() : nullptr)
        , pp_prt(nullptr)
        , k1(k1)
        , k2(k2)
        , k3(k3)
        , iav(nullptr)
    {
        validate();
        create_auxiliary_symbols();
    }

    /**
     * The member name for a specific symbol of this kind.
     *
     * @param tk1 The first tk.
     * @param tk2 The second tk.
     * @param av1 The first av.
     * @param av2 The second av.
     * @param prt The prt.
     * @param k1  The first ConstantSymbol.
     * @param k2  The second ConstantSymbol.
     * @param k3  The third ConstantSymbol.
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

    /**
     * Creates the given symbol, or returns it if it already exists
     *
     * @param agent    The agent.
     * @param tk1      The first tk.
     * @param tk2      The second tk.
     * @param av1      The first av.
     * @param av2      The second av.
     * @param prt      The prt.
     * @param k1       The first ConstantSymbol.
     * @param k2       The second ConstantSymbol.
     * @param k3       The third ConstantSymbol.
     * @param decl_loc The declaration location.
     *
     * @return null if it fails, else the new symbol.
     */
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

    /**
     * Creates the given symbol, or returns it if it already exists
     *
     * @param agent    The agent.
     * @param tk1      The token.
     * @param decl_loc The declaration location.
     *
     * @return The symbol.
     */
    static Symbol * create_symbol(const Symbol* agent,
                                  token_type tk1,
                                  yy::location decl_loc)
    {
        // parser guarantees:
        assert(agent);
        return create_symbol(agent, tk1, token::TK_unused, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, decl_loc);
    }

    /**
     * Creates the given symbol, or returns it if it already exists
     *
     * @param agent    The agent.
     * @param tk1      The token.
     * @param av1      The agentvar.
     * @param k1       The constant.
     * @param decl_loc The declaration location.
     *
     * @return The symbol.
     */
    static Symbol * create_symbol(const Symbol* agent,
                                  token_type tk1,
                                  const Symbol *av1,
                                  const ConstantSymbol *k1,
                                  yy::location decl_loc)
    {
        // parser guarantees:
        assert(agent);
        assert(av1);
        assert(k1);
        return create_symbol(agent, tk1, token::TK_unused, av1, nullptr, nullptr, k1, nullptr, nullptr, decl_loc);
    }

    /**
     * Creates the given symbol, or returns it if it already exists
     *
     * @param agent    The agent.
     * @param tk1      The token.
     * @param av1      The agentvar #1
     * @param k1       The constant.
     * @param av2      The agentvar #2
     * @param decl_loc The declaration location.
     *
     * @return The symbol.
     */
    static Symbol * create_symbol(const Symbol* agent,
                                  token_type tk1,
                                  const Symbol *av1,
                                  const ConstantSymbol *k1,
                                  const Symbol *av2,
                                  yy::location decl_loc)
    {
        // parser guarantees:
        assert(agent);
        assert(av1);
        assert(k1);
        assert(av2);
        return create_symbol(agent, tk1, token::TK_unused, av1, av2, nullptr, k1, nullptr, nullptr, decl_loc);
    }

    /**
     * Creates the given symbol, or returns it if it already exists
     *
     * @param agent    The agent.
     * @param av1      The agentvar.
     * @param decl_loc The declaration location.
     *
     * @return The symbol.
     */
    static Symbol * create_symbol(const Symbol* agent,
                                  token_type tk1,
                                  const Symbol *av1,
                                  yy::location decl_loc)
    {
        // parser guarantees:
        assert(agent);
        assert(av1);
        return create_symbol(agent, tk1, token::TK_unused, av1, nullptr, nullptr, nullptr, nullptr, nullptr, decl_loc);
    }

    /**
     * Creates the given symbol, or returns it if it already exists
     *
     * @param agent    The agent.
     * @param av1      The agentvar (#1)
     * @param av2      The agentvar (#2)
     * @param decl_loc The declaration location.
     *
     * @return The symbol.
     */
    static Symbol * create_symbol(const Symbol* agent,
                                  token_type tk1,
                                  const Symbol *av1,
                                  const Symbol *av2,
                                  yy::location decl_loc)
    {
        // parser guarantees:
        assert(agent);
        assert(av1);
        assert(av2);
        return create_symbol(agent, tk1, token::TK_unused, av1, av2, nullptr, nullptr, nullptr, nullptr, decl_loc);
    }

    /**
     * Validate argument signatures.
     */
    void validate();

    /**
     * Create auxiliary symbols associated with this symbol.
     */
    void create_auxiliary_symbols();

    /**
     * Assign the datatype of this symbol.
     */
    void assign_data_type();

    /**
     * Create side-effects to maintain this symbol.
     */
    void create_side_effects();

    void post_parse(int pass);

    virtual string pretty_name();

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

    /**
     * The identity agentvar symbol, used for conditions and spells
     */
    IdentityAgentVarSymbol *iav;
};

