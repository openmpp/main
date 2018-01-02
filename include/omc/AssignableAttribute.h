/**
 * @file    AssignableAttribute.h
 * Declares the AssignableAttribute template
 *         
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once

#include "Attribute.h"

using namespace std;


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
    //AssignableAgentVar(const AssignableAgentVar&) = delete;
    //AssignableAgentVar& operator=(const AssignableAgentVar&) = delete;

    // ctor
    AssignableAgentVar()
    {
    }

    // operator: copy assignment
    //AssignableAgentVar& operator=(const AssignableAgentVar & other)
    //{
    //    this->set( other.get() );
    //    return *this;
    //}

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
