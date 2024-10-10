/**
* @file    omc_markup.h
*
* Function to markup model code file
*
*/
// Copyright (c) 2024-2024 OpenM++ Contributors (see AUTHORS.txt)
// This code is licensed under the MIT license (see LICENSE.txt)

#pragma once

#include <string>

using namespace std;

/**
 * Executes the markup operation
 *
 * @param   om_developer_cpp_path   Full path of model code passthrough file.
 * @param   om_definitions_cpp_path Full pathname of the om_definitions C++ file.
 */
extern void do_markup(const string& om_developer_cpp_path, const string& om_definitions_cpp_path);
