/**
* @file    globals0.h
* Global functions available to all models (early)
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once

// The following global functions are defined in framework modules,
// as indicated. They are mostly documented in the Modgen Developer's Guide.

// defined in use/common.ompp
extern void ModelExit(const char * msg); // declare early for error handling in fundamental classes, e.g. Event.h
