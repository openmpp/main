/**
 * @file    Event.h
 * Declares the Event class and associated classes and templates
 *         
 */
// Copyright (c) 2013-2016 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include <set>
#include <sstream>
#include <cassert>
#include "omc/less_deref.h"
#include "omc/Entity.h"
#include "omc/event_priorities.h"
#include "om_types0.h" // for Time


// for testing
#define OPTIMIZE_DISABLED_EVENTS 1

class BaseEvent
{
public:
    // ctor
    explicit BaseEvent()
    {
    }

    // initialization
    void initialize( Time initial_value = time_infinite )
    {
		event_time = initial_value;
        memory = 0;
        in_queue = false;
        is_dirty = false;
        is_zombie = false;
    }

    void make_dirty()
    {
        assert(dirty_events);
        if ( ! is_zombie && ! is_dirty ) {
            is_dirty = true;
            dirty_events->insert( this );
        }
    }

    void make_zombie()
    {
        assert(dirty_events);
        if ( ! is_zombie ) {
            is_zombie = true;
            dirty_events->insert( this );
        }
    }

    virtual int get_event_id() const = 0;

    virtual int get_event_priority() const = 0;

    virtual int get_modgen_event_num() const = 0;

    virtual int get_entity_id() = 0;

    virtual Time get_time() = 0;

    virtual void call_implement_func() = 0;

    virtual Time call_time_func() = 0;

    virtual void call_age_agent() = 0;

    /**
     * Verify time, then age the entity of this event to the time of event occurrence.
     */
    void verified_age_agent()
    {
        if (!allow_time_travel && event_time < get_time()) {
            // The time of this event is in the local past of the entity within which the event occurs.
            // This is an error in model logic.
            // Write error message to log and throw run-time exception.
            handle_backwards_time(
                get_time(),
                event_time,
                get_event_id(),
                get_entity_id()
            );
        }
        call_age_agent();
    }

    /**
     * Cleans this event. Removes if zombie and updates event time if dirty.
     */
    void clean()
    {
        assert(event_queue);
        if ( is_zombie ) {
            if (in_queue) {
                event_queue->erase(this);
                in_queue = false;
            }
        }
        else if ( is_dirty ) {
            Time new_event_time = call_time_func();
            if ( in_queue ) {
                if ( new_event_time != event_time ) {
                    event_queue->erase( this );
                    event_time = new_event_time;
#if OPTIMIZE_DISABLED_EVENTS
                    if (event_time == time_infinite) {
                        in_queue = false;
                    }
                    else {
                        event_queue->insert(this);
                    }
#else
                    event_queue->insert( this );
#endif
                }
            }
            else // ! in_queue
            {
                event_time = new_event_time;
#if OPTIMIZE_DISABLED_EVENTS
                if (event_time != time_infinite) {
                    event_queue->insert(this);
                    in_queue = true;
                }
#else
                event_queue->insert( this );
                in_queue = true;
#endif
            }
        }
        is_dirty = false;
    }

    /**
     * event comparison. This is a true observer function but is not declared as const due to issues
     * with get_event_id().
     *
     * @param [in,out] rhs The right hand side.
     *
     * @return true if the right-hand side is less than the left-hand side (this object).
     */
    bool operator< ( BaseEvent& rhs )
    {
		// earlier event time wins
		if ( event_time < rhs.event_time ) return true;
		if ( event_time > rhs.event_time ) return false;

		// higher event priority wins
		int event_priority = get_event_priority();
		int rhs_event_priority = rhs.get_event_priority();
		if ( event_priority > rhs_event_priority ) return true;
		if ( event_priority < rhs_event_priority ) return false;

		// lower event_id wins (earlier in alphabetic order by event name)
		int event_id = get_event_id();
		int rhs_event_id = rhs.get_event_id();
		if ( event_id < rhs_event_id ) return true;
		if ( event_id > rhs_event_id ) return false;

		// lower agent_id wins (created earlier)
		int agent_id = get_entity_id();
		int rhs_agent_id = rhs.get_entity_id();
		if ( agent_id < rhs_agent_id ) return true;
		else return false;
    }

