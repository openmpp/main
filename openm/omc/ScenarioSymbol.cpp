/**
* @file    ScenarioSymbol.cpp
* Definitions for the ScenarioSymbol class.
*/
// Copyright (c) 2013-2016 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <cassert>
#include "ScenarioSymbol.h"
#include "LanguageSymbol.h"
#include "CodeBlock.h"

using namespace std;
using namespace openm;

void ScenarioSymbol::post_parse(int pass)
{
    // Hook into the hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case eAssignLabel:
    {
        // Check for a note specified using NOTE comment
        // in scenario input files (.dat and .odat), for each language.
        for (int j = 0; j < LanguageSymbol::number_of_languages(); j++) {
            auto lang_sym = LanguageSymbol::id_to_sym[j];
            string key = unique_name + "," + lang_sym->name;
            auto search = notes_input.find(key);
            if (search != notes_input.end()) {
                auto text = (search->second).first;
                auto loc = (search->second).second;
                pp_notes[j] = text;
            }
        }
        break;
    }

    default:
        break;
    }
}

