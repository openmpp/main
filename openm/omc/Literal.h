/**
 * @file    Literal.h
 * Declares the Literal class and sub-classes.
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

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

    string value() const
    {
        return cxx_token;
    }

protected:
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

    /**
     * Constructor.
     *
     * @param tok The literal, with no leading minus sign
     */

    IntegerLiteral( const string& tok )
        : Literal (tok)
    {
    }

    /**
    * Integer literal validator.
    *
    * @return true if 'i_value' string represent valid integer literal
    */
    static bool is_valid_literal(const char * i_value);
};

/**
* A C++ floating point literal
*
* Example: 3.14159
*/

class FloatingPointLiteral : public Literal
{
public:

    /**
     * Constructor.
     *
     * @param tok The literal, with no leading minus sign.
     */

    FloatingPointLiteral(const string& tok)
        : Literal(tok)
    {
    }

    /**
    * Floating point literal validator.
    *
    * @return true if 'i_value' string represent valid floating point literal
    */
    static bool is_valid_literal(const char * i_value);
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

    /**
    * String literal validator.
    *
    * @return true if 'i_value' string is not a NULL.
    */
    static bool is_valid_literal(const char * i_value) { return i_value != nullptr; };
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

    /**
    * Boolean literal validator.
    *
    * @return true if 'i_value' string represent valid boolean literal
    */
    static bool is_valid_literal(const char * i_value);
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

    /**
    * Time literal validator.
    *
    * @return true if 'i_value' string represent valid Time literal
    */
    static bool is_valid_literal(const char * i_value);
};

