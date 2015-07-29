/**
* @file    EnumerationSymbol.h
* Declarations for the EnumerationSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include <string>
#include <list>
#include "TypeSymbol.h"

class EnumeratorSymbol;

using namespace std;

/**
* EnumerationSymbol.
*/
class EnumerationSymbol : public TypeSymbol
{
private:
    typedef TypeSymbol super;

public:
    bool is_base_symbol() const { return false; }

    /**
     * Constructor.
     *
     * @param [in,out] sym The symbol to be morphed.
     * @param storage_type Type of the storage.
     * @param dicId        The kind of type.
     * @param decl_loc     (Optional) the declaration location.
     */
    EnumerationSymbol(Symbol *sym, token_type storage_type, int dicId, yy::location decl_loc = yy::location())
        : TypeSymbol(sym, decl_loc)
        , storage_type(storage_type)
        , metadata_needed(false)
        , dicId(dicId)
    {
    }

    /**
     * Constructor.
     *
     * @param unm          The unique Symbol name.
     * @param storage_type Type of the storage.
     * @param dicId        The kind of type.
     */
    EnumerationSymbol(const string unm, token_type storage_type, int dicId)
        : TypeSymbol(unm)
        , storage_type(storage_type)
        , metadata_needed(false)
        , dicId(dicId)
    {
    }

    virtual const string default_initial_value() const = 0;

    void post_parse(int pass);

    string metadata_signature() const;

    void populate_metadata(openm::MetaModelHolder & metaRows);

    /**
     * Gets the size of the enumeration.
     *
     * @return An int.
     */
    virtual int pp_size() const = 0;

    /**
     * Type used to store enumerators of this enumeration.
     */
    token_type storage_type;

    /**
     * True if metadata needed.
     * 
     * Metadata for this enumeration is needed if the enumeration is used
     * as a dimension of a table or a dimension of an external parameter,
     * or the storage type of an external parameter.
     */
    bool metadata_needed;

    /**
     * Indicates the kind of type using enum kind_of_type.
     * 
     * Used to communicate with meta-data API.
     */
    int dicId;
};

