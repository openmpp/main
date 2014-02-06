/**
* @file    ConditionedDurationAgentVarSymbol.h
* Declarations for the ConditionedDurationAgentVarSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "AgentVarSymbol.h"
#include "AgentFuncSymbol.h"
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

    ConditionedDurationAgentVarSymbol(const Symbol *agent, const Symbol *observed, const Literal *constant)
        : AgentVarSymbol(ConditionedDurationAgentVarSymbol::member_name(observed, constant),
                        agent,
                        NumericSymbol::find(token::TK_Time) )
        , observed(observed->stable_rp())
        , constant(constant)
        , pp_observed(nullptr)
    {
        // Create an AgentFuncSymbol for the condition function
        condition_fn = new AgentFuncSymbol("om_condition_" + name, agent, "bool", "");
        assert(condition_fn); // out of memory check
        condition_fn->func_body += "return (" + observed->name + " == " + constant->value() + ");" ;
        condition_fn->doc_block = doxygen_short("Evaluate condition for " + pretty_name() + ".");
    }

    /**
    * The member name for a specific symbol of this kind
    *
    * @param   observed    The observed agentvar
    * @param   constant    The constant with which the agentvar is compared to condition the duration
    *
    * @return  The member namne as a string
    */

    static string member_name(const Symbol *observed, const Literal *constant);

    static string symbol_name(const Symbol* agent, const Symbol* observed, const Literal* constant);

    static Symbol * create_symbol(const Symbol* agent, const Symbol* observed, const Literal* constant);

    void post_parse(int pass);

    /**
    * Get name of member function which implements the condition expression
    *
    * Example:  om_duration_happy_true_condition.
    *
    * @return  .
    */

    const string condition_func();

    /**
    * Get declaration of static member function which implements the condition expression
    *
    * Example:  bool om_duration_happy_true_condition()
    */

    const string condition_decl();

    /**
    * Get declaration of static member function which implements the condition expression
    *
    * Example:  bool Person::om_duration_happy_true_condition()
    */

    const string condition_decl_qualified();

    CodeBlock cxx_declaration_agent();

    CodeBlock cxx_definition_agent();

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

    const Literal *constant;

    /** The condition function.*/
    AgentFuncSymbol *condition_fn;
};

