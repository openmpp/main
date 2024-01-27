/**
* @file    omc_model_doc.h
*
* Function to create Model doc
*
*/
// Copyright (c) 2023-2023 OpenM++ Contributors (see AUTHORS.txt)
// This code is licensed under the MIT license (see LICENSE.txt)

#pragma once

#include <string>
#include "CodeGen.h"

using namespace std;

extern void do_model_doc(bool devMode, string& outDir, string& omrootDir, string& model_name, CodeGen& cg);
