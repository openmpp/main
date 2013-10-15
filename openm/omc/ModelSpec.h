/**
 * @file    ModelSpec.h
 * Declares the model specification class.
 */
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include <string>
#include <unordered_set>
#include <forward_list>
#include <map>
#include "parser_helper.h"
#include "location.hh"

using namespace std;

class ModelSpec {
public:
    ModelSpec();
    ~ModelSpec();

    void process_cxx_comment(const string& cmt, yy::location& loc);
    void process_c_comment(const string& cmt, yy::location& loc);

    bool is_om_outer_keyword(const token_type& tok);
    bool is_om_developer_function(const char* nm);

private:
    void initialize_om_outer_keywords();
    void initialize_om_developer_functions();

private:
    unordered_set<token_type, std::hash<int> > om_outer_keywords;
    unordered_set<string> om_developer_functions;

    // TODO comments are just dumped into placeholder collections for testing.
    // Need to retrieve by location searching, at some point, to match up with identifiers, languages, etc.
    forward_list<string> cxx_comments;
    forward_list<string> c_comments;
};
