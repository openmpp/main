/**
* @file    omc_missing_documentation.cpp
*          
* Function which emits warnings for missing model documentation
* 
*/
// Copyright (c) 2023-2023 OpenM++ Contributors (see AUTHORS.txt)
// This code is licensed under the MIT license (see LICENSE.txt)

#include <iostream>
#include <fstream>

#include "Symbol.h"
#include "LanguageSymbol.h"
#include "ParameterSymbol.h"
#include "TableSymbol.h"
#include "ParameterGroupSymbol.h"
#include "TableGroupSymbol.h"
#include "EntitySymbol.h"
#include "EntityDataMemberSymbol.h"
#include "EnumerationSymbol.h"
#include "EnumeratorSymbol.h"
#include "ClassificationSymbol.h"
#include "EntityTableSymbol.h"
#include "omc_missing_documentation.h"

using namespace std;

void do_missing_documentation(void)
{
    for (auto& s : Symbol::pp_all_enumerations) {
        if (!s->is_label_supplied()) {
            if (Symbol::option_missing_label_warning_enumeration) {
                s->pp_warning(LT("warning : missing label for enumeration '") + s->name + "'");
            }
            if (Symbol::option_missing_label_warning_published_enumeration && s->is_published()) {
                s->pp_warning(LT("warning : missing label for published enumeration '") + s->name + "'");
            }
        }
        if (s->is_classification()) {
            auto c = dynamic_cast<ClassificationSymbol*>(s);
            assert(c); // logic guarantee
            for (auto& l : c->pp_enumerators) {
                if (!l->is_label_supplied()) {
                    if (Symbol::option_missing_label_warning_enumeration) {
                        l->pp_warning(LT("warning : missing label for enumerator '") + l->name + "'");
                    }
                    if (Symbol::option_missing_label_warning_published_enumeration && s->is_published()) {
                        l->pp_warning(LT("warning : missing label for published enumerator '") + l->name + "'");
                    }
                }
            }
        }
    }
    for (auto& s : Symbol::pp_all_parameters) {
        if (!s->is_label_supplied()) {
            if (Symbol::option_missing_label_warning_parameter) {
                s->pp_warning(LT("warning : missing label for parameter '") + s->name + "'");
            }
            if (Symbol::option_missing_label_warning_published_parameter && s->is_published()) {
                s->pp_warning(LT("warning : missing label for published parameter '") + s->name + "'");
            }
        }
        if (!s->is_note_supplied()) {
            if (Symbol::option_missing_note_warning_published_parameter && s->is_published()) {
                s->pp_warning(LT("warning : missing note for published parameter '") + s->name + "'");
            }
        }
    }
    for (auto& s : Symbol::pp_all_parameter_groups) {
        if (!s->is_label_supplied()) {
            if (Symbol::option_missing_label_warning_parameter) {
                s->pp_warning(LT("warning : missing label for parameter group '") + s->name + "'");
            }
            if (Symbol::option_missing_label_warning_published_parameter && s->is_published()) {
                s->pp_warning(LT("warning : missing label for published parameter group '") + s->name + "'");
            }
        }
    }

    for (auto& s : Symbol::pp_all_tables) {
        if (!s->is_label_supplied()) {
            if (Symbol::option_missing_label_warning_table) {
                s->pp_warning(LT("warning : missing label for table '") + s->name + "'");
            }
            if (Symbol::option_missing_label_warning_published_table && s->is_published()) {
                s->pp_warning(LT("warning : missing label for published table '") + s->name + "'");
            }
        }
        if (!s->is_note_supplied()) {
            if (Symbol::option_missing_note_warning_published_table && s->is_published()) {
                s->pp_warning(LT("warning : missing note for published table '") + s->name + "'");
            }
        }
        int pos = 0;
        for (auto& expr : s->pp_measures) {
            if (!expr->is_label_supplied()) {
                if (Symbol::option_missing_label_warning_table) {
                    expr->pp_warning(LT("warning : missing label for expression ") + to_string(pos) + LT(" of table '") + s->name + "'");
                }
                if (Symbol::option_missing_label_warning_published_table && s->is_published()) {
                    expr->pp_warning(LT("warning : missing label for expression ") + to_string(pos) + LT(" of published table '") + s->name + "'");
                }
            }
            ++pos;
        }
    }
    for (auto& s : Symbol::pp_all_table_groups) {
        if (!s->is_label_supplied()) {
            if (Symbol::option_missing_label_warning_table) {
                s->pp_warning(LT("warning : missing label for table group '") + s->name + "'");
            }
            if (Symbol::option_missing_label_warning_published_table && s->is_published()) {
                s->pp_warning(LT("warning : missing label for published table group '") + s->name + "'");
            }
        }
    }

    for (auto& e : Symbol::pp_all_entities) {
        for (auto& s : e->pp_data_members) {
            if (s->is_builtin_attribute() || s->is_simple_attribute() || s->is_identity_attribute()) {
                if (!s->is_label_supplied()) {
                    if (Symbol::option_missing_label_warning_attribute) {
                        s->pp_warning(LT("warning : missing label for attribute '") + s->name + LT("' in entity '") + e->name + "'.");
                    }
                    if (Symbol::option_missing_label_warning_published_attribute && s->is_published()) {
                        s->pp_warning(LT("warning : missing label for published attribute '") + s->name + "'");
                    }
                }
                if (!s->is_note_supplied()) {
                    if (Symbol::option_missing_note_warning_published_attribute && s->is_published()) {
                        s->pp_warning(LT("warning : missing note for published attribute '") + s->name + "'");
                    }
                }
            }
        }
    }

    {
        // block to handle untranslated labels and notes
        int nlang = LanguageSymbol::number_of_languages();
        if (nlang > 1) {
            for (auto& the_pair : Symbol::pp_symbols) {
                // iterate all symbols
                auto s = the_pair.second;
                if (s->is_base_symbol()) {
                    // skip 'space junk' symbols in symbol table
                    // which were never morphed to a 'real' kind of Symbol in the Symbol hierarchy.
                    continue;
                }
                if (s->is_label_supplied()) {
                    // label was supplied in the model's default language
                    for (int j = 1; j < nlang; ++j) {
                        // iterate secondary languages
                        auto lang_sym = LanguageSymbol::id_to_sym[j];
                        auto lang_code = lang_sym->name;
                        auto lang0_pos = s->pp_labels_pos[0]; // code position of label in default language
                        if (!s->pp_labels_explicit[j]) {
                            if (Symbol::option_missing_translated_label_warning_any) {
                                s->pp_warning(LT("warning : missing '") + lang_code + LT("' translated label for symbol '") + s->name + "'", lang0_pos);
                            }
                            if (Symbol::option_missing_translated_label_warning_published_any && s->is_published()) {
                                s->pp_warning(LT("warning : missing '") + lang_code + LT("' translated label for published symbol '") + s->name + "'", lang0_pos);
                            }
                        }
                    }
                }
                if (s->is_note_supplied()) {
                    // note was supplied in the model's default language
                    for (int j = 1; j < nlang; ++j) {
                        // iterate secondary languages
                        auto lang_sym = LanguageSymbol::id_to_sym[j];
                        auto lang_code = lang_sym->name;
                        auto lang0_pos = s->pp_notes_pos[0]; // code position of note in default language
                        if (s->pp_notes[j].length() == 0) {
                            if (Symbol::option_missing_translated_note_warning_any) {
                                s->pp_warning(LT("warning : missing '") + lang_code + LT("' translated note for symbol '") + s->name + "'", lang0_pos);
                            }
                            if (Symbol::option_missing_translated_note_warning_published_any && s->is_published()) {
                                s->pp_warning(LT("warning : missing '") + lang_code + LT("' translated note for published symbol '") + s->name + "'", lang0_pos);
                            }
                        }
                    }
                }
            }
        }
    }
}