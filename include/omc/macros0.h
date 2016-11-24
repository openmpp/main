/**
* @file    macros0.h
* Macros and templates available to all models (early)
*/
// Copyright (c) 2013-2016 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#define TRUE true
#define FALSE false
//#define WAIT( delta_time ) static_cast<Time>((time + static_cast<Time::type>(delta_time)))
#define WAIT( delta_time ) (TIME_t(time) + TIME_t(delta_time))

// Implementation of old-style Modgen macros related to classifications, ranges, and partitions
#define MIN( symbol )				(symbol::min)
#define MAX( symbol )				(symbol::max)
#define SIZE( symbol )				(symbol::size)
#define RANGE_POS( rname, val )     (rname::to_index(val))
#define COERCE( rname, val )        (rname::bound(val))

#define WITHIN( rname, val )        (rname::within(val))
#define POINTS( symbol )		   	(symbol::upper_bounds())
#define SPLIT( val, symbol )        (symbol::value_to_interval(val))

// Ignore IMPLEMENT_HOOK 
#define IMPLEMENT_HOOK()

// A (very) direct implementation of the variadic WriteDebugLogEntry() Modgen function
#define WriteDebugLogEntry theTrace->logFormatted

// A (very) direct implementation of the variadic WriteLogEntry() Modgen function
#define WriteLogEntry theLog->logFormatted

// A (very) direct implementation of the variadic WarningMsg() Modgen function
#define WarningMsg theLog->logFormatted

// Fixed precision time arithmetic is implemented using the wrapped Time typoe in ompp.
#define CoarsenMantissa(x) Time(x)

// map actor_id to entity_id (allows use of actor_id member in C++ model code)
#define actor_id entity_id
