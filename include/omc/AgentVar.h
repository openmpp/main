/**
 * @file    AgentVar.h
 * Declares the AgentVar class and associated classes and templates
 *         
 */
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "link.h"

using namespace std;

/**
 * Base template for agentvars.
 * 
 * The return value casting operator is implemented in AgentVar, so C++ code can use the
 * agentvar in expressions. No assignment operators are implemented, making agentvars declared
 * using AgentVar read-only. Side-effects are evaluated if a call to set() changes the value.
 *
 * @tparam T            Type being wrapped
 * @tparam T2           Type of the type being wrapped (e.g. range has inner type int).
 * @tparam A            Type of containing agent.
 * @tparam side_effects Function implementing assignment side effects (constant).
 */
template<typename T, typename T2, typename A, void (A::*side_effects)(T old_value, T new_value) = nullptr>
class AgentVar 
{
public:
    // ctor
    AgentVar()
    {
    }

    // 1-argument ctor for creating r-values for assignment to 'real' AgentVar's in Agent
    AgentVar(T assign_value)
    {
        // N.B. no side-effects
        value = assign_value;
    }

    // 1-argument ctor for creating r-values for assignment to 'real' AgentVar's in Agent, for wrapped types
    //AgentVar(T2 assign_value)
    //{
    //    // N.B. no side-effects
    //    value = assign_value;
    //}

    // initialization
    void initialize( T initial_value )
    {
        value = initial_value;
    }

    // operator: cast to T (use agentvar containing fundamental type)
    operator T()
    {
        return (T) get();
    }

    operator T2()
    {
        return (T2) get();
    }

    // member template to pass conversion inwards to the contained type to do the cast
    //template<typename T1>
    //operator T1()
    //{
    //    return (T1)get();
    //}

    // get pointer to containing agent
    A *agent()
    {
        return (A *) ( (char *)this - offset_in_agent );
    }

    void set( T new_value )
    {
        T old_value = get();
        if ( old_value != new_value ) {
            value = new_value;
            if ( side_effects != nullptr ) {
                (agent()->*side_effects)(old_value, new_value);
            }
        }

    }

    T get() const
    {
        return value;
    }

    // storage
    T value;

	// offset to containing agent
	static size_t offset_in_agent;
};

/**
 * AgentVar offset in Agent (static definition)
 * 
 * The offset is used within an instance of agentvar to gain access to the enclosing agent to
 * call the side-effects function in the context of the agent and with access to all agentvars
 * in the agent.  Agentvars do not contain a pointer to the continaing agent because the memory
 * cost would be prohibitive for simulations with many agents containing many agentvars.
 */
template<typename T, typename T2, typename A, void (A::*side_effects)(T old_value, T new_value)>
size_t AgentVar<T, T2, A, side_effects>::offset_in_agent = 0;

/**
 * Template for assignable agentvars.
 * 
 * All C++ assignment operators are implemented in AssignableAgentVar, allowing developer code to
 * assign values.  Side-effects are performed if the assignment changes the value.
 *
 * @tparam T            Generic type parameter.
 * @tparam A            Type of containing agent.
 * @tparam side_effects Function implementing assignment side effects (constant).
 */
template<typename T, typename T2, typename A, void (A::*side_effects)(T old_value, T new_value) = nullptr>
class AssignableAgentVar : public AgentVar<T, T2, A, side_effects>
{
public:

    // ctor
    AssignableAgentVar()
    {
    }

    // operator: copy assignment
    AssignableAgentVar& operator=(const AssignableAgentVar & other)
    {
        this->set( other.get() );
        return *this;
    }

    // operator: direct assignment of wrapped type
    AssignableAgentVar& operator=( T new_value )
    {
        this->set( new_value );
        return *this;
    }

    // operator: assignment by sum
    AssignableAgentVar& operator+=( T modify_value )
    {
        T new_value = this->get() + modify_value;
        this->set( new_value );
        return *this;
    }

    // operator: assignment by difference
    AssignableAgentVar& operator-=( T modify_value )
    {
        T new_value = this->get() - modify_value;
        this->set( new_value );
        return *this;
    }

    // operator: assignment by product
    AssignableAgentVar& operator*=( T modify_value )
    {
        T new_value = this->get() * modify_value;
        this->set( new_value );
        return *this;
    }

    // operator: assignment by quotient
    AssignableAgentVar& operator/=( T modify_value )
    {
        T new_value = this->get() / modify_value;
        this->set( new_value );
        return *this;
    }

    // operator: assignment by remainder
    AssignableAgentVar& operator%=( T modify_value )
    {
        T new_value = this->get() % modify_value;
        this->set( new_value );
        return *this;
    }

    // operator: assignment by bitwise left shift
    AssignableAgentVar& operator<<=( T modify_value )
    {
        T new_value = this->get() << modify_value;
        this->set( new_value );
        return *this;
    }