    /**
     * Comparison helper function to order events in dirty list.
     * 
     * @param [in,out] rhs The right hand side.
     * @param [in,out] lhs The left hand side.
     *
     * @return true if the right-hand side is less than the left-hand side.
     */
    static bool dirty_cmp(BaseEvent *lhs, BaseEvent *rhs)
    {
        // earlier event time wins in event queue, but is ignored in dirty queue ordering
        //if (lhs->event_time < rhs->event_time) return true;
        //if (lhs->event_time > rhs->event_time) return false;

        // higher event priority wins
        int lhs_event_priority = lhs->get_event_priority();
        int rhs_event_priority = rhs->get_event_priority();
        if (lhs_event_priority > rhs_event_priority) return true;
        if (lhs_event_priority < rhs_event_priority) return false;

        // lower event_id wins (earlier in alphabetic order by event name)
        int lhs_event_id = lhs->get_event_id();
        int rhs_event_id = rhs->get_event_id();
        if (lhs_event_id < rhs_event_id) return true;
        if (lhs_event_id > rhs_event_id) return false;

        // lower agent_id wins (created earlier)
        int lhs_agent_id = lhs->get_entity_id();
        int rhs_agent_id = rhs->get_entity_id();
        if (lhs_agent_id < rhs_agent_id) return true;
        else return false;
    }


    /**
     * Initializes run-time before simulation.
     */
    static void initialize_simulation_runtime()
    {
        event_queue = new std::set<BaseEvent *, less_deref<BaseEvent *> > ;
        dirty_events = new std::set<BaseEvent *, decltype(dirty_cmp)*>(dirty_cmp) ;
        global_time = new Time(0);
        global_event_counter = 0;
        current_event_id = -1;
        current_entity_id = -1;
        event_checksum_reset();
    }

    /**
     * Clean up run-time after simulation.
     */
    static void finalize_simulation_runtime()
    {
        assert(event_queue->empty());
        delete event_queue;
        event_queue = nullptr;

        assert(dirty_events->empty());
        delete dirty_events;
        dirty_events = nullptr;

        delete global_time;
        global_time = nullptr;
    }

    /**
     * Cleans all events in the dirty list.
     */
    static void clean_all()
    {
        assert(dirty_events);
        auto it = dirty_events->begin();
        while (it != dirty_events->end()) {
            auto evt = *it;
            it = dirty_events->erase(it); // set::erase() returns iterator to next element
            evt->clean();
        }
    }

    /**
     * Peek next event in the queue.
     * 
     * Same as do_next_event, but does not implement the event.
     * NB: Updates dirty events in the event queue.
     *
     * @return next event.
     */
    static BaseEvent * peek_next_event()
    {
        BaseEvent::clean_all();
        BaseEntity::free_all_zombies();

        return *BaseEvent::event_queue->begin();
    }


    /**
     * Time of next event.
     *
     * NB: Updates dirty events in the event queue.
     *
     * @return Time of next event, time_infinite if none.
     */
    static Time time_next_event()
    {
        BaseEvent::clean_all();
        BaseEntity::free_all_zombies();

        assert(event_queue);
        if ( BaseEvent::event_queue->empty() ) {
            return time_infinite;
        }
        else {
            // get the next event from front of the event queue
            auto *evt = *BaseEvent::event_queue->begin();
            return evt->event_time;
        }
    }

