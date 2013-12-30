/**
* @file    AgentMemberSymbol.h
* Declares the AgentMemberSymbol derived class of the Symbol class.
* Also declares sub-classes.
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "Symbol.h"

using namespace std;

class AgentMemberSymbol : public Symbol
{
private:
    typedef Symbol super;

public:
    AgentMemberSymbol(Symbol *sym, const Symbol *agent)
        : Symbol(sym)
        , agent(agent->get_rpSymbol())
        , pp_agent(nullptr)
    {
    }

    AgentMemberSymbol(const string name, const Symbol *agent)
        : Symbol(name, agent)
        , agent(agent->get_rpSymbol())
        , pp_agent(nullptr)
    {
    }

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
};

/**
* Symbol for an Agentevent
*
* An AgentEventSymbol is created when an 'event' statement is encountered
* in the model source code.  The name of the symbol
* is the name of the event implement function specified in the 'event' statement.
*/
class AgentEventSymbol : public AgentMemberSymbol
{
private:
    typedef AgentMemberSymbol super;

public:

    /**
    * Constructor.
    *
    * @param [in,out]  sym     The symbol to be morphed (event implement function)
    * @param   agent           The agent.
    * @param   time_function   The symbol for the associated event time function.
    */

    AgentEventSymbol(Symbol *sym, const Symbol *agent, const Symbol *time_function)
        : AgentMemberSymbol(sym, agent)
        , time_function(time_function->get_rpSymbol())
    {
    }

    CodeBlock cxx_declaration_agent_scope();

    void post_parse(int pass);

    /** The Symbol for the time function of the event.*/
    Symbol*& time_function;

    /**
    * Numeric identifier for the event.
    * The numeric identifier is the ordinal of the event name
    * among all events in the model.  If two agents have an
    * event with the same name, \a pp_event_id will be identical
    * for those two events.
    */

    int pp_event_id;
};

class AgentDataMemberSymbol : public AgentMemberSymbol
{
private:
    typedef AgentMemberSymbol super;

public:
    AgentDataMemberSymbol(Symbol *sym, const Symbol *agent, token_type type)
        : AgentMemberSymbol(sym, agent)
        , type(type)
    {
    }

    AgentDataMemberSymbol(const string member_name, const Symbol *agent, token_type type)
        : AgentMemberSymbol(member_name, agent)
        , type(type)
    {
    }


    /**
    * Gets the initial value for the data member.
    *
    * @return The initial value as a string.
    */

    virtual string initial_value() const;

    virtual CodeBlock cxx_initialize_expression() const;


    /**
    * Get the c++ declaration code (in agent scope) associated with the symbol.
    *
    * @return A CodeBlock.
    */

    virtual CodeBlock cxx_declaration_agent_scope();


    /**
    * Get the c++ definition code associated with the symbol.
    *
    * @return A CodeBlock.
    */

    virtual CodeBlock cxx_definition();


    /**
    * Post-parse operations for the symbol.
    *
    * @param pass The pass.
    */

    virtual void post_parse(int pass);


    /**
    * C++ fundamental type.
    * 
    * Recorded as a token value, e.g. int_KW.
    */

    token_type type;

};

/**
* Internal data symbol.
* Abstracts a member of an agent class which stores information
* used internally by om.  These members are not accessible to
* developer code, and have no side-effects.
*/

class AgentInternalSymbol : public AgentDataMemberSymbol
{
private:
    typedef AgentDataMemberSymbol super;

public:
    AgentInternalSymbol(Symbol *sym, const Symbol *agent, token_type type)
        : AgentDataMemberSymbol(sym, agent, type)
    {
    }

    AgentInternalSymbol(const string member_name, const Symbol *agent, token_type type)
        : AgentDataMemberSymbol(member_name, agent, type)
    {
    }

    void post_parse(int pass);

    virtual CodeBlock cxx_initialize_expression() const;

    /**
    * Get the c++ declaration code associated with the symbol.
    */

    virtual CodeBlock cxx_declaration_agent_scope();

    /**
    * Get the c++ definition code associated with the symbol.
    */

    virtual CodeBlock cxx_definition();

};

/**
* Agent callback member symbol.
* Contains functionality to support callback functions at containing agent scope.
*/

class AgentCallbackMemberSymbol : public AgentDataMemberSymbol
{
private:
    typedef AgentDataMemberSymbol super;

public:
    AgentCallbackMemberSymbol(Symbol *sym, const Symbol *agent, token_type type)
        : AgentDataMemberSymbol(sym, agent, type)
    {
    }

    AgentCallbackMemberSymbol(const string member_name, const Symbol *agent, token_type type)
        : AgentDataMemberSymbol(member_name, agent, type)
    {
    }

    void post_parse(int pass);

    /**
    * Get the c++ declaration code associated with the symbol.
    */

    virtual CodeBlock cxx_declaration_agent_scope();

    /**
    * Get the c++ definition code associated with the symbol.
    */

    virtual CodeBlock cxx_definition();

};

/**
* Event time member symbol for an event.
* Abstracts a member of an agent class which stores the event time
* of an agent event.
*/

class AgentEventTimeSymbol : public AgentCallbackMemberSymbol
{
private:
    typedef AgentCallbackMemberSymbol super;

public:
    AgentEventTimeSymbol(const string member_name, const Symbol *agent, const Symbol *event)
        : AgentCallbackMemberSymbol(member_name, agent, token::TK_Time)
        , event(event->get_rpSymbol())
    {
    }