    // operator: assignment by bitwise right shift
    AssignableAgentVar& operator>>=( T modify_value )
    {
        T new_value = this->get() >> modify_value;
        this->set( new_value );
        return *this;
    }

    // operator: assignment by bitwise AND
    AssignableAgentVar& operator&=( T modify_value )
    {
        T new_value = this->get() & modify_value;
        this->set( new_value );
        return *this;
    }

    // operator: assignment by bitwise XOR
    AssignableAgentVar& operator^=( T modify_value )
    {
        T new_value = this->get() ^ modify_value;
        this->set( new_value );
        return *this;
    }

    // operator: assignment by bitwise OR
    AssignableAgentVar& operator|=( T modify_value )
    {
        T new_value = this->get() | modify_value;
        this->set( new_value );
        return *this;
    }

    // operator: prefix increment
    AssignableAgentVar& operator++()
    {
        T new_value = this->get() + 1;
        this->set( new_value );
        return *this;
    }

    // operator: prefix decrement
    AssignableAgentVar& operator--()
    {
        T new_value = this->get() - 1;
        this->set( new_value );
        return *this;
    }

    // operator: postfix increment
    T operator++(int)
    {
        T new_value = 1 + this->get();
        this->set( new_value );
        return new_value;
    }

    // operator: postfix decrement
    T operator--(int)
    {
        T new_value = this->get() - 1;
        this->set( new_value );
        return new_value;
    }
};

/**
 * Template for expression agentvars.
 * 
 * No assignment operators are implemented, making the agentvar read-only in C++ code.  A
 * constant template argument holds the function used to evaluate the expression.
 *
 * @tparam T                  Generic type parameter.
 * @tparam A                  Type of containing agent.
 * @tparam side_effects       Function implementing assignment side effects (constant).
 * @tparam expression         Function implementing expression evaluation (constant).
 */
template<typename T, typename T2, typename A, void (A::*side_effects)(T old_value, T new_value), T (A::*expression)()>
class ExpressionAgentVar : public AgentVar<T, T2, A, side_effects>
{
public:
    // evaluate expression and update
    void evaluate()
    {
        T new_value = ((this->agent())->*expression)();
        this->set( new_value );
    }
};

/**
 * Template for duration agentvars.
 * 
 * No assignment operators are implemented, making the agentvar read-only in C++ code.  A
 * constant template argument holds the function used to evaluate the logical condition during
 * which duration will be cumulated.
 *
 * @tparam T            Generic type parameter.
 * @tparam A            Type of containing agent.
 * @tparam side_effects Function implementing assignment side effects (constant).
 * @tparam condition    Function implementing the condition (constant).
 */
template<typename T, typename T2, typename A, void (A::*side_effects)(T old_value, T new_value) = nullptr , bool (A::*condition)() = nullptr>
class DurationAgentVar : public AgentVar<T, T2, A, side_effects>
{
public:
    // update duration 
    void advance( T delta_time )
    {
        bool active = ( condition == nullptr || ((this->agent())->*condition)() );        if ( active ) {
            this->set( this->get() + delta_time );
        }
    }

    // Reset duration to zero
    void reset()
    {
        this->set( 0 );
    }
};

/**
 * Template for link agentvars.
 * 
 * A link agentvar is a smart pointer between agents.  Every link agentvar has a reciprocal link
 * agentvar in another agent.  This makes all links two-way.  That allows side-effects of
 * agentvar changes in an agent to propagate 'backwards' across agent links, to affect events,
 * tables, and expression agentvars in other agents.
 * 
 * When used to retrieve the value of an agentvar in another agent, a link will first
 * synchronize the time of the other agent if the just-in-time simulation option is activated.
 * 
 * To simplify their use in expression agentvars, the pointer and dereference operators can be
 * used even if the link agentvar is nullptr.  In that case, a pre-constructed static 'null
 * agent' whose agentvars are in a freshly initialized state (generally 0) is used for the
 * operation.
 *
 * @tparam T            Storage type, e.g. link<Thing>
 * @tparam A            Type of containing agent, e.g. Person
 * @tparam B            Type of the agent in the link, e.g. Thing
 * @tparam side_effects Function implementing assignment side effects (constant).
 */
template<typename T, typename T2, typename A, typename B, void (A::*side_effects)(T old_value, T new_value) = nullptr>
class LinkAgentVar : public AgentVar<T, T2, A, side_effects>
{
public:

    bool is_nullptr() const {
        return nullptr == this->get().get();
    }

    // operator: direct assignment
    LinkAgentVar& operator=( T new_value )
    {
        this->set( new_value );
        return *this;
    }

    //// operator: assignment of pointer
    //LinkAgentVar& operator=( B *new_value )
    //{
    //    this->set( new_value );
    //    return *this;
    //}

    // operator: pointer
    // Pass it on the contained <link>
    T operator->()
    {
        auto val = this->get();
        return val;
    }

    // operator: dereference
    // Pass it on the contained <link>
    T operator*()
    {
        auto val = this->get();
        return *val;
    }
};