    /**
     * Advances time to the next event and implements it.
     *
     * @return false if there are for certain no events in the queue, true otherwise.
     */
    static bool do_next_event()
    {
        BaseEvent::clean_all();
        BaseEntity::free_all_zombies();

        assert(event_queue);
        if ( BaseEvent::event_queue->empty() ) {
            return false;
        }

        // get the next event from front of the event queue
        auto *evt = *BaseEvent::event_queue->begin();

        // debug check that event time is not infinite
        // TODO handle run-time error (model developer error)
        assert(evt->event_time != time_infinite);

        // update global time
        assert(global_time);
        *global_time = evt->event_time;
        // update global event counter
        global_event_counter++;

        if (just_in_time) {
            // age the agent to the time of the event
            evt->verified_age_agent();
        }
        else {
            // Age all agents to the time of the event.
            // The first argument is the time of the event.
            // The second argument is the entity_id of the entity within which the event occurred.
            // The third argument is the event_id of the event in entity_id.
            // The second argument allows age_all_agents to detect the model error condition
            // in which time is running backwards in the entity within which the event occurred.
            BaseEntity::age_all_agents(evt->event_time, evt->get_entity_id(), evt->get_event_id());
        }

        // update the global event checksum
        if (event_checksum_enabled && evt->get_event_priority() != openm::event_priority_self_scheduling) evt->event_checksum_update();

        // record information about this event for context in messages
        current_event_id = evt->get_event_id();
        current_entity_id = evt->get_entity_id();

        // implement the event (can mark other events dirty as side-effect)
        evt->call_implement_func();

        // mark the event as requiring recalculation of occurrence time
        evt->make_dirty();

        return true;
    }

    /**
     * Gets global time.
     *
     * @return The global time.
     */
    static Time get_global_time()
    {
        assert(global_time);
        return *global_time;
    }

    /**
     * Sets global time.
     */
    static void set_global_time(Time t)
    {
        assert(global_time);
        *global_time = t;
    }

    /**
     * Scheduled time of event.
     */
    Time event_time;

    /**
     * Storage for memory events.
     */
    int memory;

    /**
     * Event is in \a event_queue.
     */
    bool in_queue : 1;

    /**
     * Event needs calculation of \a event_time.
     */
    bool is_dirty : 1;

    /**
     * Event is pending deletion.
     */
    bool is_zombie : 1;

    /**
     * The event queue (declaration)
     * 
     * Defined by C++ code generated by omc.
     */
    static thread_local std::set<BaseEvent *, less_deref<BaseEvent *> > *event_queue;

    /**
     * The dirty event list (declaration)
     * Contains events whose times require calculation.
     * 
     * Defined by C++ code generated by omc.
     */
    static thread_local std::set<BaseEvent*, decltype(dirty_cmp)* > *dirty_events;

    /**
     * The event_id of the current event
     *
     * Defined by C++ code generated by omc.
     */
    static thread_local int current_event_id;

    /**
     * The entity_id of the current event
     *
     * Defined by C++ code generated by omc.
     */
    static thread_local int current_entity_id;

    /**
     * Value of just-in-time option
     * 
     * True if active, false if inactive.
     * This constant is defined by C++ code generated by omc.
     */
    static const bool just_in_time;


    /**
     * True if time travel is allowed
     * 
     * This constant is defined by C++ code generated by omc.
     */
    static const bool allow_time_travel;

    /**
     * True if clairvoyance is allowed
     *
     * This constant is defined by C++ code generated by omc.
     */
    static const bool allow_clairvoyance;

    /**
     * The global event counter.
     *
     * Defined by C++ code generated by omc.
     */
    static thread_local big_counter global_event_counter;

    /**
     * true if running event checksum is activated.
     *
     * This constant is defined by C++ code generated by omc.
     */
    static const bool event_checksum_enabled;

    /**
     * The current value of the running event checksum.
     *
     * Defined by C++ code generated by omc.
     */
    static thread_local double event_checksum_value;

    /**
     * Reset the running event checksum.
     */
    static void event_checksum_reset()
    {
        event_checksum_value = 0.0;
    }

    /**
     * Update the running event checksum using this event
     */
    void event_checksum_update()
    {
        event_checksum_value += event_time * (get_modgen_event_num() + 1);
    }

    /**
     * Get the value of the running event checksum.
     */
    static double get_event_checksum()
    {
        return event_checksum_value;
    }

private:

    /**
     * The global time.
     */
    static thread_local Time *global_time;

};

