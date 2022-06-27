/**
 * @file    CodeGen.cpp
 * Implements the code generation class.
 */
// Copyright (c) 2013-2022 OpenM++ Contributors
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <iostream>
#include <fstream>
#include <algorithm>
#include <utility>
#include <typeinfo>
#include "libopenm/db/modelBuilder.h"
#include "CodeGen.h"
#include "CodeBlock.h"
#include "ast.h"

using namespace std;
using namespace openm;

void CodeGen::do_all()
{
	do_preamble();

	do_types();
    do_weight_support();
    do_aggregations();
	do_parameters();
	do_entity_tables();
	do_derived_tables();
    do_imports();
    do_groups();
    do_table_interface();
    do_table_dependencies();
    do_agents();
	do_entity_sets();
    do_event_queue();
    do_event_names();

    h += "void StartSimulation(int id);";
    h += "void EndSimulation();";

    do_RunOnce();
    do_RunInit();
    do_ModelStartup();
    do_RunModel();
    do_ModelShutdown();
    do_API_entries();
    do_ParameterNameSize();
    if (!Symbol::pp_all_strings.empty()) do_model_strings();

    // set meta row values and calculate metadata digests: model, types, parameters, output tables
    modelBuilder->setModelMetaRows(metaRows);

    do_name_digest();   // insert common header, model name and digest in all files

    *oat0 << t0;
    *oat1 << t1;
    *oah << h;
    *oac << c;
    *oaz << z;
}

void CodeGen::do_preamble()
{
    // Get the model symbol
    ModelSymbol *model_symbol = dynamic_cast<ModelSymbol *>(Symbol::find_a_symbol(typeid(ModelSymbol)));
    assert(model_symbol);
    model_symbol->populate_metadata(metaRows);

    // Get the model_type symbol
    auto *model_type_symbol = ModelTypeSymbol::find();
    assert(model_type_symbol);
    model_type_symbol->populate_metadata(metaRows);

    // Get the version symbol
    VersionSymbol *version_symbol = dynamic_cast<VersionSymbol *>(Symbol::find_a_symbol(typeid(VersionSymbol)));
    assert(version_symbol);
    version_symbol->populate_metadata(metaRows);

    // populate metadata for model languages
    for (auto lang : Symbol::pp_all_languages) {
        lang->populate_metadata(metaRows);
    }

    // om_types0.h
    t0 += "";
    t0 += "#pragma once";
    t0 += "#include <cmath>";
    t0 += "#include <string>";
    t0 += "#include \"omc/fixed_precision.h\"";
    t0 += "";

    // om_types1.h
    t1 += "" ;
	t1 += "#pragma once";
    t1 += "#include \"om_types0.h\"";
    t1 += "#include \"omc/Range.h\"";
    t1 += "#include \"omc/Partition.h\"";
    t1 += "#include \"omc/Classification.h\"";
    t1 += "#include \"omc/entity_ptr.h\"";
    t1 += "";

    // om_declarations.h
    h += "" ;
	h += "#pragma once";
    h += "";

    h += "// PreSimulation, PostSimulation, UserTables functions in model source code";
    for (auto sg : {&Symbol::pre_simulation, &Symbol::post_simulation, &Symbol::derived_tables}) {
        for (auto suffix : sg->suffixes) {
            h += "extern void " + sg->prefix + suffix + "();";
        }
        for (size_t id = 0; id < sg->ambiguous_count; ++id) {
            h += "extern void " + sg->disambiguated_name(id) + "();";
        }
    }
    h += "";

    // om_definitions.cpp
	c += "";
    c += "#include \"omc/omPch.h\"";
    c += "#include \"omc/omSimulation.h\"";
    c += "#include <typeinfo>";
    c += "";

    // control warnings produced in generated code
    c += "#if defined(_MSC_VER)";
    c += "    // compiler-specific pragmas for MSVC";
    c += "#endif";
    c += "#if defined(__GNUC__)";
    c += "    // compiler-specific pragmas for gcc";
    c += "    #pragma GCC diagnostic ignored \"-Wunused-variable\"";
    c += "    #pragma GCC diagnostic ignored \"-Wunused-but-set-variable\"";
    c += "#endif";

    // Get definition code associated with the model_type symbol
    c += model_type_symbol->cxx_definition_global();
    c += "";

    // Get definition code associated with the model symbol (name, time stamp)
    c += model_symbol->cxx_definition_global();
    c += "";

	c += "using namespace openm;";
	c += "";

    if (Symbol::option_bounds_errors) {
        t0 += doxygen_short("Model was built with bounds_errors = on.");
        t0 += "constexpr bool om_bounds_errors = true;";
        t0 += "";
    }
    else {
        t0 += doxygen_short("Model was built with bounds_errors = off.");
        t0 += "constexpr bool om_bounds_errors = false;";
        t0 += "";
    }

    if (Symbol::option_censor_event_time) {
        t0 += doxygen_short("Model was built with censor_event_time = on.");
        t0 += "constexpr bool om_censor_event_time_on = true;";
        t0 += "";
    }
    else {
        t0 += doxygen_short("Model was built with censor_event_time = off.");
        t0 += "constexpr bool om_censor_event_time_on = false;";
        t0 += "";
    }

    if (Symbol::option_weighted_tabulation) {
        t0 += doxygen_short("Model was built with weighted_tabulation = on.");
        t0 += "constexpr bool om_weighted_tabulation_on = true;";
        t0 += "";
    }
    else {
        t0 += doxygen_short("Model was built with weighted_tabulation = off.");
        t0 += "constexpr bool om_weighted_tabulation_on = false;";
        t0 += "";
    }

    if (Symbol::option_resource_use) {
        t0 += doxygen_short("Model was built with resource_use = on.");
        t0 += "constexpr bool om_resource_use_on = true;";
        t0 += "";
    }
    else {
        t0 += doxygen_short("Model was built with resource_use = off.");
        t0 += "constexpr bool om_resource_use_on = false;";
        t0 += "";
    }

    if (Symbol::option_event_trace) {
        t0 += doxygen_short("Model was built with event trace capability.");
        t0 += "constexpr bool om_event_trace_capable = true;";
        t0 += "";
        c += "// Model has event trace capability. Event tracing is active unless turned off in model code";
        c += "thread_local bool BaseEntity::event_trace_on = true;";
    }
    else {
        t0 += doxygen_short("Model was not built with event trace capability.");
        t0 += "constexpr bool om_event_trace_capable = false;";
        t0 += "";
        c += "// Model does not have event trace capability. Nevertheless, this static member requires a definition.";
        c += "thread_local bool BaseEntity::event_trace_on = false;";
    }

    // static members for event trace control
    c += "enum BaseEntity::et_report_style BaseEntity::event_trace_report_style = BaseEntity::et_report_style::eModgen;";
    c += "bool BaseEntity::event_trace_show_queued_events = true;";
    c += "bool BaseEntity::event_trace_show_queued_self_scheduling_events = true;";
    c += "bool BaseEntity::event_trace_show_enter_simulation = true;";
    c += "bool BaseEntity::event_trace_show_exit_simulation = true;";
    c += "bool BaseEntity::event_trace_show_self_scheduling_events = true;";
    c += "double BaseEntity::event_trace_minimum_time = -std::numeric_limits<double>::infinity();";
    c += "double BaseEntity::event_trace_maximum_time = std::numeric_limits<double>::infinity();";
    c += "std::unordered_set<int> BaseEntity::event_trace_selected_entities;";
    c += "std::unordered_set<int> BaseEntity::event_trace_selected_events;";
    c += "int BaseEntity::event_trace_maximum_lines = 100000;";
    c += "";

    // Let the run-time know whether to generate a running checksum for events
    if (Symbol::option_case_checksum) {
        c += "const bool BaseEvent::event_checksum_enabled = true;";
    }
    else {
        c += "const bool BaseEvent::event_checksum_enabled = false;";
    }
    // The checksum must be defined in any case
    c += "thread_local double BaseEvent::event_checksum_value = 0.0;";

    if (Symbol::option_allow_time_travel) {
        // Let the run-time know that time travel is allowed
        c += "const bool BaseEvent::allow_time_travel = true;";
    }
    else {
        c += "const bool BaseEvent::allow_time_travel = false;";
    }

    if (Symbol::option_allow_clairvoyance) {
        // Let the run-time know that clairvoyance is allowed
        c += "const bool BaseEvent::allow_clairvoyance = true;";
    }
    else {
        c += "const bool BaseEvent::allow_clairvoyance = false;";
    }

    c += "// allow or disallow access to null entity through null pointer";
    c += "thread_local bool BaseEntity::om_access_to_null_entity = false;";

    if (Symbol::option_verify_attribute_modification) {
        t0 += doxygen_short("Model was built with verification of attribute modification.");
        t0 += "constexpr bool om_verify_attribute_modification = true;";
        t0 += "";
        c += "// Model runtime prohibits attribute modification in event time functions using the following flag.";
        c += "thread_local bool BaseEntity::om_permit_attribute_modification  = true;";
    }
    else {
        t0 += doxygen_short("Model was built without verification of attribute modification.");
        t0 += "constexpr bool om_verify_attribute_modification = false;";
        t0 += "";
        c += "// Model runtime does not prohibit attribute modification in event time functions, but still requires that the following flag be defined.";
        c += "thread_local bool BaseEntity::om_permit_attribute_modification  = true;";
    }
    c += "";

    if (Symbol::option_verify_timelike_attribute_access) {
        t0 += doxygen_short("Model was built with verification of time-like attribute access.");
        t0 += "constexpr bool om_verify_timelike_attribute_access = true;";
        t0 += "";
        c += "// Model runtime prohibits access to time-like attributes in event time functions using the following flag.";
        c += "thread_local bool BaseEntity::om_permit_timelike_attribute_access = true;";
    }
    else {
        t0 += doxygen_short("Model was built without verification of time-like attribute access.");
        t0 += "constexpr bool om_verify_timelike_attribute_access = false;";
        t0 += "";
        c += "// Model runtime does not prohibit access to time-like attributes in event time functions, but still requires that the following flag be defined.";
        c += "thread_local bool BaseEntity::om_permit_timelike_attribute_access = true;";
    }
    c += "";

    // om_fixed_parms.cpp
	z += "";
    z += "#include \"omc/cumrate.h\"";
    z += "#include \"om_types1.h\"";
	z += "";
    z += "const double QNAN_F = std::numeric_limits<float>::quiet_NaN();";
    z += "const double QNAN_D = std::numeric_limits<double>::quiet_NaN();";
    z += "const double QNAN_LD = std::numeric_limits<long double>::quiet_NaN();";
    z += "";
}

// insert commomn headers in all files and define model name and digest
void CodeGen::do_name_digest(void)
{
    // common elements of all file headers
    string model_str = " for model '" + metaRows.modelDic.name + "' - Generated by openM++ compiler";
    string version_str = " Model version = " + metaRows.modelDic.version;
    string created_str = " Model created = " + metaRows.modelDic.createDateTime;
    string digest_str = " Model digest = " + metaRows.modelDic.digest;

    // om_types0.h
    t0.push_header(doxygen(
        "@file   om_types0.h",
        "Fundamental types and constexpr" + model_str,
        " ",
        version_str,
        created_str,
        digest_str
    ));

    // om_types1.h
    t1.push_header(doxygen(
        "@file   om_types1.h",
        "Types" + model_str,
        " ",
        version_str,
        created_str,
        digest_str
    ));

    // om_declarations.h
    h.push_header(doxygen(
        "Declarations" + model_str,
        " ",
        version_str,
        created_str,
        digest_str
    ));

    // om_definitions.cpp
    c.push_header(doxygen(
        "@file   om_definitions.cpp",
        "Definitions" + model_str,
        " ",
        version_str,
        created_str,
        digest_str
    ));

    // define model name and digest
    c += "// model name";
    c += "const char * OM_MODEL_NAME = \"" + metaRows.modelDic.name + "\";";
    c += "";
    c += "// model metadata digest";
    c += "const char * OM_MODEL_DIGEST = \"" + metaRows.modelDic.digest + "\";";

    // om_fixed_parms.cpp
    z.push_header(doxygen(
        "@file   om_fixed_parms.cpp",
        "Definitions for fixed parameters" + model_str,
        " ",
        version_str,
        created_str,
        digest_str
    ));
}

