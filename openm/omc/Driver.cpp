/**
 * @file    Driver.cpp
 * Implements the driver class.
 */
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <iostream>
#include <fstream>
#include "Driver.h"
#include "ModelSpec.h"
#include "ParseContext.h"
#include "location.hh"

using namespace std;

Driver::Driver ()
  : trace_scanning (false), trace_parsing (false)
{
}

Driver::~Driver ()
{
}

int Driver::parse (const string& in_filename, const string& mname, ofstream *outside_output, ModelSpec& ms, ParseContext& pc)
{
    pc.InitializeForModule();
	file = in_filename;
	stem = mname;

	outside = outside_output;

	// Create symbol for this module.
	// This symbol can be overidden later by model code in the module
    auto sym = new ModuleSymbol( file );

    scan_begin ();
    yy::parser parser (*this, ms, pc);
    parser.set_debug_level (trace_parsing);
    int res = parser.parse ();
    scan_end ();

	return res;
}

void Driver::error (const yy::location& l, const string& m)
{
  cerr << l << ": " << m << endl;
}

void Driver::error (const string& m)
{
  cerr << m << endl;
}

