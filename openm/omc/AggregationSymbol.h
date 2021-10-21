/**
* @file    AggregationSymbol.h
* Declarations for the AggregationSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include <list>
#include <map>
#include "Symbol.h"
#include "ClassificationSymbol.h"
#include "EnumeratorSymbol.h"

/**
* AggregationSymbol.
*
*/
class AggregationSymbol : public Symbol
{
private:
    typedef Symbol super;

public:
    bool is_base_symbol() const { return false; }

    /**
    * Constructor.
    * 
    * A unique name is generated using a static counter.
    */
    explicit AggregationSymbol(const Symbol *from, const Symbol *to, omc::location decl_loc = omc::location())
        : Symbol(symbol_name(from, to), decl_loc)
        , from(from->stable_rp())
        , pp_from(nullptr)
        , to(to->stable_rp())
        , pp_to(nullptr)
        , pp_generate_To(false)
    {
    }

    /**
     * Construct symbol name for the aggregation
     * 
     * Example: om_aggregation_DISEASE_PHASE_om_APPEARANCE.
     *
     * @param from The source classification
     * @param to   The destination classification
     *
     * @return A string.
     */
    static string symbol_name(const Symbol* from, const Symbol* to);

    /**
     * Determine if aggregation exists
     *
     * @param from The source classification
     * @param to   The destination classification
     *
     * @return true if exists, else false
     */
    static bool exists(const Symbol* from, const Symbol* to);

    void post_parse(int pass);

    CodeBlock cxx_declaration_global();

    CodeBlock cxx_definition_global();

    /**
     * The name of the XXX_To_YYY function for developer use.
     *
     * @return A string.
     */
    string pp_helper_name() const;

    /**
     * Reference to pointer to the 'from' classification
     * 
     * Stable to symbol morphing during parse phase.
     */
    Symbol *& from;

    /**
     * Direct pointer to the 'from' classification
     * 
     * For use post-parse.
     */
    ClassificationSymbol * pp_from;

    /**
     * Reference to pointer to the 'to' classification
     * 
     * Stable to symbol morphing during parse phase.
     */
    Symbol *& to;

    /**
     * Direct pointer to the 'to' classification
     * 
     * For use post-parse.
     */
    ClassificationSymbol * pp_to;

    /**
     * List of symbols in the map.
     * 
     * The list is gathered during parsing.  It consists of pairs of enumerators from each of the
     * classifications, which are resolved to a map during post-parse processing.
     */
    list<Symbol **> symbol_list;

    /**
     * The enumerator map for the aggregation (post-parse)
     */
    map<EnumeratorSymbol *, EnumeratorSymbol *> pp_enumerator_map;

    /**
     * true to generate the XXX_To_YYY transformation global function
     */
    bool pp_generate_To;

};
