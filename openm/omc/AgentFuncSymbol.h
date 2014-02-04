/**
* @file    AgentFuncSymbol.h
* Declarations for the AgentFuncSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include <string>
#include "AgentMemberSymbol.h"
#include "CodeBlock.h"

using namespace std;

/**
* Symbol for a function member of an agent.
*
* The symbol table contains an AgentFuncSymbol for each agent function,
* apart from event implement functions, which are represented by AgentEventSymbol's.
*/
class AgentFuncSymbol : public AgentMemberSymbol
{
private:
    typedef AgentMemberSymbol super;

public:
    bool is_base_symbol() const { return false; }


    /**
    * Constructor.
    *
    * @param [in,out]  sym The symbol to be morphed.
    */
    AgentFuncSymbol(Symbol *sym, const Symbol *agent, string return_decl, string arg_list_decl, bool suppress_defn = false, yy::location decl_loc = yy::location())
        : AgentMemberSymbol(sym, agent, decl_loc)
        , return_decl(return_decl)
        , arg_list_decl(arg_list_decl)
        , suppress_defn(suppress_defn)
    {
    }

    AgentFuncSymbol(const string name, const Symbol *agent, string return_decl, string arg_list_decl, bool suppress_defn = false, yy::location decl_loc = yy::location())
        : AgentMemberSymbol(name, agent, decl_loc)
        , return_decl(return_decl)
        , arg_list_decl(arg_list_decl)
        , suppress_defn(suppress_defn)
    {
    }
    AgentFuncSymbol(const string name, const Symbol *agent)
        : AgentFuncSymbol(name, agent, "void", "", false)
    {
    }

    void post_parse(int pass);


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
     * Flag to suppress generation of function definition
     */

    bool suppress_defn;

    /**
     * The function body.
     */

    CodeBlock func_body;

};

