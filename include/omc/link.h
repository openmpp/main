/**
 * @file    Link.h
 * Declares the Link class and associated classes and templates
 *         
 */
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include <vector>
#include "omc/Agent.h"
#include "omc/Event.h"

using namespace std;

template <class A>
class link
{
    // Storage for the real pointer to the agent
    A* ptr;

public:
    // constructor
    link(A* p = nullptr)
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
            // update time of target agent at *ptr
            // using the global time (just-in-time algorithm)
            if (BaseEvent::just_in_time) ptr->time.set(BaseEvent::get_global_time());
            // return pointer to agent
            return ptr;
        }
        else {
            // link is nullptr, return pointer to the 'null' agent
            return &A::om_null_agent;
        }
    }

    // overload of dereference operator *
    A& operator*()
    {
        if (ptr) {
            // update time of target agent at *ptr
            // using the global time (just-in-time algorithm)
            if (BaseEvent::just_in_time) ptr->time.set(BaseEvent::global_time);
            // return reference to agent
            return *ptr;
        }
        else {
            // link is nullptr, return reference to the 'null' agent
            return A::om_null_agent;
        }
    }
};

template <class A>
inline bool operator==(const link<A>& lhs, const link<A>& rhs) { return lhs.get() == rhs.get(); }

template <class A>
inline bool operator==(link<A>& lhs, link<A>& rhs) { return lhs.get() == rhs.get(); }

template <class A>
inline bool operator!=(const link<A>& lhs, const link<A>& rhs) { return !(lhs == rhs); }

template <class A>
inline bool operator!=(link<A>& lhs, link<A>& rhs) { return !(lhs == rhs); }

