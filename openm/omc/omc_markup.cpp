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
#include <chrono>
#include <thread>

//#include "omc_file.h"

#include "Symbol.h"
#include "omc_markup.h"

void do_markup(const string& om_developer_cpp_path)
{
    if (!Symbol::option_index_errors) {
        // no markup needed
        return;
    }

    theLog->logMsg("Model code markup - start");

    // wait 1 second for previous close of om_developer_cpp to settle
    this_thread::sleep_for(chrono::milliseconds(1000));

    // TODO: use u8string for conde_in and code_out when available on all target platforms.

    /// Original model code (om_developer.cpp)
    string code_in;
    /// Modified model code (om_developer.cpp)
    string code_out;

    {
        // Slurp om_developer.cpp to string code_in
        // Note that om_developer_cpp is a UTF-8 file, so no conversion is needed.
        ifstream ifs(om_developer_cpp_path, ios::in | ios::binary);
        std::ostringstream sstr;
        sstr << ifs.rdbuf();
        ifs.close();
        code_in = sstr.str();
    }

    // create code_out form code_in
    code_out = code_in;
    // test modification
    code_out += "\n// Hello code!\n";

    {
        // Dump string code_out to om_developer.cpp
        // wait 1 second for close of om_developer_cpp to settle
        this_thread::sleep_for(chrono::milliseconds(1000));
        ofstream ofs(om_developer_cpp_path, ios::out | ios::trunc | ios::binary);
        ofs << code_out;
        ofs.close();
    }

    theLog->logMsg("Model code markup - finish");
}