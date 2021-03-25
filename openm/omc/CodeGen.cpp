/**
 * @file    CodeGen.cpp
 * Implements the code generation class.
 */
// Copyright (c) 2013-2015 OpenM++
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

    // Get definition code associated with the model_type symbol
    c += model_type_symbol->cxx_definition_global();
    c += "";

    // Get definition code associated with the model symbol (name, time stamp)
    c += model_symbol->cxx_definition_global();
    c += "";

	c += "using namespace openm;";
	c += "";

    if (Symbol::option_event_trace) {
        // Let the run-time know if trace event is enabled
        c += "const bool BaseEntity::event_trace_capable = true;";
        // if event_trace option is on, tracing is active unless turned off
        c += "thread_local bool BaseEntity::event_trace_on = true;";
    }
    else {
        // Let the run-time know if trace event is enabled
        c += "const bool BaseEntity::event_trace_capable = false;";
        // independent of the event_trace option, this static member must be defined
        c += "thread_local bool BaseEntity::event_trace_on = false;";
    }

    // static members for event trace control
    c += "enum BaseEntity::et_report_style BaseEntity::event_trace_report_style = BaseEntity::eModgen;";
    c += "bool BaseEntity::event_trace_show_scheduling = true;";
    c += "double BaseEntity::event_trace_minimum_time = -std::numeric_limits<double>::infinity();";
    c += "double BaseEntity::event_trace_maximum_time = std::numeric_limits<double>::infinity();";
    c += "std::unordered_set<int> BaseEntity::event_trace_selected_entities;";
    c += "std::unordered_set<int> BaseEntity::event_trace_selected_events;";
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
        "Fundamental types" + model_str,
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
    c += "extern void process_trace_options(IRunBase* const i_runBase);";
    c += "// Process model dev options for EventTrace";
    c += "process_trace_options(i_runBase);";
    c += "";
    c += "theLog->logMsg(\"Get scenario parameters for process\");";
    c += "";
    for (auto parameter : Symbol::pp_all_parameters) {
        if (parameter->source == ParameterSymbol::scenario_parameter) {
            c += parameter->cxx_read_parameter();
        }
    }
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
        c += agent->name + "::om_null_agent.om_assign_member_offsets();";
        c += agent->name + "::om_null_agent.om_initialize_data_members0();";
        c += "";
    }

    c += "// Entity static initialization part 2: Initialize null entity dependent attributes";
    for (auto agent : Symbol::pp_all_agents) {
        c += "// Entity - " + agent->name;
        c += agent->name + "::om_null_agent.om_initialize_identity_attributes();";
        c += agent->name + "::om_null_agent.om_initialize_derived_attributes();";
        c += agent->name + "::om_null_agent.om_reset_derived_attributes();";
    }
    c += "";

    c += "// Entity table instantiation";
    for (auto et : Symbol::pp_all_entity_tables) {
        c += "assert(!" + et->cxx_instance + "); ";
        if (!et->is_internal) {
            c += "if (!is_suppressed_compute(\"" + et->name + "\", i_model)) {";
            c += et->cxx_instance + " = new " + et->cxx_type + "(" + et->cxx_initializer() + ");";
            c += "}";
        }
        else {
            c += et->cxx_instance + " = new " + et->cxx_type + "(" + et->cxx_initializer() + ");";
        }
    }
    c += "";

    c += "// Derived table instantiation";
    for (auto dt : Symbol::pp_all_derived_tables) {
        c += "assert(!" + dt->cxx_instance + "); ";
        if (!dt->is_internal) {
            c += "if (!is_suppressed_compute(\"" + dt->name + "\", i_model)) {";
            c += dt->cxx_instance + " = new " + dt->cxx_type + "(" + dt->cxx_initializer() + ");";
            c += "}";
        }
        else {
            c += dt->cxx_instance + " = new " + dt->cxx_type + "(" + dt->cxx_initializer() + ");";
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
    c += "// write entity tables (accumulators) and release accumulators memory";
    for ( auto table : Symbol::pp_all_entity_tables ) {
        if (!table->is_internal) {
            c += "if (!is_suppressed_write(\"" + table->name + "\", i_model)) i_model->writeOutputTable(\"" +
                table->name + "\", " + table->cxx_instance + "->n_cells, " + table->cxx_instance + "->acc_storage);";
        }
    }
    c += "// at this point table->acc[k][j] will cause memory access violation";
    c += "";

    c += "// write derived tables (measures) and release measures memory";
    for ( auto derived_table : Symbol::pp_all_derived_tables ) {
        if (!derived_table->is_internal) {
            c += "if (" + derived_table->cxx_instance + ") i_model->writeOutputTable(\"" +
                derived_table->name + "\", " + 
                derived_table->cxx_instance + "->n_cells, " + 
                derived_table->cxx_instance + "->measure_storage);";
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

	h += "// forward declarations of model agent classes (for links)";
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
	    h += "//";
	    h += "// function members in " + agent->name + " agent";
	    h += "//";
	    h += "";

	    h += "// operator overload for entity comparison based on entity_id";
        h += "bool operator< ( " + agent->name + " & rhs )";
        h += "{";
        h += "return entity_id < rhs.entity_id;";
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
	    h += "// Entity table increment members in " + agent->name + " entity";
	    h += "//";
	    h += "";
        for ( auto table : agent->pp_entity_tables ) {
            auto incr = table->increment;
            assert(incr); // logic guarantee - every entity table has an associated increment in the entity.
            h += incr->cxx_declaration_agent();
            c += incr->cxx_definition_agent();
        }
	    h += "";
	    c += "";

	    h += "";
	    h += "//";
	    h += "// data members in " + agent->name + " agent";
	    h += "//";
	    h += "";
        for ( auto data_member : agent->pp_agent_data_members ) {
            h += data_member->cxx_declaration_agent();
            c += data_member->cxx_definition_agent();
        }
	    h += "";
	    c += "";

        h += "// The declaration of the static member " + agent->name;
        h += "// used to retrieve (zero) values when dereferencing nullptr link attributes.";
        h += "static thread_local " + agent->name + " " + "om_null_agent;";

	    h += "}; // class " + agent->name;
	    h += "";

        c += "// The definition of the static member " + agent->name;
        c += "// used to retrieve (zero) values when dereferencing nullptr link attributes.";
        c += "thread_local " + agent->name + " " + agent->name + "::om_null_agent;";
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
    c += "agents = new std::list<BaseEntity *>;";
    for ( auto agent : Symbol::pp_all_agents ) {
        // e.g. Person::zombies = new forward_list<Person *>;";
        c += agent->name + "::zombies = new std::forward_list<" + agent->name + " *>;";
        c += agent->name + "::available = new std::forward_list<" + agent->name + " *>;";
    }
    c += "event_trace_on = event_trace_capable;";
    c += "}";
    c += "";

    c += "void BaseEntity::finalize_simulation_runtime()";
    c += "{";
    c += "assert(agents->empty());";
    c += "delete agents;";
    c += "agents = nullptr;";
    c += "";
    for (auto agent : Symbol::pp_all_agents) {
        c += "assert(" + agent->name + "::zombies->empty());";
        c += "delete " + agent->name + "::zombies;";
        c += agent->name + "::zombies = nullptr;";
        c += "";
        c += "for (auto ent : *" + agent->name + "::available) {";
        c += "delete ent;";
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
            h += es->cxx_declaration_global();
            c += es->cxx_definition_global();
        }

	    h += "";
	    c += "";
    }
}

void CodeGen::do_entity_tables()
{
	h += "// entity tables";
	c += "// entity tables";

	for ( auto table : Symbol::pp_all_entity_tables ) {
        h += table->cxx_declaration_global();
        c += table->cxx_definition_global();
        if (!table->is_internal) {
            table->populate_metadata(metaRows);
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
    c += "const std::map<std::string, std::pair<int, int>> om_table_measure = {";
    for (auto table : Symbol::pp_all_tables) {
        for (auto measure : table->pp_measures) {
            auto key = table->name + "." + measure->measure_name;
            auto table_id = to_string(table->pp_table_id);
            auto measure_id = to_string(measure->index);
            c += "{\"" + key + "\", {" + table_id + ", " + measure_id + "} },";
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
    c += "thread_local std::list<BaseEntity *> *BaseEntity::agents = nullptr;";
    c += "";
    c += "// definition of event_id of current event (declaration in Event.h)";
    c += "thread_local int BaseEvent::current_event_id;";
    c += "";
    c += "// definition of entity_id of current event (declaration in Event.h)";
    c += "thread_local int BaseEvent::current_entity_id;";
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
            c += "return (event_id < " + std::to_string(id) + ") ? event_name[event_id] : \"\";";
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
    c += "int mem_id = i_model->subValueId();";
    c += "int mem_count = i_model->subValueCount();";
    c += "RunSimulation(mem_id, mem_count, i_model); // Defined by the model framework, generally in a 'use' module";
    c += "";
    c += "BaseEvent::finalize_simulation_runtime();";
    c += "BaseEntity::finalize_simulation_runtime();";
    c += "}";
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
