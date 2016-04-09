/**
* @file    EntitySet.h
* Declares the EntitySet template
*
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once

#define RB_TREE 1 // for development branch purposes only - remove once working

#if RB_TREE
	#include "omc/rb_tree.h"
#else
	#include <set>
#endif

#include "omc/less_deref.h"
#include "omc/entity_ptr.h"

using namespace std;


/** Comparison operator for link<E> */
template<typename E>
struct linkE_comp {
  bool operator() (const entity_ptr<E>& lhs, const entity_ptr<E>& rhs) const
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
    EntitySet()
#if RB_TREE
#else
        : is_dirty(true)
#endif
    {}

    ~EntitySet()
    {
#if RB_TREE
        entities.clear(entities.root);
#else
        entities.clear();
        entities_ra.clear();
#endif
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

    entity_ptr<E> at(size_t index)
    {
        if (index < 0 || index >= entities.size()) {
            return nullptr;
        }
#if RB_TREE
        auto result_node = entities.os_select(index + 1);
        //assert(entities.os_rank(result_node) == index + 1); // test of os_rank failed
        return result_node->key;
#else
        if (is_dirty) {
            // reconstruct the random access vector
            entities_ra.clear();
            for (auto lnk : entities) {
                entities_ra.push_back(lnk);
            }
            is_dirty = false;
        }
        return entities_ra[index];
#endif
    }

    // For Modgen compatibility
    entity_ptr<E> Item(size_t index)
    {
        return at(index);
    }

    entity_ptr<E> GetRandom(double uniform_draw)
    {
        if (entities.size() > 0) {
            size_t index = (size_t) (uniform_draw * entities.size());
            return at(index);
        }
        else {
            return nullptr;
        }
    }

    //TODO: Consider recycling rb_node removed by rb_delete in subsequent rb_insert
    // in other tree, possibly
    void insert(E * entity)
    {
#if RB_TREE
        auto tmp = new rb_node<entity_ptr<E>>(entity_ptr<E>(entity), entities.NIL);
	    entities.rb_insert(tmp);
#else
        entities.insert(entity);
        is_dirty = true;
#endif
    }

    void erase(E * entity)
    {
#if RB_TREE
        auto tmp = entities.iterative_tree_search(entity);
        assert(tmp != entities.NIL);
	    entities.rb_delete(tmp);
        delete tmp;
#else
	    entities.erase(entity);
        is_dirty = true;
#endif
    }

    /**
     * Gets the rank (order statistic) of the given entity in an entity set.
     *
     * @param entity If non-null, the entity.
     *
     * @return An int.
     */
    int rank(const E * entity) const
    {
#if RB_TREE
        auto tmp = entities.iterative_tree_search(entity);
        if (tmp == entities.NIL) {
            return 0; // not found
        }
	    return entities.os_rank(tmp);
#else
        for (int i = 0; i < size(); ++i) {
            if (entities_ra[i] == entity) {
                return i + 1;
            }
        }
        return 0; // not found
#endif
    }

private:

    //* storage - a set of entity links, ordered by entity_id
#if RB_TREE
    rb_tree<entity_ptr<E>, linkE_comp<E> > entities;
#else
    set<entity_ptr<E>, linkE_comp<E> > entities;

    //* storage - a vector (for random access) of entity links, ordered as in entities
    vector<entity_ptr<E> > entities_ra;

    //* indicates that entities_ra requires reconstruction
    bool is_dirty;
#endif
};

