/**
 * @file    AgentVar.cpp
 * Define template static members associated with AgentVar
 *         
 */
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "omc/AgentVar.h"

namespace mm {

template<typename T, typename A, void (A::*side_effects)(T old_value, T new_value)>
size_t AgentVar<T, A, side_effects>::offset_in_agent = 0;

} // namespace mm
