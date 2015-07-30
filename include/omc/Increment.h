/**
 * @file    Increment.h
 * Declares templates and classes associated with table increments and accumulators.
 *         
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once

using namespace std;

/**
 * A table increment.
 *
 * @tparam A                 Type of containing entity
 * @tparam NT_init_increment Function which initializes the increment
 * @tparam NT_push_increment Function which finalizes the increment and pushes it to the accumulator
 */
template<
    typename A,
    string const *NT_name,
    void(A::*NT_init_increment)(int, big_counter),
    void(A::*NT_push_increment)(int, int, big_counter)
>
class Increment
{
public:
    Increment()
    {
    };

    /**
     * Initialize the increment before the entity enters the simulation.
     */
    void initialize_increment()
    {
        active = false;
        pending = false;
        if (filter) {
            //update_cell();
            start();
        }
    }

    /**
     * Finalize the increment as the entity leaves the simulation.
     */
    void finalize_increment()
    {
        if (pending) {
            // All values are definitive (entity is leaving the simulation)
            if (active) {
                // Finish the pending increment
                finish();
            }
            if (filter) {
                // Start a final increment
                start();
            }
            pending = false;
        }
        if (active) {
            // Finish the final increment
            finish();
        }
    }

    /**
     * Start the increment.
     */
    void start()
    {
        assert(!active);
        active = true;
        cell_in = cell;

        // Assign starting attributes required by the increment
        (agent()->*NT_init_increment)(pending, pending_event_counter);
    }

    /**
     * Finish the increment.
     */
    void finish()
    {
        assert(active);

        // Compute the increment and and push it to the accumulators
        (agent()->*NT_push_increment)(cell_in, pending, pending_event_counter);
        active = false;
    }

    void start_pending()
    {
        //update_cell();
        if (active && filter && cell == cell_in) {
            pending = false;
        }
        else {
            pending = true;
            pending_event_counter = BaseEvent::global_event_counter;
        }
    }

    void finish_pending()
    {
        if (pending && BaseEvent::global_event_counter > pending_event_counter) {
            if (active) {
                finish();
            }
            if (filter) {
                //update_cell();
                start();
            }
            pending = false;
        }
    }

    void set_cell(int cl)
    {
        cell = cl;
    }

    void set_filter(bool fltr)
    {
        filter = fltr;
    }

    /**
     * The value of the global event counter at the previous table update.
     * 
     * Table updates are lagged so that simultaneous changes in multiple classificatory dimensions
     * and/or filter are processed together as a single table update.  This agentvar together with
     * the current value of the global event counter is used to detect that the previous table
     * update is definitively finished and can be processed.
     * 
     * Because the global event counter is 1 for the first event, the default initialization value
     * of zero for this agentvar works as intended.
     */
    big_counter pending_event_counter;

    /**
     * The current index of the table cell.
     */
    int cell;

    /**
     * The index of the table cell at the start of the increment.
     */
    int cell_in;

    /**
     * The current value of the table filter.
     */
    bool filter;

    /**
     * The increment is open and has not yet been pushed to accumulators.
     * 
     * An active increment will be pushed sooner or later to accumulators.
     */
    bool active;

    /**
     * The increment is pending
     * 
     * The increment may be complete because the filter or cell has changed in the current event.
     * But if the filter or cell return to previous values before the event completes, the increment
     * will no longer be pending.  A pending increment will be processed once the event is
     * definitively complete.
     */
    bool pending;

    /**
     * get pointer to containing agent.
     */
    A *agent()
    {
        return (A *) ( (char *)this - offset_in_agent );
    }

    /**
     * offset to containing agent.
     */
	static size_t offset_in_agent;
};

/**
 * Increment offset in entity (static definition)
 * 
 * The offset is used within an instance of Increment to gain access to the enclosing entity to
 * call functions in the context of the entity and with access to all attributes
 * in the entity.
 */
template<
    typename A,
    string const *NT_name,
    void(A::*NT_init_increment)(int, big_counter),
    void(A::*NT_push_increment)(int, int, big_counter)
>
size_t Increment<A, NT_name, NT_init_increment, NT_push_increment>::offset_in_agent = 0;
