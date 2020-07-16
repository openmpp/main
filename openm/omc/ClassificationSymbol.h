/**
* @file    ClassificationSymbol.h
* Declarations for the ClassificationSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include <list>
#include "EnumerationWithEnumeratorsSymbol.h"
#include "Constant.h"

using namespace std;
using namespace openm;


/**
* ClassificationSymbol.
*/
class ClassificationSymbol : public EnumerationWithEnumeratorsSymbol
{
private:
    typedef EnumerationWithEnumeratorsSymbol super;

public:
    bool is_base_symbol() const { return false; }

    /**
     * Constructor.
     *
     * @param [in,out] sym The symbol to be morphed.
     */

    ClassificationSymbol(Symbol *sym, yy::location decl_loc = yy::location())
        : EnumerationWithEnumeratorsSymbol(sym, token::TK_int, kind_of_type::classification_type, decl_loc)
        , pp_generate_IntIs(false)
        , pp_generate_IntTo(false)
        , pp_generate_IntFrom(false)
    {
    }

    void post_parse(int pass);

    CodeBlock cxx_declaration_global();

    CodeBlock cxx_definition_global();

    bool is_valid_constant(const Constant &k) const;

    /**
    * Create new classification Constant from 'i_value' enum literal.
    *
    * @return pointer to a new Constant or nullptr on error.
    */
    Constant * make_constant(const string & i_value) const override;

    /**
     * The name of the C++ enum used for classification values.
     *
     * @return A string.
     */
    string enum_name() const
    {
        return "om_enum_" + name;
    }

    string format_for_storage(const Constant &k) const;

    /**
     * Query if the classification is a candidate for integer transformations.
     * 
     * To be a candidate, at least one of the enumerators must have trailing digits.
     *
     * @return true if trailing digits, false if not.
     */
    bool candidate_for_integer_transforms();

    /**
     * true to generate the IntIs_ integer-classification transformation global function
     */
    bool pp_generate_IntIs;

    /**
     * true to generate the IntTo_ integer-classification transformation global function
     */
    bool pp_generate_IntTo;

    /**
     * true to generate the IntFrom_ integer-classification transformation global function
     */
    bool pp_generate_IntFrom;
};

