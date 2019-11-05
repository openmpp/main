/**
* @file    ImportSymbol.h
* Declarations for the ImportSymbol class.
*/
// Copyright (c) 2013-2019 OpenM++ Contributors
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include "ParameterSymbol.h"

/**
* ImportSymbol
*
* Holds information in an import statement
*
*/
class ImportSymbol : public Symbol
{
private:
    typedef Symbol super;

public:
    bool is_base_symbol() const { return false; }

    /**
    * Constructor.
    * 
    * A unique name is generated using a counter.
    */
    ImportSymbol(Symbol* target_param, string upstream_model, string upstream_table, bool sample_dimension_opt, yy::location decl_loc = yy::location())
        : Symbol("om_import_" + to_string(++counter), decl_loc)
        , target_param(target_param->stable_rp())
        , upstream_model(upstream_model)
        , upstream_table(upstream_table)
        , sample_dimension_opt(sample_dimension_opt)
        , pp_target_param(nullptr)
    {
    }

    /**
     * The target parameter of the import
     */
    Symbol*& target_param;

    /**
     * The upstream model name for the import
     */
    string upstream_model;

    /**
     * The upstream table name for the import
     */
    string upstream_table;

    /**
     * The sample dimension option for the import
     */
    bool sample_dimension_opt;

    /**
     * The target parameter for the import
     */
    ParameterSymbol* pp_target_param;

    void post_parse(int pass);

    void populate_metadata(openm::MetaModelHolder& metaRows);

    static int counter;
};
