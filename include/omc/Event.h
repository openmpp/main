/**
 * @file    Event.h
 * Declares the Event class and associated classes and templates
 *         
 */
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include <set>
#include <cassert>
#include "omc/tailed_forward_list.h"
#include "omc/less_deref.h"
#include "omc/Agent.h"
#include "omc/event_priorities.h"
#include "om_types0.h" // for Time

using namespace std;

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
        in_queue = false;
        is_dirty = false;
        is_zombie = false;
    }

    void make_dirty()
    {
        if ( ! is_zombie && ! is_dirty ) {
            is_dirty = true;
            dirty_events.push_back( this );
        }
    }

    void make_zombie()
    {
        if ( ! is_zombie ) {
            is_zombie = true;
            dirty_events.push_back( this );
        }
    }

    virtual int get_event_id() const = 0;

    virtual int get_event_priority() const = 0;

    virtual int get_agent_id() = 0;

    virtual void implement_event() = 0;

    virtual void clean() = 0;

    virtual void age_agent() = 0;

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
		int agent_id = get_agent_id();
		int rhs_agent_id = rhs.get_agent_id();
		if ( agent_id < rhs_agent_id ) return true;
		else return false;
    }

    /**
     * Cleans all events in the dirty list.
     */
    static void clean_all()
    {
        while ( ! dirty_events.empty() ) {
            auto evt = dirty_events.front();
            evt->clean();
            dirty_events.pop_front();
        }
    }

    /**
     * Time of next event.
     *
     * @return Time of next event, time_infinite if none.
     */
    static Time time_next_event()
    {
        BaseEvent::clean_all();
        BaseAgent::free_all_zombies();

        if ( BaseEvent::event_queue.empty() ) {
            return time_infinite;
        }
        else {
            // get the next event from front of the event queue
            auto *evt = *BaseEvent::event_queue.begin();
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
        BaseAgent::free_all_zombies();

        if ( BaseEvent::event_queue.empty() ) {
            return false;
        }

        // get the next event from front of the event queue
        auto *evt = *BaseEvent::event_queue.begin();

        // debug check that event time is not infinite (model error)
        assert(evt->event_time != time_infinite);

        // update global time
        global_time = evt->event_time;
        // update global event counter
        global_event_counter++;

        if ( just_in_time ) {
            // age the agent to the time of the event
            evt->age_agent();
        }
        else {
            // age all agents to the time of the event
            BaseAgent::age_all_agents( evt->event_time );
        }

        // implement the event
        evt->implement_event();

        // mark the event as requiring recalculation of occurrence time
        evt->make_dirty();

        return true;
    }

    /**
     * Scheduled time of event.
     */
    Time event_time;

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
     * @return A Queue of events.
     */
    static set<BaseEvent *, less_deref<BaseEvent *> > event_queue;

    /**
     * The dirty event list (declaration)
     * Contains events whose times require calculation.
     */
    static tailed_forward_list<BaseEvent *> dirty_events;

    /**
     * Value of just-in-time option
     * 
     * True if active, false if inactive.
     */
    static const bool just_in_time;

    /**
     * true to enable event logging (use API)
     */
    static bool trace_event_on;

    /**
     * The global time.
     */
    static Time global_time;

    /**
     * The global event counter.
     */
    static big_counter global_event_counter;
};

template<typename A, const int event_id, const int event_priority, void (A::*implement_function)(), Time (A::*time_function)()>
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

    // get pointer to containing agent
    A *agent()
    {
        return (A *) ( (char *)this - offset_in_agent );
    }

    int get_agent_id()
    {
        return (agent()->get_entity_id)();
    }

    void implement_event()
    {
        // Note that implement_function is not a class member.
        // It is a compile-time constant supplied as a template argument.
        (agent()->*implement_function)();
    }

    /**
     * Cleans this event. Removes if zombie and updates event time if dirty.
     */
    void clean()
    {
        if ( is_zombie && in_queue ) {
            event_queue.erase( this );
            in_queue = false;
        }
        else if ( is_dirty ) {
            Time new_event_time = (agent()->*time_function)();
            if ( in_queue ) {
                if ( new_event_time != event_time ) {
                    event_queue.erase( this );
                    event_time = new_event_time;
                    event_queue.insert( this );
                }
            }
            else // ! in_queue
            {
                event_time = new_event_time;
                event_queue.insert( this );
                in_queue = true;
            }
        }
        is_dirty = false;
    }

    /**
     * Age the agent to the time of event occurrence.
     */
    void age_agent()
    {
        agent()->age_agent( event_time );
    }

	// offset to containing agent
	static size_t offset_in_agent;
};

/**
* Event offset in Agent (definition)
*/
template<typename A, const int event_id, const int event_priority, void (A::*implement_function)(), Time(A::*time_function)()>
size_t Event<A, event_id, event_priority, implement_function, time_function>::offset_in_agent = 0;

