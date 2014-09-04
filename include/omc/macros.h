/**
* @file    macros.h
* Macros available to all models.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

// The following macro is not defined for Modgen models.
// It can be used to exclude Modgen-specific C++ code during C++ compilation by openM++.
// Both the Modgen compiler and omc ignore preprocessor directives in model source code.
#define OPENM

#define TRUE true
#define FALSE false
#define WAIT( delta_time ) static_cast<Time>((time + static_cast<Time::type>(delta_time)))

// Implementation of old-style Modgen macros related to classifications, ranges, and partitions
#define MIN( symbol )				(symbol::min)
#define MAX( symbol )				(symbol::max)
#define SIZE( symbol )				(symbol::size)
#define RANGE_POS( rname, val )     (rname::to_index(val))
#define COERCE( rname, val )        (((val) > rname::max) ? rname::max : ((val) < rname::min) ? rname::min : (val))
#define WITHIN( rname, val )        (rname::within(val))
#define POINTS( symbol )		   	(symbol::upper_bounds())
#define SPLIT( val, symbol )        (symbol::value_to_interval(val))

// Ignore IMPLEMENT_HOOK 
#define IMPLEMENT_HOOK()

// override std:min and std::max to enable type conversion of arguments
#ifndef min
#define min(a,b) (((a)< (b)) ? (a) : (b))
#endif

#ifndef max
#define max(a,b) (((a)> (b)) ? (a) : (b))
#endif

// translate use of CString in model developer code to C++ string for portability
#define CString string

// A (very) direct implementation of the variadic WriteDebugLogEntry() Modgen function
#define WriteDebugLogEntry theTrace->logFormatted

// map logical to bool (for local variable declarations in Modgen models)
#define logical bool

