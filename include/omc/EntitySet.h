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


/** Comparison operator for link<E> */
template<typename E>
struct linkE_comp {
  bool operator() (const link<E>& lhs, const link<E>& rhs) const
  { return *lhs.get() < *rhs.get(); }
};


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
        : is_dirty(true)
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

    size_t size()
    {
        return entities.size();
    }

    // For Modgen compatibility
    size_t Count()
    {
        return size();
    }

    link<E> at(size_t index)
    {
        if (index < 0 || index >= entities.size()) {
            return nullptr;
        }
        if (is_dirty) {
            // reconstruct the random access vector
            entities_ra.clear();
            for (auto lnk : entities) {
                entities_ra.push_back(lnk);
            }
            is_dirty = false;
        }
        return entities_ra[index];
    }

    // For Modgen compatibility
    link<E> Item(size_t index)
    {
        return at(index);
    }

    link<E> GetRandom(double uniform_draw)
    {
        if (entities.size() > 0) {
            size_t index = (size_t) (uniform_draw * entities.size());
            return entities_ra[index];
        }
        else {
            return nullptr;
        }
    }

    void insert(E * entity)
    {
        entities.insert(entity);
        is_dirty = true;
    }

    void erase(E * entity)
    {
        entities.erase(entity);
        is_dirty = true;
    }

private:

    //* storage - a set of entity links, ordered by entity_id
    set<link<E>, linkE_comp<E> > entities;

    //* storage - a vector of entity links, ordered as in entities
    vector<link<E> > entities_ra;

    //* indicates that entities_ra requires reconstruction
    bool is_dirty;
};

