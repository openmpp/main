/**
* @file    globals0.h
* Global functions available to all models (early)
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once

#include <string>

// The following global functions are defined in framework modules,
// as indicated.
// They are declared early to allow use in templates, e.g. <Event>

// defined in use/common.ompp
void handle_backwards_time(double the_current_time, double the_event_time,  int the_event, int the_entity);

// defined in use/common.ompp
void handle_clairvoyance(double the_current_time, double the_future_time, int the_future_entity);

// defined in use/common.ompp
void handle_null_dereference();

// defined in use/common.ompp
void handle_prohibited_attribute_assignment(const std::string name);

// defined in use/common.ompp
void handle_prohibited_timelike_attribute_access(const std::string name);

// defined in use/common.ompp
void handle_derived_table_API_invalid_rank(const char* name, size_t indices_rank, size_t indices_count);

// defined in use/common.ompp
void handle_derived_table_API_invalid_index(const char* name, size_t index_position, size_t index_max, int index_value);

// defined in use/common.ompp
void handle_bounds_error(const std::string name, int min_value, int max_value, int value);

// defined in use/common.ompp
double get_global_time();

