/**
* @file    ClassificationLevelSymbol.h
* Declarations for the ClassificationLevelSymbol class.
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "Symbol.h"

class ClassificationSymbol;

using namespace std;

class ClassificationLevelSymbol : public Symbol
{
private:
    typedef Symbol super;

public:
    ClassificationLevelSymbol(Symbol *sym, const Symbol *classification, int ordinal)
        : Symbol(sym)
        , classification(classification->get_rpSymbol())
        , ordinal(ordinal)
        , pp_classification(nullptr)
    {
    }

    void post_parse(int pass);

    /**
    * Reference to pointer to classification.
    *
    * Stable to symbol morhing during parse phase.
    */

    Symbol*& classification;


    /**
     * ordinal giving order of level within classification
     */

    int ordinal;


    /**
     * Direct pointer to classification.
     * 
     * Set during post-parse for convenience.
     */

    ClassificationSymbol *pp_classification;
};


