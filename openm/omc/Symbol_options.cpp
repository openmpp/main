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
unordered_multimap<string, string> Symbol::options;
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
bool Symbol::option_missing_translated_label_warning_published_any = false;
bool Symbol::option_missing_translated_note_warning_published_any = false;
bool Symbol::option_alternate_attribute_dependency_implementation = false;
string Symbol::option_memory_popsize_parameter;

void Symbol::do_options()
{
    {
        string key = "event_trace";
        auto iter = options.find(key);
        if (iter != options.end()) {
            string value = iter->second;
            if (value == "on") {
                option_event_trace = true;
            }
            else if (value == "off") {
                option_event_trace = false;
            }
        }
    }

    {
        string key = "event_trace_warning";
        auto iter = options.find(key);
        if (iter != options.end()) {
            string value = iter->second;
            if (value == "on") {
                option_event_trace_warning = true;
            }
            else if (value == "off") {
                option_event_trace_warning = false;
            }
        }
    }

    {
        string key = "bounds_errors";
        auto iter = options.find(key);
        if (iter != options.end()) {
            string value = iter->second;
            if (value == "on") {
                option_bounds_errors = true;
            }
            else if (value == "off") {
                option_bounds_errors = false;
            }
        }
    }

    {
        string key = "case_checksum";
        auto iter = options.find(key);
        if (iter != options.end()) {
            string value = iter->second;
            if (value == "on") {
                option_case_checksum = true;
            }
            else if (value == "off") {
                option_case_checksum = false;
            }
        }
    }

    {
        string key = "allow_time_travel";
        auto iter = options.find(key);
        if (iter != options.end()) {
            string value = iter->second;
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
        }
    }

    {
        string key = "allow_clairvoyance";
        auto iter = options.find(key);
        if (iter != options.end()) {
            string value = iter->second;
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
        }
    }

    {
        string key = "time_infinite_is_32767";
        auto iter = options.find(key);
        if (iter != options.end()) {
            string value = iter->second;
            if (value == "on") {
                option_time_infinite_is_32767 = true;
            }
            else if (value == "off") {
                option_time_infinite_is_32767 = false;
            }
        }
    }

    {
        string key = "time_undef_is_minus_one";
        auto iter = options.find(key);
        if (iter != options.end()) {
            string value = iter->second;
            if (value == "on") {
                option_time_undef_is_minus_one = true;
            }
            else if (value == "off") {
                option_time_undef_is_minus_one = false;
            }
        }
    }

    {
        string key = "verify_attribute_modification";
        auto iter = options.find(key);
        if (iter != options.end()) {
            string value = iter->second;
            if (value == "on") {
                option_verify_attribute_modification = true;
            }
            else if (value == "off") {
                option_verify_attribute_modification = false;
            }
        }
    }

    {
        string key = "verify_timelike_attribute_access";
        auto iter = options.find(key);
        if (iter != options.end()) {
            string value = iter->second;
            if (value == "on") {
                option_verify_timelike_attribute_access = true;
            }
            else if (value == "off") {
                option_verify_timelike_attribute_access = false;
            }
        }
    }

    {
        string key = "verify_valid_table_increment";
        auto iter = options.find(key);
        if (iter != options.end()) {
            string value = iter->second;
            if (value == "on") {
                option_verify_valid_table_increment = true;
            }
            else if (value == "off") {
                option_verify_valid_table_increment = false;
            }
        }
    }

    {
        string key = "weighted_tabulation";
        auto iter = options.find(key);
        if (iter != options.end()) {
            string value = iter->second;
            if (value == "on") {
                option_weighted_tabulation = true;
            }
            else if (value == "off") {
                option_weighted_tabulation = false;
            }
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
            string value = iter->second;
            if (value == "on") {
                option_entity_member_packing = true;
            }
            else if (value == "off") {
                option_entity_member_packing = false;
            }
        }
    }

    {
        string key = "all_attributes_visible";
        auto iter = options.find(key);
        if (iter != options.end()) {
            string value = iter->second;
            if (value == "on") {
                option_all_attributes_visible = true;
            }
            else if (value == "off") {
                option_all_attributes_visible = false;
            }
        }
    }

    {
        string key = "microdata_output";
        auto iter = options.find(key);
        if (iter != options.end()) {
            string value = iter->second;
            if (value == "on") {
                option_microdata_output = true;
            }
            else if (value == "off") {
                option_microdata_output = false;
            }
        }
    }

    {
        string key = "microdata_output_warning";
        auto iter = options.find(key);
        if (iter != options.end()) {
            string value = iter->second;
            if (value == "on") {
                option_microdata_output_warning = true;
            }
            else if (value == "off") {
                option_microdata_output_warning = false;
            }
        }
    }

    {
        string key = "microdata_write_on_enter";
        auto iter = options.find(key);
        if (iter != options.end()) {
            string value = iter->second;
            if (value == "on") {
                option_microdata_write_on_enter = true;
            }
            else if (value == "off") {
                option_microdata_write_on_enter = false;
            }
        }
    }

    {
        string key = "microdata_write_on_exit";
        auto iter = options.find(key);
        if (iter != options.end()) {
            string value = iter->second;
            if (value == "on") {
                option_microdata_write_on_exit = true;
            }
            else if (value == "off") {
                option_microdata_write_on_exit = false;
            }
        }
    }

    {
        string key = "microdata_write_on_event";
        auto iter = options.find(key);
        if (iter != options.end()) {
            string value = iter->second;
            if (value == "on") {
                option_microdata_write_on_event = true;
            }
            else if (value == "off") {
                option_microdata_write_on_event = false;
            }
        }
    }

    {
        string key = "resource_use";
        auto iter = options.find(key);
        if (iter != options.end()) {
            string value = iter->second;
            if (value == "on") {
                option_resource_use = true;
            }
            else if (value == "off") {
                option_resource_use = false;
            }
        }
    }

    {
        string key = "use_heuristic_short_names";
        auto iter = options.find(key);
        if (iter != options.end()) {
            string value = iter->second;
            if (value == "on") {
                option_use_heuristic_short_names = true;
            }
            else if (value == "off") {
                option_use_heuristic_short_names = false;
            }
        }
    }

    {
        string key = "enable_heuristic_names_for_enumerators";
        auto iter = options.find(key);
        if (iter != options.end()) {
            string value = iter->second;
            if (value == "on") {
                option_enable_heuristic_names_for_enumerators = true;
            }
            else if (value == "off") {
                option_enable_heuristic_names_for_enumerators = false;
            }
        }
    }

    {
        string key = "convert_modgen_note_syntax";
        auto iter = options.find(key);
        if (iter != options.end()) {
            string value = iter->second;
            if (value == "on") {
                option_convert_modgen_note_syntax = true;
            }
            else if (value == "off") {
                option_convert_modgen_note_syntax = false;
            }
        }
    }

    {
        string key = "short_name_max_length";
        auto iter = options.find(key);
        if (iter != options.end()) {
            string value = iter->second;
            short_name_max_length = std::stoi(value);
            if (short_name_max_length < 8 || short_name_max_length > OM_NAME_DB_MAX) {
                pp_error(omc::location(), LT("error : '") + value + LT("' is invalid - must be between 8 and 255"));
            }
        }
    }

    {
        string key = "censor_event_time";
        auto iter = options.find(key);
        if (iter != options.end()) {
            string value = iter->second;
            if (value == "on") {
                option_censor_event_time = true;
            }
            else if (value == "off") {
                option_censor_event_time = false;
            }
        }
    }

    {
        string key = "ascii_infinity";
        auto iter = options.find(key);
        if (iter != options.end()) {
            string value = iter->second;
            if (value == "on") {
                option_ascii_infinity = true;
            }
            else if (value == "off") {
                option_ascii_infinity = false;
            }
        }
    }

    {
        string key = "missing_label_warning_enumeration";
        auto iter = options.find(key);
        if (iter != options.end()) {
            string value = iter->second;
            if (value == "on") {
                option_missing_label_warning_enumeration = true;
            }
        }
    }

    {
        string key = "missing_label_warning_parameter";
        auto iter = options.find(key);
        if (iter != options.end()) {
            string value = iter->second;
            if (value == "on") {
                option_missing_label_warning_parameter = true;
            }
        }
    }

    {
        string key = "missing_label_warning_table";
        auto iter = options.find(key);
        if (iter != options.end()) {
            string value = iter->second;
            if (value == "on") {
                option_missing_label_warning_table = true;
            }
        }
    }

    {
        string key = "missing_label_warning_attribute";
        auto iter = options.find(key);
        if (iter != options.end()) {
            string value = iter->second;
            if (value == "on") {
                option_missing_label_warning_attribute = true;
            }
        }
    }

    {
        string key = "missing_label_warning_published_enumeration";
        auto iter = options.find(key);
        if (iter != options.end()) {
            string value = iter->second;
            if (value == "on") {
                option_missing_label_warning_published_enumeration = true;
            }
        }
    }

    {
        string key = "missing_label_warning_published_parameter";
        auto iter = options.find(key);
        if (iter != options.end()) {
            string value = iter->second;
            if (value == "on") {
                option_missing_label_warning_published_parameter = true;
            }
        }
    }

    {
        string key = "missing_label_warning_published_table";
        auto iter = options.find(key);
        if (iter != options.end()) {
            string value = iter->second;
            if (value == "on") {
                option_missing_label_warning_published_table = true;
            }
        }
    }

    {
        string key = "missing_label_warning_published_attribute";
        auto iter = options.find(key);
        if (iter != options.end()) {
            string value = iter->second;
            if (value == "on") {
                option_missing_label_warning_published_attribute = true;
            }
        }
    }

    {
        string key = "missing_note_warning_published_parameter";
        auto iter = options.find(key);
        if (iter != options.end()) {
            string value = iter->second;
            if (value == "on") {
                option_missing_note_warning_published_parameter = true;
            }
        }
    }

    {
        string key = "missing_note_warning_published_table";
        auto iter = options.find(key);
        if (iter != options.end()) {
            string value = iter->second;
            if (value == "on") {
                option_missing_note_warning_published_table = true;
            }
        }
    }

    {
        string key = "missing_note_warning_published_attribute";
        auto iter = options.find(key);
        if (iter != options.end()) {
            string value = iter->second;
            if (value == "on") {
                option_missing_note_warning_published_attribute = true;
            }
        }
    }

    {
        string key = "missing_translated_label_warning_published_any";
        auto iter = options.find(key);
        if (iter != options.end()) {
            string value = iter->second;
            if (value == "on") {
                option_missing_translated_label_warning_published_any = true;
            }
        }
    }

    {
        string key = "missing_translated_note_warning_published_any";
        auto iter = options.find(key);
        if (iter != options.end()) {
            string value = iter->second;
            if (value == "on") {
                option_missing_translated_note_warning_published_any = true;
            }
        }
    }

    {
        string key = "alternate_attribute_dependency_implementation";
        auto iter = options.find(key);
        if (iter != options.end()) {
            string value = iter->second;
            if (value == "on") {
                option_alternate_attribute_dependency_implementation = true;
            }
            else if (value == "off") {
                option_alternate_attribute_dependency_implementation = false;
            }
        }
    }

    {
        string key = "memory_popsize_parameter";
        auto iter = options.find(key);
        if (iter != options.end()) {
            string value = iter->second;
            option_memory_popsize_parameter = value;
        }
    }

    {
        string key = "memory_MB_constant_per_instance";
        auto iter = options.find(key);
        if (iter != options.end()) {
            string value = iter->second;
            // store in the unique ModelSymbol object
            ModelSymbol* model_symbol = dynamic_cast<ModelSymbol*>(Symbol::find_a_symbol(typeid(ModelSymbol)));
            assert(model_symbol);
            model_symbol->memory_MB_constant_per_instance = std::stoi(value);
        }
    }

    {
        string key = "memory_MB_constant_per_sub";
        auto iter = options.find(key);
        if (iter != options.end()) {
            string value = iter->second;
            // store in the unique ModelSymbol object
            ModelSymbol* model_symbol = dynamic_cast<ModelSymbol*>(Symbol::find_a_symbol(typeid(ModelSymbol)));
            assert(model_symbol);
            model_symbol->memory_MB_constant_per_sub = std::stoi(value);
        }
    }

    {
        string key = "memory_MB_popsize_coefficient";
        auto iter = options.find(key);
        if (iter != options.end()) {
            string value = iter->second;
            // store in the unique ModelSymbol object
            ModelSymbol* model_symbol = dynamic_cast<ModelSymbol*>(Symbol::find_a_symbol(typeid(ModelSymbol)));
            assert(model_symbol);
            model_symbol->memory_MB_popsize_coefficient = std::stod(value);
        }
    }

    {
        string key = "memory_safety_factor";
        auto iter = options.find(key);
        if (iter != options.end()) {
            string value = iter->second;
            // store in the unique ModelSymbol object
            ModelSymbol* model_symbol = dynamic_cast<ModelSymbol*>(Symbol::find_a_symbol(typeid(ModelSymbol)));
            assert(model_symbol);
            model_symbol->memory_safety_factor = std::stod(value);
        }
    }

    {
        string key = "local_random_streams";
        auto iter = options.find(key);
        if (iter != options.end()) {
            // Can't validate or process this option here
            // because pp_all_entities has not bben created by post_parse_all
            // Note: multiple entries allowed, unlike other options.
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
            string value = iter->second;
            if (value == "aggregate") {
                measures_are_aggregated = true;
            }
            else if (value == "average") {
                measures_are_aggregated = false;
            }
            else {
                pp_error(omc::location(), LT("error : '") + value + LT("' is invalid - measures_method must be either aggregate or average"));
            }
        }
    }
}
