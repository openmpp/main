/**
 * @file    Agent.cpp
 * Define template static members associated with Agent
 *         
 */
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "omc/Agent.h"

namespace mm {

/**
 * Agent zombie list (definition)
 */

template<typename A>
forward_list<A *> Agent<A>::zombies;

/**
 * Agent available list (definition)
 */

template<typename A>
forward_list<A *> Agent<A>::available;

} // namespace mm