void CodeGen::do_model_strings()
{
    c += "// implementation of Modgen API for model-specific strings";
    c += "const char * ModelString(const char * string_name)";
    c += "{";
    c += "static std::map<std::string, std::string> string_map {";
    for (int j = 0; j < LanguageSymbol::number_of_languages(); j++) {
        auto lang_sym = LanguageSymbol::id_to_sym[j];
        auto lang_name = lang_sym->name;
        c += "";
        c += "// strings for " + lang_name;
        for (auto modgen_string : Symbol::pp_all_strings) {
            c += "{"
                  "\"" + normalizeLanguageName(lang_name) + "@" + modgen_string->name + "\""
                  ", "
                  "\"" + modgen_string->pp_labels[j] + "\""
                  "},";
        }
    }
    c += "};";
    c += "";
    c += "// get list of user prefered languages, if user language == en_CA.UTF-8 then list is: (en-ca, en)";
    c += "static std::list<std::string> langLst = theLog->getLanguages();";
    c += "";
    c += "for (const std::string & lang : langLst) {";
    c += "auto search = string_map.find(lang + \"@\" + string_name);";
    c += "if (search != string_map.end()) {";
    c += "return search->second.c_str();";
    c += "}";
    c += "}";
    c += "";
    c += "return \"\"; // string not found";
    c += "}";
    c += "";
}

void CodeGen::do_types()
{
    // om_types0.h - fundamental types declaration
    t0 += "// fundamental types";
    for (auto type : Symbol::pp_all_types0) {
        t0 += type->cxx_declaration_global();
    }
    t0 += "";

    // om_types1.h - templated types declaration
    t1 += "// templated types";
    for (auto type : Symbol::pp_all_types1) {
        t1 += type->cxx_declaration_global();
    }
    t1 += "";

    // om_definitions.cpp - types definitions
    c += "// fundamental types";
    for (auto type : Symbol::pp_all_types0) {
        c += type->cxx_definition_global();
    }
    c += "";

    c += "// templated types";
    for (auto type : Symbol::pp_all_types1) {
        c += type->cxx_definition_global();
    }
    c += "";

    // populate meta-data for types
    for (auto type : Symbol::pp_all_types0) {
        type->populate_metadata(metaRows);
    }
    for (auto type : Symbol::pp_all_types1) {
        type->populate_metadata(metaRows);
    }
}

void CodeGen::do_aggregations()
{
    if (Symbol::pp_all_aggregations.empty()) return;

    // parameter declarations
	h += "// model aggregations";
	c += "// model aggregations";
    for ( auto aggregation : Symbol::pp_all_aggregations ) {
        h += aggregation->cxx_declaration_global();
        c += aggregation->cxx_definition_global();
    }
	h += "";
	c += "";
}

void CodeGen::do_weight_support()
{
    h += "// weight support";
    h += "";
    h += "extern thread_local double om_initial_weight;";
    h += "extern thread_local double om_sum_weight;";
    h += "";
    h += doxygen_short("set initial weight");
    h += "void set_initial_weight(double w);";
    h += "";
    h += doxygen_short("get initial weight");
    h += "double get_initial_weight(void);";
    h += "";
    h += doxygen_short("get sum of weights");
    h += "double get_sum_weight(void);";
    h += "";
    h += doxygen_short("reset weight API");
    h += "void reset_weight(void);";
    h += "";

    c += "// weight support";
    c += "";
    c += "thread_local double om_initial_weight = 1.0;";
    c += "thread_local double om_sum_weight = 0.0;";
    c += "void set_initial_weight(double w)";
    c += "{";
    if (Symbol::option_weighted_tabulation) {
        c += "om_initial_weight = w;";
    }
    else {
        c += "// weighted tabulation is off in this model, so ignore argument";
        c += "om_initial_weight = 1.0;";
    }
    c +=     "om_sum_weight += om_initial_weight;";
    c += "}";
    c += "";
    c += "double get_initial_weight()";
    c += "{";
    c +=     "return om_initial_weight;";
    c += "}";
    c += "";
    c += "double get_sum_weight()";
    c += "{";
    c +=     "return om_sum_weight;";
    c += "}";
    c += "";
    c += "void reset_weight_api()";
    c += "{";
    c +=     "om_initial_weight = 1.0;";
    c +=     "om_sum_weight += 0.0;";
    c += "}";
    c += "";
}

void CodeGen::do_parameters()
{
    // parameter declarations
    h += "// model scenario parameters";
    h += "";
    h += "#ifdef OM_DEBUG_PARAMETERS";
    h += "";
    for ( auto parameter : Symbol::pp_all_parameters ) {
        if (parameter->source == ParameterSymbol::scenario_parameter) {
            h += parameter->cxx_declaration_global_scenario_debug();
        }
    }
    h += "";
    h += "#else // OM_DEBUG_PARAMETERS";
    h += "";
    for (auto parameter : Symbol::pp_all_parameters) {
        if (parameter->source == ParameterSymbol::scenario_parameter) {
            h += parameter->cxx_declaration_global_scenario_release();
        }
    }
    h += "";
    h += "#endif // OM_DEBUG_PARAMETERS";
    h += "";
    
    h += "// model parameters (fixed, derived, missing)";
    h += "";
    for (auto parameter : Symbol::pp_all_parameters) {
        if (parameter->source != ParameterSymbol::scenario_parameter) {
            h += parameter->cxx_declaration_global();
        }
    }
    h += "";

    for (auto parameter : Symbol::pp_all_parameters) {
        if (parameter->cumrate) {
            h += parameter->lookup_fn->cxx_declaration_global();
        }
    }
    h += "";
    
    // parameter definitions & initializers
	c += "// model scenario parameters";
    c += "";
    c += "#ifdef OM_DEBUG_PARAMETERS";

    for (auto parameter : Symbol::pp_all_parameters) {
        if (parameter->source == ParameterSymbol::scenario_parameter) {
            c += "";
            c += parameter->cxx_definition_global_scenario_debug();
        }
    }
    c += "";
    c += "#else // OM_DEBUG_PARAMETERS";

    for (auto parameter : Symbol::pp_all_parameters) {
        if (parameter->source == ParameterSymbol::scenario_parameter) {
            c += "";
            c += parameter->cxx_definition_global_scenario_release();
        }
    }
    c += "";
    c += "#endif // OM_DEBUG_PARAMETERS";
    c += "";

    c += "// model parameters (derived, missing)";
    z += "// model parameters (fixed)";
    for (auto parameter : Symbol::pp_all_parameters) {
        if (parameter->source == ParameterSymbol::fixed_parameter) {
            // place definition (with initializer) in the cpp module for fixed parameters
            z += parameter->cxx_definition_global();
        }
        if (parameter->source == ParameterSymbol::derived_parameter || parameter->source == ParameterSymbol::missing_parameter) {
            c += "";
            c += parameter->cxx_definition_global();
        }
    }
    c += "";

    for (auto parameter : Symbol::pp_all_parameters) {
        if (parameter->cumrate) {
            c += parameter->cxx_definition_cumrate();
            c += parameter->lookup_fn->cxx_definition_global();
        }
    }
    c += "";

    // populate meta-data for parameters
    for ( auto parameter : Symbol::pp_all_parameters ) {
        parameter->populate_metadata(metaRows);
    }
}

void CodeGen::do_RunOnce()
{
    c += "// Model one-time initialization";
    c += "void RunOnce(IRunBase * const i_runBase)";
    c += "{";
    c += "// report model build environment";
    c += "theLog->logFormatted(\"Model build    : % s % s % s\", omr::modelTargetOsName, omr::modelTargetConfigName, omr::modelTargetMpiUseName);";
    c += "";
    c += "theLog->logMsg(\"Prepare fixed and missing parameters\");";
    // Missing parameters are handled like fixed parameters.
    bool any_missing_parameters = false;
    for (auto parameter : Symbol::pp_all_parameters) {
        // Process only fixed and missing parameters in this for loop
        // These parameters are shared by all simulation threads.
        if (parameter->source != ParameterSymbol::fixed_parameter && parameter->source != ParameterSymbol::missing_parameter) continue;

        // Create contents of helpful .dat format file specifying missing parameters.
        if (parameter->source == ParameterSymbol::missing_parameter) {
            if (!any_missing_parameters) {
                m += "parameters {";
                any_missing_parameters = true;
            }
            m += parameter->dat_definition();
            // create warning message which the model will output whenever it is run
            c += "theLog->logFormatted(\"model : warning : parameter " + parameter->name + " was missing when model was published\");";
        }

        if (parameter->cumrate) {
            // prepare cumrate for parameter
            c += parameter->cxx_initialize_cumrate();
        }

        if (parameter->is_extendable) {
            c += parameter->cxx_extend();
        }
    }
    if (any_missing_parameters) {
        m += "};";
        theLog->logMsg("Note: Missing parameters were written to Missing.dat.tmp");
    }
    c += "";

    c += "}";
    c += "";
}

void CodeGen::do_RunInit()
{
    c += "// Model run initialization";
	c += "void RunInit(IRunBase * const i_runBase)";
	c += "{";
    if (!Symbol::option_verify_attribute_modification) {
        c += "theLog->logFormatted(LT(\"Warning : prohibited attribute assignment not detected with verify_attribute_modification = off\"));";
        c += "";
    }
    if (!Symbol::option_verify_timelike_attribute_access) {
        c += "theLog->logFormatted(LT(\"Warning : prohibited time-like attribute access is not detected with verify_timelike_attribute_access = off\"));";
        c += "";
    }
    c += "extern void process_trace_options(IRunBase* const i_runBase);";
    c += "extern int64_t report_parameter_read_progress(int paramNumber, int paramCount, const char * name, int64_t lastTime);";
    c += "";
    c += "// Process model dev options for EventTrace";
    c += "process_trace_options(i_runBase);";
    c += "";
    c += "theLog->logMsg(\"Get scenario parameters for process\");";
    c += "";
    c += "int64_t last_progress_ms = getMilliseconds();";
    c += "";
    int nCount = 0;
    for (auto parameter : Symbol::pp_all_parameters) {
        if (parameter->source == ParameterSymbol::scenario_parameter) nCount++;
    }
    int n = 0;
    for (auto parameter : Symbol::pp_all_parameters) {
        if (parameter->source == ParameterSymbol::scenario_parameter) {
            c += parameter->cxx_read_parameter(++n, nCount);
        }
    }
    c += "";
    c += "// Process development model run options";
    c += "ProcessDevelopmentOptions(i_runBase);";
    c += "}";
    c += "";
}

