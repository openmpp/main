/**
* @file    ClassificationSymbol.h
* Declarations for the ClassificationSymbol class.
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include <list>
#include "TypeSymbol.h"

class ClassificationLevelSymbol;

using namespace std;

/**
* ClassificationSymbol.
*/
class ClassificationSymbol : public TypeSymbol
{
private:
    typedef TypeSymbol super;

public:

    /**
     * Constructor.
     *
     * @param [in,out] sym The symbol to be morphed.
     */

    ClassificationSymbol(Symbol *sym)
        : TypeSymbol(sym)
    {
    }

    void post_parse(int pass);

    CodeBlock cxx_declaration_global();

    void populate_metadata(openm::MetaModelHolder & metaRows);

    /**
    * The classification levels of this classification
    *
    *  Populated after parsing is complete.
    */

    list<ClassificationLevelSymbol *> pp_classification_levels;
};

