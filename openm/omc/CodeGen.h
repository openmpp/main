/**
 * @file    CodeGen.h
 * Declares the code generation class.
 */
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include <iostream>
#include <fstream>
#include "ModelSpec.h"
#include "CodeBlock.h"

#include "libopenm/db/metaModelHolder.h"

using namespace std;

class CodeGen
{
public:
	CodeGen( 
        ModelSpec& ms_arg, 
        ofstream *oat_arg, 
        ofstream *oah_arg, 
        ofstream *oac_arg, 
        const string & i_timestamp, 
        openm::MetaModelHolder & io_metaRows 
        )
		: ms (ms_arg)
		, oat (oat_arg)
		, oah (oah_arg)
		, oac (oac_arg)
        , modelTimeStamp(i_timestamp)
        , metaRows(io_metaRows)
	{
	}

	~CodeGen()
	{
	}

	void do_all();
	void do_preamble( CodeBlock& t, CodeBlock& h, CodeBlock& c );
	void do_model_name( CodeBlock& h, CodeBlock& c );
	void do_time_stamp( CodeBlock& h, CodeBlock& c );
	void do_types( CodeBlock& t, CodeBlock& c );
	void do_parameters( CodeBlock& h, CodeBlock& c );
	void do_agents( CodeBlock& h, CodeBlock& c );
	void do_tables( CodeBlock& h, CodeBlock& c );
	void do_event_queue( CodeBlock& h, CodeBlock& c );
	void do_ModelStartup( CodeBlock& h, CodeBlock& c );
	void do_RunModel( CodeBlock& h, CodeBlock& c );
	void do_ModelShutdown( CodeBlock& h, CodeBlock& c );
    void do_API_entries( CodeBlock& h, CodeBlock& c );

	ModelSpec ms;

    /** output stream for om_types.h */
	ofstream *oat;

    /** output stream for om_agents.h */
	ofstream *oah;

    /** output stream for om_agents.cpp */
	ofstream *oac;

private:
    const string modelTimeStamp;        // model compilation timestamp
    openm::MetaModelHolder & metaRows;  // model metadata rows
};