/**
 * @file optional_IDE_helper.h
 *          
 * Boilerplate to help an IDE editor understand C++ code in mpp and ompp modules.
 * 
 * Optionally, include this file at the top of an mpp or ompp model code module.
 * Doing so allows the IDE to understand model symbols used in C++ code in the module.
 */
 // Copyright (c) 2024-2024 OpenM++ Contributors
 // This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once

#if !defined(MODGEN) // Not for Modgen build
#include "omc/omSimulation.h" // The master include file for a model
#endif
