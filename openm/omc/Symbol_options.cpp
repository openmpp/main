/**
 * @file    Symbol_options.cpp
 *          
 * Definition of the Symbol::do_options function.
 */
// Copyright (c) 2013-2023 OpenM++ Contributors (see AUTHORS.txt)
// This code is licensed under the MIT license (see LICENSE.txt)

#include <cassert>
#include <limits>
#include <algorithm>
#include <typeinfo>
#include <unordered_map>
#include <set>
#include <map>
#include "Symbol.h"
#include "ModelTypeSymbol.h"
#include "ModelSymbol.h"

using namespace std;
using namespace openm;

// option-related static member definitions
unordered_multimap<string, pair<string, omc::location>> Symbol::options;
bool Symbol::option_event_trace = false;
bool Symbol::option_event_trace_warning = true;
bool Symbol::option_bounds_errors = true;
bool Symbol::option_case_checksum = false;
bool Symbol::option_allow_time_travel = false;
bool Symbol::option_allow_clairvoyance = false;
bool Symbol::option_time_infinite_is_32767 = false;
bool Symbol::option_time_undef_is_minus_one = false;
bool Symbol::option_verify_attribute_modification = true;
bool Symbol::option_verify_timelike_attribute_access = true;
bool Symbol::option_verify_valid_table_increment = true;
bool Symbol::option_weighted_tabulation = false;
bool Symbol::option_resource_use = false;
bool Symbol::option_entity_member_packing = false;
bool Symbol::option_microdata_output = false;
bool Symbol::option_microdata_output_warning = true;
bool Symbol::option_microdata_write_on_enter = false;
bool Symbol::option_microdata_write_on_exit = false;
bool Symbol::option_microdata_write_on_event = false;
bool Symbol::option_all_attributes_visible = false;
bool Symbol::option_use_heuristic_short_names = false;
bool Symbol::option_enable_heuristic_names_for_enumerators = true;
bool Symbol::option_convert_modgen_note_syntax = true;
size_t Symbol::short_name_max_length = 32;
bool Symbol::option_censor_event_time = false;
bool Symbol::option_ascii_infinity = false;
bool Symbol::option_missing_label_warning_enumeration = false;
bool Symbol::option_missing_label_warning_parameter = false;
bool Symbol::option_missing_label_warning_table = false;
bool Symbol::option_missing_label_warning_attribute = false;
bool Symbol::option_missing_label_warning_published_enumeration = false;
bool Symbol::option_missing_label_warning_published_parameter = false;
bool Symbol::option_missing_label_warning_published_table = false;
bool Symbol::option_missing_label_warning_published_attribute = false;
bool Symbol::option_missing_note_warning_published_parameter = false;
bool Symbol::option_missing_note_warning_published_table = false;
bool Symbol::option_missing_note_warning_published_attribute = false;
bool Symbol::option_missing_translated_label_warning_any = false;
bool Symbol::option_missing_translated_note_warning_any = false;
bool Symbol::option_missing_translated_label_warning_published_any = false;
bool Symbol::option_missing_translated_note_warning_published_any = false;
bool Symbol::option_missing_name_warning_classification = false;
bool Symbol::option_missing_name_warning_parameter = false;
bool Symbol::option_missing_name_warning_table = false;
bool Symbol::option_missing_name_warning_published_classification = false;
bool Symbol::option_missing_name_warning_published_parameter = false;
bool Symbol::option_missing_name_warning_published_table = false;
bool Symbol::option_generated_documentation = true;
bool Symbol::option_authored_documentation = true;
bool Symbol::option_symref_developer_edition = false;
bool Symbol::option_symref_unpublished_symbols = false;
bool Symbol::option_symref_main_topic = true;
bool Symbol::option_symref_model_symbol = true;
bool Symbol::option_symref_parameter_major_groups = true;
bool Symbol::option_symref_table_major_groups = true;
bool Symbol::option_symref_parameter_hierarchy = true;
bool Symbol::option_symref_table_hierarchy = true;
bool Symbol::option_symref_parameters_alphabetic = true;
bool Symbol::option_symref_tables_alphabetic = true;
bool Symbol::option_symref_enumerations_alphabetic = true;
bool Symbol::option_symref_parameter_topics = true;
bool Symbol::option_symref_table_topics = true;
bool Symbol::option_symref_enumeration_topics = true;
bool Symbol::option_symref_notes = true;
bool Symbol::option_alternate_attribute_dependency_implementation = false;
string Symbol::option_memory_popsize_parameter;

