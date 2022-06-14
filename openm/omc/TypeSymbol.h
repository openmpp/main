/**
* @file    TypeSymbol.h
* Declarations for the TypeSymbol class.
*/
// Copyright (c) 2013-2022 OpenM++ Contributors
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include "libopenm/omError.h"
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
    bool is_base_symbol() const override { return false; }

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
    explicit TypeSymbol(Symbol *sym, omc::location decl_loc = omc::location())
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
    bool is_unknown() const;

    /**
     * Determines derived type.
     *
     * @return true if bool, else false.
     */
    bool is_bool() const;

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
    bool is_classification() const;

    /**
     * Determines derived type.
     *
     * @return true if range, else false.
     */
    bool is_range() const;

    /**
     * Determines derived type.
     *
     * @return true if partition, else false.
     */
    bool is_partition() const;

    /**
     * Determines derived type.
     *
     * @return true if time type, else false
     */
    bool is_time() const;

    /**
     * Determines derived type.
     *
     * @return true if numeric or bool, else false (classification, range, or partition).
     */
    bool is_numeric_or_bool() const;

    /**
     * Determines derived type.
     *
     * @return true if is enumeration (includes bool), false otherwise.
     */
    bool is_enumeration() const;

    /**
     * Determines derived type.
     *
     * @return true if the type is float or double (or equivalent).
     */
    bool is_floating() const;

    /**
     * Determines derived type.
     *
     * @return true if the type is numeric.
     */
    bool is_numeric() const;

    /**
     * Determines derived type.
     *
     * @return true if the type is numeric and one of: "real", "float", "double" or "ldouble".
     */
    bool is_numeric_floating() const;

    /**
     * Determines derived type.
     *
     * @return true if the type is numeric but neither floating nor time.
     */
    bool is_numeric_integer() const;

    /**
     * Determines if the type is a wrapped type, e.g. time, classificaiton, etc.
     *
     * @return true if the type is wrapped, false otherwise
     */
    bool is_wrapped() const
    {
        return is_time()
            || is_classification()
            || is_range()
            || is_partition()
            ;
    }

    /**
    * Create new Constant from 'i_value' literal.
    *
    * @return new Constant or NULL on error.
    */
    virtual Constant * make_constant(const string & i_value) const {
        return nullptr;
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

