/**
* @file    DimensionSymbol.h
* Declarations for the DimensionSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include <string>
#include "Symbol.h"

class AttributeSymbol;
class EunumerationSymbol;

using namespace std;

/**
* DimensionSymbol.
* 
* Corresponds to a dimension of a table, entity set, or parameter.
*/
class DimensionSymbol : public Symbol
{
private:
    typedef Symbol super;

public:
    bool is_base_symbol() const { return false; }

    DimensionSymbol(Symbol *symbol_with_dimensions, string *pname, int index, bool after_analysis_dim, Symbol *attribute, Symbol *enumeration, bool has_margin, omc::location decl_loc = omc::location())
        : Symbol(symbol_name(symbol_with_dimensions, index, after_analysis_dim), decl_loc)
        , short_name(pname ? *pname : "Dim" + to_string(index))
        , short_name_default("Dim" + to_string(index))
        , short_name_explicit(pname ? *pname : "")
        , index(index)
        , after_analysis_dim(after_analysis_dim)
        , has_margin(has_margin)
        , attribute(attribute ? attribute->stable_pp() : nullptr)
        , pp_attribute(nullptr)
        , enumeration(enumeration ? enumeration->stable_pp() : nullptr)
        , pp_enumeration(nullptr)
    {
        // exactly one of either attribute or enumeration is specified
        assert((attribute && !enumeration) || (enumeration && !attribute)); // grammar guarantee

        // override to use dimension naming for LABEL and NOTE
        modgen_unique_name = modgen_symbol_name(symbol_with_dimensions, index, after_analysis_dim);
    }

    /**
     * Construct symbol name for the dimension symbol. Example: BasicDemography::Dim0.
     *
     * @param symbol_with_dimensions The table or entity set containing the this dimension.
     * @param index          0-based index of the enumeration dimension among all enumeration dimensions in the table.
     *
     * @return A string.
     */
    static string symbol_name(const Symbol* symbol_with_dimensions, int index, bool after_expression_dim);

    /**
     * Construct symbol name for the dimension symbol following modgen naming scheme of LABEL and NOTE
     */
    static string modgen_symbol_name(const Symbol* symbol_with_dimensions, int index, bool after_expression_dim);

    /**
     * Make dimension name suitable for database column name: it must be unique column name consist of 255 alphanumeric characters.
     *
     * @param i_ownerName    name of parameter or table.
     * @param i_dimLst       list of dimensions.
     * @param[in.out] io_dim the dimension which name must be validated and updated if required.
     */
    static void to_column_name(const string & i_ownerName, const list<DimensionSymbol *> i_dimLst, DimensionSymbol * io_dim);

    void post_parse(int pass);

    /**
    * Heuristically-generated short name for the dimension
    */
    string heuristic_short_name(void) const;

    /**
    * Short name of the dimension
    */
    string short_name;

    /**
    * Short name of the dimension (generated default name)
    */
    string short_name_default;

    /**
    * Short name of the dimension (provided in model source)
    */
    string short_name_explicit;

    /**
     * Zero-based index of the dimension in the table.
     * 
     * Note that index in DimensionSymbol does not count the special analysis dimension.
     */
    int index;

    /**
     * In a table, true if this enumeration dimension comes after this special analysis dimension
     * 
     * In an entity set, is always false.
     */
    bool after_analysis_dim;

    /**
     * true if this dimension has a margin (using '+')
     */
    bool has_margin;

    /**
     * The attribute for the dimension (reference to pointer)
     * 
     * Stable to symbol morphing during parse phase. Is nullptr for derived tables.
     */
    Symbol** attribute;

    /**
     * The attribute for the dimension
     * 
     * Only valid after post-parse.
     */
    AttributeSymbol* pp_attribute;

    /**
     * The enumeration for the dimension (reference to pointer)
     * 
     * Stable to symbol morphing during parse phase. Is nullptr except for derived tables, for which
     * the enumeration is specified explicitly in model code.
     */
    Symbol** enumeration;

    /**
     * The enumeration for the dimension
     * 
     * Only valid after post-parse.
     */
    EnumerationSymbol* pp_enumeration;
};

