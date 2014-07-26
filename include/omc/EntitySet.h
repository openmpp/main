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
#include "omc/link.h"

using namespace std;

/**
 * An EntitySet.
 *
 * @tparam E Entity type.
 */
template<
    typename E
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
    EntitySet<E>* operator->()
    {
        // return pointer to this object
        return this;
    }

    // For Modgen compatibility
    size_t Count()
    {
        return entities.size();
    }

    link<E> Item(size_t index)
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

    link<E> GetRandom(double uniform_draw)
    {
        if (entities.size() > 0) {
            size_t index = (size_t) (uniform_draw * entities.size());
            return Item(index);
        }
        else {
            return nullptr;
        }
    }

    void insert(E * entity)
    {
        entities.insert(entity);
    }

    void erase(E * entity)
    {
        entities.erase(entity);
    }

private:
    // storage - a set of entity links, ordered by entity_id
    set<link<E>, less_deref< link<E> > > entities;
    //set<link<E> > entities;
};

