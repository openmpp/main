/**
* @file    NumericSymbol.h
* Declarations for the NumericSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include "TypeSymbol.h"
#include "Constant.h"

/**
* NumericSymbol.
*
* The symbol table is initialized with one NumericSymbol
* corresponding to each kind of type declaration statement in openM++, e.g. time_type, real_type.
* The symbol type (first argument of constructor) is the token associated with the corresponding type name,
* e.g. TK_Time for the time_type statement, TK_real to the real_type statement.
* These default symbols are morphed if the corresponding statement is found
* in the model source code.
*/
class NumericSymbol : public TypeSymbol
{
private:
    typedef TypeSymbol super;

public:
    bool is_base_symbol() const { return false; }

    /**
     * Constructor.
     *
     * @param type          The token for the keyword of the type, e.g. token::KW_Time.
     * @param initial_value The initial value.
     */
    NumericSymbol(token_type type, const string & initial_value)
        : TypeSymbol(token_to_string(type))
        , type(type)
        , initial_value(initial_value)
    {
        Set_keywords();
    }

    /**
     * Constructor.
     *
     * @param type          The token for the keyword of the type, e.g. token::KW_Time.
     * @param kw1           Keyword #1 for the associated typedef, e.g. token::KW_double.
     * @param initial_value The initial value.
     */
    NumericSymbol(token_type type, token_type kw1, const string & initial_value)
        : TypeSymbol(token_to_string(type))
        , type(type)
        , initial_value(initial_value)
    {
        Set_keywords(kw1);
    }

    /**
     * Constructor.
     *
     * @param type          The token for the keyword of the type, e.g. token::KW_uint.
     * @param kw1           Keyword #1 for the associated typedef, e.g. token::KW_unsigned.
     * @param kw2           Keyword #2 for the associated typedef, e.g. token::KW_int.
     * @param initial_value The initial value.
     */
    NumericSymbol(token_type type, token_type kw1, token_type kw2, const string & initial_value)
        : TypeSymbol(token_to_string(type))
        , type(type)
        , initial_value(initial_value)
    {
        Set_keywords(kw1, kw2);
    }

    /**
     * Constructor.
     *
     * @param type          The token for the keyword of the type, e.g. token::KW_ullong.
     * @param kw1           Keyword #1 for the associated typedef, e.g. token::KW_unsigned.
     * @param kw2           Keyword #2 for the associated typedef, e.g. token::KW_long.
     * @param kw3           Keyword #3 for the associated typedef, e.g. token::KW_long.
     * @param initial_value The initial value.
     */
    NumericSymbol(token_type type, token_type kw1, token_type kw2, token_type kw3, const string & initial_value)
        : TypeSymbol(token_to_string(type))
        , type(type)
        , initial_value(initial_value)
    {
        Set_keywords(kw1, kw2, kw3);
    }

    void Set_keywords() {
        keywords.clear();
    }

    void Set_keywords( token_type kw1 ) {
        Set_keywords();
        keywords.push_back(kw1);
    }

    void Set_keywords(token_type kw1, token_type kw2) {
        Set_keywords(kw1);
        keywords.push_back(kw2);
    }

    void Set_keywords(token_type kw1, token_type kw2, token_type kw3) {
        Set_keywords(kw1, kw2);
        keywords.push_back(kw3);
    }

    const string default_initial_value() const {
        return initial_value;
    }

    void post_parse(int pass);

    CodeBlock cxx_declaration_global();

    void populate_metadata(openm::MetaModelHolder & metaRows);

    /**
     * The type, e.g. TK_uint.
     */
    token_type type;

    /**
     * The initial value for a quantity of this type.
     */
    const string initial_value;

    /**
     * A list of keywords used to construct the typedef statement, e.g. TK_unsigned, TK_int.
     * 
     * The Time type has specialized logic to construct the typedef, so keywords is left empty.
     */
    list<token_type> keywords;

    /**
     * Gets the NumericSymbol for a given type.
     *
     * @param type The type.
     *
     * @return null if it fails, else the NumericSymbol.
     */
    static NumericSymbol *find(token_type type);

    /**
     * Query if 'k' is valid constant for this type
     *
     * @param k The constant to verify.
     *
     * @return true if valid constant, false if not.
     */
    bool is_valid_constant(const Constant &k) const;
    
    /**
    * Create new numeric Constant of float or integer type from 'i_value' literal.
    *
    * @return pointer to a new Constant or nullptr on error.
    */
    Constant * make_constant(const string & i_value) const override;

    /**
     * Formats the constant for the data store.
     *
     * @param k The constant to be formatted.
     *
     * @return The formatted for storage.
     */
    string format_for_storage(const Constant &k) const override;
};
