/**
 * @file    entity_ptr.h
 * Declares the entity_ptr class and associated classes and templates
 *         
 */
// Copyright (c) 2013-2021 OpenM++ contributors
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once

#include <vector>
#include <cmath>
#include "omc/Entity.h"
#include "omc/Event.h"

template <class A>
class entity_ptr
{
    // Storage for the real pointer to the entity
    A* ptr;

public:
    // ctor
    entity_ptr()
        : ptr(nullptr)
    {
    }

    // ctor
    entity_ptr(A *p)
        : ptr(p)
    {
    }

    // conversion to bool (to check if nullptr)
    operator bool() const
    {
        return nullptr != ptr;
    }

    // backdoor to wrapped pointer
    A *get() const
    {
        return ptr;
    }

    // overload of pointer operator ->
    A* operator->()
    {
        if (ptr) {
            // Update time of target entity at *ptr with the global time
            // if using the just-in-time algorithm.
            if (BaseEvent::just_in_time) {
                auto current_global_time = BaseEvent::get_global_time();
                // Do not synchronize time if the global time is -inf.
                // Global time is -inf before the first event in the simulation.
                if (!std::isinf(current_global_time)) {
                    if (ptr->time < current_global_time) {
                        // Synchronize time of the entity referred to through the pointer.
                        if (om_verify_attribute_modification) { // is constexpr
                            // construct a sandwich allowing modification of time attribute for just-in-time
                            bool saved = BaseEntity::om_permit_attribute_modification;
                            BaseEntity::om_permit_attribute_modification = true;
                            ptr->time.set(current_global_time);
                            BaseEntity::om_permit_attribute_modification = saved;
                        }
                        else {
                            ptr->time.set(current_global_time);
                        }
                    }
                    else if (!BaseEvent::allow_clairvoyance && ptr->time > current_global_time) {
                        // This is an attempt to access an entity in the local future of the current entity.
                        // Write error message to log and throw run-time exception.
                        handle_clairvoyance(current_global_time, ptr->time, ptr->entity_id);
                    }
                    else {
                        // Time is already synchronized between the two entities.
                        // Nothing to do.
                    }
                }
            }
            // return pointer to entity
            return ptr;
        }
        else {
            if (BaseEntity::om_access_to_null_entity) {
                // we are here only if evaluating an identity attribute expression
                return &A::om_null_agent;
            }
            else {
                // an attempt to derefernece null pointer to entity in model code
                handle_null_dereference();
                // NOT_REACHED because exception thrown above
                return nullptr;
            }
        }
    }

    // overload of dereference operator *
    A& operator*()
    {
        // a nearly exact copy of the -> operator defined above
        if (ptr) {
            // Update time of target entity at *ptr with the global time
            // if using the just-in-time algorithm.
            if (BaseEvent::just_in_time) {
                auto current_global_time = BaseEvent::get_global_time();
                // Do not synchronize time if the global time is -inf.
                // Global time is -inf before the first event in the simulation.
                if (!std::isinf(current_global_time)) {
                    if (ptr->time < current_global_time) {
                        // Synchronize time of the entity referred to through the pointer.
                        ptr->time.set(current_global_time);
                    }
                    else if (!BaseEvent::allow_clairvoyance && ptr->time > current_global_time) {
                        // This is an attempt to access an entity in the local future of the current entity.
                        // Write error message to log and throw run-time exception.
                        handle_clairvoyance(current_global_time, ptr->time, ptr->entity_id);
                    }
                    else {
                        // Time is already synchronized between the two entities.
                        // Nothing to do.
                    }
                }
            }
            // return reference to entity
            return *ptr;
        }
        else {
            // entity_ptr is nullptr, return reference to the 'null' entity
            return A::om_null_agent;
        }
    }
};

template <class A>
inline bool operator==(const entity_ptr<A>& lhs, const entity_ptr<A>& rhs) { return lhs.get() == rhs.get(); }

template <class A>
inline bool operator==(entity_ptr<A>& lhs, entity_ptr<A>& rhs) { return lhs.get() == rhs.get(); }

template <class A>
inline bool operator!=(const entity_ptr<A>& lhs, const entity_ptr<A>& rhs) { return !(lhs == rhs); }

template <class A>
inline bool operator!=(entity_ptr<A>& lhs, entity_ptr<A>& rhs) { return !(lhs == rhs); }

