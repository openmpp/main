/**
 * @file    Multilink.h
 * Declares the Multilink class and associated classes and templates
 *         
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once

#include "link_ptr.h"

using namespace std;

/**
 * Template for multilink agent member.
 * 
 * A multilink is a set of links which form one side of a one-to-many or many-to-mnay
 * link among agents.
 *
 * @tparam T            Type of the link in the collection, e.g. link<Thing>
 * @tparam A            Type of containing agent, e.g. Person
 * @tparam B            Type of the agent in the multi-link set, e.g. Thing
 * @tparam side_effects Function implementing assignment side effects (constant).
 */
template<typename T, typename A, typename B, void (A::*side_effects)(), void (A::*insert_reciprocal)(T lnk), void (A::*erase_reciprocal)(T lnk) >
class Multilink
{
public:

    // ctor
    explicit Multilink()
    {
    }

    // initialization
    void initialize( T initial_value )
    {
        // set is empty as part of agent construction
    }

    // get pointer to containing agent
    A *agent()
    {
        return (A *) ( (char *)this - offset_in_agent );
    }

    // standard member functions for std::set containers
    void clear()
    {
        for (auto &item : storage) {
            auto lnk = item.get();
            if (lnk != nullptr) {
                item = nullptr;
                // maintain reciprocal link
                (agent()->*erase_reciprocal)(lnk);
            }
        }
		// process side-effects
        (agent()->*side_effects)();
    }

    size_t size()
    {
        size_t non_empty = 0;
        for (auto &item : storage) {
            if (item.get() != nullptr) ++non_empty;
        }
        return non_empty;
    }

    void insert(link_ptr<B> lnk)
    {
        bool found = false;
        link_ptr<B> *hole = nullptr; // pointer to the first hole in the vector, if there is one

	    for (auto &item : storage) {
		    if (item  == lnk) {
			    found = true;
			    break;
		    }
		    if (item.get() == nullptr && hole == nullptr) {
                // save pointer to the first hole in the vector
			    hole = &item;
		    }
	    }
	    if ( !found ) {
            // element not found, so add it.
		    if ( hole != nullptr ) {
                // found a hole in the vector, store the new element there
			    *hole = lnk;
		    }
		    else {
                // append the new element to the end of the array
			    storage.push_back(lnk);
		    }
    		// process side-effects
            (agent()->*side_effects)();
            // maintain reciprocal link
            (agent()->*insert_reciprocal)(lnk);
	    }
    }

    void erase(link_ptr<B> lnk)
    {
        for (auto &item : storage) {
            if (item == lnk) {
                item = nullptr;
    		    // process side-effects
                (agent()->*side_effects)();
                // maintain reciprocal link
                (agent()->*erase_reciprocal)(lnk);
                break;
            }
        }
    }

    // Overload of pointer operator to support Modgen syntax for multilink functions.
    // Modgen stores a multi-link as a pointer to a separately allocated object,
    // so model code looks like
    //     mlChildren->Add(newborn);
    // whereas ompp stores a multilink as a member, so natural model code looks like this
    //     mlChildren.insert(newborn).;
    // The following overload of the pointer operator in effect translates the "->" to ".".
    Multilink<T,A,B,side_effects,insert_reciprocal,erase_reciprocal>* operator->()
    {
        // return pointer to this agentvar
        return this;
    }

    // Modgen member functions for multilinks
    void Add(link_ptr<B> lnk)
    {
        insert(lnk);
    }

    void Remove(link_ptr<B> lnk)
    {
        erase(lnk);
    }

    link_ptr<B> GetNext(int start, int *next_pos)
    {
        int	index;
        link_ptr<B>	lnk;
        link_ptr<B>	next_lnk = nullptr;

        *next_pos = -1;
        for ( index = start; index < (int)storage.size(); ++index ) {
            lnk = storage[index];
            if ( lnk ) {
                next_lnk = lnk;
                *next_pos = index;
                break;
            }
        }
        return next_lnk;
    }

    void RemoveAll()
    {
        clear();
    }

    void FinishAll()
    {
        for (auto &item : storage) {
            auto lnk = item.get();
            if (lnk != nullptr) {
                item->Finish();
                item = nullptr;
            }
        }
		// process side-effects
        (agent()->*side_effects)();
    }

    // TODO provide access to embedded iterator in std::vector

    // storage
    vector<link_ptr<B>> storage;

	// offset to containing agent
	static size_t offset_in_agent;

};

/**
 * offset in Agent (static definition)
 * 
 * The offset is used within an instance of multilink to gain access to the enclosing agent to
 * call the side-effects function in the context of the agent and with access to all agentvars
 * in the agent.
 */
template<typename T, typename A, typename B, void (A::*side_effects)(), void (A::*insert_reciprocal)(T lnk), void (A::*erase_reciprocal)(T lnk) >
size_t Multilink<T, A, B, side_effects, insert_reciprocal, erase_reciprocal>::offset_in_agent = 0;
