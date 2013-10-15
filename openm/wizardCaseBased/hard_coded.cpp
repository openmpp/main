// hard-coded pending implementation in omc

// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "om_agents.h"

using namespace mm;

void StartCase( int case_counter )
{
    Person *thePerson = new Person;
    thePerson->Start();

    // TODO need to initialize agent_id within Start, before tabulation, etc.
    thePerson->agent_id.set( case_counter );
}

void EndCase()
{
}


/**
 * Processes the next event in the event queue.
 *
 * @return  true if event queue not empty, false otherwise.
 */

bool DoNextEvent()
{
    return BaseEvent::do_next_event();
}