    static string member_name(const Symbol *event);

    static string symbol_name(const Symbol *agent, const Symbol *event);

    void post_parse(int pass);

    /**
    * Gets the initial value for the data member
    * Events are disabled at initialization by setting the event_time to time_infinite.
    * @return  The initial value as a string.
    */

    virtual string initial_value() const
    {
        return token_to_string(token::TK_time_infinite);
    }

    /**
    * Get the c++ declaration code associated with the symbol.
    */

    virtual CodeBlock cxx_declaration_agent_scope();

    /**
    * Get the c++ definition code associated with the symbol.
    */

    virtual CodeBlock cxx_definition();

    /**
    * The associated event.
    */

    Symbol*& event;

    /**
    * The associated event.
    */

    AgentEventSymbol *pp_event;
};

class AgentVarSymbol : public AgentCallbackMemberSymbol
{
private:
    typedef AgentCallbackMemberSymbol super;

public:
    AgentVarSymbol(Symbol *sym, const Symbol *agent, token_type type)
        : AgentCallbackMemberSymbol(sym, agent, type)
    {
    }

    AgentVarSymbol(const string member_name, const Symbol *agent, token_type type)
        : AgentCallbackMemberSymbol(member_name, agent, type)
    {
    }

    void post_parse(int pass);


    /**
    * Get the c++ declaration code associated with the symbol.
    *
    * @return A CodeBlock.
    */

    virtual CodeBlock cxx_declaration_agent_scope();

    /**
    * Get the c++ definition code associated with the symbol.
    */

    virtual CodeBlock cxx_definition();

    /**
    * Get name of member function which implements side-effects
    *
    * Example:  Person::time_side_effects
    *
    * @return  The qualified function name as a string
    */

    const string side_effects_func();

    /**
    * Get declaration of member function which implements side-effects
    *
    * Example:  void time_side_effects(Time old_value, Time new_value)
    * Note that there is no terminating semicolon.
    */

    const string side_effects_decl();

    /**
    * Get qualified declaration of member function which implements side-effects
    *
    * Example:  void Person::time_side_effects(Time old_value, Time new_value)
    * Note that there is no terminating semicolon.
    */

    const string side_effects_decl_qualified();

    /**
    * The lines of C++ code which implement side-effects.
    */

    list<string> pp_side_effects;

};

/**
* BuiltinAgentVarSymbol.
*/
class BuiltinAgentVarSymbol : public AgentVarSymbol
{
private:
    typedef AgentVarSymbol super;

public:
    BuiltinAgentVarSymbol(const string member_name, const Symbol *agent, token_type type)
        : AgentVarSymbol(member_name, agent, type)
    {
    }

    void post_parse(int pass);

    /**
    * Gets the c++ declaration for the builtin agentvar.
    */

    CodeBlock cxx_declaration_agent_scope();

};

/**
* SimpleAgentVarSymbol.
*/
class SimpleAgentVarSymbol : public AgentVarSymbol
{
private:
    typedef AgentVarSymbol super;

public:
    SimpleAgentVarSymbol(Symbol *sym, const Symbol *agent, token_type type, Literal *initializer)
        : AgentVarSymbol(sym, agent, type)
        , initializer(initializer)
    {
    }

    ~SimpleAgentVarSymbol()
    {
        if (initializer != nullptr) delete initializer;
    }

    /**
    * Gets the initial value for the agentvar
    *
    * @return  The initial value as a string.
    */
    string initial_value() const;

    void post_parse(int pass);

    /**
    * Gets the c++ declaration for the simple agentvar.
    */

    CodeBlock cxx_declaration_agent_scope();

    // members

    /** C++ literal to initialize simple agentvar at agent creation */
    Literal *initializer;

};

/**
* DurationAgentVarSymbol.
*
* Symbol for derived agentvars of the form duration().
*/

class DurationAgentVarSymbol : public AgentVarSymbol
{
private:
    typedef AgentVarSymbol super;

public:
    // constructor for 0-argument derived agentvars, e.g. duration()
    DurationAgentVarSymbol(const Symbol *agent)
        : AgentVarSymbol(DurationAgentVarSymbol::member_name(), agent, token::TK_Time)
    {
    }

    static string member_name();

    static string symbol_name(const Symbol *agent);

    /**
    * Get a symbol for a derived agentvar of form duration()
    *
    * @param   agent   The agent.
    *
    * @return  The symbol.
    */

    static Symbol * get_symbol(const Symbol *agent);

    void post_parse(int pass);

    /**
    * Generates the c++ declaration for the derived agentvar.
    */

    CodeBlock cxx_declaration_agent_scope();

};

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

    /**
    * Generates the c++ declaration for the derived agentvar.
    */

    CodeBlock cxx_declaration_agent_scope();

    /**
    * Generates the c++ definition for the derived agentvar.
    */

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

/**
* Symbol for a function member of an agent.
*
* The symbol table contains an AgentFuncSymbol for each agent function,
* apart from event implement functions, which are represented by AgentEventSymbol's.
*/
class AgentFuncSymbol : public AgentMemberSymbol
{
private:
    typedef AgentMemberSymbol super;

public:

    /**
    * Constructor.
    *
    * @param [in,out]  sym The symbol to be morphed.
    */
    AgentFuncSymbol(Symbol *sym, const Symbol *agent)
        : AgentMemberSymbol(sym, agent)
    {
    }

    void post_parse(int pass);

};

