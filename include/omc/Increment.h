/**
 * @file    Increment.h
 * Declares templates and classes associated with table increments and accumulators.
 *         
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

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
    void(A::*NT_init_increment)(int, big_counter),
    void(A::*NT_push_increment)(int, big_counter)
>
class Increment
{
public:
    Increment()
    {
    };

    /**
     * Initializes the increment before the entity enters the simulation.
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
     * Finalizes the increment as the entity leaves the simulation.
     */
    void finalize_increment()
    {
        if (pending) {
            if (active) {
                finish();
            }
            if (filter) {
                start();
            }
            pending = false;
        }
        if (active) {
            finish();
        }
    }

    /**
     * Starts the increment.
     */
    void start()
    {
        active = true;
        cell_in = cell;

        // Assign starting attributes required by the increment
        (agent()->*NT_init_increment)(pending, pending_event_counter);
    }

    /**
     * Finishes the increment.
     */
    void finish()
    {
        assert(active);

        // Compute the increment and and push it to the accumulator
        (agent()->*NT_push_increment)(pending, pending_event_counter);
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

    // storage
    big_counter pending_event_counter;
    int cell;
    int cell_in;
    bool filter;
    bool active;
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
