/**
 * @file    ModelSpec.cpp
 * Implements the model specification class.
 */
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include <algorithm>
#include <sstream>
#include "ast.h"
#include "parser_helper.h"

ModelSpec::ModelSpec()
{
	Symbol::default_symbols();

    initialize_om_outer_keywords();
    initialize_om_developer_functions();
}

ModelSpec::~ModelSpec()
{
}

void ModelSpec::process_cxx_comment(const string& cmt, yy::location& loc)
{
    // TODO just a placeholder container
    cxx_comments.push_front( cmt );
}

void ModelSpec::process_c_comment(const string& cmt, yy::location& loc)
{
    // TODO just a placeholder container
    c_comments.push_front( cmt );
}


bool ModelSpec::is_om_outer_keyword(const token_type& tok)
{
    return om_outer_keywords.count( tok ) == 0 ? false : true;
}

/**
 * Query if 'nm' is an om developer-supplied function
 * These are functions with special names, e.g. Start
 *
 * @param   nm  The name.
 *
 * @return  true if om developer function, false if not.
 */

bool ModelSpec::is_om_developer_function(const char* nm)
{
    return om_developer_functions.count( nm ) == 0 ? false : true;
}

void ModelSpec::initialize_om_outer_keywords()
{
    // TODO Implement as initializer when supported by compiler (C++11)
    // NB: There is a direct correspondence ibetween the following lines and code in parser_helper.cpp

    om_outer_keywords.insert(token::TK_agent);
    om_outer_keywords.insert(token::TK_agent_set);
    om_outer_keywords.insert(token::TK_aggregation);
    om_outer_keywords.insert(token::TK_classification);
    om_outer_keywords.insert(token::TK_counter_type);
    om_outer_keywords.insert(token::TK_dependency);
    om_outer_keywords.insert(token::TK_extend_parameter);
    om_outer_keywords.insert(token::TK_hide);
    om_outer_keywords.insert(token::TK_import);
    om_outer_keywords.insert(token::TK_index_type);
    om_outer_keywords.insert(token::TK_integer_type);
    om_outer_keywords.insert(token::TK_languages);
    om_outer_keywords.insert(token::TK_link);
    om_outer_keywords.insert(token::TK_model_generated_parameter_group);
    om_outer_keywords.insert(token::TK_model_type);
    om_outer_keywords.insert(token::TK_options);
    om_outer_keywords.insert(token::TK_parameter_group);
    om_outer_keywords.insert(token::TK_parameters);
    om_outer_keywords.insert(token::TK_partition);
    om_outer_keywords.insert(token::TK_range);
    om_outer_keywords.insert(token::TK_real_type);
    om_outer_keywords.insert(token::TK_string);
    om_outer_keywords.insert(token::TK_table);
    om_outer_keywords.insert(token::TK_table_group);
    om_outer_keywords.insert(token::TK_time_type);
    om_outer_keywords.insert(token::TK_track);
    om_outer_keywords.insert(token::TK_user_table);
    om_outer_keywords.insert(token::TK_version);

}

void ModelSpec::initialize_om_developer_functions()
{
    // TODO Implement as initializer when supported by compiler (C++11)
    om_developer_functions.insert("Simulation");
    om_developer_functions.insert("CaseSimulation");
    om_developer_functions.insert("Start");
    om_developer_functions.insert("Finish");
    om_developer_functions.insert("PreSimulation");
    om_developer_functions.insert("UserTables");
}