/**
 * An event, within a given kind of entity
 *
 * @tparam A                          The entity class.
 * @tparam event_id                   The constant event identifier.
 * @tparam event_priority             The constant event priority.
 * @tparam modgen_event_num           The constant modgen event number.
 * @tparam (A::*implement_function)() The constant implement function for the event.
 * @tparam (A::*time_function)()      The constant time function for the event.
 */
template<typename A, const int event_id, const int event_priority, const int modgen_event_num, void (A::*implement_function)(), Time (A::*time_function)()>
class Event : public BaseEvent
{
public:

    int get_event_id() const
    {
        // Note that event_id is not a class member.
        // It is a compile-time constant supplied as a template argument.
        return event_id;
    }

    int get_event_priority() const
    {
        // Note that event_priority is not a class member.
        // It is a compile-time constant supplied as a template argument.
        return event_priority;
    }

    int get_modgen_event_num() const
    {
        // Note modgen_event_num is not a class member.
        // It is a compile-time constant supplied as a template argument.
        return modgen_event_num;
    }

    // get pointer to containing agent
    A *agent()
    {
        return (A *) ( (char *)this - offset_in_agent );
    }

    int get_entity_id()
    {
        return (agent()->om_get_entity_id)();
    }

    Time get_time()
    {
        return agent()->time.get();
    }

    void call_implement_func()
    {
        // Note that implement_function is not a class member.
        // It is a compile-time constant supplied as a template argument.
        (agent()->*implement_function)();
    }

    Time call_time_func()
    {
        return (agent()->*time_function)();
    }

    void call_age_agent()
    {
        agent()->age_agent(event_time);
    }

	// offset to containing agent
	static size_t offset_in_agent;
};

/**
* Event offset in Agent (definition)
*/
template<typename A, const int event_id, const int event_priority, const int modgen_event_num, void (A::*implement_function)(), Time(A::*time_function)()>
size_t Event<A, event_id, event_priority, modgen_event_num, implement_function, time_function>::offset_in_agent = 0;


/**
 * A memory event, within a given kind of entity
 *
 * @tparam A                          The entity class.
 * @tparam event_id                   The constant event identifier.
 * @tparam event_priority             The constant event priority.
 * @tparam modgen_event_num           The constant modgen event number.
 * @tparam (A::*implement_function)(int) The constant implement function for the event.
 * @tparam (A::*time_function)(int *)    The constant time function for the event.
 */
template<typename A, const int event_id, const int event_priority, const int modgen_event_num, void (A::*implement_function)(int), Time (A::*time_function)(int *)>
class MemoryEvent : public BaseEvent
{
public:

    int get_event_id() const
    {
        // Note that event_id is not a class member.
        // It is a compile-time constant supplied as a template argument.
        return event_id;
    }

    int get_event_priority() const
    {
        // Note that event_priority is not a class member.
        // It is a compile-time constant supplied as a template argument.
        return event_priority;
    }

    int get_modgen_event_num() const
    {
        // Note modgen_event_num is not a class member.
        // It is a compile-time constant supplied as a template argument.
        return modgen_event_num;
    }

    // get pointer to containing agent
    A *agent()
    {
        return (A *) ( (char *)this - offset_in_agent );
    }

    int get_entity_id()
    {
        return (agent()->om_get_entity_id)();
    }

    Time get_time()
    {
        return agent()->time.get();
    }

    void call_implement_func()
    {
        // Note that implement_function is not a class member.
        // It is a compile-time constant supplied as a template argument.
        (agent()->*implement_function)(memory);
    }

    Time call_time_func()
    {
        return (agent()->*time_function)(&memory);
    }

    void call_age_agent()
    {
        agent()->age_agent(event_time);
    }

	// offset to containing agent
	static size_t offset_in_agent;
};

/**
* MemoryEvent offset in Agent (definition)
*/
template<typename A, const int event_id, const int event_priority, const int modgen_event_num, void (A::*implement_function)(int), Time(A::*time_function)(int *)>
size_t MemoryEvent<A, event_id, event_priority, modgen_event_num, implement_function, time_function>::offset_in_agent = 0;

