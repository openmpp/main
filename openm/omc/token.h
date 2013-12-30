/**
* @file    token.h
* Declares typedefs and functions to use the bison-generated enum for token values.
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include <string>
#include "parser.hpp"

using namespace std;

/**
* Defines an alias used to reference bison-generated token values.
*
* For example token::TK_agent is the value of the bison-generated token associated with the
* openM++ keyword 'agent'.
*/

typedef yy::parser::token token;

/**
* Defines an alias for the bison-generated enum holding token values.
*
* Used to declare members, function arguments, and return values for token values.
*/

typedef yy::parser::token_type token_type;

const string token_to_string(const token_type& e);
const token_type string_to_token(const char * s);
const token_type modgen_cumulation_operator_to_acc(const token_type& e);
const token_type modgen_cumulation_operator_to_incr(const token_type& e);
