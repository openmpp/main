/**
* @file    AgentFuncSymbol.h
* Declarations for the AgentFuncSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include <string>
#include <set>
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
    * Morphing constructor.
    *
    * @param [in,out]  sym The symbol to be morphed.
    */
    AgentFuncSymbol(Symbol *sym, const Symbol *agent, string return_decl, string arg_list_decl, bool suppress_defn = false, yy::location decl_loc = yy::location())
        : AgentMemberSymbol(sym, agent, decl_loc)
        , arg_list_decl(arg_list_decl)
        , return_decl(return_decl)
        , suppress_defn(suppress_defn)
        , suppress_code_if_empty(false)
    {
    }

    /**
     * Constructor by name.
     *
     * @param name          The name.
     * @param agent         The agent.
     * @param return_decl   The return declaration.
     * @param arg_list_decl The argument list declaration.
     * @param suppress_defn (Optional) Suppress definition.
     * @param decl_loc      (Optional) the declaration location.
     */
    AgentFuncSymbol(const string name, const Symbol *agent, string return_decl, string arg_list_decl, bool suppress_defn = false, yy::location decl_loc = yy::location())
        : AgentMemberSymbol(name, agent, decl_loc)
        , arg_list_decl(arg_list_decl)
        , return_decl(return_decl)
        , suppress_defn(suppress_defn)
        , suppress_code_if_empty(false)
        , has_line_directive(false)
    {
    }

    /**
     * Constructor by name, with defaults.
     *
     * @param name  The name.
     * @param agent The agent.
     */
    AgentFuncSymbol(const string name, const Symbol *agent)
        : AgentFuncSymbol(name, agent, "void", "", false)
    {
    }

    void post_parse(int pass);

    CodeBlock cxx_declaration_agent();

    CodeBlock cxx_definition_agent();

    /**
     * Checks whether the function body is empty.
     *
     * @return true if empty, false otherwise.
     */
    bool empty() const
    {
        return func_body.empty();
    }

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
     * 
     * True if developer-supplied, false if generated
     */
    bool suppress_defn;

    /**
     * Flag to suppress declaration and definition if function body is empty.
     * 
     * Default is false.
     */
    bool suppress_code_if_empty;

    /**
     * Indicates if the body contains a #line directive
     * 
     * Can be used by calling code to restore the #line after the function definition.
     * 
     * Default is false.
     */
    bool has_line_directive;

    /**
     * The function body (if generated)
     */
    CodeBlock func_body;

    /**
     * The identifiers in the function body (if developer-suppied)
     */
    set<string> body_identifiers;

    /**
     * The name of the AgentFuncSymbol which implements hooks to the function.
     *
     * @param to_name Function being hooked to
     *
     * @return A string.
     */
    static string hook_implement_name(string to_name)
    {
        return "hook_" + to_name;
    }
};

