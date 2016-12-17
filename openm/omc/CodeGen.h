/**
 * @file    CodeGen.h
 * Declares the code generation class.
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include <iostream>
#include <fstream>
#include "CodeBlock.h"

#include "libopenm/db/metaModelHolder.h"
#include "libopenm/db/modelBuilder.h"

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
        CodeBlock & missing_dat,
        const openm::IModelBuilder * i_builder,
        bool no_line_directives,
        string c_fname,
        openm::MetaModelHolder & io_metaRows 
        )
		: oat0(oat0_arg)
        , oat1(oat1_arg)
        , oah(oah_arg)
		, oac(oac_arg)
        , c_fname(c_fname)
		, oaz(oaz_arg)
        , m(missing_dat)
        , no_line_directives(no_line_directives)
        , modelBuilder(i_builder)
        , metaRows(io_metaRows)

	{
	}

	~CodeGen()
	{
	}

	void do_all();
	void do_preamble();
    void do_model_strings();
	void do_types();
	void do_aggregations();
	void do_parameters();
	void do_agents();
	void do_entity_sets();
	void do_entity_tables();
	void do_derived_tables();
    void do_name_digest(void);

    /**
     * Generate model-specific helper code for the table read/write model developer API.
     */
	void do_table_interface();
	void do_event_queue();
	void do_RunInit();
	void do_ModelStartup();
	void do_RunModel();
	void do_ModelShutdown();
    void do_API_entries();
    void do_ParameterNameSize(void);

    /**
     * CodeBlock for om_types0.h.
     */
    CodeBlock t0;

    /** output stream for om_types0.h */
    ofstream *oat0;

    /**
     * CodeBlock for om_types1.h.
     */
    CodeBlock t1;

    /** output stream for om_types1.h */
    ofstream *oat1;

    /**
     * CodeBlock for om_declarations.h.
     */
    CodeBlock h;

    /** output stream for om_declarations.h */
	ofstream *oah;

    /**
     * CodeBlock for om_fixed_parms.cpp.
     */
    CodeBlock c;

    /** output stream for om_definitions.cpp */
	ofstream *oac;

    /** full name of om_definitions.cpp */
	string c_fname;

    /**
     * CodeBlock for om_initializers.cpp.
     */
    CodeBlock z;

    /** output stream for om_fixed_parms.cpp */
	ofstream *oaz;

    /**
     * CodeBlock for Missing.dat.
     */
    CodeBlock &m;

    /** disable #line directives */
	bool no_line_directives;

private:
    const openm::IModelBuilder * modelBuilder;  // model builder to make sql from meta rows
    openm::MetaModelHolder & metaRows;          // model metadata rows
};