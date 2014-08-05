/**
* @file    ConditionedDurationAgentVarSymbol.h
* Declarations for the ConditionedDurationAgentVarSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "AgentVarSymbol.h"
#include "AgentFuncSymbol.h"
#include "ConstantSymbol.h"
#include "NumericSymbol.h"
#include "Literal.h"

using namespace std;

/**
* ConditionedDurationAgentVarSymbol
*
* Symbol for derived agentvars of the form duration( agentvar, value).
* E.g. duration(happy, true)
*/

class ConditionedDurationAgentVarSymbol : public AgentVarSymbol
{
private:
    typedef AgentVarSymbol super;

public:
    bool is_base_symbol() const { return false; }

    /**
     * Constructor for literal second argument
     *
     * @param agent    The agent.
     * @param observed The observed.
     * @param constant The constant.
     */
    ConditionedDurationAgentVarSymbol(const Symbol *agent, const Symbol *observed, const ConstantSymbol *constant, yy::location decl_loc = yy::location())
        : AgentVarSymbol(ConditionedDurationAgentVarSymbol::member_name(observed, constant),
                        agent,
                        NumericSymbol::find(token::TK_Time),
                        decl_loc
                        )
        , observed(observed->stable_rp())
        , pp_observed(nullptr)
        , constant(constant)
    {
        // Create an AgentFuncSymbol for the condition function
        condition_fn = new AgentFuncSymbol(name + "_condition", agent, "bool", "");
        assert(condition_fn); // out of memory check
        condition_fn->func_body += "return (" + observed->name + " == " + constant->value() + ");" ;
        condition_fn->doc_block = doxygen_short("Evaluate condition for " + pretty_name() + ".");
    }

    /**
     * The member name for a specific symbol of this kind.
     *
     * @param observed The observed agentvar.
     * @param constant The constant with which the agentvar is compared to condition the duration.
     *
     * @return The member namne as a string.
     */
    static string member_name(const Symbol *observed, const ConstantSymbol *constant);

    static string symbol_name(const Symbol* agent, const Symbol* observed, const ConstantSymbol* constant);

    static Symbol * create_symbol(const Symbol* agent, const Symbol* observed, const ConstantSymbol* constant, yy::location decl_loc);

    void post_parse(int pass);

    CodeBlock cxx_declaration_agent();

    /**
     * agentvar observed (reference to pointer)
     * 
     * Stable to symbol morphing during parse phase.
     */
    Symbol*& observed;

    /**
     * agentvar observed (pointer)
     * 
     * Only valid after post-parse phase 1.
     */
    AgentVarSymbol *pp_observed;

    const ConstantSymbol *constant;

    /** The condition function.*/
    AgentFuncSymbol *condition_fn;
};

