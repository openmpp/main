/**
* @file    DeveloperTableAnalysisSymbol.h
* Declarations for the DeveloperTableAnalysisSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "Symbol.h"

/**
* DeveloperTableAnalysisSymbol.
*
*/
class DeveloperTableAnalysisSymbol : public Symbol
{
private:
    typedef Symbol super;

public:
    bool is_base_symbol() const { return false; }

    /**
    * Morphing constructor.
    *
    * @param [in,out]  sym The symbol to be morphed.
    */
    DeveloperTableAnalysisSymbol(Symbol *sym, yy::location decl_loc = yy::location())
        : Symbol(sym, decl_loc)
    {
    }

    void post_parse(int pass);
};
