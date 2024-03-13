/**
* @file    MultilinkAttributeSymbol.h
* Declarations for the MultilinkAttributeSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include "MaintainedAttributeSymbol.h"
#include "EntityMultilinkSymbol.h"
#include "NumericSymbol.h"
#include "UnknownTypeSymbol.h"

class EntityMultilinkSymbol;

using namespace std;

/**
 * MultilinkAttributeSymbol
 * 
 * Symbol for multilink function attributes like count(multilink), sum_over(multilink, attribute),
 * etc.
 */
class MultilinkAttributeSymbol : public MaintainedAttributeSymbol
{
private:
    typedef MaintainedAttributeSymbol super;

public:
    bool is_base_symbol() const { return false; }

    /**
     * Constructor.
     *
     * @param ent       The entity.
     * @param func      The function, e.g. TK_min_over
     * @param multilink The multilink, e.g. things
     * @param attribute  (Optional) the attribute.
     *                  
     *  The 'data_type' argument to the AttributeSymbol constructor is interim and
     *  may be changed once the data type of attribute is known.
     */
    MultilinkAttributeSymbol(const Symbol * ent, token_type func, const Symbol *multilink, const string attribute)
        : MaintainedAttributeSymbol(MultilinkAttributeSymbol::member_name(func, multilink, attribute),
                        ent,
                        UnknownTypeSymbol::find() )
        , func(func)
        , multilink(multilink->stable_rp())
        , pp_multilink(nullptr)
        , attribute(attribute)
        , pp_attribute(nullptr)
    {
        create_auxiliary_symbols();
    }

    /**
     * Builds the function body of the evaluate function.
     */
    void build_body_evaluate();

    static string member_name(token_type func, const Symbol *multilink, const string attribute);

    static string symbol_name(const Symbol * ent, token_type func, const Symbol *multilink, const string attribute);

    /**
     * Create a symbol for a multilink attribute.
     *
     * @param ent       The containing entity.
     * @param func      The function, e.g. TK_sum_over
     * @param multilink The multilink, e.g. things
     * @param attribute  (Optional) the attribute.
     *
     * @return The symbol.
     */
    static Symbol * create_symbol(const Symbol * ent, token_type func, const Symbol *multilink, const string attribute);

    /**
     * Create auxiliary symbols associated with this symbol.
     */
    void create_auxiliary_symbols();

    void post_parse(int pass);

    CodeBlock cxx_declaration_entity();

    string pretty_name() const override
    {
        string result = token_to_string(func);
        result += "(";
        result += pp_multilink->name;
        if (func != token::TK_count) {
            result += ",";
            result += pp_attribute->pretty_name();
        }
        result += ")";
        return result;
    }

    /** The function which computes the current value of the attribute from the multilink */
    EntityFuncSymbol *evaluate_fn;

    /**
     * The aggregate function applied to the multi-link
     * 
     * Is one of TK_count, TK_sum_over, TK_min_over, TK_max_over
     */
    token_type func;

    /**
     * The multilink over which the aggregate function is performed.
     */
    Symbol*& multilink;

    /**
     * The multilink over which the aggregate function is performed.
     */
    EntityMultilinkSymbol *pp_multilink;

    /**
     * The attribute the aggregate function applies to.  Is empty for TK_count
     */
    const string attribute;
    AttributeSymbol *pp_attribute;

};

