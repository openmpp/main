/**
* @file    EnumTypeSymbol.h
* Declarations for the EnumTypeSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
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
     * @param dicId        The kind of type.
     */

    EnumTypeSymbol(Symbol *sym, int dicId, yy::location decl_loc = yy::location())
        : TypeSymbol(sym, decl_loc)
        , dicId(dicId)
    {
    }


    /**
     * Constructor.
     *
     * @param unm   The unique Symbol name.
     * @param dicId The kind of type.
     */

    EnumTypeSymbol(const string unm, int dicId)
        : TypeSymbol(unm)
        , dicId(dicId)
    {
    }

    const string get_initial_value() const;

    void post_parse(int pass);

    void populate_metadata(openm::MetaModelHolder & metaRows);


    /**
     * Gets the size of the enumeration
     *
     * @return An int.
     */

    int pp_size()
    {
        return pp_enumerators.size();
    }

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

