/**
* @file    RangeSymbol.h
* Declarations for the RangeSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include <list>
#include "EnumerationSymbol.h"
#include "EnumeratorSymbol.h"
#include "Constant.h"

using namespace std;
using namespace openm;


/**
* RangeSymbol.
*/
class RangeSymbol : public EnumerationSymbol
{
private:
    typedef EnumerationSymbol super;

public:
    bool is_base_symbol() const override { return false; }

    /**
     * Constructor.
     *
     * @param [in,out] sym The symbol to be morphed.
     * @param lower_bound  The lower bound.
     * @param upper_bound  The upper bound.
     * @param decl_loc     (Optional) the declaration location.
     */
    RangeSymbol(Symbol *sym, int lower_bound, int upper_bound, omc::location decl_loc = omc::location())
        : EnumerationSymbol(sym, token::TK_int, kind_of_type::range_type, decl_loc)
        , lower_bound(lower_bound)
        , upper_bound(upper_bound)
    {
    }

    const string default_initial_value() const override;

    void post_parse(int pass) override;

    CodeBlock cxx_declaration_global() override;

    CodeBlock cxx_definition_global() override;

    void populate_metadata(openm::MetaModelHolder & metaRows) override;

    bool is_valid_constant(const Constant &k) const override;

    /**
    * Create new range Constant from 'i_value' enum literal.
    *
    * @return pointer to a new Constant or nullptr on error.
    */
    Constant * make_constant(const string & i_value) const override;

    string format_for_storage(const Constant &k) const override;

    /**
     * Gets the size of the enumeration (specialization for RangeSymbol)
     *
     * @return An int.
     */
    int pp_size() const override
    {
        return upper_bound - lower_bound + 1;
    }

    /**
     * The lower bound of the range.
     */
    int lower_bound;

    /**
     * The upper bound of the range.
     */
    int upper_bound;

private:
    /**
    * Range value literal validator.
    *
    * @return true if 'i_value' string represent valid literal of the range type
    */
    bool is_valid_literal(const char * i_value) const;
};

