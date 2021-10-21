/**
* @file    ModelTypeSymbol.h
* Declarations for the ModelTypeSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include <cassert>
#include "Symbol.h"

/**
* Symbol for the model type information
*
* A single instance of ModelTypeSymbol exists in the symbol table, with name 'model_type'.
* A default ModelTypeSymbol with value 'case_based' is created at initialization.
* This default ModelTypeSymbol will be morphed if a 'model_type' statement
* is encountered in the model source code.
*/
class ModelTypeSymbol : public Symbol
{
private:
    typedef Symbol super;

public:
    bool is_base_symbol() const { return false; }

    /**
     * Constructor.
     *
     * @param value        The token for the model type, e.g. token::KW_case_based.
     * @param just_in_time (Optional) the just in time.
     * @param decl_loc     (Optional) the declaration location.
     */
    ModelTypeSymbol(token_type value, bool just_in_time = false, omc::location decl_loc = omc::location())
        : Symbol(token_to_string(token::TK_model_type), decl_loc)
        , value(value)
        , just_in_time(just_in_time)
    {
        // compiler guarantee
        assert(value == token::TK_case_based || value == token::TK_time_based);
    }

    CodeBlock cxx_definition_global();

    void populate_metadata(openm::MetaModelHolder & metaRows);

    /**
     * Gets the one and only ModelTypeSymbol.
     *
     * @return null if it fails, else a ModelTypeSymbol*.
     */
    static ModelTypeSymbol *find();

    bool is_case_based() const
    {
        return value == token::TK_case_based;
    }

    /**
     * The model type
     * 
     *  This can be either TK_case_based or TK_time_based.
     */
    token_type value;

    /**
     * Value of just-in-time option
     * 
     * True if active, false if inactive.
     */
    bool just_in_time;
private:

    /** model type as db-integer */
    enum class ModelType
    {
        /** case based model */
        caseBased,

        /** time based model */
        timeBased
    };
};

