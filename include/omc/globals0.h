/**
* @file    globals0.h
* Global functions available to all models (early)
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once

// The following global functions are defined in framework modules,
// as indicated.
// They are declared early to allow use in templates, e.g. <Event>

// defined in use/common.ompp
void handle_backwards_time(double the_current_time, double the_event_time,  int the_event, int the_entity);

// defined in use/common.ompp
void handle_clairvoyance(double the_current_time, double the_future_time, int the_future_entity);

// defined in use/common.ompp
double get_global_time();

