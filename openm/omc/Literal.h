/**
 * @file    Literal.h
 * Declares the Literal class and sub-classes.
 */
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include <string>

using namespace std;

/**
 * Literal encapsulates a C++ literal or an openM++ literal.
 * 
 * When a C++ literal is identified by the scanner
 * it creates an instance of a sub-class of Literal which encapsulates
 * the C++ representation of the literal as a string, and passes it to the parser.
 * For example, a FloatingPointLiteral might encapsulate a string
 * consisting of the 7 characters in 2.71828.
 * 
 * The sub-classes of Literal have no additional members or functions - 
 * they just provide RTTI used by the parser and code generator.
 * 
 * The sub-class TimeLiteral is the only literal which is not a C++ literal.
 */

class Literal
{
public:
	virtual ~Literal() { }
    Literal( const string& tok )
        : cxx_token ( tok )
    {
    }

    const string cxx_token;
};

/**
 * A C++ integer literal
 * 
 * Example: 2147483647
 */

class IntegerLiteral : public Literal
{
public:
    IntegerLiteral( const string& tok )
        : Literal (tok)
    {
    }
};

/**
 * A C++ character literal
 * 
 * Example: '\n'
 */

class CharacterLiteral : public Literal
{
public:
    CharacterLiteral( const string& tok )
        : Literal (tok)
    {
    }
};

/**
 * A C++ floating point literal
 * 
 * Example: 3.14159
 */

class FloatingPointLiteral : public Literal
{
public:
    FloatingPointLiteral( const string& tok )
        : Literal (tok)
    {
    }
};

/**
 * A C++ string literal
 * 
 * Example: "Hello world!\n"
 */

class StringLiteral : public Literal
{
public:
    StringLiteral( const string& tok )
        : Literal (tok)
    {
    }
};

/**
 * A C++ boolean literal
 * 
 * There are two C++ boolean literals: true and false.
 */

class BooleanLiteral : public Literal
{
public:
    BooleanLiteral( const string& tok ) : Literal (tok) 
    { 
    }
};

/**
 * A C++ pointer literal
 * 
 * The only C++ pointer literal is nullptr
 */

class PointerLiteral : public Literal
{
public:
    PointerLiteral( const string& tok ) 
        : Literal (tok) 
    {
    }
};

/**
 * An openM++ time literal
 * 
 * A time literal is recognized as a literal by openM++, but is not a C++ literal.
 * A time literal can have the values time_infinite or time_undef.
 * It can also be any constant of type Time, as declared in the model source.
 */

class TimeLiteral : public Literal
{
public:
    TimeLiteral( const string& tok )
        : Literal (tok) 
    {
    }
};