void CodeGen::do_ModelStartup()
{
    c += "// Model startup method: Initialization for a simulation member";
    c += "void ModelStartup(IModel * const i_model)";
    c += "{";
    c += "// obtain simulation member to use for log messages";
    c += "int simulation_member = i_model->subValueId();";
    c += "";
    c += "// reset the weight support API";
    c += "reset_weight_api();";
    c += "";
    c += "// Bind scenario parameter references to thread local values (for scenario parameters).";
    c += "// Until this is done scenario parameter values are undefined and cannot be used by the model.";
    c += "";
    c += "theLog->logFormatted(\"member=%d Bind scenario parameters\", simulation_member);";

    c += "";
    c += "#ifdef OM_DEBUG_PARAMETERS";
    c += "";
    for (auto parameter : Symbol::pp_all_parameters) {
        // Process only scenario parameters in this for loop
        if (parameter->source != ParameterSymbol::scenario_parameter) continue;

        if (parameter->size() > 1 || parameter->cumrate) {
            // memcpy(om_value_UnionDurationBaseline, om_param_UnionDurationBaseline[i_model->parameterSubValueIndex("UnionDurationBaseline")].get(), 12 * sizeof(double));
            c +=
                "memcpy(" \
                "om_value_" + parameter->name + ", "
                + parameter->alternate_name() + "[i_model->parameterSubValueIndex(\"" + parameter->name + "\")].get(), " +
                to_string(parameter->size()) + " * sizeof(" + parameter->cxx_type_of_parameter() + "));";
        }
        else {
            // om_value_startSeed = om_param_startSeed[i_model->parameterSubValueIndex("startSeed")];
            c += 
                "om_value_" + parameter->name + " = "
                + parameter->alternate_name() + "[i_model->parameterSubValueIndex(\"" + parameter->name + "\")];";
        }
        if (parameter->cumrate) {
            // prepare cumrate for scenario parameter
            c += parameter->cxx_initialize_cumrate();
        }
    }
    c += "";
    c += "#else // OM_DEBUG_PARAMETERS";
    c += "";
    for (auto parameter : Symbol::pp_all_parameters) {
        // Process only scenario parameters in this for loop
        if (parameter->source != ParameterSymbol::scenario_parameter) continue;

        if (parameter->size() > 1 || parameter->cumrate) {
            // om_value_ageSex = om_param_ageSex[i_model->parameterSubValueIndex("ageSex")].get();
            c +=
                "om_value_" + parameter->name + " = "
                + parameter->alternate_name() + "[i_model->parameterSubValueIndex(\"" + parameter->name + "\")].get();";
        }
        else {
            // om_value_startSeed = om_param_startSeed[i_model->parameterSubValueIndex("startSeed")];
            c +=
                "om_value_" + parameter->name + " = "
                + parameter->alternate_name() + "[i_model->parameterSubValueIndex(\"" + parameter->name + "\")];";
        }
        if (parameter->cumrate) {
            // prepare cumrate for scenario parameter
            c += parameter->cxx_initialize_cumrate();
        }
    }
    c += "";
    c += "#endif // OM_DEBUG_PARAMETERS";
    c += "";

    // A second pass of scenario parameters is required to extend parameters
    // because index series parameter(s) must first be bound in first pass above.
    for (auto parameter : Symbol::pp_all_parameters) {
        if (parameter->source == ParameterSymbol::scenario_parameter && parameter->is_extendable) {
            c += parameter->cxx_extend();
        }
    }

    c += "";
    c += "// Zero-initialize derived parameters.";
    c += "";
    for (auto parameter : Symbol::pp_all_parameters) {

        // Process only derived parameters in this for loop
        if (parameter->source != ParameterSymbol::derived_parameter) continue;

        if (parameter->size() > 1) {
            // array
            // std::memset(BreastScreeningProtocol, 0, 5200 * sizeof(double));
            c += "std::memset(" + parameter->name + ", " \
                "0, " +
                to_string(parameter->size()) + " * sizeof(" + parameter->pp_datatype->name + "));";
        }
        else {
            // scalar
            c += parameter->name + " = (" + parameter->pp_datatype->name + ") 0;";
        }
    }

    c += "";
    c += "// Parameters are now ready and can be used by the model.";
    c += "";

    c += "theLog->logFormatted(\"member=%d Compute derived parameters\", simulation_member);";
    auto & sg = Symbol::pre_simulation;
    c += "int mem_id = i_model->subValueId();";
    c += "int mem_count = i_model->subValueCount();";
    c += "before_presimulation(mem_id, mem_count); // defined in model framework module";
    c += "// Call " + to_string(sg.ambiguous_count) + " PreSimulation functions without suffix";
    for (size_t id = 0; id < sg.ambiguous_count; ++id) {
        // The following can be useful to track down run-time errors occurring in a PreSimulation function
        c += "#ifdef _DEBUG";
        c += "theLog->logMsg(\"  call " + sg.disambiguated_name(id) + "\");";
        c += "#endif";
        c += sg.disambiguated_name(id) + "();";
    }
    c += "// Call " + to_string(sg.suffixes.size()) + " PreSimulation functions with suffix";
    for (auto suffix : sg.suffixes) {
        // The following can be useful to track down run-time errors occurring in a PreSimulation function
        c += "#ifdef _DEBUG";
        c += "theLog->logMsg(\"  call " + sg.prefix + suffix + "\");";
        c += "#endif";
        c += sg.prefix + suffix + "();";
    }
    c += "after_presimulation(); // defined in model framework module";
    c += "";

    for (auto parameter : Symbol::pp_all_parameters) {
        // Process only derived parameters in this for loop
        if (parameter->source != ParameterSymbol::derived_parameter) continue;
        if (parameter->cumrate) {
            // prepare cumrate for derived parameter
            c += parameter->cxx_initialize_cumrate();
        }
    }

    c += "theLog->logFormatted(\"member=%d Prepare for simulation\", simulation_member);";

    c += "// Entity static initialization part 1: Initialize entity attribute offsets & null entity data members";
    for (auto agent : Symbol::pp_all_agents) {
        c += "// Entity - " + agent->name;
        c += agent->name + "::om_null_entity.om_assign_member_offsets();";
        c += agent->name + "::om_null_entity.om_initialize_data_members0();";
        c += "";
    }

    c += "// Entity static initialization part 2: Initialize null entity dependent attributes";
    for (auto agent : Symbol::pp_all_agents) {
        c += "// Entity - " + agent->name;
        c += agent->name + "::om_null_entity.om_initialize_identity_attributes();";
        c += agent->name + "::om_null_entity.om_initialize_derived_attributes();";
        c += agent->name + "::om_null_entity.om_reset_derived_attributes();";
    }
    c += "";

    c += "{";
    c +=     "// Remove run-time suppressed tables from om_table_names";
    c +=     "std::list<std::string> suppressed;";
    c +=     "for (auto nm : om_table_names) {";
    c +=         "if (om_internal_table_names.count(nm)) continue; // skip internal tables";
    c +=         "if (is_suppressed_compute(nm, i_model)) {";
    c +=             "suppressed.push_back(nm);";
    c +=         "}";
    c +=     "}";
    c +=     "for (auto nm : suppressed) {";
    c +=         "om_table_names.erase(nm);";
    c +=     "}";
    c += "}";
    c += "";

    c += "// Entity table instantiation";
    for (auto et : Symbol::pp_all_entity_tables) {

        // example of generated code block:
        //    assert(!thePopulationByCity);
        //    if (!is_suppressed_compute("PopulationByCity", i_model)) {
        //        thePopulationByCity = new PopulationByCity("PopulationByCity", { 101, 5 });
        //    }

        c += "assert(!" + et->cxx_instance + "); ";
        if (!et->is_internal) {
            c += "if (!is_suppressed_compute(\"" + et->name + "\", i_model)) {";
        }
        c += et->cxx_instance + " = new " + et->cxx_type + "(\"" + et->name + "\", " + et->cxx_initializer() + ");";
        if (!et->is_internal) {
            c += "}";
        }
    }
    c += "";

    c += "// Derived table instantiation";
    for (auto dt : Symbol::pp_all_derived_tables) {

        // example of generated code block:
        //    assert(!theut1_feeder_with_names);
        //    if (!is_suppressed_compute("ut1_feeder_with_names", i_model)) {
        //        theut1_feeder_with_names = new ut1_feeder_with_names("ut1_feeder_with_names", { 5, 2 });
        //    }

        c += "assert(!" + dt->cxx_instance + "); ";
        if (!dt->is_internal) {
            c += "if (!is_suppressed_compute(\"" + dt->name + "\", i_model)) {";
        }
        c += dt->cxx_instance + " = new " + dt->cxx_type + "(\"" + dt->name + "\", " + dt->cxx_initializer() + ");";
        if (!dt->is_internal) {
            c += "}";
        }
    }
    c += "";

    c += "// Entity set instantiation";
    for (auto es : Symbol::pp_all_entity_sets) {
        c += "{";
        if (es->dimension_count() == 0) {
            c += "assert(!" + es->name + ");";
            c += es->name + " = new EntitySet<" + es->pp_agent->name + ">;";
        }
        else {
            c += "EntitySet<" + es->pp_agent->name + "> ** flattened_array = reinterpret_cast<EntitySet<" + es->pp_agent->name + "> **>(" + es->name + ");";
            c += "const size_t cells = " + to_string(es->cell_count()) + ";";
            c += "for (size_t cell = 0; cell < cells; ++cell) {";
            c += "assert(!flattened_array[cell]);";
            c += "flattened_array[cell] = new EntitySet<" + es->pp_agent->name + ">;";
            c += "}";
        }
        c += "}";
    }
    c += "";

    c += "}";
    c += "";
}

void CodeGen::do_ModelShutdown()
{
	c += "// Model shutdown method: outputs";
	c += "void ModelShutdown(IModel * const i_model)";
	c += "{";
    c += "// obtain simulation member to use for log messages";
    c += "int simulation_member = i_model->subValueId();";
    c += "";
    c += "// extract accumulators, and scale them to population size";
    for ( auto table : Symbol::pp_all_entity_tables ) {
	    c += "if (" + table->cxx_instance +") " + table->cxx_instance + "->extract_accumulators();";
	    c += "if (" + table->cxx_instance + ") " + table->cxx_instance + "->scale_accumulators();";
    }
	c += "";

    c += "// compute table expressions using accumulators";
    for ( auto table : Symbol::pp_all_entity_tables ) {
	    c += "if (" + table->cxx_instance + ") " + table->cxx_instance + "->compute_expressions();";
    }
    c += "";

    {
        auto & sg = Symbol::post_simulation;
        if (sg.suffixes.size() > 0 || sg.ambiguous_count > 0) {
            c += "theLog->logFormatted(\"member=%d Compute post-simulation\", simulation_member);";
            for (size_t id = 0; id < sg.ambiguous_count; ++id) {
                c += sg.disambiguated_name(id) + "();";
            }
            for (auto suffix : sg.suffixes) {
                c += sg.prefix + suffix + "();";
            }
            c += "";
        }
    }

    {
        auto & sg = Symbol::derived_tables;
        if (sg.suffixes.size() > 0 || sg.ambiguous_count > 0) {
            c += "theLog->logFormatted(\"member=%d Compute derived tables\", simulation_member);";
            for (size_t id = 0; id < sg.ambiguous_count; ++id) {
                c += sg.disambiguated_name(id) + "();";
            }
            for (auto suffix : sg.suffixes) {
                c += sg.prefix + suffix + "();";
            }
            c += "";
        }
    }

    c += "theLog->logFormatted(\"member=%d Write output tables - start\", simulation_member);";
    c += "";
    c += "extern int64_t report_table_write_progress(int member, int tableNumber, const char * name, int64_t lastTime);";
    c += "";
    c += "// write entity tables (accumulators) and release accumulators memory";
    c += "int64_t last_progress_ms = getMilliseconds();";
    c += "int n_table = 0;";
    c += "";
    for ( auto table : Symbol::pp_all_entity_tables ) {
        if (!table->is_internal) {
            c += "if (!is_suppressed_write(\"" + table->name + "\", i_model)) {"; 
            c += "last_progress_ms = report_table_write_progress(simulation_member, ++n_table, \"" + table->name + "\", last_progress_ms);";
            c += "i_model->writeOutputTable(\"" +
                table->name + "\", " + table->cxx_instance + "->n_cells, " + table->cxx_instance + "->acc_storage);";
            c += "}";
        }
    }
    c += "// at this point table->acc[k][j] will cause memory access violation";
    c += "";

    c += "// write derived tables (measures) and release measures memory";
    for ( auto derived_table : Symbol::pp_all_derived_tables ) {
        if (!derived_table->is_internal) {
            c += "if (" + derived_table->cxx_instance + ") {";
            c += "last_progress_ms = report_table_write_progress(simulation_member, ++n_table, \"" + derived_table->name + "\", last_progress_ms);";
            c += " i_model->writeOutputTable(\"" +
                derived_table->name + "\", " + 
                derived_table->cxx_instance + "->n_cells, " + 
                derived_table->cxx_instance + "->measure_storage);";
            c += "}";
        }
    }
    c += "// at this point table->measure[k][j] will cause memory access violation";
    c += "";
    c += "theLog->logFormatted(\"member=%d Write output tables - finish\", simulation_member);";

    c += "// Entity table destruction";
    for (auto table : Symbol::pp_all_entity_tables) {
        c += "if (" + table->cxx_instance + ") {";
        c += "delete " + table->cxx_instance + ";";
        c += table->cxx_instance + " = nullptr;";
        c += "}";
    }
    c += "";

    c += "// Derived table destruction";
    for (auto derived_table : Symbol::pp_all_derived_tables) {
        c += "if (" + derived_table->cxx_instance + ") {";
        c += "delete " + derived_table->cxx_instance + ";";
        c += derived_table->cxx_instance + " = nullptr;";
        c += "}";
    }
    c += "";

    c += "// Entity set destruction";
    for (auto es : Symbol::pp_all_entity_sets) {
        c += "{";
        if (es->dimension_count() == 0) {
            c += "assert(" + es->name + ");";
            c += "delete(" + es->name + ");";
			c += es->name + " = nullptr;";
		}
        else {
            c += "EntitySet<" + es->pp_agent->name + "> ** flattened_array = reinterpret_cast<EntitySet<" + es->pp_agent->name + "> **>(" + es->name + ");";
            c += "const size_t cells = " + to_string(es->cell_count()) + ";";
            c += "for (size_t cell = 0; cell < cells; ++cell) {";
            c += "assert(flattened_array[cell]);";
            c += "delete flattened_array[cell];";
            c += "flattened_array[cell] = nullptr;";
            c += "}";
        }
        c += "}";
    }
    c += "";

    c += "}";
	c += "";
}

