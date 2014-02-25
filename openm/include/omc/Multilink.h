/**
 * @file    Multilink.h
 * Declares the Multilink class and associated classes and templates
 *         
 */
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "link.h"

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
template<typename T, typename A, typename B, void (A::*side_effects)(T insert_link, T erase_link) = nullptr>
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
        // set is empty when constructed
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
            if (item.get() != nullptr) {
                item = nullptr;
			    //if ( prThing->spawner == prParentPerson ) {
				   // prThing->Set_spawner( NULL );
			    //}
            }
        }
		//UpdateReferences();
		storage.clear();
    }

    size_t size()
    {
        size_t non_empty = 0;
        for (auto &item : storage) {
            if (item.get() != nullptr) ++non_empty;
        }
        return non_empty;
    }

    void insert(link<B> lnk)
    {
        bool found = false;
        link<B> *hole = nullptr; // pointer to the first hole in the vector, if there is one

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
		    //UpdateReferences();
		    //prThing->Set_spawner( prParentPerson );
	    }
    }

    void erase(link<B> lnk)
    {
        for (auto &item : storage) {
            if (item == lnk) {
                item = nullptr;
			    //UpdateReferences();
			    //if ( prThing->spawner == prParentPerson ) {
				   // prThing->Set_spawner( NULL );
			    //}
                break;
            }
        }
    }

    // Overload of pointer operator to support Modgen syntax for multilink functions.
    // Modgen stores a multi-link as a pointer to a separately allocated object,
    // so model code looks like
    //     mlChildren->Add(newborn);
    // Whereas ompp stores a multilink as a member, so natural model code looks like this
    //     mlChildren.insert(newborn).;
    // The following overload of the pointer operator in effect translates the "->" to ".".
    Multilink<T,A,B,side_effects>* operator->()
    {
        // return pointer to this agentvar
        return this;
    }

    // Modgen member functions for multilinks
    void Add(link<B> lnk)
    {
        insert(lnk);
    }

    void Remove(link<B> lnk)
    {
        erase(lnk);
    }

    link<B> GetNext(int start, int *next_pos)
    {
        int	index;
        link<B>	lnk;
        link<B>	next_lnk = nullptr;

        *next_pos = -1;
        for ( index = start; index < storage.size(); ++index ) {
            lnk = storage[index];
            if ( lnk != nullptr ) {
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
    }

    // TODO provide access to embedded iterator in std::vector

    // storage
    vector<link<B>> storage;

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
template<typename T, typename A, typename B, void (A::*side_effects)(T insert_link, T erase_link)>
size_t Multilink<T, A, B, side_effects>::offset_in_agent = 0;
