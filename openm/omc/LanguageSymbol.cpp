/**
* @file    LanguageSymbol.cpp
* Definitions for the LanguageSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/common/omHelper.h" // for trim
#include "LanguageSymbol.h"

using namespace std;

void LanguageSymbol::post_parse(int pass)
{
    // Hook into the hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case eAssignLabel:
    {
        // The label for language symbols follows a special syntax
        // which does not include a language prefix, e.g.
        //      EN, // English
        // The label assignment below will replace the default label assignment
        // done by Symbol::post_parse in the hierarchical calling chain.

        string lang_lbl = name;
        assert(decl_loc != omc::location()); // grammar guarantee - language symbol has code location
        // Construct key for lookup in map of all // comments
        omc::position pos(decl_loc.begin.filename, decl_loc.begin.line, 0);
        auto cmt_search = cxx_comments.find(pos);
        if (cmt_search != cxx_comments.end()) {
            // There was a // comment on the given line.
            lang_lbl = openm::trim(cmt_search->second);
        }
        else {
            //TODO error - language label required ?
        }
        // use the string designating this language, in all languages
        // Note that pp_labels was previously populated with default values 
        // in this pass by Symbol::post_parse
        for (auto & lbl : pp_labels) {
            lbl = lang_lbl;
        }

        break;
    }
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
