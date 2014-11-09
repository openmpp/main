/**
 * @file    CodeBlock.h
 * Declarations for the CodeBlock class.
 */
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include <string>
#include <list>
#include <ostream>

using namespace std;

class CodeBlock : public list<string>
{
public:
    CodeBlock ()
        : raw_mode ( false )
        , one_indent ( "    " )
        , current_indent ( "" )
    {
    }


    /**
     * Increment operator - increase indentation by one level for appended content.
     *
     * @param parameter1 The first parameter.
     *
     * @return The same CodeBlock, but with an increased indent for appended content.
     */

    CodeBlock & operator++(int);


    /**
     * Decrement operator - decrease indentation by one level for appended content.
     *
     * @param parameter1 The first parameter.
     *
     * @return The same CodeBlock, but with a decreased indent for appended content.
     */

    CodeBlock & operator--(int);


    /**
     * Assignment by sum operator - Append line of code at current indentation level. If the line
     * contains { or } the indent level is incremented or decremented.
     *
     * @param line The line of code to be added, with no trailing newline.
     *
     * @return The CodeBlock with an additional appended line.
     */

    CodeBlock & operator+=(const string & line);

    /**
     * Assignment by sum operator - Append block of code at current indentation level.
     * Each line of \a append_block will be indented further
     * using the current indentation level.
     *
     * @param   append_block The block of code to be added.
     *
     * @return  The CodeBlock with an additional appended block of code lines.
     */

    CodeBlock & operator+=(const CodeBlock & append_block);

    /**
     * Control smart indenting.
     * Default is true.
     *
     * @param   val true or false.
     */

    void smart_indenting(bool val);

    /**
    * Creates a doxygen-style description comment.
    * To force the creation of blank lines, use an argument with a single space like " ".
    *
    * @param   t1  The first string.
    * @param   t2  (Optional) the second  string.
    * @param   t3  (Optional) the third  string.
    * @param   t4  (Optional) the fourth  string.
    * @param   t5  (Optional) the fifth  string.
    *
    * @return  The result as a CodeBlock.
    */

private:
    bool raw_mode;
    string one_indent;
    string current_indent;
};

/**
 * Creates a doxygen-style description comment.
 * 
 * To force the creation of blank lines, use an argument with a single space like " ".
 *
 * @param   t0  Line #1.
 * @param   t1  Line #2.
 * @param   t2  Line #3.
 * @param   t3  Line #4.
 * @param   t4  Line #5.
 * @param   t5  Line #6.
 * @param   t6  Line #7.
 * @param   t7  Line #8.
 * @param   t8  Line #9.
 * @param   t9  Line #10.
 *
 * @return  The result as a CodeBlock.
 */

CodeBlock doxygen(string t0, string t1 = "", string t2 = "", string t3 = "", string t4 = "", string t5 = "", string t6 = "", string t7 = "", string t8 = "", string t9 = "");


/**
 * Creates a one-line doxygen style descriptive comment
 *
 * @param t1 The first string.
 *
 * @return A CodeBlock.
 */

CodeBlock doxygen_short(string t1);


/**
 * Intercept output stream redirection.
 *
 * @param [in,out]  ostr    the destination output stream.
 * @param   code_block      a reference to the CodeBlock to redirect.
 *
 * @return  The destination output stream..
 */

template <typename YYChar>
inline std::basic_ostream<YYChar>&
operator<< (std::basic_ostream<YYChar>& ostr, const CodeBlock& code_block)
{
    for ( string line : code_block )
        ostr << line << "\n";
    return ostr;
}
