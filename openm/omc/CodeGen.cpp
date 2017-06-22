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
	do_table_interface();
	do_agents();
	do_entity_sets();
    do_event_queue();

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
    t0 += "#include \"omc/fixed_precision_float.h\"";
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
        c += "const bool BaseEvent::trace_event_enabled = true;";
        // if event_trace option is on, tracing is active unless turned off
        c += "thread_local bool BaseEvent::trace_event_on = true;";
    }
    else {
        // Let the run-time know if trace event is enabled
        c += "const bool BaseEvent::trace_event_enabled = false;";
        // independent of the event_trace option, this static member must be defined
        c += "thread_local bool BaseEvent::trace_event_on = false;";
    }

    // Let the run-time know whether to generate a running checksum for events
    if (Symbol::option_case_checksum) {
        c += "const bool BaseEvent::event_checksum_enabled = true;";
    }
    else {
        c += "const bool BaseEvent::event_checksum_enabled = false;";
    }
    // The checksum must be defined in any case
    c += "thread_local double BaseEvent::event_checksum_value = 0.0;";

    c += "";

    // om_fixed_parms.cpp
	z += "";
    z += "#include \"omc/cumrate.h\"";
    z += "#include \"om_types1.h\"";
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
    c += "const char * OM_MODEL_NAME =\"" + metaRows.modelDic.name + "\";";
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
    c += "static list<string> langLst = theLog->getLanguages();";
    c += "";
    c += "for (const string & lang : langLst) {";
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
	h += "// model parameters";
    for ( auto parameter : Symbol::pp_all_parameters ) {
        h += parameter->cxx_declaration_global();
        if (parameter->cumrate) {
            h += parameter->lookup_fn->cxx_declaration_global();
        }
    }
	h += "";

	// parameter definitions & initializers
	c += "// model parameters (scenario, derived, missing)";
	z += "// model parameters (fixed)";
    for (auto parameter : Symbol::pp_all_parameters) {
        if (parameter->source == ParameterSymbol::fixed_parameter) {
            // place definition (with initializer) in the cpp module for fixed parameters
            z += parameter->cxx_definition_global();
        }
        else {
            c += "";
            c += parameter->cxx_definition_global();
        }
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
    c += "theLog->logMsg(\"Process fixed and missing parameters\");";
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

        // TODO transform fixed haz1rate parameters

    }
    if (any_missing_parameters) {
        m += "};";
        theLog->logMsg("Missing parameters written to Missing.dat.tmp");
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

    c += "theLog->logMsg(\"Get scenario parameters\");";
    for (auto parameter : Symbol::pp_all_parameters) {
        if (parameter->source == ParameterSymbol::scenario_parameter) {
            c += parameter->cxx_read_parameter();
        }
    }
    c += "";

	c += "}";
	c += "";
}

