/**
 * @file    omc_model_code_metrics.h
 * omc function to rpoduce Model Code Metrics Report
 */
// Copyright (c) 2023-2023 OpenM++ Contributors (see AUTHORS.txt)
// This code is licensed under the MIT license (see LICENSE.txt)

#pragma once

#include <string>
#include "CodeGen.h"

using namespace std;

extern void do_model_metrics_report(string& outDir, string& model_name, CodeGen& cg);
