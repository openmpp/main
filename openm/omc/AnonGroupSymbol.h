/**
* @file    AnonGroupSymbol.h
* Declarations for the AnonGroupSymbol class.
*/
// Copyright (c) 2013-2021 OpenM++ Contributors
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include "GroupSymbol.h"

/**
* AnonGroupSymbol.
* 
* An instance holds parsed information from one of the following model code statements:
* hide, parameters_suppress, parameters_retain, tables_suppress, tables_retain, parameters_to_tables
*
*/
class AnonGroupSymbol : public GroupSymbol
{
private:
    typedef GroupSymbol super;

public:
    bool is_base_symbol() const { return false; }

    /**
     * Enumeration indicating the originating model code statement.
     */
    enum class eKind {
        ///< group is a 'hide' statement.
        hide,
        ///< group is a parameters_suppress statement.
        parameters_suppress,
        ///< group is a parameters_retain statement.
        parameters_retain,
        ///< group is a tables_suppress statement.
        tables_suppress,
        ///< group is a tables_retain statement.
        tables_retain,
        ///< group is a parameters_to_tables statement.
        parameters_to_tables,
    };

    /**
    * Constructor.
    * 
    * A unique name is generated using a counter.
    */
    AnonGroupSymbol(const eKind anon_kind, yy::location decl_loc = yy::location())
        : GroupSymbol("om_anon_" + to_string(++counter), decl_loc)
        , anon_kind(anon_kind)
    {
    }

    void post_parse(int pass);

    /**
     * The kind of the anonymous group
     */
    const eKind anon_kind;

    static int counter;
};
