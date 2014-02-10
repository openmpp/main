/**
 * @file    Link.h
 * Declares the Link class and associated classes and templates
 *         
 */
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include <vector>

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

    // overload of dereference operator *
    A& operator*()
    {
        if (ptr) {
            // update time of target agent at *ptr
            // using the global time (just-in-time algorithm)
            ptr->time = A::global_time;
            // return reference to agent
            return *ptr;
        }
        else {
            // link is nullptr, return reference to the 'null' agent
            return A::om_null_agent;
        }
    }

    // overload of pointer operator ->
    A* operator->()
    {
        if (ptr) {
            // update time of target agent at *ptr
            // using the global time (just-in-time algorithm)
            ptr->time = A::global_time;
            // return pointer to agent
            return ptr;
        }
        else {
            // link is nullptr, return pointer to the 'null' agent
            return &A::om_null_agent;
        }
    }
};


template <class A>
class multi_link
{
public:
    vector<link<A>> list;
};