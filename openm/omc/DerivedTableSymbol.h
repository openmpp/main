/**
* @file    DerivedTableSymbol.h
* Declarations for the DerivedTableSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include <list>
#include "Symbol.h"
#include "TableSymbol.h"

class CodeBlock;

/**
* DerivedTableSymbol.
*
*/
class DerivedTableSymbol : public TableSymbol
{
private:
    typedef TableSymbol super;

public:
    bool is_base_symbol() const { return false; }

    /**
    * Morphing constructor.
    *
    * @param [in,out]  sym The symbol to be morphed.
    */
    DerivedTableSymbol(Symbol *sym, omc::location decl_loc = omc::location())
        : TableSymbol(sym, decl_loc)
    {
    }

    void post_parse(int pass);

    CodeBlock cxx_declaration_global();

    CodeBlock cxx_definition_global();

    string cxx_initializer() const;

    void populate_metadata(openm::MetaModelHolder & metaRows);

};
