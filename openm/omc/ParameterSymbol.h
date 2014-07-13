/**
* @file    ParameterSymbol.h
* Declarations for the ParameterSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "Symbol.h"
#include "NumericSymbol.h"

/**
* ParameterSymbol.
*/
class ParameterSymbol : public Symbol
{
private:
    typedef Symbol super;

public:
    bool is_base_symbol() const { return false; }

    ParameterSymbol(Symbol *sym, Symbol *datatype, yy::location decl_loc = yy::location())
        : Symbol(sym, decl_loc)
        , datatype(datatype->stable_rp())
        , pp_datatype(nullptr)
        , source(external_parameter)
    {
    }

    void post_parse(int pass);

    CodeBlock cxx_declaration_global();

    CodeBlock cxx_definition_global();

    /**
     * C++ initializer list for parameter
     * 
     * Return value looks like "= { ... }" if initial value supplied in model source. otherwise is
     * an empty CodeBlock.
     *
     * @return A CodeBlock.
     */
    CodeBlock cxx_initializer();

    void populate_metadata(openm::MetaModelHolder & metaRows);


    /**
     * C++ code fragment with parameter name followed by dimension list.
     *
     * @param use_zero Generate [0] instead of [n] where n is the dimension size.
     *
     * @return A string.
     */

    string cxx_name_and_dimensions(bool use_zero = false);


    /**
     * Gets the rank of the parameter.
     *
     * @return A long.
     */

    unsigned long rank();

    /**
     * Gets the total number of cells in the parameter
     *
     * @return An unsigned long.
     */

    unsigned long cells();

    /**
     * readParameter C++ code fragment
     *
     * @return A string.
     */

    string cxx_read_parameter();


    /**
     * assert C++ code fragment to verify that storage type has the same size as the readParameter type.
     *
     * @return A string.
     */

    string cxx_assert_sanity();

    /**
     * Enumeration indicating source of parameter value.
     */
    enum parameter_source {
        ///< Parameter value invariant and stored internally.
        internal_parameter,
        ///< Parameter value from an external source.
        external_parameter,
        ///< Parameter value computed from other parameters.
        derived_parameter
    };

    /**
     * Source for the parameter value
     */
    parameter_source source;

    /**
    *  The data type of the parameter contents (parse phase reference to pointer)
    */

    Symbol*& datatype;


    /**
     * The data type of the parameter contents (post-parse phase pointer)
     */

    TypeSymbol *pp_datatype;


    /**
     * List of dimensions (parse phase references to pointers)
     */

    list<Symbol **> dimension_list;

    /**
     * List of dimensions (post-parse phase pointers)
     */

    list<EnumerationSymbol *> pp_dimension_list;


    /**
     * List of dimensions - redeclaration (parse phase references to pointers)
     */

    list<Symbol **> dimension_list2;

    /**
     * List of dimensions - redeclaration (post-parse phase pointers)
     */

    list<EnumerationSymbol *> pp_dimension_list2;


    /**
     * List of initializers.
     */

    list<string *> initializer_list;


    /**
     * Numeric identifier of the parameter. Used for communicating with the metadata API.
     */

    int pp_parameter_id;
};

