/**
* @file    ParameterSymbol.h
* Declarations for the ParameterSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include <numeric>
#include "Symbol.h"
#include "NumericSymbol.h"
#include "Constant.h"
#include "DimensionSymbol.h"

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
        , source(missing_parameter)
        , cumrate(false)
        , cumrate_dims(0)
        , datatype(datatype->stable_rp())
        , pp_datatype(nullptr)
        , lookup_fn(nullptr)
        , haz1rate(false)
        , is_hidden(false)
        , datatype2(nullptr)
        , pp_datatype2(nullptr)
        , pp_parameter_id(0)
    {
    }

    void post_parse(int pass);

    /**
     * Alternate name
     * 
     * The name and alternate name become C++ identifiers which can be used to access the parameter
     * in C++ code using different cv-qualifiers (specifically const and non-const).
     *
     * @return A string.
     */
    string alternate_name() const
    {
        return "om_param_" + name;
    }

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

    /**
     * Validates the initializer.
     * 
     * This function validates the number of elements in the initializer and checks that each
     * element meets the requirements of the parameter data type. It logs all errors.
     */
    void validate_initializer();

    /**
     * Initializer list for storage.
     * 
     * Builds a list of strings containing initializer values suitable for the data store.
     *
     * @return a list of strings;
     */
    list<string> initializer_for_storage();

    /**
     * Definition of the parameter in .dat format
     * 
     * Content is set to the default value for the type (normally zero)
     *
     * @return A CodeBlock.
     */
    CodeBlock dat_definition() const;

    /**
     * The parameter value notes for each language.
     */
    vector<string> pp_value_notes;

    void populate_metadata(openm::MetaModelHolder & metaRows);

    /**
     * C++ code fragment with parameter name followed by dimension list.
     *
     * @param use_alternate (Optional) Use the alternate name instead of the true name.
     * @param use_zero   (Optional) Generate [0] instead of [n] where n is the dimension size.
     *
     * @return A string.
     */
    string cxx_dimensions(bool use_zero = false) const;

    /**
    * C++ code to read parameter from data store.
    *
    * @return A block of code.
    */
    CodeBlock cxx_read_parameter();

    /**
    * return C++ type name of parameter value(s) to read from data store.
    */
    const string cxx_type_of_parameter(void) const;

    /**
    * return C++ code fragment with name, type and size of parameter to read from data store.
    */
    const string cxx_parameter_name_type_size(void) const;

    /**
     * C++ code to initialize cumrate for parameter.
     *
     * @return A string.
     */
    string cxx_initialize_cumrate();

    /**
     * C++ code to perform haz1rate transformation on parameter.
     *
     * @return A CodeBlock.
     */
    CodeBlock cxx_transform_haz1rate();

    /**
     * assert C++ code fragment to verify that storage type has the same size as the readParameter
     * type.
     *
     * @return A string.
     */
    string cxx_type_check();

    /**
     * Enumeration indicating source of parameter value.
     */
    enum parameter_source {
        ///< Parameter value is missing.
        missing_parameter,
        ///< Parameter value fixed and stored in executable.
        fixed_parameter,
        ///< Parameter value from an external source, e.g. data store
        scenario_parameter,
        ///< Parameter value computed from other parameters.
        derived_parameter
    };

    /**
     * Source for the parameter value
     */
    parameter_source source;

    /**
     * True if parameter declared as cumrate
     */
    bool cumrate;

    /**
     * Number of trailing non-conditioning dimensions for cumrate parameters
     */
    int cumrate_dims;

    /**
     * Name of the cumrate object in generated code
     */
    string cumrate_name() const
    {
        return "om_cumrate_" + name;
    }

    /**
     * The data type of the parameter contents (parse phase reference to pointer)
     */
    Symbol*& datatype;

    /**
     * The data type of the parameter contents (post-parse phase pointer)
     */
    TypeSymbol *pp_datatype;

    /**
     * The dimensions in the table (dimension symbols)
     */
    list<DimensionSymbol *> dimension_list;

    /**
     * List of dimensions (enumerations) (parse phase references to pointers)
     */
    list<Symbol **> enumeration_list;

    /**
     * Gets the rank of the parameter.
     *
     * @return A long.
     */
    size_t rank() const
    {
        return pp_enumeration_list.size();
    }

    /**
     * Dimensions (enumerations)
     */
    list<EnumerationSymbol *> pp_enumeration_list;

    /**
     * Shape
     */
    list<size_t> pp_shape;

    /**
     * Gets the size (number of cells)
     *
     * @return A size_t.
     */
    size_t size() const
    {
        return accumulate(pp_shape.begin(), pp_shape.end(), 1, multiplies<size_t>());
    }

    /**
     * Number of leading conditioning dimensions for cumrate parameters
     *
     * @return An int.
     */
    size_t conditioning_dims() const
    {
        return rank() - cumrate_dims;
    }

    /**
     * List of conditioning dimensions (enumerations) (leading dimensions)
     */
    list<EnumerationSymbol *> pp_conditioning_enumeration_list;

    /**
     * Shape of conditioning portion (leading dimensions)
     */
    list<size_t> pp_conditioning_shape;

    /**
     * Total size of conditioning portion (leading dimensions)
     */
    size_t conditioning_size()
    {
        return accumulate(pp_conditioning_shape.begin(), pp_conditioning_shape.end(), 1, multiplies<size_t>());
    }

    /**
     * Number of trailing distribution dimensions for cumrate parameters
     *
     * @return An int.
     */
    int distribution_dims() const
    {
        return cumrate_dims;
    }

    /**
     * List of distribution dimensions (enumerations) (trailing dimensions)
     */
    list<EnumerationSymbol *> pp_distribution_enumeration_list;

    /**
     * Shape of distribution portion (trailing dimensions)
     */
    list<size_t> pp_distribution_shape;

    /**
     * Total size of distribution portion (trailing dimensions)
     */
    size_t distribution_size()
    {
        return accumulate(pp_distribution_shape.begin(), pp_distribution_shape.end(), 1, multiplies<size_t>());
    }

    /**
     * List of dimensions (enumerations) - redeclaration (parse phase references to pointers)
     */
    list<Symbol **> enumeration_list2;

    /**
     * List of dimensions (enumerations) - redeclaration (post-parse phase pointers)
     */
    list<EnumerationSymbol *> pp_enumeration_list2;

    /**
     * List of initializers.
     */
    list<Constant *> initializer_list;

    /**
     * The lookup function for cumrate parameters
     */
    GlobalFuncSymbol * lookup_fn;

    /**
     * True if parameter declared as haz1rate
     */
    bool haz1rate;

    /**
     * True if the parameter is hidden
     */
    bool is_hidden;

    /**
     * The data type of the parameter contents - redeclaration
     */
    Symbol ** datatype2;

    /**
     * The data type of the parameter contents - redeclaration
     */
    TypeSymbol *pp_datatype2;

    /**
     * Numeric identifier of the parameter. Used for communicating with the metadata API.
     */
    int pp_parameter_id;
};

