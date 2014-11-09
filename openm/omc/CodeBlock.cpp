/**
* @file    CodeBlock.cpp
* Definitions for the CodeBlock class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include "CodeBlock.h"

CodeBlock & CodeBlock::operator++(int)
{
    current_indent += one_indent;
    return *this;
}

CodeBlock & CodeBlock::operator--(int)
{
    if (current_indent.length() > 0)
        current_indent.resize(current_indent.length() - one_indent.length());
    else
        assert(0); // indentation error
    return *this;
}

CodeBlock & CodeBlock::operator += (const string & line)
{
    if (line.length() > 0) {
        if (raw_mode) {
            push_back(current_indent + line);
        }
        else {
            bool lbrace = string::npos != line.find('{');
            bool rbrace = string::npos != line.find('}');
            bool trailing_colon = false;
            if (line.back() == ':') {
                if (string::npos != line.find("case ")
                    || string::npos != line.find("private")
                    || string::npos != line.find("public")
                    ) trailing_colon = true;
            }
            // if both { and } are found in the same line, leave indenting alone
            if (rbrace && lbrace) rbrace = lbrace = false;
            // decrease indent before the line
            if (rbrace || trailing_colon) (*this)--;
            push_back(current_indent + line);
            // increase indent after the line
            if (lbrace || trailing_colon) (*this)++;
        }
    }
    else
        // no indent for empty lines
        push_back("");
    return *this;
}

CodeBlock & CodeBlock::operator += (const CodeBlock & append_block)
{
    for (string line : append_block) {
        if (line.length() > 0) {
            push_back(current_indent + line);
        }
        else {
            // no indent for empty lines
            push_back("");
        }
    }
    return *this;
}

void CodeBlock::smart_indenting(bool val)
{
    raw_mode = !val;
}

CodeBlock doxygen(string t0, string t1, string t2, string t3, string t4, string t5, string t6, string t7, string t8, string t9)
{
    // TODO: Generalize using a variadic template

    CodeBlock result;
    result += "/**";
    result += " * " + t0;
    if (t1.length() > 0) result += " * " + t1;
    if (t2.length() > 0) result += " * " + t2;
    if (t3.length() > 0) result += " * " + t3;
    if (t4.length() > 0) result += " * " + t4;
    if (t5.length() > 0) result += " * " + t5;
    if (t6.length() > 0) result += " * " + t6;
    if (t7.length() > 0) result += " * " + t7;
    if (t8.length() > 0) result += " * " + t8;
    if (t9.length() > 0) result += " * " + t9;
    result += " */";
    return result;
}

CodeBlock doxygen_short(string t1)
{
    CodeBlock result;
    result += "/** " + t1 + " */";
    return result;
}
