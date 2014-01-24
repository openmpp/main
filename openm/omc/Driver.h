/**
 * @file    Driver.h
 * Declares the Driver class.
 *         
 * The Driver class manages communication between the parser and the scanner.
 */
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include <iostream>
#include <fstream>
#include "Symbol.h"

class ParseContext;

using namespace std;

// Tell Flex the lexer's prototype ...
# define YY_DECL                                        \
  yy::parser::token_type                         \
  yylex (yy::parser::semantic_type* yylval,      \
         yy::parser::location_type* yylloc,      \
         Driver& drv,    \
         ParseContext& pc )
// ... and declare it for the parser's sake.
YY_DECL;

// Conducting the whole scanning and parsing of ompp,
// and contextual information for parser/scanner communication
class Driver
{
public:
    Driver ( ParseContext& pc );
    virtual ~Driver ();

    // pointer to output stream for pass-through / mark-up
    ofstream *outside;

    // pointer to the input source file name (persistent)
    string *pfile;

    // input source file (transient)
    string file;

    // Stem of the file name (used later to generated default ModelName)
    string stem;

    int result;

    // Handling the scanner.
    void scan_begin ();
    void scan_end ();
    bool trace_scanning;

    // Run the parser.  Return 0 on success.
    int parse(string *in_filename, const string& module_name, const string& file_stem, ofstream *markup_stream);

    bool trace_parsing;

    // Error handling.
    void error (const yy::location& l, const string& m);
    void error (const string& m);

private:
    ParseContext &pc;
};
