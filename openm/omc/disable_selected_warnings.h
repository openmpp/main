/**
* @file    disable_selected_warnings.h
*
* Disable selected warnings for C++ compilers.
*
*/
// Copyright (c) 2023-2023 OpenM++ Contributors (see AUTHORS.txt)
// This code is licensed under the MIT license (see LICENSE.txt)

#pragma once

//
// The following pragmas disable selected benign warnings
//
#if defined(_MSC_VER)
    // compiler-specific pragmas for MSVC
#endif
#if defined(__GNUC__)
    // compiler-specific pragmas for gcc
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wunused-function"
#endif
#if defined(__clang__)
    // compiler-specific pragmas for clang
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-but-set-variable"
#pragma clang diagnostic ignored "-Wunused-function"
#endif