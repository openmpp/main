/**
* @file    ModelTypeSymbol.cpp
* Definitions for the ModelTypeSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <cassert>
#include "ModelTypeSymbol.h"
#include "CodeBlock.h"
#include "libopenm/db/modelBuilder.h"

using namespace std;
using namespace openm;

CodeBlock ModelTypeSymbol::cxx_definition_global()
{
    // Hook into the hierarchical calling chain
    CodeBlock c = super::cxx_definition_global();

    // Perform operations specific to this level in the Symbol hierarchy.

    switch (value) {
    case token::TK_case_based:
    {
        c += "namespace omr {";
        c +=     "constexpr bool model_is_case_based = true;";
        c +=     "constexpr bool model_is_time_based = false;";
        c += "} // namespace omr";
        break;
    }
    case token::TK_time_based:
    {
        c += "namespace omr {";
        c +=     "constexpr bool model_is_case_based = false;";
        c +=     "constexpr bool model_is_time_based = true;";
        c += "} // namespace omr";
        break;
    }
    default:
        // NOT_REACHED
        assert(false);
    }

    if ( just_in_time ) {
        c += "const bool BaseEvent::just_in_time = true;" ;
    }
    else {
        c += "const bool BaseEvent::just_in_time = false;" ;
    }

    return c;
}

void ModelTypeSymbol::populate_metadata(openm::MetaModelHolder & metaRows)
{
    // Hook into the hierarchical calling chain
    super::populate_metadata(metaRows);

    // Perform operations specific to this level in the Symbol hierarchy.
    if (value == token::TK_case_based) {
        metaRows.modelDic.type = static_cast<int>(ModelType::caseBased);
    }
    else
    {
        // compiler guarantee
        assert(value == token::TK_time_based);
        metaRows.modelDic.type = static_cast<int>(ModelType::timeBased);
    }
};

ModelTypeSymbol *ModelTypeSymbol::find()
{
    auto mts = dynamic_cast<ModelTypeSymbol *>(get_symbol(token_to_string(token::TK_model_type)));
    assert(mts); // only called when valid
    return mts;
}

