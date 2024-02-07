/**
* @file    ModuleSymbol.h
* Declarations for the ModuleSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include "Symbol.h"

/**
* Symbol for a source code module.
*
* The name of a ModuleSymbol is the name of the model source code file,
* including the .mpp or .ompp file extension.
*/
class ModuleSymbol : public Symbol
{
private:
    typedef Symbol super;

public:
    bool is_base_symbol() const { return false; }

    /**
    * Constructor.
    *
    * Unlike most symbols, a ModuleSymbol is created outside of
    * the parser and scanner.
    *
    * @param   nm  The name of the module.
    *
    */
    ModuleSymbol(string nm)
        : Symbol(nm)
        , provenance(unknown)
    {
    }

    void post_parse(int pass);

    /**
     * Enumeration indicating provenance of module
     */
    enum module_provenance {
        ///< unknown
        unknown,
        ///< Model source code in MODEL/code
        from_code,
        ///< Use source code module in OM_ROOT/use
        from_use,
        ///< Parameter value file in MODEL/parameters
        from_dat
    };

    bool is_code(void) const
    {
        return (provenance == from_code);
    }

    bool is_use(void) const
    {
        return (provenance == from_use);
    }

    bool is_dat(void) const
    {
        return (provenance == from_dat);
    }

    /**
     * Provenance of the module.
     */
    module_provenance provenance;
};

