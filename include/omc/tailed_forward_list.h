/**
 * @file    Event.h
 * Declares the Event class and associated classes and templates
 *         
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include <forward_list>
using std::forward_list;

/**
    * A forward_list with push_back()
    *
    * @tparam  T   Generic type parameter.
    */

template<typename T>
class tailed_forward_list : public forward_list<T>
{
public:
    tailed_forward_list()
    {
        tail = this->begin();
    }

    void push_back(const T& value)
    {
        if (this->empty()) {
            this->push_front(value);
            tail = this->begin();
        }
        else {
            tail = this->insert_after(tail, value);
        }
    }

private:
    typename::forward_list<T>::iterator tail;
};
