/**
* @file    TypeSymbol.h
* Declarations for the TypeSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include "Symbol.h"
#include "Constant.h"

/**
* TypeSymbol.
*
*/
class TypeSymbol : public Symbol
{
private:
    typedef Symbol super;

public:
    bool is_base_symbol() const { return false; }

    /**
     * Constructor.
     *
     * @param unm The unique name of the symbol, e.g. "int".
     */
    explicit TypeSymbol(const string unm)
        : Symbol(unm)
    {
        type_id = next_type_id;
        next_type_id++;
    }

    /**
    * Constructor.
    *
    * @param [in,out]  sym The symbol to be morphed.
    */
    explicit TypeSymbol(Symbol *sym, yy::location decl_loc = yy::location())
        : Symbol(sym, decl_loc)
    {
        type_id = next_type_id;
        next_type_id++;
    }

    /**
     * Gets the default initial value for a quantity of this type.
     *
     * @return The default initial value as a string.
     */
    virtual const string default_initial_value() const = 0;

    /**
     * Determines derived type.
     *
     * @return true if unknown, else false.
     */
    bool is_unknown();

    /**
     * Determines derived type.
     *
     * @return true if bool, else false.
     */
    bool is_bool();

    /**
     * Determines derived type.
     *
     * @return true if string, else false.
     */
    bool is_string() const;

    /**
     * Determines derived type.
     *
     * @return true if classification, else false.
     */
    bool is_classification();

    /**
     * Determines derived type.
     *
     * @return true if range, else false.
     */
    bool is_range();

    /**
     * Determines derived type.
     *
     * @return true if partition, else false.
     */
    bool is_partition();

    /**
     * Determines derived type.
     *
     * @return true if time type, else false
     */
    bool is_time();

    /**
     * Determines derived type.
     *
     * @return true if numeric or bool, else false (classification, range, or partition).
     */
    bool is_numeric_or_bool();

    /**
     * Determines derived type.
     *
     * @return true if is enumeration (includes bool), false otherwise.
     */
    bool is_enumeration();

    /**
     * Determines derived type.
     *
     * @return true if the type is float or double (or equivalent).
     */
    bool is_floating() const;

    /**
     * Determines if the type is a wrapped type, e.g. time, cassificaiton, etc.
     *
     * @return true if the type is wrapped, false otherwise
     */
    bool is_wrapped()
    {
        return is_time()
            || is_classification()
            || is_range()
            || is_partition()
            ;
    }

    /**
     * Determines appropriate type to use for sums or differences of this type.
     *
     * @return type symbol for integer or real.
     */
    TypeSymbol * summing_type();

    /**
     * Gets the exposed type.
     * 
     * The return value is supplied as an argument to attribute templates where it is used to define
     * type conversion overloads.
     *
     * @return The exposed type, e.g. "int".  If none, returns "void"..
     */
    string exposed_type();

    /**
     * Query if 'k' is valid constant for this type
     *
     * @param k The constant to verify.
     *
     * @return true if valid constant, false if not.
     */
    virtual bool is_valid_constant(const Constant &k) const
    {
        // Polymorphic overrides do the validation.
        return false;
    }

    /**
     * Formats the constant for the data store.
     *
     * @param k The constant to be formatted.
     *
     * @return The formatted for storage.
     */
    virtual string format_for_storage(const Constant &k) const
    {
        // Note presence of polymorphic overrides.
        return k.value();
    }

    /**
     * Identifier for the type.
     */
    int type_id;

    /**
     * type_id for the next TypeSymbol
     * 
     * A shared counter used to assign sequentially increasing type_id to each TypeSymbol.
     */
    static int next_type_id;
};


namespace openm {

    /**
    * Indicates the kind of the type.
    *
    * Used to communicate with meta-data API through the dicId member
    * of things like TypeDicRow.
    */

    enum kind_of_type {
        simple_type = 0,
        logical_type = 1,
        classification_type = 2,
        range_type = 3,
        partition_type = 4,
        link_type = 5,
    };
}

