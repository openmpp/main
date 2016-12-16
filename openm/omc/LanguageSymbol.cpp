/**
* @file    LanguageSymbol.cpp
* Definitions for the LanguageSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "LanguageSymbol.h"

using namespace std;

void LanguageSymbol::post_parse(int pass)
{
    // Hook into the hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case ePopulateCollections:
    {
        // Add this language to the complete list of languages.
        pp_all_languages.push_back(this);
        break;
    }
    default:
        break;
    }
}

// static members
int LanguageSymbol::next_language_id = 0;
map<string, int> LanguageSymbol::name_to_id;
vector<LanguageSymbol *> LanguageSymbol::id_to_sym;

/** populate language metadata: id, language code, language name */
void LanguageSymbol::populate_metadata(openm::MetaModelHolder & metaRows)
{
    using namespace openm;

    // Hook into the hierarchical calling chain
    super::populate_metadata(metaRows);

    // Perform operations specific to this level in the Symbol hierarchy.
    LangLstRow langRow(language_id);
    langRow.code = name;
    langRow.name = label();
    if (langRow.name.empty()) langRow.name = langRow.code;  // language name cannot be empty, use code as fallback

    metaRows.langLst.push_back(langRow);
}
