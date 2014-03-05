/**
* @file    MultilinkAgentVarSymbol.h
* Declarations for the MultilinkAgentVarSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "AgentVarSymbol.h"
#include "NumericSymbol.h"

class AgentMultilinkSymbol;

using namespace std;

/**
 * MultilinkAgentVarSymbol
 * 
 * Symbol for multilink agentvars of the form count(multilink), sum_over(multilink, agentvar),
 * etc.
 */
class MultilinkAgentVarSymbol : public AgentVarSymbol
{
private:
    typedef AgentVarSymbol super;

public:
    bool is_base_symbol() const { return false; }

    MultilinkAgentVarSymbol(const Symbol *agent, token_type func, const Symbol *multilink, const Symbol *agentvar = nullptr)
        : AgentVarSymbol(MultilinkAgentVarSymbol::member_name(func, multilink, agentvar),
                        agent,
                        NumericSymbol::find(token::TK_integer) )
        , func(func)
        , multilink(multilink->stable_rp())
        , pp_multilink(nullptr)
        , agentvar(agentvar ? agentvar->stable_pp() : nullptr)
        , pp_agentvar(nullptr)
    {

        create_auxiliary_symbols();
    }

    static string member_name(token_type func, const Symbol *multilink, const Symbol *agentvar = nullptr);

    static string symbol_name(const Symbol *agent, token_type func, const Symbol *multilink, const Symbol *agentvar = nullptr);

    /**
     * Create a symbol for a multilink agentvar.
     *
     * @param agent     The containing agent.
     * @param func      The function, e.g. TK_sum_over
     * @param multilink The multilink, e.g. things
     * @param agentvar  (Optional) the agentvar.
     *
     * @return The symbol.
     */
    static Symbol * create_symbol(const Symbol *agent, token_type func, const Symbol *multilink, const Symbol *agentvar = nullptr);

    /**
     * Create auxiliary symbols associated with this symbol.
     */
    void create_auxiliary_symbols();

    void post_parse(int pass);

    CodeBlock cxx_declaration_agent();

    /**
     * Builds the function body of the expression function.
     */
    void build_body_evaluate();

    /** The function which computes the current value of the agentvar from the multilink */
    AgentFuncSymbol *evaluate_fn;

    /**
     * The aggregate function applied to the multi-link
     * 
     * Is one of TK_count, TK_sum_over, TK_min_over, TK_max_over
     */
    token_type func;

    /**
     * The multilink over which the aggregate function is performed.
     */
    Symbol*& multilink;

    /**
     * The multilink over which the aggregate function is performed.
     */
    AgentMultilinkSymbol *pp_multilink;

    /**
     * The agentvar the aggregate function applies to.
     * 
     * For TK_count, is nullptr.
     */
    Symbol** agentvar;

    /**
     * The agentvar the aggregate function applies to.
     * 
     * For TK_count, is nullptr.
     */
    AgentVarSymbol *pp_agentvar;

};

