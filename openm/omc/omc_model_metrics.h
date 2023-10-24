/**
* @file    omc_model_metrics.h
*          
* Function to create Model Metrics Report
*
*/
// Copyright (c) 2023-2023 OpenM++ Contributors (see AUTHORS.txt)
// This code is licensed under the MIT license (see LICENSE.txt)

#pragma once

#include <string>
#include "CodeGen.h"

using namespace std;

extern void do_model_metrics_report(string& outDir, string& model_name, CodeGen& cg);
