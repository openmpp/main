/**
* @file    omSimulation.h
* Common declarations for all models.
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include <cmath>
#include "om_types.h" // model-specific types
#include "omc/Agent.h"
#include "omc/AgentVar.h"
#include "omc/Event.h"
#include "libopenm/omModel.h"
#include "om_agents.h" // model-specific classes


// The following macro is not defined for Modgen models.
// It can be used to exclude Modgen-specific C++ code during C++ compilation by openM++.
// Both the Modgen compiler and omc ignore preprocessor directives in model source code.
#define OPENM

#define TRUE true
#define FALSE false
#define WAIT( delta_time ) (time + (delta_time))

// The following are temporary kludges
#define WriteDebugLogEntry printf
#define GetCaseSample() 0
#define case_id 0

namespace openm {

    // model API entries holder
    struct ModelEntryHolder
    {
        ModelEntryHolder(
        OM_STARTUP_HANDLER i_ModelStartupHandler,
        OM_EVENT_LOOP_HANDLER i_RunModelHandler,
        OM_SHUTDOWN_HANDLER i_ModelShutdownHandler
        )
        {
            ::ModelStartupHandler = i_ModelStartupHandler;    // link model startup function
            ::RunModelHandler = i_RunModelHandler;            // link user portion of model event loop
            ::ModelShutdownHandler = i_ModelShutdownHandler;  // link model shutdown function
        }
    };

    // run-time support functions
	double RandUniform(int strm);
}

extern void StartSimulation(int id);
extern void EndSimulation();

