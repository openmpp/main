/**
* @file    EntityFuncSymbol.h
* Declarations for the EntityFuncSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include <string>
#include <set>
#include <algorithm>
#include "omc_location.hh"
#include "EntityMemberSymbol.h"
#include "CodeBlock.h"

class EntityEventSymbol;

using namespace std;

/**
* Symbol for a function member of an entity.
*
* The symbol table contains an EntityFuncSymbol for each entity function,
* apart from event implement functions, which are represented by EntityEventSymbol's.
*/
class EntityFuncSymbol : public EntityMemberSymbol
{
private:
    typedef EntityMemberSymbol super;

public:
    bool is_base_symbol() const { return false; }


    /**
    * Morphing constructor.
    *
    * @param [in,out]  sym The symbol to be morphed.
    */
    EntityFuncSymbol(Symbol *sym, const Symbol * ent, string return_decl, string arg_list_decl, bool suppress_defn = false, omc::location decl_loc = omc::location())
        : EntityMemberSymbol(sym, ent, decl_loc)
        , arg_list_decl(arg_list_decl)
        , return_decl(return_decl)
        , suppress_defn(suppress_defn)
        , suppress_code_if_empty(false)
        , has_line_directive(false)
        , associated_event(nullptr)
    {
    }

    /**
     * Constructor by name.
     *
     * @param name          The name.
     * @param ent           The entity.
     * @param return_decl   The return declaration.
     * @param arg_list_decl The argument list declaration.
     * @param suppress_defn (Optional) Suppress definition.
     * @param decl_loc      (Optional) the declaration location.
     */
    EntityFuncSymbol(const string name, const Symbol * ent, string return_decl, string arg_list_decl, bool suppress_defn = false, omc::location decl_loc = omc::location())
        : EntityMemberSymbol(name, ent, decl_loc)
        , arg_list_decl(arg_list_decl)
        , return_decl(return_decl)
        , is_developer_supplied(false)
        , suppress_defn(suppress_defn)
        , suppress_code_if_empty(false)
        , has_line_directive(false)
        , associated_event(nullptr)
    {
    }

    /**
     * Constructor by name, with defaults.
     *
     * @param name  The name.
     * @param ent   The entity.
     */
    EntityFuncSymbol(const string name, const Symbol * ent)
        : EntityFuncSymbol(name, ent, "void", "", false)
    {
    }

    void post_parse(int pass);

    CodeBlock cxx_declaration_entity();

    CodeBlock cxx_definition_entity();

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
     * Determines if the function is a side-effects function.
     *
     * @return true if a side-effect function, false otherwise.
     */
    bool is_side_effects() const
    {
        return (0 == name.find("om_side_effects_"));
    }

    /**
     * Determines if the function is a notify function.
     *
     * @return true if a notify function, false otherwise.
     */
    bool is_notify() const
    {
        return (0 == name.find("om_notify_"));
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
     * True if declared in model code.
     */
    bool is_developer_supplied;

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
     * For event time and implement functions, points to the event
     */
    EntityEventSymbol* associated_event;

    /**
     * The function body (if generated)
     */
    CodeBlock func_body;

    /**
     * The definition location (if supplied)
     * 
     * Set using location information of syntactic elements during parsing.
     */
    omc::location defn_loc;

    /**
     * The identifiers in the function body (if developer-supplied)
     */
    set<string> body_identifiers;

    /**
     * The pointers A->B in the function body (if developer-supplied)
     */
    set<pair<string,string>> body_pointers;

    /**
     * The RNG streams used in the function body (if developer-supplied)
     */
    set<int> rng_streams;

    /**
     * The RNG Normal streams used in the function body (if developer-supplied)
     */
    set<int> rng_normal_streams;

    /**
     * The name of the EntityFuncSymbol which implements hooks to the function.
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

