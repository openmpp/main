/**
 * @file    Driver.cpp
 * Implements the driver class.
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <iostream>
#include <fstream>
#include "Driver.h"
#include "ParseContext.h"
#include "location.hh"
#include "ModuleSymbol.h"
#include "libopenm/omLog.h"

using namespace std;

Driver::Driver(ParseContext& pc)
	: trace_scanning(false)
	, trace_parsing(false)
    , pc(pc)
{
}

Driver::~Driver()
{
}

int Driver::parse(string * in_filename, const string & module_name, const string & file_stem, ofstream * markup_stream)
{
    pfile = in_filename;
    file = *pfile;
    stem = file_stem;
    outside = markup_stream;

	// Create symbol for this module.
    auto sym = new ModuleSymbol(module_name);
    assert(sym);

    pc.InitializeForModule();

    scan_begin();
    yy::parser parser(*this, pc);
    parser.set_debug_level(trace_parsing);
    int res = parser.parse();
    scan_end();

	return res;
}

void Driver::error(const yy::location & l, const string & m)
{
    pc.parse_errors++;
    theLog->logFormatted("%s(%d) : %s", l.begin.filename->c_str(), l.begin.line, m.c_str());
    //cerr << l << ": " << m << endl;
}

void Driver::warning(const yy::location & l, const string & m)
{
    pc.parse_warnings++;
    theLog->logFormatted("%s(%d) : %s", l.begin.filename->c_str(), l.begin.line, m.c_str());
    //cerr << l << ": " << m << endl;
}

void Driver::error(const string & m)
{
    pc.parse_errors++;
    theLog->logFormatted("%s", m.c_str());
    //cerr << m << endl;
}

