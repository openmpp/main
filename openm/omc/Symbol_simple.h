/**
* @file    Symbol_simple.h
* Declares simple derived classes of the Symbol class.
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "Symbol.h"

/**
* Symbol for a supported human language in the model.
*
* Each of the comma-separated language codes in the 'languages' statement
* becomes a LanguageSymbol in the symbol table.  The first language in the
* languages statement becomes the default language for the model.
*/
class LanguageSymbol : public Symbol
{
private:
    typedef Symbol super;

public:

    /**
    * Constructor.
    *
    * @param [in,out]  sym Symbol to be morphed
    * @param   deflang     true if this is the model's default language.
    */
    LanguageSymbol(Symbol *sym, bool deflang)
        : Symbol(sym)
        , default_language(deflang)
    {
    }

    /**
     * Flag indicating if this is the default language.
     */

    bool default_language;
};

/**
* Symbol for the model itself.
* 
* A single instance of ModelSymbol exists in the symbol table
* after parsing is complete.  It is constructed from the file name
* of the module which contains the developer-supplied definition
* of the C++ function Simulation().
* 
*/
class ModelSymbol : public Symbol
{
private:
    typedef Symbol super;

public:
    ModelSymbol(string nm)
        : Symbol(nm)
    {
    }
};

/**
* Symbol for a source code module.
*
* The name of a ModuleSymbol is the name of the model source code file,
* including the .mpp or .ompp file extension.
*/
class ModuleSymbol : public Symbol
{
private:
    typedef Symbol super;

public:

    /**
    * Constructor.
    *
    * Unlike most symbols, a ModuleSymbol is created outside of
    * the parser and scanner.
    *
    * @param   nm  The name of the module.
    *
    */
    ModuleSymbol(string nm)
        : Symbol(nm)
    {
    }
};


// remove GNU macro defines for major and minor
#ifdef major
#undef major
#endif
#ifdef minor
#undef minor
#endif

/**
* Symbol for the model version information
*
* A single instance of VersionSymbol exists in the symbol table, with name 'version'.
* A default VersionSymbol with value 1,0,0,0 is created at initialization.
* This default VersionSymbol will be morphed if a 'version' statement
* is encountered in the model source code.
*/
class VersionSymbol : public Symbol
{
private:
    typedef Symbol super;

public:

    /**
    * Constructor.
    *
    * @param[in]   i_major           Major model version.
    * @param[in]   i_minor           Minor model version.
    * @param[in]   i_sub_minor       Sub-minor model version.
    * @param[in]   i_sub_sub_minor   Sub-sub minor version.
    */
    VersionSymbol(int i_major, int i_minor, int i_sub_minor, int i_sub_sub_minor)
        : Symbol(token_to_string(token::TK_version))
        , major(i_major)
        , minor(i_minor)
        , sub_minor(i_sub_minor)
        , sub_sub_minor(i_sub_sub_minor)
    {
    }

    /** major model version (position 1) */
    int major;

    /** minor model version (position 2) */
    int minor;

    /** sub-minor model version (position 3) */
    int sub_minor;

    /** sub-sub-minor model version (position 4) */
    int sub_sub_minor;
};

/**
* TypeDeclSymbol.
*
* The symbol table is initialized with one TypeDeclSymbol
* corresponding to each kind of type declaration statement in openM++, e.g. time_type, real_type.
* The symbol type (first argument of constructor) is the token associated with the corresponding type name,
* e.g. TK_Time for the time_type statement, TK_index to the index_type statement.
* The one exception is the model_type statement which has an associated TypeDeclSymbol with 'type' TK_model_type.
* These default symbols are morphed if the corresponding statement is found
* in the model source code.
*/
class TypeDeclSymbol : public Symbol
{
private:
    typedef Symbol super;

public:

    /**
    * Constructor.
    *
    * @param   type    The token for the keyword of the type, e.g. token::KW_Time
    * @param   value   The token for the associated type, e.g. token::KW_double
    */
    TypeDeclSymbol(token_type type, token_type value)
        : Symbol(token_to_string(type))
        , value(value)
    {
    }

    /** The C++ type of the given openM++ type */
    token_type value;
};