void CodeGen::do_ModelStartup()
{
    c += "// Model startup method: Initialization for a simulation thread";
    c += "void ModelStartup(IModel * const i_model)";
    c += "{";

    c += "// Bind scenario parameter references to thread local values (for scenario parameters).";
    c += "// Until this is done scenario parameter values are undefined and cannot be used by the model.";
    c += "";
    for (auto parameter : Symbol::pp_all_parameters) {
        // Process only scenario parameters in this for loop
        if (parameter->source != ParameterSymbol::scenario_parameter) continue;

        if (parameter->size() > 1) {
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
    c += "// Bind derived parameter references to thread local values (if derived parameter is an array).";
    c += "";
    for (auto parameter : Symbol::pp_all_parameters) {

        // Process only derived parameters in this for loop
        if (parameter->source != ParameterSymbol::derived_parameter) continue;

        if (parameter->size() > 1) {
            // om_value_NearestCity = reinterpret_cast<CITY *>(om_param_NearestCity);
            c += "om_value_" + parameter->name + " = "
                + "reinterpret_cast<" + parameter->pp_datatype->name + " *>" +
                +"(" + parameter->alternate_name() + ");";
        }
    }

    c += "";
    c += "// Parameters are now ready and can be used by the model.";
    c += "";

    c += "theLog->logMsg(\"compute derived parameters\");";
    auto & sg = Symbol::pre_simulation;
    if (sg.suffixes.size() > 0 || sg.ambiguous_count > 0) {
        for (size_t id = 0; id < sg.ambiguous_count; ++id) {
            c += sg.disambiguated_name(id) + "();";
        }
        for (auto suffix : sg.suffixes) {
            c += sg.prefix + suffix + "();";
        }
        c += "";
    }

    for (auto parameter : Symbol::pp_all_parameters) {
        // Process only derived parameters in this for loop
        if (parameter->source != ParameterSymbol::derived_parameter) continue;
        if (parameter->cumrate) {
            // prepare cumrate for derived parameter
            c += parameter->cxx_initialize_cumrate();
        }
    }

    c += "theLog->logMsg(\"Initialize invariant entity data\");";

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
        c += et->cxx_instance + " = new " + et->cxx_type + "(" + et->cxx_initializer() + ");";
    }
    c += "";

    c += "// Derived table instantiation";
    for (auto dt : Symbol::pp_all_derived_tables) {
        c += "assert(!" + dt->cxx_instance + "); ";
        c += dt->cxx_instance + " = new " + dt->cxx_type + "(" + dt->cxx_initializer() + ");";
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
	    c += "the" + table->name + "->extract_accumulators();";
	    c += "the" + table->name + "->scale_accumulators();";
    }
	c += "";

    c += "// compute table expressions using accumulators";
    for ( auto table : Symbol::pp_all_entity_tables ) {
	    c += "the" + table->name + "->compute_expressions();";
    }
    c += "";

    {
        auto & sg = Symbol::post_simulation;
        if (sg.suffixes.size() > 0 || sg.ambiguous_count > 0) {
            c += "theLog->logFormatted(\"member=%d Running post-simulation\", simulation_member);";
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
            c += "theLog->logFormatted(\"member=%d Computing derived tables\", simulation_member);";
            for (size_t id = 0; id < sg.ambiguous_count; ++id) {
                c += sg.disambiguated_name(id) + "();";
            }
            for (auto suffix : sg.suffixes) {
                c += sg.prefix + suffix + "();";
            }
            c += "";
        }
    }

    c += "theLog->logFormatted(\"member=%d writing output tables - start\", simulation_member);";
    c += "// write entity tables (accumulators) and release accumulators memory";
    for ( auto table : Symbol::pp_all_entity_tables ) {
        if (!table->is_internal) {
            c += "i_model->writeOutputTable(\"" +
                table->name + "\", the" + table->name + "->n_cells, the" + table->name + "->acc_storage);";
        }
    }
    c += "// at this point table->acc[k][j] will cause memory access violation";
    c += "";

    c += "// write derived tables (measures) and release measures memory";
    for ( auto derived_table : Symbol::pp_all_derived_tables ) {
        if (!derived_table->is_internal) {
            c += "i_model->writeOutputTable(\"" +
                derived_table->name + "\", " + 
                derived_table->cxx_instance + "->n_cells, " + 
                derived_table->cxx_instance + "->measure_storage);";
        }
    }
    c += "// at this point table->measure[k][j] will cause memory access violation";
    c += "";
    c += "theLog->logFormatted(\"member=%d write output tables - finish\", simulation_member);";

    c += "// Entity table destruction";
    for (auto table : Symbol::pp_all_entity_tables) {
        c += "assert(the" + table->name + "); ";
        c += "delete the" + table->name + ";";
        c += "the" + table->name + " = nullptr;";
    }
    c += "";

    c += "// Derived table destruction";
    for (auto derived_table : Symbol::pp_all_derived_tables) {
        c += "assert(" + derived_table->cxx_instance + "); ";
        c += "delete " + derived_table->cxx_instance + ";";
        c += derived_table->cxx_instance + " = nullptr;";
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

	    h += "// model agent classes";
        // e.g. class Person : public Agent<Person>
	    h += "class " + agent->name + " : public Agent<" + agent->name + ">";
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
    c += "void BaseAgent::free_all_zombies()";
    c += "{";
    for ( auto agent : Symbol::pp_all_agents ) {
        // e.g. Person::free_zombies();
        c += agent->name + "::free_zombies();";
    }
    c += "}";
    c += "";

    c += "void BaseAgent::initialize_simulation_runtime()";
    c += "{";
    c += "agents = new list<BaseAgent *>;";
    for ( auto agent : Symbol::pp_all_agents ) {
        // e.g. Person::zombies = new forward_list<Person *>;";
        c += agent->name + "::zombies = new forward_list<" + agent->name + " *>;";
        c += agent->name + "::available = new forward_list<" + agent->name + " *>;";
    }
    c += "}";
    c += "";

    c += "void BaseAgent::finalize_simulation_runtime()";
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

void CodeGen::do_table_interface()
{
    c += "const map<string, pair<int, int>> om_table_measure = {";
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

    c += "double * om_get_table_measure_address(int table_id, int measure_id, vector<int> indices)";
    c += "{";
    c += "switch (table_id) {";
    for (auto table : Symbol::pp_all_tables) {
        c += "case " + to_string(table->pp_table_id) + ": return " + table->cxx_instance + "->get_measure_address(measure_id, indices);";
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
    c += "thread_local set<BaseEvent *, less_deref<BaseEvent *> > *BaseEvent::event_queue = nullptr;";
    c += "";
    c += "// definition of dirty_events (declaration in Event.h)";
    c += "thread_local tailed_forward_list<BaseEvent *> *BaseEvent::dirty_events = nullptr;";
    c += "";
    c += "// definition of global_event_counter (declaration in Event.h)";
    c += "thread_local big_counter BaseEvent::global_event_counter;";
    c += "";
    c += "// definition of global_time (declaration in Event.h)";
    c += "thread_local Time *BaseEvent::global_time = nullptr;";
    c += "";
    c += "// definition of active agent list (declaration in Agent.h)";
    c += "thread_local list<BaseAgent *> *BaseAgent::agents = nullptr;";
    c += "";
}


void CodeGen::do_RunModel()
{
	c += "// Model simulation";
	c += "void RunModel(IModel * const i_model)";
    c += "{";

    c += "// initialize entity tables";
	for ( auto table : Symbol::pp_all_entity_tables ) {
        c += "the" + table->name + "->initialize_accumulators();";
    }
    c += "";
    c += "BaseEvent::initialize_simulation_runtime();";
    c += "BaseAgent::initialize_simulation_runtime();";
    c += "";
    c += "int mem_id = i_model->subValueId();";
    c += "int mem_count = i_model->subValueCount();";
    c += "RunSimulation(mem_id, mem_count); // Defined by the model framework, generally in a 'use' module";
    c += "";
    c += "BaseEvent::finalize_simulation_runtime();";
    c += "BaseAgent::finalize_simulation_runtime();";
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
