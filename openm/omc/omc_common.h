/**
 * @file    omc_common.h
 * Common declarations for omc
 */
// Copyright (c) 2013-2016 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once

#include "libopenm/omLog.h"

using namespace std;

/** LT localisation function: return is temporary const char* and must be copied to avoid memory violation crash. */
#define LT(sourceMessage) ((theLog->getMessage(sourceMessage)).c_str())
