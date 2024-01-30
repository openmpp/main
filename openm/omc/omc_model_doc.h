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

/** create model user documentation and model developer documentation.
 *
 * @param[in] devMode     if true then create developer documentation
 * @param[in] model_name  model name
 * @param[in] docOutDir   output documentation directory, e.g.: ompp/bin/doc
 * @param[in] mdOutDir    intermediate directory to write .md files: ompp/src
 * @param[in] omrootDir   openM++ "root" deirectory to find HTML styles
 * @param[in] cg          code generator to get model metadata
 */
extern void do_model_doc(
	const string & model_name, bool devMode, const string & docOutDir, const string & mdOutDir, const string & omrootDir, const CodeGen & cg
);