void CodeGen::do_agents()
{
    // early forward declarations of entity classes and pointers
    t1 += "// early forward declarations of entity classes and pointers\n";
    for (auto agent : Symbol::pp_all_agents) {
        t1 += "class " + agent->name + ";";
        t1 += "typedef entity_ptr<" + agent->name + "> " + agent->name + "_ptr;";
    }

	h += "// forward declarations of model entity classes (for links)";
    for (auto agent : Symbol::pp_all_agents) {
        h += "class " + agent->name + ";";
    }
    h += "";
    for ( auto agent : Symbol::pp_all_agents ) {

        h += "// model entity classes";
        // e.g. class Person : public Entity<Person>
	    h += "class " + agent->name + " : public Entity<" + agent->name + ">";
        h += "{";
	    h += "public:";

	    h += "";
        h += doxygen_short("The name of this entity");
        h += "static constexpr const char * entity_name = \"" + agent->name + "\";";
        h += "";

        h += doxygen_short("Get the name of this entity");
        h += "const char * om_get_entity_name()";
        h += "{";
        h +=     "return entity_name;";
        h += "}";
        h += "";

        h += "//";
	    h += "// function members in " + agent->name + " entity";
	    h += "//";
	    h += "";

	    h += doxygen_short("operator overload for entity comparison based on entity_id");
        h += "bool operator< ( " + agent->name + " & rhs )";
        h += "{";
        h +=     "return entity_id < rhs.entity_id;";
        h += "}";
	    h += "";

        for ( auto func_member : agent->pp_agent_funcs ) {
            h += func_member->cxx_declaration_agent();
            c += func_member->cxx_definition_agent();
            if (func_member->has_line_directive) {
                c += 
                    (no_line_directives ? "//#line " : "#line ")
                    + to_string(c.size() + 2 + 8) // additional 8 lines for subsequently inserted header
                    + " \""
                    + c_fname
                    + "\"";
            }
        }

	    h += "";
	    h += "//";
	    h += "// Data members in " + agent->name + " entity";
	    h += "//";
	    h += "";
        if (Symbol::option_entity_member_packing) {
            // create a temporary copy of the list of data members in the entity
            auto lst = agent->pp_agent_data_members;
            // sort it in descending order of alignment_size
            // but sort lexicographically within alignment_size groups
            lst.sort([](EntityDataMemberSymbol* a, EntityDataMemberSymbol* b) 
                { return (a->alignment_size() == b->alignment_size()) ? 
                         (a->name < b->name) :
                         (a->alignment_size() > b->alignment_size());
                }
            );
            // iterate the sorted list and generate declaration h and definition c
            size_t alignment_group = 16; // something bigger than 8
            for (auto data_member : lst) {
                size_t member_alignment = data_member->alignment_size();
                if (member_alignment < alignment_group) {
                    alignment_group = member_alignment;
                    h += "";
                    h += "//////////////////////////////////////////////";
                    h += "// Alignment Group Size = " + std::to_string(alignment_group);
                    h += "//////////////////////////////////////////////";
                    h += "";
                }
                h += data_member->cxx_declaration_agent();
                c += data_member->cxx_definition_agent();
            }
        }
        else {
            // create a temporary copy of the list of data members in the entity
            auto lst = agent->pp_agent_data_members;
            // sort it in ascending order of layout group
            // but sort lexicographically within layout groups
            lst.sort([](EntityDataMemberSymbol* a, EntityDataMemberSymbol* b)
                { return (a->layout_group() == b->layout_group()) ?
                (a->name < b->name) :
                (a->layout_group() < b->layout_group());
                }
            );
            size_t layout_group_current = 0; // a value less than any layout group
            for (auto data_member : lst) {
                size_t layout_group_this = data_member->layout_group();
                if (layout_group_this > layout_group_current) {
                    layout_group_current = layout_group_this;
                    h += "";
                    h += "//////////////////////////////////////////////";
                    h += "// Layout Group = " + std::to_string(layout_group_current);
                    h += "//////////////////////////////////////////////";
                    h += "";
                }
                h += data_member->cxx_declaration_agent();
                c += data_member->cxx_definition_agent();
            }
        }
	    h += "";
	    c += "";

        h += "// The declaration of the static member " + agent->name;
        h += "// used to retrieve (zero) values when dereferencing nullptr link attributes.";
        h += "static thread_local " + agent->name + " " + "om_null_entity;";

	    h += "}; // class " + agent->name;
	    h += "";

        c += "// The definition of the static member " + agent->name;
        c += "// used to retrieve (zero) values when dereferencing nullptr link attributes.";
        c += "thread_local " + agent->name + " " + agent->name + "::om_null_entity;";
    }

    c += doxygen("Free all zombie agents");
    c += "void BaseEntity::free_all_zombies()";
    c += "{";
    for ( auto agent : Symbol::pp_all_agents ) {
        // e.g. Person::free_zombies();
        c += agent->name + "::free_zombies();";
    }
    c += "}";
    c += "";

    c += "void BaseEntity::initialize_simulation_runtime()";
    c += "{";
    c += "entities = new std::list<BaseEntity *>;";
    for ( auto ent : Symbol::pp_all_agents ) {
        // e.g. Person::zombies = new forward_list<Person *>;";
        c += ent->name + "::zombies = new std::forward_list<" + ent->name + " *>;";
        c += ent->name + "::available = new std::forward_list<" + ent->name + " *>;";
        c += "";
        c += "// reset resource use information for " + ent->name;
        c += ent->name + "::resource_use_reset();";
        c += "";
        c += "// reset resource use information for events in " + ent->name;
        for (auto evt : ent->pp_agent_events) {
            c += ent->name + "::om_null_entity." + evt->name + ".resource_use_reset();";
        }
        c += "// reset resource use information for multilinks in " + ent->name;
        for (auto ml : ent->pp_multilink_members) {
            c += ent->name + "::om_null_entity." + ml->name + ".resource_use_reset();";
        }
        c += "";
    }
    c += "event_trace_on = om_event_trace_capable;";
    c += "}";
    c += "";

    c += "void BaseEntity::finalize_simulation_runtime()";
    c += "{";
    c +=     "assert(entities->empty());";
    c +=     "delete entities;";
    c +=     "entities = nullptr;";
    c +=     "";
    for (auto agent : Symbol::pp_all_agents) {
        c += "assert(" + agent->name + "::zombies->empty());";
        c += "delete " + agent->name + "::zombies;";
        c += agent->name + "::zombies = nullptr;";
        c += "";
        c += "for (auto ent : *" + agent->name + "::available) {";
        c +=     "delete ent;";
        c += "}";
        c += "delete " + agent->name + "::available;";
        c += agent->name + "::available = nullptr;";
        c += "";
    }
    c += "}";
    c += "";
}

void CodeGen::do_entity_sets()
{
    if (Symbol::pp_all_entity_sets.size() > 0) {
	    h += "// entity sets";
	    c += "// entity sets";

	    for ( auto es : Symbol::pp_all_entity_sets ) {
            // the entity set itself
            h += es->cxx_declaration_global();
            c += es->cxx_definition_global();

            // the resource use function <name>_resource_use
            h += es->resource_use_gfn->cxx_declaration_global();
            c += es->resource_use_gfn->cxx_definition_global();

            // the resource use function <name>_resource_use_reset
            h += es->resource_use_reset_gfn->cxx_declaration_global();
            c += es->resource_use_reset_gfn->cxx_definition_global();
        }

	    h += "";
	    c += "";
    }
}

void CodeGen::do_entity_tables()
{
	h += "// entity tables";
	c += "// entity tables";

	for ( auto tbl : Symbol::pp_all_entity_tables ) {
        // the entity table itself
        h += tbl->cxx_declaration_global();
        c += tbl->cxx_definition_global();

        // the resource use function <name>_resource_use
        h += tbl->resource_use_gfn->cxx_declaration_global();
        c += tbl->resource_use_gfn->cxx_definition_global();

        // the resource use function <name>_resource_use_reset
        h += tbl->resource_use_reset_gfn->cxx_declaration_global();
        c += tbl->resource_use_reset_gfn->cxx_definition_global();

        if (!tbl->is_internal) {
            tbl->populate_metadata(metaRows);
        }
    }
}

void CodeGen::do_derived_tables()
{
	h += "// derived tables";
	c += "// derived tables";

	for (auto derived_table : Symbol::pp_all_derived_tables) {
        h += derived_table->cxx_declaration_global();
        c += derived_table->cxx_definition_global();
        if (!derived_table->is_internal) {
            derived_table->populate_metadata(metaRows);
        }
    }

    h += "";
	c += "";
}

void CodeGen::do_imports()
{
    for (auto imp : Symbol::pp_all_imports) {
        imp->populate_metadata(metaRows);
    }
}

void CodeGen::do_groups()
{
    if (Symbol::pp_all_parameter_groups.size()) {
        for (auto grp : Symbol::pp_all_parameter_groups) {
            grp->populate_metadata(metaRows);
        }
    }

    if (Symbol::pp_all_table_groups.size()) {
        for (auto grp : Symbol::pp_all_table_groups) {
            grp->populate_metadata(metaRows);
        }
    }
}

