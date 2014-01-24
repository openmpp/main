/**
* @file    VersionSymbol.h
* Declarations for the VersionSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "Symbol.h"

// remove GNU macro defines for major and minor
#ifdef major
#undef major
#endif
#ifdef minor
#undef minor
#endif

/**
* Symbol for the model version information
*
* A single instance of VersionSymbol exists in the symbol table, with name 'version'.
* A default VersionSymbol with value 1,0,0,0 is created at initialization.
* This default VersionSymbol will be morphed if a 'version' statement
* is encountered in the model source code.
*/
class VersionSymbol : public Symbol
{
private:
    typedef Symbol super;

public:

    /**
    * Constructor.
    *
    * @param[in]   i_major           Major model version.
    * @param[in]   i_minor           Minor model version.
    * @param[in]   i_sub_minor       Sub-minor model version.
    * @param[in]   i_sub_sub_minor   Sub-sub minor version.
    */
    VersionSymbol(int i_major, int i_minor, int i_sub_minor, int i_sub_sub_minor, yy::location decl_loc = yy::location())
        : Symbol(token_to_string(token::TK_version), decl_loc)
        , major(i_major)
        , minor(i_minor)
        , sub_minor(i_sub_minor)
        , sub_sub_minor(i_sub_sub_minor)
    {
    }

    string formatted_value() const
    {
        return to_string(major) + "." + to_string(minor) + "." + to_string(sub_minor) + "." + to_string(sub_sub_minor);
    }

    void populate_metadata(openm::MetaModelHolder & metaRows);

    /** major model version (position 1) */
    int major;

    /** minor model version (position 2) */
    int minor;

    /** sub-minor model version (position 3) */
    int sub_minor;

    /** sub-sub-minor model version (position 4) */
    int sub_sub_minor;
};

