/**
 * @file    Multilink.h
 * Declares the Multilink class and associated classes and templates
 *         
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once

#include "entity_ptr.h"


namespace omr {
    /// The multilink size warning threshold
    constexpr size_t multilink_size_warning_threshold = 1000;
}

/**
 * Template for multilink entity member.
 * 
 * A multilink is a set of links which form one side of a one-to-many or many-to-mnay
 * link among entities.
 *
 * @tparam T            Type of the link in the collection, e.g. link<Thing>
 * @tparam A            Type of containing entity, e.g. Person
 * @tparam B            Type of the entity in the multi-link set, e.g. Thing
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
        // set is empty as part of entity construction
    }

    // get pointer to containing entity
    A *entity()
    {
        return (A *) ( (char *)this - offset_in_entity );
    }

    // standard member functions for std::set containers
    void clear()
    {
        for (auto &item : storage) {
            auto lnk = item.get();
            if (lnk != nullptr) {
                item = nullptr;
                // maintain reciprocal link
                (entity()->*erase_reciprocal)(lnk);
            }
        }
		// process side-effects
        (entity()->*side_effects)();
    }

    size_t size()
    {
        size_t non_empty = 0;
        for (auto &item : storage) {
            if (item.get() != nullptr) ++non_empty;
        }
        return non_empty;
    }

    void insert(entity_ptr<B> lnk)
    {
        bool found = false;
        entity_ptr<B> *hole = nullptr; // pointer to the first hole in the vector, if there is one

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
                if (!multilink_size_warning_issued) {
                    if (omr::multilink_size_warning_threshold < storage.size()) {
                        theLog->logFormatted(LT("warning : A multilink has grown larger than %d - possible performance impact"), omr::multilink_size_warning_threshold);
                        multilink_size_warning_issued = true;
                    }
                }
                if constexpr (om_resource_use_on) {
                    ++slot_count;
                    if (storage.size() > slot_max) {
                        slot_max = storage.size();
                        slot_max_id = entity()->om_get_entity_id();
                    }
                }
            }
    		// process side-effects
            (entity()->*side_effects)();
            // maintain reciprocal link
            (entity()->*insert_reciprocal)(lnk);
	    }
    }

    void erase(entity_ptr<B> lnk)
    {
        for (auto &item : storage) {
            if (item == lnk) {
                item = nullptr;
    		    // process side-effects
                (entity()->*side_effects)();
                // maintain reciprocal link
                (entity()->*erase_reciprocal)(lnk);
                break;
            }
        }
    }

    /**
     * @fn  std::string Multilink::contents(void) const
     *
     * @brief   Gets the contents as a comma-separated list of entity_id's
     *
     * @returns A std::string.
     */
    std::string contents(void) const
    {
        std::string result;
        bool is_first = true;
        for (auto& ptr : storage) {
            if (!ptr) continue; // skip holes indicated by nullptr
            if (is_first) {
                is_first = false;
            }
            else {
                result += ",";
            }
            result += std::to_string(ptr.get()->entity_id);
        }
        return result;
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
        // return pointer to this attribute
        return this;
    }

    // Modgen member functions for multilinks
    void Add(entity_ptr<B> lnk)
    {
        insert(lnk);
    }

    void Remove(entity_ptr<B> lnk)
    {
        erase(lnk);
    }

    entity_ptr<B> GetNext(int start, int *next_pos)
    {
        int	index;
        entity_ptr<B>	lnk;
        entity_ptr<B>	next_lnk = nullptr;

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
        (entity()->*side_effects)();
    }

    // TODO provide access to embedded iterator in std::vector

    // storage
    std::vector<entity_ptr<B>> storage;

    /**
     * Resource use information for the multilink
     */
    static auto resource_use()
    {
        struct result { size_t total_slots; size_t max_slots; size_t max_slots_id; };
        return result { slot_count, slot_max, slot_max_id };
    }

    /**
     * Reset resource use information for the multilink
     */
    static void resource_use_reset()
    {
        slot_count = 0;
        slot_max = 0;
        slot_max_id = 0;
    }

    // offset to containing entity
	static size_t offset_in_entity;

    /**
    * Count of slots in multilink
    *
    * The number of slots in this multilink over all entities.
    */
    static thread_local size_t slot_count;

    /**
    * Max slots in the multilink
    *
    * The maximum number of slots in this multilink over all entities.
    */
    static thread_local size_t slot_max;

    /**
    * entity_id with max slots
    *
    * An entity_id with the maximum number of slots.
    */
    static thread_local size_t slot_max_id;

    /**
    * The run-time warning for excessive link size has been issued for this multilink
    */
    static thread_local bool multilink_size_warning_issued;
};

/**
 * offset in Entity (static definition)
 * 
 * The offset is used within an instance of multilink to gain access to the enclosing entity to
 * call the side-effects function in the context of the entity and with access to all attributes
 * in the entity.
 */
template<typename T, typename A, typename B, void (A::*side_effects)(), void (A::*insert_reciprocal)(T lnk), void (A::*erase_reciprocal)(T lnk) >
size_t Multilink<T, A, B, side_effects, insert_reciprocal, erase_reciprocal>::offset_in_entity = 0;

template<typename T, typename A, typename B, void (A::* side_effects)(), void (A::* insert_reciprocal)(T lnk), void (A::* erase_reciprocal)(T lnk) >
thread_local size_t Multilink<T, A, B, side_effects, insert_reciprocal, erase_reciprocal>::slot_count = 0;

template<typename T, typename A, typename B, void (A::* side_effects)(), void (A::* insert_reciprocal)(T lnk), void (A::* erase_reciprocal)(T lnk) >
thread_local size_t Multilink<T, A, B, side_effects, insert_reciprocal, erase_reciprocal>::slot_max = 0;

template<typename T, typename A, typename B, void (A::* side_effects)(), void (A::* insert_reciprocal)(T lnk), void (A::* erase_reciprocal)(T lnk) >
thread_local size_t Multilink<T, A, B, side_effects, insert_reciprocal, erase_reciprocal>::slot_max_id = 0;

template<typename T, typename A, typename B, void (A::* side_effects)(), void (A::* insert_reciprocal)(T lnk), void (A::* erase_reciprocal)(T lnk) >
thread_local bool Multilink<T, A, B, side_effects, insert_reciprocal, erase_reciprocal>::multilink_size_warning_issued = false;