void CodeGen::do_table_interface()
{
    h += doxygen_short("Table names in the model");
    h += "extern std::unordered_set<std::string> om_table_names;";
    h += "extern std::unordered_set<std::string> om_internal_table_names;";
    h += "";

    c += "std::unordered_set<std::string> om_table_names = {";
    for (auto tbl : Symbol::pp_all_tables) {
        c += "\"" + tbl->name + "\",";
    }
    c += "};";
    c += "";

    c += "std::unordered_set<std::string> om_internal_table_names = {";
    for (auto tbl : Symbol::pp_all_tables) {
        if (tbl->is_internal) {
            c += "\"" + tbl->name + "\",";
        }
    }
    c += "};";
    c += "";

    c += "const std::map<std::string, std::pair<int, int>> om_table_measure = {";
    for (auto table : Symbol::pp_all_tables) {
        for (auto measure : table->pp_measures) {
            auto key = table->name + "." + measure->short_name;
            auto table_id = to_string(table->pp_table_id);
            auto measure_id = to_string(measure->index);
            c += "{\"" + key + "\", {" + table_id + ", " + measure_id + "} },";
            if (measure->short_name != measure->short_name_default) {
                // also recognize default short_name in GetTableValue API, eg "MyTable.Expr1"
                auto key = table->name + "." + measure->short_name_default;
                c += "{\"" + key + "\", {" + table_id + ", " + measure_id + "} },";
            }
        }
    }
    c += "};";
    c += "";

    c += "double * om_get_table_measure_address(int table_id, int measure_id, std::vector<int> indices)";
    c += "{";
    c += "switch (table_id) {";
    for (auto table : Symbol::pp_all_tables) {
        c += "case " + to_string(table->pp_table_id) + ": return " + table->cxx_instance + " ? " + table->cxx_instance + "->get_measure_address(measure_id, indices) : nullptr;";
    }
    c += "default: assert(false); // logic guarantee";
    c += "}";
    c += "return nullptr;";
    c += "}";
    c += "";

	c += "";

}

void CodeGen::do_event_queue()
{
    c += "// definition of event_queue (declaration in Event.h)";
    c += "thread_local std::set<BaseEvent *, less_deref<BaseEvent *> > *BaseEvent::event_queue = nullptr;";
    c += "";
    c += "// definition of dirty_events (declaration in Event.h)";
    c += "thread_local std::set<BaseEvent *, decltype(BaseEvent::dirty_cmp)* > *BaseEvent::dirty_events = nullptr;";
    c += "";
    c += "// definition of global_event_counter (declaration in Event.h)";
    c += "thread_local big_counter BaseEvent::global_event_counter;";
    c += "";
    c += "// definition of global_time (declaration in Event.h)";
    c += "thread_local Time *BaseEvent::global_time = nullptr;";
    c += "";
    c += "// definition of active entity list (declaration in Entity.h)";
    c += "thread_local std::list<BaseEntity *> *BaseEntity::entities = nullptr;";
    c += "";
    c += "// definition of event_id of current event (declaration in Event.h)";
    c += "thread_local int BaseEvent::current_event_id;";
    c += "";
    c += "// definition of entity_id of current event (declaration in Event.h)";
    c += "thread_local int BaseEvent::current_entity_id;";
    c += "";
    c += "// definition of event_id of event time function being recomputed (declaration in Event.h)";
    c += "thread_local int BaseEvent::timefunc_event_id;";
    c += "";
    c += "// definition of entity_id of event time function being recomputed (declaration in Event.h)";
    c += "thread_local int BaseEvent::timefunc_entity_id;";
    c += "";
}

void CodeGen::do_event_names()
{
    {
        c += "// get event name given event id";
        c += "const char * event_id_to_name(int event_id) {";
        c += "static const char * event_name[] = {";
        int id = 0;
        for (auto nm : Symbol::pp_all_event_names) {
            c += "\"" + nm + "\", // event_id " + std::to_string(id);
            ++id;
        }
        if (id == 0) {
            c += "\"\", // no events in model ";
        }
        c += "};";
        if (id > 0) {
            c += "return (event_id >= 0 && event_id < " + std::to_string(id) + ") ? event_name[event_id] : \"(no event)\";";
        }
        else {
            c += "return \"\"; // no events in model";
        }
        c += "}";
    }
    c += "";
    {
        c += "// get event id given event name";
        c += "const int event_name_to_id(const std::string event_name) {";
        c += "static const std::unordered_map<std::string, int> name_to_id = {";
        int id = 0;
        for (auto nm : Symbol::pp_all_event_names) {
            c += "{\"" + nm + "\", " + std::to_string(id) + "},";
            ++id;
        }
        c += "};";
        c += "auto srch = name_to_id.find(event_name);";
        c += "return (srch != name_to_id.end()) ? srch->second : -1;";
        c += "}";
    }
    c += "";
}

void CodeGen::do_table_dependencies()
{
    c += "// table dependencies";

    c += "const std::map<std::string, std::set<std::string>> om_tables_required =";
    c += "{";
    for (auto tbl_required : Symbol::pp_all_tables) {
        if (tbl_required->pp_tables_requiring.size()) {
            // if no tables require this table, omit it from the map
            c += "{";
            c += "\"" + tbl_required->name + "\", // is required by";
            c += "{";
            for (auto tbl_requiring : tbl_required->pp_tables_requiring) {
                c += "\"" + tbl_requiring->name + "\",";
            }
            c += "},";
            c += "},";
        }
    }
    c += "};";
    c += "";

    c += "bool is_suppressed_compute(std::string table_name, IModel* const i_model)";
    c += "{";
    c +=     "bool is_suppressed = i_model->isSuppressed(table_name.c_str()); ";
    c +=     "if (is_suppressed) {";
    c +=         "auto search = om_tables_required.find(table_name);";
    c +=         "if (search == om_tables_required.end()) {";
    c +=             "return true;  // no other tables require this table";
    c +=         "}";
    c +=         "else {";
    c +=             "for (auto tbl_requiring : search->second) {";
    c +=                 "if (!i_model->isSuppressed(tbl_requiring.c_str())) { ";
    c +=                     "return false; // a non-suppressed table requires this table";
    c +=                 "}";
    c +=             "}";
    c +=             "return true; // no non-suppressed table requires this table";
    c +=         "}";
    c +=     "}";
    c +=     "else {";
    c +=         "return false; // this table is not suppressed";
    c +=     "}";
    c += "}";
    c += "";

    c += "bool is_suppressed_write(std::string table_name, IModel* const i_model)";
    c += "{";
    c +=     "return i_model->isSuppressed(table_name.c_str()); ";
    c += "}";
    c += "";

}

