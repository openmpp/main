/**
* @file    GlobalFuncSymbol.h
* Declarations for the GlobalFuncSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include <string>
#include "Symbol.h"
#include "CodeBlock.h"

using namespace std;

/**
* GlobalFuncSymbol.
*
*/
class GlobalFuncSymbol : public Symbol
{
private:
    typedef Symbol super;

public:
    bool is_base_symbol() const { return false; }

    /**
    * Constructor by name (for generated functions)
    *
    * @param [in,out]  sym The symbol to be morphed.
    */
    GlobalFuncSymbol(const string name, string return_decl, string arg_list_decl, yy::location decl_loc = yy::location())
        : Symbol(name, decl_loc)
        , arg_list_decl(arg_list_decl)
        , return_decl(return_decl)
        , suppress_defn(false)
    {
    }

    /**
     * Constructor by name (for presumed global functions in identity expressions)
     *
     * @param name     The symbol to be morphed.
     * @param decl_loc (Optional) the declaration location.
     */
    GlobalFuncSymbol(const string name, yy::location decl_loc = yy::location())
        : Symbol(name, decl_loc)
        , suppress_defn(true)
    {
    }

    void post_parse(int pass);

    CodeBlock cxx_declaration_global();

    CodeBlock cxx_definition_global();

    /**
     * The comment block which immediately preceeds the function declaration.
     */
    CodeBlock doc_block;

    /**
     * The argument list portion of the function declaration.
     */
    string arg_list_decl;

    /**
     * The return value part of the function declaration.
     */
    string return_decl;

    /**
     * Flag to suppress generation of function definition.
     */
    bool suppress_defn;

    /**
     * The function body.
     */
    CodeBlock func_body;
};