void Symbol::do_options()
{
    {
        string key = "event_trace";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            if (value == "on") {
                option_event_trace = true;
            }
            else if (value == "off") {
                option_event_trace = false;
            }
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "event_trace_warning";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            if (value == "on") {
                option_event_trace_warning = true;
            }
            else if (value == "off") {
                option_event_trace_warning = false;
            }
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "bounds_errors";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            if (value == "on") {
                option_bounds_errors = true;
            }
            else if (value == "off") {
                option_bounds_errors = false;
            }
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "case_checksum";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            if (value == "on") {
                option_case_checksum = true;
            }
            else if (value == "off") {
                option_case_checksum = false;
            }
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "allow_time_travel";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            if (value == "on") {
                option_allow_time_travel = true;
                // This option should be off to detect model logic errors
                // so always emit a warning if it is on.
                theLog->logMsg(LT("warning : option allow_time_travel is on."));
                Symbol::post_parse_warnings++;
            }
            else if (value == "off") {
                option_allow_time_travel = false;
            }
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "allow_clairvoyance";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            if (value == "on") {
                option_allow_clairvoyance = true;
                // This option should be off to detect model logic errors
                // so always emit a warning if it is on.
                theLog->logMsg(LT("warning : option allow_clairvoyance is on."));
                Symbol::post_parse_warnings++;
            }
            else if (value == "off") {
                option_allow_clairvoyance = false;
            }
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "time_infinite_is_32767";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            if (value == "on") {
                option_time_infinite_is_32767 = true;
            }
            else if (value == "off") {
                option_time_infinite_is_32767 = false;
            }
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "time_undef_is_minus_one";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            if (value == "on") {
                option_time_undef_is_minus_one = true;
            }
            else if (value == "off") {
                option_time_undef_is_minus_one = false;
            }
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "verify_attribute_modification";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            if (value == "on") {
                option_verify_attribute_modification = true;
            }
            else if (value == "off") {
                option_verify_attribute_modification = false;
            }
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "verify_timelike_attribute_access";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            if (value == "on") {
                option_verify_timelike_attribute_access = true;
            }
            else if (value == "off") {
                option_verify_timelike_attribute_access = false;
            }
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "verify_valid_table_increment";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            if (value == "on") {
                option_verify_valid_table_increment = true;
            }
            else if (value == "off") {
                option_verify_valid_table_increment = false;
            }
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "weighted_tabulation";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            if (value == "on") {
                option_weighted_tabulation = true;
            }
            else if (value == "off") {
                option_weighted_tabulation = false;
            }
            // remove processed option
            options.erase(iter);
        }
        // Find the one and only ModelTypeSymbol
        auto mts = ModelTypeSymbol::find();
        assert(mts);
        bool is_time_based = !mts->is_case_based();
        if (is_time_based && option_weighted_tabulation) {
            pp_error(mts->decl_loc, LT("error : weighted tabulation is not allowed with a time-based model, use population scaling instead."));
        }
    }

    {
        string key = "entity_member_packing";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            if (value == "on") {
                option_entity_member_packing = true;
            }
            else if (value == "off") {
                option_entity_member_packing = false;
            }
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "all_attributes_visible";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            if (value == "on") {
                option_all_attributes_visible = true;
            }
            else if (value == "off") {
                option_all_attributes_visible = false;
            }
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "microdata_output";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            if (value == "on") {
                option_microdata_output = true;
            }
            else if (value == "off") {
                option_microdata_output = false;
            }
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "microdata_output_warning";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            if (value == "on") {
                option_microdata_output_warning = true;
            }
            else if (value == "off") {
                option_microdata_output_warning = false;
            }
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "microdata_write_on_enter";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            if (value == "on") {
                option_microdata_write_on_enter = true;
            }
            else if (value == "off") {
                option_microdata_write_on_enter = false;
            }
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "microdata_write_on_exit";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            if (value == "on") {
                option_microdata_write_on_exit = true;
            }
            else if (value == "off") {
                option_microdata_write_on_exit = false;
            }
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "microdata_write_on_event";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            if (value == "on") {
                option_microdata_write_on_event = true;
            }
            else if (value == "off") {
                option_microdata_write_on_event = false;
            }
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "resource_use";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            if (value == "on") {
                option_resource_use = true;
            }
            else if (value == "off") {
                option_resource_use = false;
            }
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "use_heuristic_short_names";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            if (value == "on") {
                option_use_heuristic_short_names = true;
            }
            else if (value == "off") {
                option_use_heuristic_short_names = false;
            }
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "enable_heuristic_names_for_enumerators";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            if (value == "on") {
                option_enable_heuristic_names_for_enumerators = true;
            }
            else if (value == "off") {
                option_enable_heuristic_names_for_enumerators = false;
            }
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "convert_modgen_note_syntax";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            if (value == "on") {
                option_convert_modgen_note_syntax = true;
            }
            else if (value == "off") {
                option_convert_modgen_note_syntax = false;
            }
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "short_name_max_length";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            short_name_max_length = std::stoi(value);
            if (short_name_max_length < 8 || short_name_max_length > OM_NAME_DB_MAX) {
                pp_error(omc::location(), LT("error : '") + value + LT("' is invalid - must be between 8 and 255"));
            }
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "censor_event_time";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            if (value == "on") {
                option_censor_event_time = true;
            }
            else if (value == "off") {
                option_censor_event_time = false;
            }
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "ascii_infinity";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            if (value == "on") {
                option_ascii_infinity = true;
            }
            else if (value == "off") {
                option_ascii_infinity = false;
            }
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "missing_label_warning_enumeration";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            if (value == "on") {
                option_missing_label_warning_enumeration = true;
            }
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "missing_label_warning_parameter";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            if (value == "on") {
                option_missing_label_warning_parameter = true;
            }
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "missing_label_warning_table";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            if (value == "on") {
                option_missing_label_warning_table = true;
            }
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "missing_label_warning_attribute";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            if (value == "on") {
                option_missing_label_warning_attribute = true;
            }
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "missing_label_warning_published_enumeration";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            if (value == "on") {
                option_missing_label_warning_published_enumeration = true;
            }
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "missing_label_warning_published_parameter";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            if (value == "on") {
                option_missing_label_warning_published_parameter = true;
            }
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "missing_label_warning_published_table";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            if (value == "on") {
                option_missing_label_warning_published_table = true;
            }
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "missing_label_warning_published_attribute";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            if (value == "on") {
                option_missing_label_warning_published_attribute = true;
            }
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "missing_note_warning_published_parameter";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            if (value == "on") {
                option_missing_note_warning_published_parameter = true;
            }
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "missing_note_warning_published_table";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            if (value == "on") {
                option_missing_note_warning_published_table = true;
            }
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "missing_note_warning_published_attribute";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            if (value == "on") {
                option_missing_note_warning_published_attribute = true;
            }
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "missing_translated_label_warning_any";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            if (value == "on") {
                option_missing_translated_label_warning_any = true;
            }
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "missing_translated_note_warning_any";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            if (value == "on") {
                option_missing_translated_note_warning_any = true;
            }
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "missing_translated_label_warning_published_any";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            if (value == "on") {
                option_missing_translated_label_warning_published_any = true;
            }
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "missing_translated_note_warning_published_any";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            if (value == "on") {
                option_missing_translated_note_warning_published_any = true;
            }
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "missing_name_warning_classification";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            if (value == "on") {
                option_missing_name_warning_classification = true;
            }
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "missing_name_warning_parameter";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            if (value == "on") {
                option_missing_name_warning_parameter = true;
            }
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "missing_name_warning_table";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            if (value == "on") {
                option_missing_name_warning_table = true;
            }
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "missing_name_warning_published_classification";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            if (value == "on") {
                option_missing_name_warning_published_classification = true;
            }
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "missing_name_warning_published_parameter";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            if (value == "on") {
                option_missing_name_warning_published_parameter = true;
            }
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "missing_name_warning_published_table";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            if (value == "on") {
                option_missing_name_warning_published_table = true;
            }
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "generated_documentation";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            option_generated_documentation = (value == "on");
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "authored_documentation";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            option_authored_documentation = (value == "on");
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "symref_developer_edition";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            option_symref_developer_edition = (value == "on");
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "symref_unpublished_symbols";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            option_symref_unpublished_symbols = (value == "on");
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "symref_main_topic";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            option_symref_main_topic = (value == "on");
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "symref_model_symbol";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            option_symref_model_symbol = (value == "on");
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "symref_parameter_hierarchy";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            option_symref_parameter_hierarchy = (value == "on");
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "symref_table_hierarchy";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            option_symref_table_hierarchy = (value == "on");
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "symref_parameter_major_groups";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            option_symref_parameter_major_groups = (value == "on");
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "symref_table_major_groups";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            option_symref_table_major_groups = (value == "on");
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "symref_parameters_alphabetic";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            option_symref_parameters_alphabetic = (value == "on");
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "symref_tables_alphabetic";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            option_symref_tables_alphabetic = (value == "on");
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "symref_enumerations_alphabetic";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            option_symref_enumerations_alphabetic = (value == "on");
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "symref_parameter_topics";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            option_symref_parameter_topics = (value == "on");
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "symref_table_topics";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            option_symref_table_topics = (value == "on");
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "symref_enumeration_topics";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            option_symref_enumeration_topics = (value == "on");
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "symref_notes";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            option_symref_notes = (value == "on");
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "alternate_attribute_dependency_implementation";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            if (value == "on") {
                option_alternate_attribute_dependency_implementation = true;
            }
            else if (value == "off") {
                option_alternate_attribute_dependency_implementation = false;
            }
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "memory_popsize_parameter";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            option_memory_popsize_parameter = value;
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "memory_MB_constant_per_instance";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            // store in the unique ModelSymbol object
            ModelSymbol* model_symbol = dynamic_cast<ModelSymbol*>(Symbol::find_a_symbol(typeid(ModelSymbol)));
            assert(model_symbol);
            model_symbol->memory_MB_constant_per_instance = std::stoi(value);
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "memory_MB_constant_per_sub";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            // store in the unique ModelSymbol object
            ModelSymbol* model_symbol = dynamic_cast<ModelSymbol*>(Symbol::find_a_symbol(typeid(ModelSymbol)));
            assert(model_symbol);
            model_symbol->memory_MB_constant_per_sub = std::stoi(value);
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "memory_MB_popsize_coefficient";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            // store in the unique ModelSymbol object
            ModelSymbol* model_symbol = dynamic_cast<ModelSymbol*>(Symbol::find_a_symbol(typeid(ModelSymbol)));
            assert(model_symbol);
            model_symbol->memory_MB_popsize_coefficient = std::stod(value);
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "memory_safety_factor";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            // store in the unique ModelSymbol object
            ModelSymbol* model_symbol = dynamic_cast<ModelSymbol*>(Symbol::find_a_symbol(typeid(ModelSymbol)));
            assert(model_symbol);
            model_symbol->memory_safety_factor = std::stod(value);
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "packing_level";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_name = iter->first;
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& opt_value = opt_pair.first;
            auto& opt_loc = opt_pair.second;
            // Ignore this Modgen option.
            pp_logmsg(opt_loc, LT("note : ignoring Modgen option '") + key + "'");
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "index_errors";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_name = iter->first;
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& opt_value = opt_pair.first;
            auto& opt_loc = opt_pair.second;
            // Ignore this Modgen option.
            pp_logmsg(opt_loc, LT("note : ignoring Modgen option '") + key + "'");
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "fp_consistency";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_name = iter->first;
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& opt_value = opt_pair.first;
            auto& opt_loc = opt_pair.second;
            // Ignore this Modgen option.
            pp_logmsg(opt_loc, LT("note : ignoring Modgen option '") + key + "'");
            // remove processed option
            options.erase(iter);
        }
    }

    {
        string key = "local_random_streams";
        auto iter = options.find(key);
        if (iter != options.end()) {
            // Can't validate or process this option here
            // because pp_all_entities has not been created by post_parse_all
            // Note: multiple entries allowed, unlike other options.
            // See EntitySymbol::post_parse()
        }
    }

    {
        // Handle option measures_method

        // First, set default measures method as a function of model type.
        // Determine if model is case-based or time-based
        auto mt = ModelTypeSymbol::find();
        assert(mt); // logic guarantee
        bool is_case_based = mt->is_case_based();

        if (is_case_based) {
            // case-based models, by default, aggregate accumulators across simulation members before evaluating the expression
            measures_are_aggregated = true;
        }
        else {
            // time-based models, by default, compute the average of the expression across simulation members
            measures_are_aggregated = false;
        }

        // Override default aggregation method if measures_method option was specified.
        string key = "measures_method";
        auto iter = options.find(key);
        if (iter != options.end()) {
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& value = opt_pair.first;
            auto& loc = opt_pair.second;
            if (value == "aggregate") {
                measures_are_aggregated = true;
            }
            else if (value == "average") {
                measures_are_aggregated = false;
            }
            else {
                pp_error(loc, LT("error : '") + value + LT("' is invalid - measures_method must be either aggregate or average"));
            }
            // remove processed option
            options.erase(iter);
        }
    }
}

void Symbol::do_unrecognized_options()
{
    // Detect and raise error for any unprocessed options.
    // If any, they are probably a mistyped option in model code.
    // So, raise an error for each.
    if (options.size() > 0) {
        for (auto iter = options.begin(); iter != options.end(); ++iter) {
            auto& opt_name = iter->first;
            auto& opt_pair = iter->second; // opt_pair is option value, option location
            string& opt_value = opt_pair.first;
            auto& opt_loc = opt_pair.second;
            pp_error(opt_loc, LT("error : unrecognized option '") + opt_name + "'");
        }
    }
}