void CodeGen::do_RunModel()
{
	c += "// Model simulation";
	c += "void RunModel(openm::IModel * const i_model)";
    c += "{";

    c += "// initialize entity tables";
	for ( auto table : Symbol::pp_all_entity_tables ) {
        c += "if (" + table->cxx_instance + ") " + table->cxx_instance + "->initialize_accumulators();";
    }
    c += "";
    c += "BaseEvent::initialize_simulation_runtime();";
    c += "BaseEntity::initialize_simulation_runtime();";
    c += "";
    c += "int sub_id = i_model->subValueId();";
    c += "int sub_count = i_model->subValueCount();";
    c += "if (om_resource_use_on) {";
    if (Symbol::pp_all_entity_sets.size()) {
        c += "// reset resource use for entity sets";
        for (auto es : Symbol::pp_all_entity_sets) {
            c += es->resource_use_reset_gfn->name + "();";
        }
        c += "";
    }
    if (Symbol::pp_all_entity_tables.size()) {
        c += "// reset resource use for entity tables";
        for (auto et : Symbol::pp_all_entity_tables) {
            c += et->resource_use_reset_gfn->name + "();";
        }
    }
    c += "} // om_resource_use_on";
    c += "";
    c += "";
    c += "RunSimulation(sub_id, sub_count, i_model); // Defined by the model framework, generally in a 'use' module";
    c += "";
    c += "";
    if (Symbol::option_resource_use) {
        // do not generate code for resource report unless requested
        c += "int resource_use_sub = 0; // hard-coded sub 0 for resource report";
        c += "if (om_resource_use_on && (resource_use_sub == sub_id)) {";
        c += "// Resource use report";
        //c +=     "std::string prefix0s = \"member=\" + std::to_string(sub_id) + \" \";";
        c += "std::string prefix0s = \" \";";
        c += "std::string prefix1s = prefix0s + \"  \"; // prefix with 1 ident";
        c += "std::string prefix2s = prefix1s + \"  \"; // prefix with 2 idents";
        c += "std::string prefix3s = prefix2s + \"  \"; // prefix with 3 idents";
        c += "std::string dashes_str (500, '-'); // string consisting of 500 -";
        c += "std::string stars_str (500, '*'); // string consisting of 500 *";
        c += "auto prefix0 = prefix0s.c_str();";
        c += "auto prefix1 = prefix1s.c_str();";
        c += "auto prefix2 = prefix2s.c_str();";
        c += "auto prefix3 = prefix3s.c_str();";
        c += "auto dashes = dashes_str.c_str();";
        c += "auto stars = stars_str.c_str();";
        c += "int set_node_bytes = 4 * sizeof(void *); // assume 3 words for b-tree, plus 1 word for payload (pointer)";
        c += "int rb_node_bytes = sizeof(rb_node<void *>);";
        c += "theLog->logFormatted(LT(\"Warning : Model built with resource_use = on\"));";
        c += "theLog->logFormatted(\"%sResource Use Report - Begin (for sub/member/replicate %d)\", prefix0, sub_id);";
        c += "theLog->logFormatted(\"%s\", prefix0);";
        c += "";
        { // resource use summary
            // Logic for aggregate MB is copied from logic in detail sections below
            c += "{ // Resource Use Summary";
            c += "double MB_Entities = 0.0;";
            c += "double MB_Multilinks = 0.0;";
            c += "double MB_Events = 0.0;";
            c += "double MB_Sets = 0.0;";
            c += "double MB_Tables = 0.0;";
            for (auto ent : Symbol::pp_all_agents) {
                c += "{";
                c += "auto ent_bytes = sizeof(" + ent->name + ");";
                c += "auto ent_result = " + ent->name + "::resource_use();";
                c += "double MB = (ent_bytes * ent_result.allocations) / 1000000.0;";
                c += "MB_Entities += MB;";
                for (auto ml : ent->pp_multilink_members) {
                    c += "{";
                    c += "auto ml_result = " + ent->name + "::om_null_entity." + ml->name + ".resource_use();";
                    c += "double avg_size = ent_result.activations ? (double)ml_result.total_slots/(double)ent_result.activations : 0.0;";
                    c += "int payload_bytes = sizeof(void *); // each element of vector is a wrapped pointer";
                    c += "double MB = ((avg_size * ent_result.allocations * payload_bytes) / 1000000.0);";
                    c += "MB_Multilinks += MB;";
                    c += "}";
                }
                for (auto evt : ent->pp_agent_events) {
                    c += "{";
                    c += "auto evt_result = " + ent->name + "::om_null_entity." + evt->name + ".resource_use();";
                    c += "int max_in_queue = (int)evt_result.max_in_queue;";
                    c += "double MB = (max_in_queue * set_node_bytes) / 1000000.0;";
                    c += "MB_Events += MB;";
                    c += "}";
                }
                for (auto entset : ent->pp_agent_entity_sets) {
                    c += "{";
                    c += "int dimension_count = " + to_string(entset->dimension_count()) + ";";
                    c += "int cell_count = " + to_string(entset->cell_count()) + ";";
                    c += "auto entset_result = " + entset->resource_use_gfn->name + "();"; // call the resource use reporting function for this entity set
                    c += "int max_count = (int)entset_result.max_count;";
                    c += "int bytes_fixed = sizeof(*" + entset->name + "); // fixed portion for cells";
                    c += "int bytes_nodes = max_count * rb_node_bytes; // variable portion for rb_tree nodes";
                    c += "double MB = (bytes_fixed + bytes_nodes) / 1000000.0;";
                    c += "MB_Sets += MB;";
                    c += "}";
                }
                for (auto tbl : ent->pp_entity_tables) {
                    c += "{";
                    c += "bool instantiated = " + tbl->cxx_instance + "; // is nullptr if suppressed at runtime";
                    c += "int cell_count = " + to_string(tbl->cell_count()) + ";";
                    c += "int accumulator_count = " + to_string(tbl->accumulator_count()) + ";";
                    c += "if (instantiated) {";
                    c += "double MB = (8 * cell_count * accumulator_count) / 1000000.0;";
                    c += "MB_Tables += MB;";
                    c += "}";
                    c += "}";
                }
                c += "}";
            }
            for (auto tbl : Symbol::pp_all_derived_tables) {
                c += "{";
                c += "bool instantiated = " + tbl->cxx_instance + "; // is nullptr if suppressed at runtime";
                c += "int cell_count = " + to_string(tbl->cell_count()) + ";";
                c += "int measure_count = " + to_string(tbl->measure_count()) + ";";
                c += "if (instantiated) {";
                c += "double MB = (cell_count * measure_count * sizeof(double)) / 1000000.0;";
                c += "MB_Tables += MB;";
                c += "}";
                c += "}";
            }
            c += "double MB_All = MB_Entities + MB_Multilinks + MB_Events + MB_Sets + MB_Tables;";
            c += "auto section_title = \"*  Resource Use Summary  *\";";
            c += "theLog->logFormatted(\"%s%.*s\", prefix1, strlen(section_title), stars);";
            c += "theLog->logFormatted(\"%s%s\", prefix1, section_title);";
            c += "theLog->logFormatted(\"%s%.*s\", prefix1, strlen(section_title), stars);";
            c += "theLog->logFormatted(\"%s\", prefix1);";

            c += "auto table_title = \"Resource Use Summary\";";
            c += "auto row_sep =         \"+-------------------+-------+\";";
            c += "int table_width = (int)strlen(row_sep);";
            c += "theLog->logFormatted(\"%s+%.*s+\", prefix2, table_width - 2, dashes);";
            c += "theLog->logFormatted(\"%s| %-*s |\", prefix2, table_width - 4, table_title);";
            c += "theLog->logFormatted(\"%s%s\", prefix2, row_sep);";
            c += "theLog->logFormatted(\"%s| Category          |    MB |\", prefix2);";
            c += "theLog->logFormatted(\"%s%s\", prefix2, row_sep);";
            c += "theLog->logFormatted(\"%s| Entities          | %5d |\", prefix2, (int)MB_Entities);";
            for (auto ent : Symbol::pp_all_agents) {
                c += "{";
                c += "auto ent_name = \"" + ent->name + "\";";
                c += "auto ent_bytes = sizeof(" + ent->name + ");";
                c += "auto ent_result = " + ent->name + "::resource_use();";
                c += "double MB = (ent_bytes * ent_result.allocations) / 1000000.0;";
                c += "theLog->logFormatted(\"%s|   %-15s | %5d |\", prefix2, ent_name, (int)MB);";
                c += "}";
            }
            c += "theLog->logFormatted(\"%s| Multilinks        | %5d |\", prefix2, (int)MB_Multilinks);";
            c += "theLog->logFormatted(\"%s| Events            | %5d |\", prefix2, (int)MB_Events);";
            c += "theLog->logFormatted(\"%s| Sets              | %5d |\", prefix2, (int)MB_Sets);";
            c += "theLog->logFormatted(\"%s| Tables            | %5d |\", prefix2, (int)MB_Tables);";
            c += "theLog->logFormatted(\"%s%s\", prefix2, row_sep);";
            c += "theLog->logFormatted(\"%s| All               | %5d |\", prefix2, (int)MB_All);";
            c += "theLog->logFormatted(\"%s%s\", prefix2, row_sep);";
            c += "theLog->logFormatted(\"%s\", prefix0);";
            c += "}";
        }

        for (auto ent : Symbol::pp_all_agents) {
            auto dml = ent->pp_agent_data_members;
            size_t members_count = dml.size();
            size_t ent_internal = std::count_if(dml.begin(), dml.end(), [this](EntityDataMemberSymbol* edms) {return edms->is_internal(); });
            size_t attribute_count = std::count_if(dml.begin(), dml.end(), [this](EntityDataMemberSymbol* edms) {return edms->is_attribute(); });
            size_t builtin_attribute_count = std::count_if(dml.begin(), dml.end(), [this](EntityDataMemberSymbol* edms) {return edms->is_builtin_attribute(); });
            size_t link_attribute_count = std::count_if(dml.begin(), dml.end(), [this](EntityDataMemberSymbol* edms) {return edms->is_link_attribute(); });
            size_t maintained_attribute_count = std::count_if(dml.begin(), dml.end(), [this](EntityDataMemberSymbol* edms) {return edms->is_maintained_attribute(); });
            size_t simple_attribute_count = std::count_if(dml.begin(), dml.end(), [this](EntityDataMemberSymbol* edms) {return edms->is_simple_attribute(); });
            size_t array_count = std::count_if(dml.begin(), dml.end(), [this](EntityDataMemberSymbol* edms) {return edms->is_array(); });
            size_t event_count = std::count_if(dml.begin(), dml.end(), [this](EntityDataMemberSymbol* edms) {return edms->is_event(); });
            size_t increment_count = std::count_if(dml.begin(), dml.end(), [this](EntityDataMemberSymbol* edms) {return edms->is_increment(); });
            size_t foreign_count = std::count_if(dml.begin(), dml.end(), [this](EntityDataMemberSymbol* edms) {return edms->is_foreign(); });
            size_t internal_count = std::count_if(dml.begin(), dml.end(), [this](EntityDataMemberSymbol* edms) {return edms->is_internal(); });
            size_t multilink_count = std::count_if(dml.begin(), dml.end(), [this](EntityDataMemberSymbol* edms) {return edms->is_multilink(); });
            size_t array_size = 0;

            c += "{ // Begin resource use tables for " + ent->name;
            c += "std::stringstream ss;";
            c += "auto ent_name = \"" + ent->name + "\";";
            c += "auto section_title = \"*  Resource Use Detail for " + ent->name + "  *\";";
            c += "theLog->logFormatted(\"%s%.*s\", prefix1, strlen(section_title), stars);";
            c += "theLog->logFormatted(\"%s%s\", prefix1, section_title);";
            c += "theLog->logFormatted(\"%s%.*s\", prefix1, strlen(section_title), stars);";
            c += "theLog->logFormatted(\"%s\", prefix1);";
            c += "auto ent_bytes = sizeof(" + ent->name + ");";
            c += "auto ent_result = " + ent->name + "::resource_use();";
            {
                c += "{ // entity instances";
                c += "auto table_title = \"    " + ent->name + " Instances\";";
                c += "auto row_sep =         \"+--------------+--------------+-------+\";";
                c += "int table_width = (int)strlen(row_sep);";
                c += "theLog->logFormatted(\"%s+%.*s+\", prefix2, table_width - 2, dashes);";
                c += "theLog->logFormatted(\"%s| %-*s |\", prefix2, table_width - 4, table_title);";
                c += "theLog->logFormatted(\"%s%s\", prefix2, row_sep);";
                c += "theLog->logFormatted(\"%s| Activations  | Allocations  |    MB |\", prefix2);";
                c += "theLog->logFormatted(\"%s%s\", prefix2, row_sep);";
                c += "theLog->logFormatted(\"%s| %12d | %12d | %5d |\", prefix2, ent_result.activations, ent_result.allocations, (ent_bytes * ent_result.allocations) / 1000000);";
                c += "theLog->logFormatted(\"%s%s\", prefix2, row_sep);";
                if (multilink_count) {
                    c += "theLog->logFormatted(\"%s%s\", prefix2, \"Note: MB does not include storage of elements of multilinks\");";
                }
                if (foreign_count) {
                    c += "theLog->logFormatted(\"%s%s\", prefix2, \"Note: MB does not include storage of elements of foreign objects\");";
                }
                c += "theLog->logFormatted(\"%s\", prefix0);";
                c += "}";
                c += "";
            }
            {
                c += "{ // entity members (summary)";
                c += "auto table_title = \"    " + ent->name + " Members\";";
                c += "auto row_sep =         \"+-------------------+-------+\";";
                c += "int table_width = (int)strlen(row_sep);";
                c += "theLog->logFormatted(\"%s+%.*s+\", prefix2, table_width - 2, dashes);";
                c += "theLog->logFormatted(\"%s| %-*s |\", prefix2, table_width - 4, table_title);";
                c += "theLog->logFormatted(\"%s%s\", prefix2, row_sep);";
                c += "theLog->logFormatted(\"%s| Member            | Count |\", prefix2);";
                c += "theLog->logFormatted(\"%s%s\", prefix2, row_sep);";
                c += "theLog->logFormatted(\"%s| Attributes        | %5d |\", prefix2, " + std::to_string(attribute_count) + ");";
                c += "theLog->logFormatted(\"%s|   Built-in        | %5d |\", prefix2, " + std::to_string(builtin_attribute_count) + ");";
                c += "theLog->logFormatted(\"%s|   Simple          | %5d |\", prefix2, " + std::to_string(simple_attribute_count) + ");";
                c += "theLog->logFormatted(\"%s|   Maintained      | %5d |\", prefix2, " + std::to_string(maintained_attribute_count) + ");";
                c += "theLog->logFormatted(\"%s|   Link            | %5d |\", prefix2, " + std::to_string(link_attribute_count) + ");";
                c += "theLog->logFormatted(\"%s| Events            | %5d |\", prefix2, " + std::to_string(event_count) + ");";
                c += "theLog->logFormatted(\"%s| Increments        | %5d |\", prefix2, " + std::to_string(increment_count) + ");";
                c += "theLog->logFormatted(\"%s| Multilink         | %5d |\", prefix2, " + std::to_string(multilink_count) + ");";
                c += "theLog->logFormatted(\"%s| Internal          | %5d |\", prefix2, " + std::to_string(internal_count) + ");";
                c += "theLog->logFormatted(\"%s| Array             | %5d |\", prefix2, " + std::to_string(array_count) + ");";
                c += "theLog->logFormatted(\"%s| Foreign           | %5d |\", prefix2, " + std::to_string(foreign_count) + ");";
                c += "theLog->logFormatted(\"%s%s\", prefix2, row_sep);";
                c += "theLog->logFormatted(\"%s| All               | %5d |\", prefix2, " + std::to_string(members_count) + ");";
                c += "theLog->logFormatted(\"%s%s\", prefix2, row_sep);";
                c += "theLog->logFormatted(\"%s\", prefix0);";
                c += "}";
            }
            if (ent->pp_multilink_members.size()) {
                c += "{ // multilinks";
                c += "auto table_title = \"    " + ent->name + " Multilink elements\";";
                std::string col1header = "multilink";
                int col1width = col1header.length();
                for (auto ml : ent->pp_multilink_members) { col1width = std::max<int>(col1width, ml->name.length()); };
                c += "int col1width = " + to_string(col1width) + ";";
                c += "const char * col1header = \"" + col1header + "\";";
                std::string row_sep = "+-" + std::string(col1width, '-') +
                    "-+--------------+--------------+--------------+-------+";
                c += "auto row_sep = \"" + row_sep + "\";";
                c += "int table_width = (int)strlen(row_sep);";
                c += "theLog->logFormatted(\"%s+%.*s+\", prefix2, table_width - 2, dashes);";
                c += "theLog->logFormatted(\"%s| %-*s |\", prefix2, table_width - 4, table_title);";
                c += "theLog->logFormatted(\"%s%s\", prefix2, row_sep);";
                c += "theLog->logFormatted(\"%s| %-*s |     max size |    entity_id |     avg size |    MB |\", prefix2, col1width, col1header);";
                c += "theLog->logFormatted(\"%s%s\", prefix2, row_sep);";
                c += "double MB_total = 0;";
                for (auto ml : ent->pp_multilink_members) {
                    // Get resource use information for each multilink in the entity
                    // The static thread_local om_null_entity is used to access the static thread_local function and counters
                    c += "{";
                    c += "// Resources for " + ml->name;
                    c += "auto ml_name = \"" + ml->name + "\";";
                    c += "auto ml_result = " + ent->name + "::om_null_entity." + ml->name + ".resource_use();";
                    c += "double avg_size = ent_result.activations ? (double)ml_result.total_slots/(double)ent_result.activations : 0.0;";
                    c += "int payload_bytes = sizeof(void *); // each element of vector is a wrapped pointer";
                    c += "double MB = ((avg_size * ent_result.allocations * payload_bytes) / 1000000.0);";
                    c += "MB_total += MB;";
                    c += "theLog->logFormatted(\"%s| %-*s | %12d | %12d | %12.4f | %5d |\", prefix2, col1width, ml_name, ml_result.max_slots, ml_result.max_slots_id, avg_size, (int)MB);";
                    c += "}";
                }
                c += "theLog->logFormatted(\"%s%s\", prefix2, row_sep);";
                c += "theLog->logFormatted(\"%s| %-*s |              |              |              | %5d |\", prefix2,  col1width, \"All\", (int)MB_total);";
                c += "theLog->logFormatted(\"%s%s\", prefix2, row_sep);";
                c += "theLog->logFormatted(\"%s%s\", prefix2, \"Note: MB does not include entity members\");";
                c += "theLog->logFormatted(\"%s\", prefix0);";
                c += "} // multilinks";
                c += "";
            }
            if (ent->pp_agent_events.size()) {
                c += "{ // events";
                c += "auto table_title = \"    " + ent->name + " Events\";";
                std::string col1header = "event";
                int col1width = col1header.length();
                for (auto evt : ent->pp_agent_events) { col1width = std::max<int>(col1width, evt->event_name.length()); };
                c += "int col1width = " + to_string(col1width) + ";";
                c += "const char * col1header = \"" + col1header + "\";";
                std::string row_sep = "+-" + std::string(col1width, '-') +
                    "-+--------------+--------------+--------------+--------------+--------------+-------+";
                c += "auto row_sep = \"" + row_sep + "\";";
                c += "int table_width = (int)strlen(row_sep);";
                c += "theLog->logFormatted(\"%s+%.*s+\", prefix2, table_width - 2, dashes);";
                c += "theLog->logFormatted(\"%s| %-*s |\", prefix2, table_width - 4, table_title);";
                c += "theLog->logFormatted(\"%s%s\", prefix2, row_sep);";
                c += "theLog->logFormatted(\"%s| %-*s |   time calcs |     censored |  occurrences |   per entity | max in queue |    MB |\", prefix2, col1width, col1header);";
                c += "theLog->logFormatted(\"%s%s\", prefix2, row_sep);";
                c += "double MB_total = 0;";
                for (auto evt : ent->pp_agent_events) {
                    // Get resource use information for each event in the entity
                    // The static thread_local om_null_entity is used to access the static thread_local function and counters
                    c += "{";
                    c += "// Resources for " + evt->event_name;
                    c += "auto evt_name = \"" + evt->event_name + "\";";
                    c += "auto evt_result = " + ent->name + "::om_null_entity." + evt->name + ".resource_use();";
                    c += "int max_in_queue = (int)evt_result.max_in_queue;";
                    c += "double per_entity = ent_result.activations ? (double)evt_result.occurrences / (double)ent_result.activations : 0.0;";
                    c += "double MB = (max_in_queue * set_node_bytes) / 1000000.0;";
                    c += "MB_total += MB;";
                    c += "theLog->logFormatted(\"%s| %-*s | %12d | %12d | %12d | %12.4f | %12d | %5d |\", prefix2, col1width, evt_name, evt_result.time_calculations, evt_result.censored_times, evt_result.occurrences, per_entity, max_in_queue, (int)MB);";
                    c += "}";
                }
                c += "theLog->logFormatted(\"%s%s\", prefix2, row_sep);";
                c += "theLog->logFormatted(\"%s| %-*s |              |              |              |              |              | %5d |\", prefix2,  col1width, \"All\", (int)MB_total);";
                c += "theLog->logFormatted(\"%s%s\", prefix2, row_sep);";
                c += "theLog->logFormatted(\"%s%s\", prefix2, \"Note: MB does not include entity members\");";
                c += "theLog->logFormatted(\"%s\", prefix0);";
                c += "} // events";
                c += "";
            }
            if (ent->pp_agent_entity_sets.size()) {
                c += "{ // sets";
                c += "auto table_title = \"    " + ent->name + " Sets\";";
                std::string col1header = "set";
                int col1width = col1header.length();
                for (auto entset : ent->pp_agent_entity_sets) { col1width = std::max<int>(col1width, entset->name.length()); };
                c += "int col1width = " + to_string(col1width) + ";";
                c += "const char * col1header = \"" + col1header + "\";";
                std::string row_sep = "+-" + std::string(col1width, '-') +
                    "-+-------+----------+----------+--------------+----------+-------+";
                c += "auto row_sep = \"" + row_sep + "\";";
                c += "int table_width = (int)strlen(row_sep);";
                c += "theLog->logFormatted(\"%s+%.*s+\", prefix2, table_width - 2, dashes);";
                c += "theLog->logFormatted(\"%s| %-*s |\", prefix2, table_width - 4, table_title);";
                c += "theLog->logFormatted(\"%s%s\", prefix2, row_sep);";
                c += "theLog->logFormatted(\"%s| %-*s |  rank |    cells |  inserts |   per entity |  max pop |    MB |\", prefix2, col1width, col1header);";
                c += "theLog->logFormatted(\"%s%s\", prefix2, row_sep);";
                c += "double MB_total = 0;";
                for (auto entset : ent->pp_agent_entity_sets) {
                    // Get resource use information for each entity set in the entity
                    // The static thread_local om_null_entity is used to access the static thread_local function and counters
                    c += "{";
                    c += "// Resources for " + entset->name;
                    c += "auto entset_name = \"" + entset->name + "\";";
                    c += "int dimension_count = " + to_string(entset->dimension_count()) + ";";
                    c += "int cell_count = " + to_string(entset->cell_count()) + ";";
                    c += "auto entset_result = " + entset->resource_use_gfn->name + "();"; // call the resource use reporting function for this entity set
                    c += "double inserts_avg = (double)entset_result.total_inserts / (double)ent_result.activations;";
                    c += "int max_count = (int)entset_result.max_count;";
                    c += "int bytes_fixed = sizeof(*" + entset->name + "); // fixed portion for cells";
                    c += "int bytes_nodes = max_count * rb_node_bytes; // variable portion for rb_tree nodes";
                    c += "double MB = (bytes_fixed + bytes_nodes) / 1000000.0;";
                    c += "MB_total += MB;";
                    c += "theLog->logFormatted(\"%s| %-*s | %5d | %8d | %8d | %12.4f | %8d | %5d |\", prefix2, col1width, entset_name, dimension_count, cell_count, entset_result.total_inserts, inserts_avg, max_count, (int)MB);";
                    c += "}";
                }
                c += "theLog->logFormatted(\"%s%s\", prefix2, row_sep);";
                c += "theLog->logFormatted(\"%s| %-*s |       |          |          |              |          | %5d |\", prefix2,  col1width, \"All\", (int)MB_total);";
                c += "theLog->logFormatted(\"%s%s\", prefix2, row_sep);";
                c += "theLog->logFormatted(\"%s%s\", prefix2, \"Note: MB does not include entity members\");";
                c += "theLog->logFormatted(\"%s\", prefix0);";
                c += "} // sets";
                c += "";
            }
            if (ent->pp_entity_tables.size()) {
                c += "{ // tables";
                c += "auto table_title = \"    " + ent->name + " Tables\";";
                std::string col1header = "table";
                int col1width = col1header.length();
                for (auto tbl : ent->pp_entity_tables) { col1width = std::max<int>(col1width, tbl->name.length()); };
                c += "int col1width = " + to_string(col1width) + ";";
                c += "int suppressed_count = 0; // number of tables suppressed at runtime";
                c += "const char * col1header = \"" + col1header + "\";";
                std::string row_sep = "+-" + std::string(col1width, '-') +
                    "-+-------+----------+-------+-------+-------+----------+--------------+-------+";
                c += "auto row_sep = \"" + row_sep + "\";";
                c += "int table_width = (int)strlen(row_sep);";
                c += "theLog->logFormatted(\"%s+%.*s+\", prefix2, table_width - 2, dashes);";
                c += "theLog->logFormatted(\"%s| %-*s |\", prefix2, table_width - 4, table_title);";
                c += "theLog->logFormatted(\"%s%s\", prefix2, row_sep);";
                c += "theLog->logFormatted(\"%s| %-*s |  rank |    cells | accum | measr | colls | incrmnts |   per entity |    MB |\", prefix2, col1width, col1header);";
                c += "theLog->logFormatted(\"%s%s\", prefix2, row_sep);";
                c += "double MB_total = 0;";
                for (auto tbl : ent->pp_entity_tables) {
                    // Get resource use information for each entity table in the entity
                    // The static thread_local om_null_entity is used to access the static thread_local function and counters
                    c += "{ // table " + tbl->name;
                    c += "auto tbl_name = \"" + tbl->name + "\";";
                    c += "bool instantiated = " + tbl->cxx_instance + "; // is nullptr if suppressed at runtime";
                    c += "int dimension_count = " + to_string(tbl->dimension_count()) + ";";
                    c += "int cell_count = " + to_string(tbl->cell_count()) + ";";
                    c += "int accumulator_count = " + to_string(tbl->accumulator_count()) + ";";
                    c += "int measure_count = " + to_string(tbl->measure_count()) + ";";
                    c += "int collection_count = " + to_string(tbl->n_collections) + ";";
                    c += "auto tbl_result = " + tbl->resource_use_gfn->name + "();"; // call the resource use reporting function for this entity set
                    c += "if (instantiated) {";
                    c += "double MB = (8 * cell_count * accumulator_count) / 1000000.0;";
                    c += "theLog->logFormatted(\"%s| %-*s | %5d | %8d | %5d | %5d | %5d | %8d | %12.4f | %5d |\", prefix2, col1width, tbl_name, dimension_count, cell_count, accumulator_count, measure_count, collection_count, tbl_result.total_increments, (double)tbl_result.total_increments/(double)ent_result.activations, (int)MB);";
                    c += "MB_total += MB;";
                    c += "}";
                    c += "else {";
                    c += "++suppressed_count;";
                    c += "}";
                    c += "} // table " + tbl->name;
                    c += "";
                }
                c += "theLog->logFormatted(\"%s%s\", prefix2, row_sep);";
                c += "theLog->logFormatted(\"%s| %-*s |       |          |       |       |       |          |              | %5d |\", prefix2,  col1width, \"All\", (int)MB_total);";
                c += "theLog->logFormatted(\"%s%s\", prefix2, row_sep);";
                c += "if (suppressed_count > 0) {";
                c += "theLog->logFormatted(\"%s %d %s table(s) suppressed at runtime\", prefix2, suppressed_count, ent_name);";
                c += "}";
                c += "theLog->logFormatted(\"%s%s\", prefix2, \"Note: MB does not include entity members\");";
                c += "theLog->logFormatted(\"%s\", prefix0);";
                c += "} // tables";
                c += "";
            }
            {
                c += "{ // entity members (detail)";
                c += "auto table_title = \"    " + ent->name + " Members (detail)\";";
                std::string col1header = "member                 "; // extra padding to force a minimum width for total block at bottom of table
                int col1width = col1header.length();
                for (auto dm : dml) { col1width = std::max<int>(col1width, 4 + dm->pretty_name().length()); }; // add +4 in case indented twice
                col1width = std::min<int>(80, col1width); // impose a maximum to the computed col1width
                c += "int col1width = " + to_string(col1width) + ";";
                c += "const char * col1header = \"" + col1header + "\";";
                std::string row_sep = "+-" + std::string(col1width, '-') +
                    "-+-------+";
                c += "auto row_sep = \"" + row_sep + "\";";
                c += "int table_width = (int)strlen(row_sep);";
                c += "int bytes_total = 0;";
                c += "theLog->logFormatted(\"%s+%.*s+\", prefix2, table_width - 2, dashes);";
                c += "theLog->logFormatted(\"%s| %-*s |\", prefix2, table_width - 4, table_title);";
                c += "theLog->logFormatted(\"%s%s\", prefix2, row_sep);";
                c += "theLog->logFormatted(\"%s| %-*s | bytes |\", prefix2, col1width, col1header);";
                c += "theLog->logFormatted(\"%s%s\", prefix2, row_sep);";
                c += "theLog->logFormatted(\"%s| %-*s | %5s |\", prefix2, col1width, \"Attributes:\", \"\");";
                c += "theLog->logFormatted(\"%s| %-*s | %5s |\", prefix2, col1width, \"  Built-in:\", \"\");";
                // The static thread_local <entity>::om_null_entity is used to access the static thread_local member to determine its size in bytes
                for (auto dm : dml) {
                    if (!dm->is_builtin_attribute()) continue;
                    c += "{";
                    c += "int bytes = (int)sizeof(" + ent->name + "::om_null_entity." + dm->name + ");";
                    c += "bytes_total += bytes;";
                    c += "theLog->logFormatted(\"%s| %-*s | %5d |\", prefix2, col1width, \"    " + dm->pretty_name() + "\", bytes);";
                    c += "}";
                }
                c += "theLog->logFormatted(\"%s| %-*s | %5s |\", prefix2, col1width, \"  Simple:\", \"\");";
                for (auto dm : dml) {
                    if (!dm->is_simple_attribute()) continue;
                    c += "{";
                    c += "int bytes = (int)sizeof(" + ent->name + "::om_null_entity." + dm->name + ");";
                    c += "bytes_total += bytes;";
                    c += "theLog->logFormatted(\"%s| %-*s | %5d |\", prefix2, col1width, \"    " + dm->pretty_name() + "\", bytes);";
                    c += "}";
                }
                c += "theLog->logFormatted(\"%s| %-*s | %5s |\", prefix2, col1width, \"  Maintained:\", \"\");";
                for (auto dm : dml) {
                    if (!dm->is_maintained_attribute()) continue;
                    c += "{";
                    c += "int bytes = (int)sizeof(" + ent->name + "::om_null_entity." + dm->name + ");";
                    c += "bytes_total += bytes;";
                    c += "theLog->logFormatted(\"%s| %-*s | %5d |\", prefix2, col1width, \"    " + dm->pretty_name() + "\", bytes);";
                    c += "}";
                }
                c += "theLog->logFormatted(\"%s| %-*s | %5s |\", prefix2, col1width, \"  Link:\", \"\");";
                for (auto dm : dml) {
                    if (!dm->is_link_attribute()) continue;
                    c += "{";
                    c += "int bytes = (int)sizeof(" + ent->name + "::om_null_entity." + dm->name + ");";
                    c += "bytes_total += bytes;";
                    c += "theLog->logFormatted(\"%s| %-*s | %5d |\", prefix2, col1width, \"    " + dm->pretty_name() + "\", bytes);";
                    c += "}";
                }
                c += "theLog->logFormatted(\"%s| %-*s | %-5s |\", prefix2, col1width, \"Events:\", \"\");";
                for (auto dm : dml) {
                    if (!dm->is_event()) continue;
                    c += "{";
                    c += "int bytes = (int)sizeof(" + ent->name + "::om_null_entity." + dm->name + ");";
                    c += "bytes_total += bytes;";
                    c += "theLog->logFormatted(\"%s| %-*s | %5d |\", prefix2, col1width, \"    " + dm->pretty_name() + "\", bytes);";
                    c += "}";
                }
                c += "theLog->logFormatted(\"%s| %-*s | %5s |\", prefix2, col1width, \"Increments:\", \"\");";
                for (auto dm : dml) {
                    if (!dm->is_increment()) continue;
                    c += "{";
                    c += "int bytes = (int)sizeof(" + ent->name + "::om_null_entity." + dm->name + ");";
                    c += "bytes_total += bytes;";
                    c += "theLog->logFormatted(\"%s| %-*s | %5d |\", prefix2, col1width, \"    " + dm->pretty_name() + "\", bytes);";
                    c += "}";
                }
                c += "theLog->logFormatted(\"%s| %-*s | %5s |\", prefix2, col1width, \"Multilink:\", \"\");";
                for (auto dm : dml) {
                    if (!dm->is_multilink()) continue;
                    c += "{";
                    c += "int bytes = (int)sizeof(" + ent->name + "::om_null_entity." + dm->name + ");";
                    c += "bytes_total += bytes;";
                    c += "theLog->logFormatted(\"%s| %-*s | %5d |\", prefix2, col1width, \"    " + dm->pretty_name() + "\", bytes);";
                    c += "}";
                }
                c += "theLog->logFormatted(\"%s| %-*s | %5s |\", prefix2, col1width, \"Internal:\", \"\");";
                for (auto dm : dml) {
                    if (!dm->is_internal()) continue;
                    c += "{";
                    c += "int bytes = (int)sizeof(" + ent->name + "::om_null_entity." + dm->name + ");";
                    c += "bytes_total += bytes;";
                    c += "theLog->logFormatted(\"%s| %-*s | %5d |\", prefix2, col1width, \"    " + dm->pretty_name() + "\", bytes);";
                    c += "}";
                }
                c += "theLog->logFormatted(\"%s| %-*s | %5s |\", prefix2, col1width, \"Array:\", \"\");";
                for (auto dm : dml) {
                    if (!dm->is_array()) continue;
                    c += "{";
                    c += "int bytes = (int)sizeof(" + ent->name + "::om_null_entity." + dm->name + ");";
                    c += "bytes_total += bytes;";
                    c += "theLog->logFormatted(\"%s| %-*s | %5d |\", prefix2, col1width, \"    " + dm->pretty_name() + "\", bytes);";
                    c += "}";
                }
                c += "theLog->logFormatted(\"%s| %-*s | %5s |\", prefix2, col1width, \"Foreign:\", \"\");";
                for (auto dm : dml) {
                    if (!dm->is_foreign()) continue;
                    c += "{";
                    c += "int bytes = (int)sizeof(" + ent->name + "::om_null_entity." + dm->name + ");";
                    c += "bytes_total += bytes;";
                    c += "theLog->logFormatted(\"%s| %-*s | %5d |\", prefix2, col1width, \"    " + dm->pretty_name() + "\", bytes);";
                    c += "}";
                }
                c += "theLog->logFormatted(\"%s%s\", prefix2, row_sep);";
                c += "theLog->logFormatted(\"%s| %-*s | %5d |\", prefix2, col1width, \"Sum of member bytes\", bytes_total);";
                c += "theLog->logFormatted(\"%s| %-*s | %5d |\", prefix2, col1width, \"Bytes per entity\", ent_bytes);";
                c += "theLog->logFormatted(\"%s| %-*s | %5.1f |\", prefix2, col1width, \"Storage efficiency (%)\", 100.0 * (double)bytes_total / (double)ent_bytes);";
                c += "theLog->logFormatted(\"%s%s\", prefix2, row_sep);";
                c += "theLog->logFormatted(\"%s\", prefix0);";
                c += "}";
            }
            c += "} // End resource use report for " + ent->name;
        }
        if (Symbol::pp_all_derived_tables.size()) {
            c += "auto section_title = \"*  Resource Use Detail for Derived Tables  *\";";
            c += "theLog->logFormatted(\"%s%.*s\", prefix1, strlen(section_title), stars);";
            c += "theLog->logFormatted(\"%s%s\", prefix1, section_title);";
            c += "theLog->logFormatted(\"%s%.*s\", prefix1, strlen(section_title), stars);";
            c += "theLog->logFormatted(\"%s\", prefix1);";
            c += "{ // derived tables";
            c += "auto table_title = \"     Derived Tables\";";
            std::string col1header = "derived table";
            int col1width = col1header.length();
            for (auto tbl : Symbol::pp_all_derived_tables) { col1width = std::max<int>(col1width, tbl->name.length()); };
            c += "int col1width = " + to_string(col1width) + ";";
            c += "int suppressed_count = 0; // number of tables suppressed at runtime";
            c += "const char * col1header = \"" + col1header + "\";";
            std::string row_sep = "+-" + std::string(col1width, '-') +
                "-+-------+----------+-------+-------+";
            c += "auto row_sep = \"" + row_sep + "\";";
            c += "int table_width = (int)strlen(row_sep);";
            c += "theLog->logFormatted(\"%s+%.*s+\", prefix2, table_width - 2, dashes);";
            c += "theLog->logFormatted(\"%s| %-*s |\", prefix2, table_width - 4, table_title);";
            c += "theLog->logFormatted(\"%s%s\", prefix2, row_sep);";
            c += "theLog->logFormatted(\"%s| %-*s |  rank |    cells | measr |    MB |\", prefix2, col1width, col1header);";
            c += "theLog->logFormatted(\"%s%s\", prefix2, row_sep);";
            c += "double MB_total = 0;";
            for (auto tbl : Symbol::pp_all_derived_tables) {
                c += "{ // table " + tbl->name;
                c += "auto tbl_name = \"" + tbl->name + "\";";
                c += "bool instantiated = " + tbl->cxx_instance + "; // is nullptr if suppressed at runtime";
                c += "int dimension_count = " + to_string(tbl->dimension_count()) + ";";
                c += "int cell_count = " + to_string(tbl->cell_count()) + ";";
                c += "int measure_count = " + to_string(tbl->measure_count()) + ";";
                c += "if (instantiated) {";
                c += "double MB = (cell_count * measure_count * sizeof(double)) / 1000000.0;";
                c += "theLog->logFormatted(\"%s| %-*s | %5d | %8d | %5d | %5d |\", prefix2, col1width, tbl_name, dimension_count, cell_count, measure_count, (int)MB);";
                c += "MB_total += MB;";
                c += "}";
                c += "else {";
                c += "++suppressed_count;";
                c += "}";
                c += "} // table " + tbl->name;
            }
            c += "theLog->logFormatted(\"%s%s\", prefix2, row_sep);";
            c += "theLog->logFormatted(\"%s| %-*s |       |          |       | %5d |\", prefix2,  col1width, \"All\", (int)MB_total);";
            c += "theLog->logFormatted(\"%s%s\", prefix2, row_sep);";
            c += "if (suppressed_count > 0) {";
            c += "theLog->logFormatted(\"%s %d derived table(s) suppressed at runtime\", prefix2, suppressed_count);";
            c += "}";
            c += "theLog->logFormatted(\"%s\", prefix0);";
            c += "} // derived tables";
        }

        c += "theLog->logFormatted(\"%sResource Use Report - End\", prefix0);";

        c += "} // om_resource_use_on";
    }

    c += "";
    c += "BaseEvent::finalize_simulation_runtime();";
    c += "BaseEntity::finalize_simulation_runtime();";
    c += "}"; // RunModel
    c += "";
}

