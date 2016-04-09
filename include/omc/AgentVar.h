/**
 * @file    AgentVar.h
 * Declares the AgentVar class and associated classes and templates
 *         
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once

#include "entity_ptr.h"

using namespace std;

/**
 * Base template for attributes.
 * 
 * The return value casting operator is implemented in this template, so C++ code can use the
 * attribute in expressions. No assignment operators are implemented, making entities declared
 * using AgentVar read-only. Side-effects are evaluated if a call to set() changes the value.
 * 
 * A bug in VC++ prevented the use of nullptr for NT_side_effects.  The parameter NT_se_present
 * is a work-around for this bug.  Ditto for NT_notify and NT_ntfy_present.
 *
 * @tparam T               Type of the attribute.
 * @tparam T2              Type of the type being wrapped (e.g. range has inner type int).
 * @tparam A               Type of containing entity.
 * @tparam NT_name         Name of the attribute (non-type parameter).
 * @tparam NT_side_effects Function implementing assignment side effects (non-type parameter).
 * @tparam NT_se_present   Assignment side-effects are present (non-type parameter).
 * @tparam NT_notify       Function implementing pre-notification of change in value (non-type parameter).
 * @tparam NT_ntfy_present Assignment pre-notification actions are present (non-type parameter).
 */
template<
    typename T,
    typename T2,
    typename A,
    string const *NT_name,
    void (A::*NT_side_effects)(T old_value, T new_value),
    bool NT_se_present,
    void (A::*NT_notify)(),
    bool NT_ntfy_present
>
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

            // Before the value of the attribute changes
            if (NT_ntfy_present) {
                (agent()->*NT_notify)();
            }

            // Change the attribute to the new value
            value = new_value;

            // After the attribute value has changed
            if (NT_se_present) {
                (agent()->*NT_side_effects)(old_value, new_value);
            }

        }

    }

    T get() const
    {
        return value;
    }

    /**
     * Gets the name of the attribute
     *
     * @return The name.
     */
    static const string & get_name()
    {
        return NT_name;
    }

    // storage
    T value;

	// offset to containing agent
	static size_t offset_in_agent;
};

/**
 * Attribute offset in entity (static definition)
 * 
 * The offset is used within an instance of attribute to gain access to the enclosing entity to
 * call the side-effects function in the context of the entity and with access to all attributes
 * in the entity.  Attributes do not contain a pointer to the containing entity because the memory
 * cost would be prohibitive for simulations with many entities containing many attributes.
 */
template<
    typename T,
    typename T2,
    typename A,
    string const *NT_name,
    void (A::*NT_side_effects)(T old_value, T new_value),
    bool NT_se_present,
    void (A::*NT_notify)(),
    bool NT_ntfy_present
>
size_t AgentVar<T, T2, A, NT_name, NT_side_effects, NT_se_present, NT_notify, NT_ntfy_present>::offset_in_agent = 0;

/**
 * Template for assignable attributes.
 * 
 * All C++ assignment operators are implemented in AssignableAgentVar, allowing developer code to
 * assign values.  Side-effects are executed if the assignment changes the value.
 *
 * @tparam T               Generic type parameter.
 * @tparam T2              Type of the type being wrapped (e.g. range has inner type int).
 * @tparam A               Type of containing agent.
 * @tparam NT_name         Name of the attribute (non-type argument).
 * @tparam NT_side_effects Function implementing assignment side effects (non-type argument).
 * @tparam NT_se_present   Assignment side-effects are present (non-type parameter).
 * @tparam NT_notify       Function implementing pre-notification of change in value (non-type parameter).
 * @tparam NT_ntfy_present Assignment pre-notification actions are present (non-type parameter).
 */
template<
    typename T,
    typename T2,
    typename A,
    string const *NT_name,
    void (A::*NT_side_effects)(T old_value, T new_value),
    bool NT_se_present,
    void (A::*NT_notify)(),
    bool NT_ntfy_present
