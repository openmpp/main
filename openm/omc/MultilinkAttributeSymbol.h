/**
* @file    MultilinkAttributeSymbol.h
* Declarations for the MultilinkAttributeSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include "AttributeSymbol.h"
#include "NumericSymbol.h"
#include "UnknownTypeSymbol.h"

class EntityMultilinkSymbol;

using namespace std;

/**
 * MultilinkAttributeSymbol
 * 
 * Symbol for multilink agentvars of the form count(multilink), sum_over(multilink, agentvar),
 * etc.
 */
class MultilinkAttributeSymbol : public AttributeSymbol
{
private:
    typedef AttributeSymbol super;

public:
    bool is_base_symbol() const { return false; }

    /**
     * Constructor.
     *
     * @param agent     The agent.
     * @param func      The function, e.g. TK_min_over
     * @param multilink The multilink, e.g. things
     * @param agentvar  (Optional) the agentvar.
     *                  
     *  The 'data_type' argument to the AttributeSymbol constructor is interim and
     *  may be changed once the data type of agentvar is known.
     */
    MultilinkAttributeSymbol(const Symbol *agent, token_type func, const Symbol *multilink, const string agentvar)
        : AttributeSymbol(MultilinkAttributeSymbol::member_name(func, multilink, agentvar),
                        agent,
                        UnknownTypeSymbol::find() )
        , func(func)
        , multilink(multilink->stable_rp())
        , pp_multilink(nullptr)
        , agentvar(agentvar)
        , pp_agentvar(nullptr)
    {
        create_auxiliary_symbols();
    }

    /**
     * Builds the function body of the evaluate function.
     */
    void build_body_evaluate();

    static string member_name(token_type func, const Symbol *multilink, const string agentvar);

    static string symbol_name(const Symbol *agent, token_type func, const Symbol *multilink, const string agentvar);

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
    static Symbol * create_symbol(const Symbol *agent, token_type func, const Symbol *multilink, const string agentvar);

    /**
     * Create auxiliary symbols associated with this symbol.
     */
    void create_auxiliary_symbols();

    void post_parse(int pass);

    CodeBlock cxx_declaration_agent();

    /** The function which computes the current value of the agentvar from the multilink */
    EntityFuncSymbol *evaluate_fn;

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
    EntityMultilinkSymbol *pp_multilink;

    /**
     * The agentvar the aggregate function applies to.  Is empty for TK_count
     */
    const string agentvar;
    AttributeSymbol *pp_agentvar;

};

