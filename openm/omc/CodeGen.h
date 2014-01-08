/**
 * @file    CodeGen.h
 * Declares the code generation class.
 */
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include <iostream>
#include <fstream>
#include "CodeBlock.h"

#include "libopenm/db/metaModelHolder.h"

using namespace std;

class CodeGen
{
public:
	CodeGen( 
        ofstream *oat_arg, 
        ofstream *oah_arg, 
        ofstream *oac_arg, 
        const string & i_timestamp, 
        openm::MetaModelHolder & io_metaRows 
        )
		: oat (oat_arg)
		, oah (oah_arg)
		, oac (oac_arg)
        , time_stamp(i_timestamp)
        , metaRows(io_metaRows)

	{
	}

	~CodeGen()
	{
	}

	void do_all();
	void do_preamble();
	void do_model_name();
	void do_time_stamp();
	void do_types();
	void do_parameters();
	void do_agents();
	void do_tables();
	void do_event_queue();
	void do_ModelStartup();
	void do_RunModel();
	void do_ModelShutdown();
    void do_API_entries();


    /**
     * CodeBlock for om_types.h
     */

    CodeBlock t;


    /**
    * CodeBlock for om_declarations.h
    */

    CodeBlock h;


    /**
    * CodeBlock for om_definitions.cpp
    */

    CodeBlock c;

    /** output stream for om_types.h */
	ofstream *oat;

    /** output stream for om_declarations.h */
	ofstream *oah;

    /** output stream for om_definitions.cpp */
	ofstream *oac;

private:
    const string time_stamp;        // model compilation timestamp
    openm::MetaModelHolder & metaRows;  // model metadata rows
};