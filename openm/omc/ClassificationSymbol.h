/**
* @file    ClassificationSymbol.h
* Declarations for the ClassificationSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include <list>
#include "EnumTypeSymbol.h"

using namespace std;
using namespace openm;


/**
* ClassificationSymbol.
*/
class ClassificationSymbol : public EnumTypeSymbol
{
private:
    typedef EnumTypeSymbol super;

public:
    bool is_base_symbol() const { return false; }

    /**
     * Constructor.
     *
     * @param [in,out] sym The symbol to be morphed.
     */

    ClassificationSymbol(Symbol *sym, yy::location decl_loc = yy::location())
        : EnumTypeSymbol(sym, token::TK_int, kind_of_type::classification_type, decl_loc)
    {
    }

    void post_parse(int pass);

    CodeBlock cxx_declaration_global();

};

