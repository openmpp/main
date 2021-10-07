/**
 * @file    LinkAttribute.h
 * Declares the LinkAttribute template
 *         
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once

#include "Attribute.h"
#include "entity_ptr.h"

/**
 * Template for link attributes.
 * 
 * A link attribute is a smart pointer between agents.  Every link attribute has a reciprocal link
 * attribute in another agent.  This makes all links two-way.  That allows side-effects of
 * attribute changes in an agent to propagate 'backwards' across agent links, to affect events,
 * tables, and expression attributes in other agents.
 * 
 * When used to retrieve the value of an attribute in another agent, a link will first
 * synchronize the time of the other agent if the just-in-time simulation option is activated.
 * 
 * To simplify their use in expression attributes, the pointer and dereference operators can be
 * used even if the link attribute is nullptr.  In that case, a pre-constructed static 'null
 * agent' whose attributes are in a freshly initialized state (generally 0) is used for the
 * operation.
 *
 * @tparam T               Storage type, e.g. link<Thing>
 * @tparam T2              Type of the type being wrapped (e.g. range has inner type int).
 * @tparam A               Type of the containing entity.
 * @tparam B               Type of the entity in the link, e.g. Thing
 * @tparam NT_name         Name of the attribute (non-type parameter).
 * @tparam NT_is_time_like Attribute can change between events, like time.
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
    std::string const *NT_name,
    bool NT_is_time_like,
    void (A::*NT_side_effects)(T old_value, T new_value),
    bool NT_se_present,
    void (A::*NT_notify)(),
    bool NT_ntfy_present
>
class LinkAttribute : public Attribute<T, T2, A, NT_name, NT_is_time_like, NT_side_effects, NT_se_present, NT_notify, NT_ntfy_present>
{
public:
    LinkAttribute(const LinkAttribute&) = delete; // copy constructor
    LinkAttribute& operator=(const LinkAttribute&) = delete; // copy initialization operator

    // ctor
    LinkAttribute()
    {
        this->set(nullptr);
    }

    bool is_nullptr() const {
        return nullptr == this->get().get();
    }

    // operator: direct assignment
    LinkAttribute& operator=( T new_value )
    {
        this->set( new_value );
        return *this;
    }

    //// operator: assignment of pointer
    //LinkAttribute& operator=( B *new_value )
    //{
    //    this->set( new_value );
    //    return *this;
    //}

    // operator: pointer
    // Pass it on to the contained <entity_ptr>
    T operator->()
    {
        auto val = this->get();
        return val;
    }

    // operator: dereference
    // Pass it on to the contained <entity_ptr>
    B& operator*()
    {
        auto val = this->get();
        return *val;
    }
};
