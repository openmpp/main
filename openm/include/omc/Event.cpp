/**
 * @file    Event.cpp
 * Define template static members associated with Event
 *         
 */
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "omc/Event.h"

namespace mm {

template<typename A, const int event_id, const int event_priority, void (A::*implement_function)(), Time (A::*time_function)()>
size_t Event<A, event_id, event_priority, implement_function, time_function>::offset_in_agent = 0;

} // namespace mm
