/**
* @file    EnumerationWithEnumeratorsSymbol.h
* Declarations for the EnumerationWithEnumeratorsSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include <string>
#include <list>
#include <map>
#include "EnumerationSymbol.h"

class EnumeratorSymbol;

using namespace std;

/**
* EnumerationWithEnumeratorsSymbol.
*/
class EnumerationWithEnumeratorsSymbol : public EnumerationSymbol
{
private:
    typedef EnumerationSymbol super;

public:
    bool is_base_symbol() const { return false; }

    /**
     * Morphing constructor.
     *
     * @param [in,out] sym The symbol to be morphed.
     * @param storage_type Type of the storage.
     * @param dicId        The kind of type.
     * @param decl_loc     (Optional) the declaration location.
     */
    EnumerationWithEnumeratorsSymbol(Symbol *sym, token_type storage_type, int dicId, yy::location decl_loc = yy::location())
        : EnumerationSymbol(sym, storage_type, dicId, decl_loc)
    {
    }

    /**
     * Constructor from name
     *
     * @param unm          The unique Symbol name.
     * @param storage_type Type of the storage.
     * @param dicId        The kind of type.
     */
    EnumerationWithEnumeratorsSymbol(const string unm, token_type storage_type, int dicId)
        : EnumerationSymbol(unm, storage_type, dicId)
    {
    }

    const string default_initial_value() const;

    void post_parse(int pass);

    void populate_metadata(openm::MetaModelHolder & metaRows);

    /**
     * Gets the number of enumerators in the enumeration.
     *
     * @return An int.
     */
    int pp_size() const;

    /**
     * The enumerators of this enumeration
     * 
     *  Populated after parsing is complete.
     */
    list<EnumeratorSymbol *> pp_enumerators;

    /**
     * Map from enumerator names to corresponding ordinal values.
     * 
     * Created after parsing is complete.
     */
    map<string, int> pp_name_to_int;
};
