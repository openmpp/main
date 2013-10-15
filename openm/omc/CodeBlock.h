/**
 * @file    CodeBlock.h
 * Declares and implements the CodeBlock class.
 */
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include <cassert>
#include <string>
#include <list>
#include <ostream>

using namespace std;

class CodeBlock : public list<string>
{
public:
    explicit CodeBlock ()
        : raw_mode ( false )
        , one_indent ( "    " )
        , current_indent ( "" )
    {
    }

    /**
     * Increment operator - increase indentation by one level for appended content
     *
     * @return  The same CodeBlock, but with an increased indent for appended content.
     */

    CodeBlock operator++(int)
    {
        current_indent += one_indent;
        return *this;
    }

    /**
     * Decrement operator - decrease indentation by one level for appended content
     *
     * @return  The same CodeBlock, but with a decreased indent for appended content.
     */

    CodeBlock operator--(int)
    {
        if ( current_indent.length() > 0 )
            current_indent.resize( current_indent.length() - one_indent.length() );
        else
            assert( 0 ); // indentation error
        return *this;
    }

    /**
     * Assignment by sum operator - Append line of code at current indentation level.
     * If the line contains { or } the indent level is incremented or decremented.
     *
     * @param   line The line of code to be added, with no trailing newline.
     *
     * @return  The CodeBlock with an additional appended line.
     *          
     */

    CodeBlock operator+=(string line)
    {
        if ( line.length() > 0 ) {
            if ( raw_mode ) {
                push_back( current_indent + line );
            }
            else {
                bool lbrace = string::npos != line.find('{');
                bool rbrace = string::npos != line.find('}');
                bool trailing_colon = false;
                if ( line.back() == ':' ) {
                    if (   string::npos != line.find("case ")
                        || string::npos != line.find("private")
                        || string::npos != line.find("public")
                       ) trailing_colon = true;
                }
                // if both { and } are found in the same line, leave indenting alone
                if ( rbrace && lbrace ) rbrace = lbrace = false;
                // decrease indent before the line
                if ( rbrace || trailing_colon ) (*this)--;
                push_back( current_indent + line );
                // increase indent after the line
                if ( lbrace || trailing_colon ) (*this)++;
            }
        }
        else
            // no indent for empty lines
            push_back( "" );
        return *this;
    }

    /**
     * Assignment by sum operator - Append block of code at current indentation level.
     * Each line of \a append_block will be indented further
     * using the current indentation level.
     *
     * @param   append_block The block of code to be added.
     *
     * @return  The CodeBlock with an additional appended block of code lines.
     */

    CodeBlock operator+=(CodeBlock append_block)
    {
        for ( string line : append_block ) {
            if ( line.length() > 0 ) {
                push_back( current_indent + line );
            }
            else {
                // no indent for empty lines
                push_back( "" );
            }
        }
        return *this;
    }

    /**
     * Control smart indenting.
     * Default is true.
     *
     * @param   val true or false.
     */

    void smart_indenting( bool val )
    {
        raw_mode = ! val;
    }

private:
    bool raw_mode;
    string one_indent;
    string current_indent;
};

/**
 * Creates a doxygen-style description comment.
 * To force the creation of blank lines, use an argument with a singel space like " ".
 *
 * @param   t1  The first string.
 * @param   t2  (Optional) the second  string.
 * @param   t3  (Optional) the third  string.
 * @param   t4  (Optional) the fourth  string.
 * @param   t5  (Optional) the fifth  string.
 *
 * @return  The result as a CodeBlock.
 */

inline CodeBlock doxygen( string t1, string t2 = "",string t3 = "", string t4 = "", string t5 = "" )
{
    // Perhaps this could be generalized in C++11 using a variadic template,
    // but in any case don't use old-style var_args.
 
    CodeBlock result;
    result += "";
    result += "/**";
    result += " * " + t1;
    if ( t2.length() > 0 ) result += " * " + t2;
    if ( t3.length() > 0 ) result += " * " + t3;
    if ( t4.length() > 0 ) result += " * " + t4;
    if ( t5.length() > 0 ) result += " * " + t5;
    result += " */";
    result += "";
    return result;
}


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
