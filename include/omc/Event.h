/**
 * @file    Event.h
 * Declares the Event class and associated classes and templates
 *         
 */
// Copyright (c) 2013-2022 OpenM++ Contributors
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include <set>
#include <sstream>
#include <cassert>
#include "omc/less_deref.h"
#include "omc/Entity.h"
#include "omc/event_priorities.h"
#include "om_types0.h" // for Time

class BaseEvent
{
public:
    // ctor
    explicit BaseEvent()
    {
        memory = 0;
        in_queue = false;
        is_dirty = false;
        is_zombie = false;
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

    virtual void call_age_entity() = 0;

    virtual Time call_get_censor_time() = 0;

    virtual void increment_censor_count() = 0;

    virtual void increment_queue_count() = 0;

    virtual void decrement_queue_count() = 0;

    /**
     * Verify time, then age the entity of this event to the time of event occurrence.
     */
    void verified_age_entity()
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
        call_age_entity();
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
                if constexpr (om_resource_use_on) {
                    decrement_queue_count();
                }
            }
        }
        else if ( is_dirty ) {
            if constexpr (om_event_trace_capable) {
                // stash the current value of the time of the event for use in event trace code
                // in cover function of time function.
                stashed_time = event_time;
            }
            Time new_event_time = call_time_func();
            bool censored = false;
            if constexpr (om_censor_event_time_on) {
                if (new_event_time == time_infinite) {
                    // event is censored if time-to-event is infinite
                    censored = true;
                }
                else {
                    // event is censored if time-to-event is strictly greater than
                    // the entity-specific censor time set in model code by call to set_censor_time().
                    auto ct = call_get_censor_time();
                    censored = (new_event_time > ct);
                }
            }
            else {
                // event is censored if time-to-event is infinite
                censored = (new_event_time == time_infinite);
            }
            if constexpr (om_resource_use_on) {
                if (censored) {
                    increment_censor_count();
                }
            }
            if ( in_queue ) {
                if ( new_event_time != event_time ) {
                    event_queue->erase( this );
                    event_time = new_event_time;
                    if (censored) {
                        in_queue = false;
                        if constexpr (om_resource_use_on) {
                            decrement_queue_count();
                        }
                    }
                    else {
                        event_queue->insert(this);
                    }
                }
            }
            else // ! in_queue
            {
                event_time = new_event_time;
                if (!censored) {
                    event_queue->insert(this);
                    in_queue = true;
                    if constexpr (om_resource_use_on) {
                        increment_queue_count();
                    }
                }
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

		// lower entity_id wins (created earlier)
		int entity_id = get_entity_id();
		int rhs_entity_id = rhs.get_entity_id();
		if ( entity_id < rhs_entity_id ) return true;
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

        // lower entity_id wins (created earlier)
        int lhs_entity_id = lhs->get_entity_id();
        int rhs_entity_id = rhs->get_entity_id();
        if (lhs_entity_id < rhs_entity_id) return true;
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
        // reset these global values so that diagnostic messages after simulation
        // don't give misleading information, e.g. handle_bounds_error
        current_event_id = -1;
        current_entity_id = -1;

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
        if (om_verify_attribute_modification) { // is constexpr
            // prohibit attribute assignment during recomputation of dirty event times
            BaseEntity::om_permit_attribute_modification = false;
        }
        if (om_verify_timelike_attribute_access) { // is constexpr
            // prohibit time-like attribute access during recomputation of dirty event times
            BaseEntity::om_permit_timelike_attribute_access = false;
        }
        assert(dirty_events);
        auto it = dirty_events->begin();
        while (it != dirty_events->end()) {
            auto evt = *it;
            it = dirty_events->erase(it); // set::erase() returns iterator to next element
            evt->clean();
        }
        if (om_verify_attribute_modification) { // is constexpr
            // permit attribute assignment after recomputation of dirty event times is complete
            BaseEntity::om_permit_attribute_modification = true;
        }
        if (om_verify_timelike_attribute_access) { // is constexpr
            // permit attribute assignment after recomputation of dirty event times is complete
            BaseEntity::om_permit_timelike_attribute_access = true;
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
            // age the entity to the time of the event
            evt->verified_age_entity();
        }
        else {
            // Age all entities to the time of the event.
            // The first argument is the time of the event.
            // The second argument is the entity_id of the entity within which the event occurred.
            // The third argument is the event_id of the event in entity_id.
            // The second argument allows age_all_entities to detect the model error condition
            // in which time is running backwards in the entity within which the event occurred.
            BaseEntity::age_all_entities(evt->event_time, evt->get_entity_id(), evt->get_event_id());
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
        if (global_time) {
            return *global_time;
        }
        else {
            // global_time may not be allocated if this function is called early in run lifecycle, eg for error message
            return time_undef;
        }
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
     * The event_id of the event whose time is being recomputed
     *
     * Valid only if constexpr om_verify_attribute_modification is true
     *
     * Defined by C++ code generated by omc.
     */
    static thread_local int timefunc_event_id;

    /**
     * The entity_id of the event whose time is being recomputed
     *
     * Valid only if constexpr om_verify_attribute_modification is true
     * 
     * Defined by C++ code generated by omc.
     */
    static thread_local int timefunc_entity_id;

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

    /**
     * Stashed scheduled time of a dirty event for use in the cover time function for event trace output.
     */
    static thread_local Time stashed_time;

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

    // get pointer to containing entity
    A *entity()
    {
        return (A *) ( (char *)this - offset_in_entity );
    }

    int get_entity_id()
    {
        return (entity()->om_get_entity_id)();
    }

    Time get_time()
    {
        return entity()->time.get();
    }

    void call_implement_func()
    {
        // Note that implement_function is not a class member.
        // It is a compile-time constant supplied as a template argument.
        (entity()->*implement_function)();
        if constexpr (om_resource_use_on) {
            ++occurrence_count;
        }
    }

    Time call_time_func()
    {
        if constexpr (om_verify_attribute_modification) {
            // record information about this event for context in possible error message
            timefunc_event_id = get_event_id();
            timefunc_entity_id = get_entity_id();
        }
        if constexpr (om_resource_use_on) {
            ++calculation_count;
        }
        return (entity()->*time_function)();
    }

    void call_age_entity()
    {
        entity()->age_entity(event_time);
    }

    Time call_get_censor_time()
    {
        return entity()->get_censor_time();
    }

    void increment_censor_count()
    {
        ++censor_count;
    }

    void increment_queue_count()
    {
        ++queue_count;
        if (queue_count_max < queue_count) {
            queue_count_max = queue_count;
        }
    }

    void decrement_queue_count()
    {
        --queue_count;
    }

    /**
     * Resource use information for the event
     */
    static auto resource_use()
    {
        struct result { size_t max_in_queue;  size_t time_calculations; size_t censored_times; size_t occurrences; };
        return result { queue_count_max, calculation_count, censor_count, occurrence_count };
    }

    /**
     * Reset resource use information for the event
     */
    static void resource_use_reset()
    {
        queue_count = 0;
        queue_count_max = 0;
        calculation_count = 0;
        censor_count = 0;
        occurrence_count = 0;
    }

    // offset to containing entity
	static size_t offset_in_entity;

    /**
    * Count in queue
    *
    * The count of this event in the queue.
    */
    static thread_local size_t queue_count;

    /**
    * Maximum count in queue
    *
    * The maximum count of this event in the queue.
    */
    static thread_local size_t queue_count_max;

    /**
    * Count of occurrences
    *
    * The number of times the event occurred.
    */
    static thread_local size_t occurrence_count;

    /**
    * Count of calculations
    *
    * The number of times the event time was computed.
    */
    static thread_local size_t calculation_count;

    /**
    * Count of censored events
    *
    * The number of times the event time is right-censored.
    * Right-censored events will not occur, so are not entered in the event queue.
    */
    static thread_local size_t censor_count;
};

template<typename A, const int event_id, const int event_priority, const int modgen_event_num, void (A::* implement_function)(), Time(A::* time_function)()>
size_t Event<A, event_id, event_priority, modgen_event_num, implement_function, time_function>::offset_in_entity = 0;

template<typename A, const int event_id, const int event_priority, const int modgen_event_num, void (A::* implement_function)(), Time(A::* time_function)()>
thread_local size_t Event<A, event_id, event_priority, modgen_event_num, implement_function, time_function>::queue_count = 0;

template<typename A, const int event_id, const int event_priority, const int modgen_event_num, void (A::* implement_function)(), Time(A::* time_function)()>
thread_local size_t Event<A, event_id, event_priority, modgen_event_num, implement_function, time_function>::queue_count_max = 0;

template<typename A, const int event_id, const int event_priority, const int modgen_event_num, void (A::* implement_function)(), Time(A::* time_function)()>
thread_local size_t Event<A, event_id, event_priority, modgen_event_num, implement_function, time_function>::occurrence_count = 0;

template<typename A, const int event_id, const int event_priority, const int modgen_event_num, void (A::* implement_function)(), Time(A::* time_function)()>
thread_local size_t Event<A, event_id, event_priority, modgen_event_num, implement_function, time_function>::calculation_count = 0;

template<typename A, const int event_id, const int event_priority, const int modgen_event_num, void (A::* implement_function)(), Time(A::* time_function)()>
thread_local size_t Event<A, event_id, event_priority, modgen_event_num, implement_function, time_function>::censor_count = 0;


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

    // get pointer to containing entity
    A *entity()
    {
        return (A *) ( (char *)this - offset_in_entity );
    }

    int get_entity_id()
    {
        return (entity()->om_get_entity_id)();
    }

    Time get_time()
    {
        return entity()->time.get();
    }

    void call_implement_func()
    {
        // Note that implement_function is not a class member.
        // It is a compile-time constant supplied as a template argument.
        (entity()->*implement_function)(memory);
        if constexpr (om_resource_use_on) {
            ++occurrence_count;
        }

    }

    Time call_time_func()
    {
        if constexpr (om_verify_attribute_modification) {
            // record information about this event for context in possible error message
            timefunc_event_id = get_event_id();
            timefunc_entity_id = get_entity_id();
        }
        if constexpr (om_resource_use_on) {
            ++calculation_count;
        }
        return (entity()->*time_function)(&memory);
    }

    void call_age_entity()
    {
        entity()->age_entity(event_time);
    }

    Time call_get_censor_time()
    {
        return entity()->get_censor_time();
    }

    void increment_censor_count()
    {
        ++censor_count;
    }

    void increment_queue_count()
    {
        ++queue_count;
        if (queue_count_max < queue_count) {
            queue_count_max = queue_count;
        }
    }

    void decrement_queue_count()
    {
        ++queue_count;
    }

    /**
     * Resource use information for the event
     */
    static auto resource_use()
    {
        struct result { size_t max_in_queue;  size_t time_calculations; size_t censored_times; size_t occurrences; };
        return result { queue_count_max, calculation_count, censor_count, occurrence_count };
    }

    /**
     * Reset resource use information for the event
     */
    static void resource_use_reset()
    {
        queue_count = 0;
        queue_count_max = 0;
        calculation_count = 0;
        censor_count = 0;
        occurrence_count = 0;
    }

    // offset to containing entity
	static size_t offset_in_entity;

    /**
    * Count in queue
    *
    * The count of this event in the queue.
    */
    static thread_local size_t queue_count;

    /**
    * Maximum count in queue
    *
    * The maximum count of this event in the queue.
    */
    static thread_local size_t queue_count_max;

    /**
    * Count of occurrences
    *
    * The number of times the event occurred.
    */
    static thread_local size_t occurrence_count;

    /**
    * Count of calculations
    *
    * The number of times the event time was computed.
    */
    static thread_local size_t calculation_count;

    /**
    * Count of censored events
    *
    * The number of times the event time is right-censored.
    * Right-censored events will not occur, so are not entered in the event queue.
    */
    static thread_local size_t censor_count;
};

template<typename A, const int event_id, const int event_priority, const int modgen_event_num, void (A::*implement_function)(int), Time(A::*time_function)(int *)>
size_t MemoryEvent<A, event_id, event_priority, modgen_event_num, implement_function, time_function>::offset_in_entity = 0;

template<typename A, const int event_id, const int event_priority, const int modgen_event_num, void (A::* implement_function)(int), Time(A::* time_function)(int*)>
thread_local size_t MemoryEvent<A, event_id, event_priority, modgen_event_num, implement_function, time_function>::queue_count = 0;

template<typename A, const int event_id, const int event_priority, const int modgen_event_num, void (A::* implement_function)(int), Time(A::* time_function)(int*)>
thread_local size_t MemoryEvent<A, event_id, event_priority, modgen_event_num, implement_function, time_function>::queue_count_max = 0;

template<typename A, const int event_id, const int event_priority, const int modgen_event_num, void (A::* implement_function)(int), Time(A::* time_function)(int*)>
thread_local size_t MemoryEvent<A, event_id, event_priority, modgen_event_num, implement_function, time_function>::occurrence_count = 0;

template<typename A, const int event_id, const int event_priority, const int modgen_event_num, void (A::* implement_function)(int), Time(A::* time_function)(int*)>
thread_local size_t MemoryEvent<A, event_id, event_priority, modgen_event_num, implement_function, time_function>::calculation_count = 0;

template<typename A, const int event_id, const int event_priority, const int modgen_event_num, void (A::* implement_function)(int), Time(A::* time_function)(int*)>
thread_local size_t MemoryEvent<A, event_id, event_priority, modgen_event_num, implement_function, time_function>::censor_count = 0;
