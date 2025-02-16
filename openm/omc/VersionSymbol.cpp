/**
* @file    VersionSymbol.cpp
* Definitions for the VersionSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <cassert>
#include "VersionSymbol.h"
#include "CodeBlock.h"
#include "libopenm/db/modelBuilder.h"

using namespace std;
using namespace openm;

CodeBlock VersionSymbol::cxx_definition_global()
{
    // Hook into the hierarchical calling chain
    CodeBlock c = super::cxx_definition_global();

    // Perform operations specific to this level in the Symbol hierarchy.

    c += "namespace omr {";
    c += "const char * model_version = \"" + formatted_value() + "\";";
    int version_int = 1000 * major + 100 * minor + 10 * sub_minor + sub_sub_minor;
    c += "const int model_version_int = " + std::to_string(version_int) + ";";
    c += "} // namespace omr";
    c += "";

    return c;
}

void VersionSymbol::populate_metadata(openm::MetaModelHolder & metaRows)
{
    // Hook into the hierarchical calling chain
    super::populate_metadata(metaRows);

    // Perform operations specific to this level in the Symbol hierarchy.
    metaRows.modelDic.version = formatted_value();
};