>
class AssignableAgentVar : public AgentVar<T, T2, A, NT_name, NT_side_effects, NT_se_present, NT_notify, NT_ntfy_present>
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
 * Template for expression attributes.
 * 
 * No assignment operators are implemented, making the attribute read-only in C++ code.  A
 * constant template argument holds the function used to evaluate the expression.
 *
 * @tparam T                  Type of the attribute.
 * @tparam T2                 Type of the type being wrapped (e.g. range has inner type int).
 * @tparam A                  Type of the containing entity.
 * @tparam NT_name            Name of the attribute (non-type argument).
 * @tparam NT_side_effects    Function implementing assignment side effects (non-type parameter).
 * @tparam NT_se_present      Assignment side-effects function is empty (non-type parameter).
 * @tparam NT_ntfy_present Assignment pre-notification actions are present (non-type parameter).
 * @tparam NT_notify          Function implementing pre-notification of change in value (non-type parameter).
 * @tparam NT_expression      Function implementing expression evaluation (non-type parameter).
 */
template<
    typename T,
    typename T2,
    typename A,
    string const *NT_name,
    void (A::*NT_side_effects)(T old_value, T new_value),
    bool NT_se_present,
    void (A::*NT_notify)(),
    bool NT_ntfy_present,
    T (A::*NT_expression)()
>
class ExpressionAgentVar : public AgentVar<T, T2, A, NT_name, NT_side_effects, NT_se_present, NT_notify, NT_ntfy_present>
{
public:
    // evaluate expression and update
    void evaluate()
    {
        T new_value = ((this->agent())->*NT_expression)();
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
 * @tparam T               Type of the attribute.
 * @tparam T2              Type of the type being wrapped (e.g. range has inner type int).
 * @tparam A               Type of the containing entity.
 * @tparam NT_name         Name of the attribute (non-type argument).
 * @tparam NT_side_effects Function implementing assignment side effects (non-type parameter).
 * @tparam NT_se_present   Assignment side-effects are present (non-type parameter).
 * @tparam NT_notify       Function implementing pre-notification of change in value (non-type parameter).
 * @tparam NT_ntfy_present Assignment pre-notification actions are present (non-type parameter).
 * @tparam NT_condition    Function implementing the condition (non-type parameter).
 */
template<
    typename T,
    typename T2,
    typename A,
    string const *NT_name,
    void (A::*NT_side_effects)(T old_value, T new_value),
    bool NT_se_present,
    void (A::*NT_notify)(),
    bool NT_ntfy_present,
    bool (A::*NT_condition)() = nullptr
>
class DurationAgentVar : public AgentVar<T, T2, A, NT_name, NT_side_effects, NT_se_present, NT_notify, NT_ntfy_present>
{
public:
    // update duration 
    void advance( T delta_time )
    {
        bool active = ( NT_condition == nullptr || ((this->agent())->*NT_condition)() );
        if ( active ) {
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
 * @tparam T               Storage type, e.g. link<Thing>
 * @tparam T2              Type of the type being wrapped (e.g. range has inner type int).
 * @tparam A               Type of the containing entity.
 * @tparam B               Type of the entity in the link, e.g. Thing
 * @tparam NT_name         Name of the attribute (non-type parameter).
 * @tparam NT_side_effects Function implementing assignment side effects (non-type parameter).
 * @tparam NT_se_present   Assignment side-effects are present (non-type parameter).
 * @tparam NT_notify       Function implementing pre-notification of change in value (non-type parameter).
 * @tparam NT_ntfy_present Assignment pre-notification actions are present (non-type parameter).
 */
template<
    typename T,
    typename T2,
    typename A,
    typename B,
    string const *NT_name,
    void (A::*NT_side_effects)(T old_value, T new_value),
    bool NT_se_present,
    void (A::*NT_notify)(),
    bool NT_ntfy_present
>
class LinkAgentVar : public AgentVar<T, T2, A, NT_name, NT_side_effects, NT_se_present, NT_notify, NT_ntfy_present>
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
    // Pass it on the contained <entity_ptr>
    T operator->()
    {
        auto val = this->get();
        return val;
    }

    // operator: dereference
    // Pass it on the contained <entity_ptr>
    T operator*()
    {
        auto val = this->get();
        return *val;
    }
};

