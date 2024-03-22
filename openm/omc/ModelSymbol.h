/**
* @file    ModelSymbol.h
* Declarations for the ModelSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include "Symbol.h"
#include "ParameterSymbol.h"

/**
* Symbol for the model itself.
*
* A single instance of ModelSymbol exists in the symbol table.
*
*/
class ModelSymbol : public Symbol
{
private:
    typedef Symbol super;

public:
    bool is_base_symbol() const override { return false; }

    ModelSymbol(string nm, string enm)
        : Symbol(nm)
        , external_name(enm)
        , pp_memory_popsize_parameter(nullptr)
        , memory_MB_constant_per_instance(0)
        , memory_MB_constant_per_sub(0)
        , memory_MB_popsize_coefficient(0.0)
        , memory_safety_factor(1.10)
    {
    }

    string default_label(const LanguageSymbol& language) const override
    {
        // Use the model name to construct the default label, e.g. "RiskPaths".
        // This is typically overridden in model code by providing a label to the 'model' symbol, e.g.
        //     //LABEL(model,EN) The model label...
        return external_name;
    }

    void post_parse(int pass) override;

    CodeBlock cxx_definition_global() override;

    void populate_metadata(openm::MetaModelHolder & metaRows) override;

    /**
     * The name of the model.
     */
    string external_name;

    /**
     * Memory prediction - The parameter containing an indication of population size.
     */
    ParameterSymbol * pp_memory_popsize_parameter;

    /**
     * Memory prediction - The value of the option memory_MB_constant_per_instance.
     */
    int memory_MB_constant_per_instance;

    /**
     * Memory prediction - The value of the option memory_MB_constant_per_sub.
     */
    int memory_MB_constant_per_sub;

    /**
     * Memory prediction - The value of the option memory_MB_popsize_coefficient.
     */
    double memory_MB_popsize_coefficient;

    /**
     * Memory prediction - The value of the option memory_safety_factor.
     */
    double memory_safety_factor;

};