void CodeGen::do_API_entries()
{
	c.smart_indenting ( false );
	c += "namespace openm";
	c += "{";
	c += "    // set entry points to the code generated by openM++ compiler";
	c += "    static ModelEntryHolder theModelEntry(::RunOnce, ::RunInit, ::ModelStartup, ::RunModel, ::ModelShutdown);";
	c += "}";
	c += "";
	c.smart_indenting ( true );
}

void CodeGen::do_ParameterNameSize(void)
{ 
    size_t nParam = 0;
    for (const auto parameter : Symbol::pp_all_parameters) {
        if (parameter->source == ParameterSymbol::scenario_parameter) nParam++;
    }

    c += "namespace openm";
    c += "{";
    c += "// size of parameters list: number of model input parameters";
    c += "const size_t PARAMETER_NAME_ARR_LEN = " + to_string(nParam) + ";";
    c += "";
    c += "// list of model input parameters name, type and size";
    c += "const ParameterNameSizeItem parameterNameSizeArr[PARAMETER_NAME_ARR_LEN] =";
    c += "{";
    size_t np = nParam;
    for (const auto parameter : Symbol::pp_all_parameters) {
        if (parameter->source == ParameterSymbol::scenario_parameter) {
            c += "{" + parameter->cxx_parameter_name_type_size() + ((--np > 0) ? "}," : "}");
        }
    }
    c += "};";
    c += "}";
    c += "";
}
