/**
* @file    ConditionedDurationAgentVarSymbol.h
* Declarations for the ConditionedDurationAgentVarSymbol class.
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "AgentVarSymbol.h"

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
    ConditionedDurationAgentVarSymbol(const Symbol *agent, const Symbol *observed, const Literal *constant)
        : AgentVarSymbol(ConditionedDurationAgentVarSymbol::member_name(observed, constant),
        agent,
        token::TK_Time
        )
        , observed(observed->get_rpSymbol())
        , constant(constant)
        , pp_observed(nullptr)
    {
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

    static Symbol * get_symbol(const Symbol* agent, const Symbol* observed, const Literal* constant);

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

    CodeBlock cxx_declaration_agent_scope();

    CodeBlock cxx_definition();

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

    const Literal *constant;
    AgentVarSymbol *pp_observed;
};

