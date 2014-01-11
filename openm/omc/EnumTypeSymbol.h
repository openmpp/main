/**
* @file    EnumTypeSymbol.h
* Declarations for the EnumTypeSymbol class.
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include <list>
#include "TypeSymbol.h"

class EnumeratorSymbol;

using namespace std;

/**
* EnumTypeSymbol.
*/
class EnumTypeSymbol : public TypeSymbol
{
private:
    typedef TypeSymbol super;

public:

    /**
    * Constructor.
    *
    * @param [in,out] sym The symbol to be morphed.
    */

    EnumTypeSymbol(Symbol *sym, int dicId)
        : TypeSymbol(sym)
        , dicId(dicId)
    {
    }

    void post_parse(int pass);

    void populate_metadata(openm::MetaModelHolder & metaRows);

    /**
    * The enumerators of this enum
    *
    *  Populated after parsing is complete.
    */

    list<EnumeratorSymbol *> pp_enumerators;


    /**
     * Indicates the kind of type using enum kind_of_type.
     * 
     * Used to communicate with meta-data API.
     */

    int dicId;
};

