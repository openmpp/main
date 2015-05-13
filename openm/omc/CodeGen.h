/**
 * @file    CodeGen.h
 * Declares the code generation class.
 */
// Copyright (c) 2013-2014 OpenM++
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
        ofstream *oat0_arg, 
        ofstream *oat1_arg,
        ofstream *oah_arg,
        ofstream *oac_arg, 
        ofstream *oaz_arg, 
        const string & i_timestamp, 
        openm::MetaModelHolder & io_metaRows 
        )
		: oat0(oat0_arg)
        , oat1(oat1_arg)
        , oah(oah_arg)
		, oac(oac_arg)
		, oaz(oaz_arg)
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
	void do_aggregations();
	void do_parameters();
	void do_agents();
	void do_entity_sets();
	void do_tables();
	void do_derived_tables();
	void do_event_queue();
	void do_RunInit();
	void do_ModelStartup();
	void do_RunModel();
	void do_ModelShutdown();
    void do_API_entries();

    /**
     * CodeBlock for om_types0.h.
     */
    CodeBlock t0;

    /**
     * CodeBlock for om_types1.h.
     */
    CodeBlock t1;

    /**
     * CodeBlock for om_declarations.h.
     */
    CodeBlock h;

    /**
     * CodeBlock for om_definitions.cpp.
     */
    CodeBlock c;

    /**
     * CodeBlock for om_initializers.cpp.
     */
    CodeBlock z;

    /** output stream for om_types0.h */
    ofstream *oat0;

    /** output stream for om_types1.h */
    ofstream *oat1;

    /** output stream for om_declarations.h */
	ofstream *oah;

    /** output stream for om_definitions.cpp */
	ofstream *oac;

    /** output stream for om_initializers.cpp */
	ofstream *oaz;

private:
    const string time_stamp;        // model compilation timestamp
    openm::MetaModelHolder & metaRows;  // model metadata rows
};