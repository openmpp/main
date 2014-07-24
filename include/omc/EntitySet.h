/**
* @file    EntitySet.h
* Declares the EntitySet template
*
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include <set>
#include "omc/less_deref.h"

using namespace std;

/**
 * An EntitySet.
 *
 * @tparam T Entity type.
 */
template<
    typename T
>
class EntitySet
{
public:
    // ctors
    EntitySet()
    {}

    // Overload of pointer operator to support Modgen syntax for entity set member functions.
    // Modgen stores an entity set as a pointer to a separately allocated object,
    // so model code looks like
    //     asAllHosts->Count();
    // whereas ompp stores an entity set as a global, so natural model code looks like this
    //     asAllHosts.Count();
    // The following overload of the pointer operator in effect translates the "->" to ".".
    EntitySet<T>* operator->()
    {
        // return pointer to this object
        return this;
    }

    // For Modgen compatibility
    size_t Count()
    {
        return entities.size();
    }

    T * Item(size_t index)
    {
        // TODO temporary solution because std::set O(n) for random access, unfortunately
        if (entities.size() > 0) {
            auto it = entities.begin();
            advance(it, index);
            return *it;
        }
        else {
            return nullptr;
        }
    }

    T * GetRandom(double uniform_draw)
    {
        if (entities.size() > 0) {
            size_t index = (size_t) (uniform_draw * entities.size());
            return Item(index);
        }
        else {
            return nullptr;
        }
    }

    void Scramble()
    {
    }

private:
    // storage - a set of entities, ordered by entity_id
    set<T *, less_deref<T *> > entities;
};

