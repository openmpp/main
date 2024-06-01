/**
* @file    omc_markup.cpp
*
* Function to markup model code file
*
*/
// Copyright (c) 2024-2024 OpenM++ Contributors (see AUTHORS.txt)
// This code is licensed under the MIT license (see LICENSE.txt)

#include <iostream>
#include <fstream>
#include <regex>

#include "Symbol.h"
#include "omc_markup.h"

void do_markup(const string om_developer_cpp_path)
{
    if (!Symbol::option_index_errors) {
        // no markup necessary
        return;
    }
    // to do
    om_developer_cpp_path;
}