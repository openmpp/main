/**
* @file    omSimulation.h
* Common declarations for all models.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include <cfloat>
#include <cmath>
#include <iostream> // for debug log only
#include <fstream> // for debug log only
#include <cstdarg> // for debug log only
#include "om_types0.h" // fundamental model types
#include "omc/link.h"
#include "omc/Range.h"
#include "omc/Agent.h"
#include "omc/AgentVar.h"
#include "omc/Event.h"
#include "omc/Multilink.h"
#include "omc/EntitySet.h"
#include "om_types1.h" // model-specific types
#include "libopenm/omModel.h"

#include "om_declarations.h" // model-specific classes


// The following macro is not defined for Modgen models.
// It can be used to exclude Modgen-specific C++ code during C++ compilation by openM++.
// Both the Modgen compiler and omc ignore preprocessor directives in model source code.
#define OPENM

#define TRUE true
#define FALSE false
#define WAIT( delta_time ) (time + (delta_time))

// Implementation of old-style Modgen macros related to classifications, ranges, and partitions
#define MIN( symbol )				(symbol::min)
#define MAX( symbol )				(symbol::max)
#define SIZE( symbol )				(symbol::size)
#define RANGE_POS( rname, val )     (rname::to_index(val))
#define COERCE( rname, val )        (static_cast<rname>(val))
#define WITHIN( rname, val )        (rname::within(val))
#define POINTS( symbol )		   	(symbol::upper_bounds())
#define SPLIT( val, symbol )        (symbol::to_index(val))

// map logical to bool (for local variable declarations in Modgen models)
#define logical bool

// Implemented in all models through the statement:
//   use "common.ompp";
extern void WriteDebugLogEntry(const char * fmt, ...);
extern void StartEventTrace();
extern void StopEventTrace();
void om_event_trace_msg(char *entity_name, int entity_id, double case_seed, char *event_name, double time);


// The following are temporary kludges to allow compilation
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

