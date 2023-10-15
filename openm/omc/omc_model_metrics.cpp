/**
* @file    omc_model_metrics.cpp
*          
* Function to create Model Metrics Report
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
#include "EntitySymbol.h"
#include "EntityDataMemberSymbol.h"
#include "EnumerationSymbol.h"
#include "MaintainedAttributeSymbol.h"
#include "EntitySetSymbol.h"
#include "EntityTableSymbol.h"
#include "EntityEventSymbol.h"
#include "ParameterGroupSymbol.h"
#include "TableGroupSymbol.h"
#include "ParseContext.h"
#include "omc_file.h"
#include "omc_model_metrics.h"

using namespace std;
using namespace openm;
using namespace omc;

void do_model_metrics_report(string& outDir, string& model_name, CodeGen& cg)
{
    const string ModelMetrics_txt_name = "ModelMetrics.txt";
    ofstream rpt(makeFilePath(outDir.c_str(), ModelMetrics_txt_name.c_str()), ios::out | ios::trunc | ios::binary);
    if (rpt.fail()) {
        string msg = "omc : warning : Unable to open " + ModelMetrics_txt_name + " for writing.";
        theLog->logMsg(msg.c_str());
    }

    string tomb_stone;
    {
        // year-month-day for tombstone
        const size_t ymd_size = 11;
        char ymd[ymd_size];
        std::time_t time = std::time({});
        std::strftime(ymd, ymd_size, "%F", std::localtime(&time));
        tomb_stone = model_name + " " + ymd;
    }

    {
        // block for report title
        rpt << LT("Model Metrics for ") << tomb_stone << NO_LT("\n");
    }

    {
        // block for model code
        int mpp_source_files_count = Symbol::mpp_source_files.size();
        int mpp_source_files_lines = 0;
        int mpp_source_files_island_lines = 0;
        for (auto& s : Symbol::mpp_source_files) {
            {
                auto it = Symbol::source_files_line_count.find(s);
                if (it != Symbol::source_files_line_count.end()) {
                    mpp_source_files_lines += it->second;
                }
            }
            {
                auto it = Symbol::source_files_island_line_count.find(s);
                if (it != Symbol::source_files_island_line_count.end()) {
                    mpp_source_files_island_lines += it->second;
                }
            }
        }
        int mpp_source_files_cpp_lines = mpp_source_files_lines - mpp_source_files_island_lines;

        int use_source_files_count = Symbol::use_source_files.size();
        int use_source_files_lines = 0;
        int use_source_files_island_lines = 0;
        for (auto& s : Symbol::use_source_files) {
            auto it = Symbol::source_files_line_count.find(s);
            if (it != Symbol::source_files_line_count.end()) {
                use_source_files_lines += it->second;
            }
            {
                auto it = Symbol::source_files_island_line_count.find(s);
                if (it != Symbol::source_files_island_line_count.end()) {
                    use_source_files_island_lines += it->second;
                }
            }
        }
        int use_source_files_cpp_lines = use_source_files_lines - use_source_files_island_lines;

        int both_source_files_count = mpp_source_files_count + use_source_files_count;
        int both_source_files_lines = mpp_source_files_lines + use_source_files_lines;
        int both_source_files_cpp_lines = mpp_source_files_cpp_lines + use_source_files_cpp_lines;
        int both_source_files_island_lines = mpp_source_files_island_lines + use_source_files_island_lines;

        int all_source_files_count = mpp_source_files_count + use_source_files_count;
        int all_source_files_lines = mpp_source_files_lines + use_source_files_lines;

        rpt << "\n";
        rpt << LT("+-----------------------------------+\n");
        rpt << LT("| CODE INPUT (model source code)    |\n");
        rpt << LT("+-----------------+-------+---------+\n");
        rpt << LT("| Source          | Files |   Lines |\n");
        rpt << LT("+-----------------+-------+---------+\n");
        rpt << LT("| Model-specific  | ") << setw(5) << mpp_source_files_count << " | " << setw(7) << mpp_source_files_lines << " |\n";
        rpt << LT("|   Islands       | ") << setw(5) << " " << " | " << setw(7) << mpp_source_files_island_lines << " |\n";
        rpt << LT("|   C++           | ") << setw(5) << " " << " | " << setw(7) << mpp_source_files_cpp_lines << " |\n";
        rpt << LT("| Common          | ") << setw(5) << use_source_files_count << " | " << setw(7) << use_source_files_lines << " |\n";
        rpt << LT("|   Islands       | ") << setw(5) << " " << " | " << setw(7) << use_source_files_island_lines << " |\n";
        rpt << LT("|   C++           | ") << setw(5) << " " << " | " << setw(7) << use_source_files_cpp_lines << " |\n";
        rpt << LT("| Both            | ") << setw(5) << both_source_files_count << " | " << setw(7) << both_source_files_lines << " |\n";
        rpt << LT("|   Islands       | ") << setw(5) << " " << " | " << setw(7) << both_source_files_island_lines << " |\n";
        rpt << LT("|   C++           | ") << setw(5) << " " << " | " << setw(7) << both_source_files_cpp_lines << " |\n";
        rpt << LT("+-----------------+-------+---------+\n");
        rpt << LT("| Total           | ") << setw(5) << all_source_files_count << " | " << setw(7) << all_source_files_lines << " |\n";
        rpt << LT("+-----------------+-------+---------+\n");
        rpt << LT("Note: Does not include parameter data.\n");
    }

    {
        // block for output code
        int decl_cpp_lines = cg.h.size() + cg.t0.size() + cg.t1.size();;
        int defn_cpp_lines = cg.c.size();
        int total_cpp_lines = decl_cpp_lines + defn_cpp_lines;

        if (false) { // not used currnetly, but code kept in case used later
            int notify_count = 0;
            int side_effect_count = 0;

            for (auto& e : Symbol::pp_all_entities) {
                for (auto f : e->pp_functions) {
                    if (!f->empty()) {
                        if (f->is_side_effects()) {
                            ++side_effect_count;
                        }
                        else if (f->is_notify()) {
                            ++notify_count;
                        }
                    }
                }
            }
        }

        rpt << "\n";
        rpt << LT("+-----------------------------------------+\n");
        rpt << LT("| CODE OUTPUT (generated C++)             |\n");
        rpt << LT("+-----------------------+-------+---------+\n");
        rpt << LT("| Description           | Files |   Lines |\n");
        rpt << LT("+-----------------------+-------+---------+\n");
        rpt << LT("| Header (.h)           | ") << setw(5) << 3 << " | " << setw(7) << decl_cpp_lines << " |\n";
        rpt << LT("| Implementation (.cpp) | ") << setw(5) << 1 << " | " << setw(7) << defn_cpp_lines << " |\n";
        rpt << LT("+-----------------------+-------+---------+\n");
        rpt << LT("| Total                 | ") << setw(5) << 4 << " | " << setw(7) << total_cpp_lines << " |\n";
        rpt << LT("+-----------------------+-------+---------+\n");
        rpt << LT("Note: Table does not include C++ initializers for burned-in parameter data.\n");
    }

    {
        // block for parameters

        int fixed_count = 0;
        int fixed_cells = 0;
        int visible_count = 0;
        int visible_cells = 0;
        int hidden_count = 0;
        int hidden_cells = 0;
        int derived_count = 0;
        int derived_cells = 0;
        for (auto& p : Symbol::pp_all_parameters) {
            if (p->source == ParameterSymbol::parameter_source::scenario_parameter) {
                if (p->is_hidden) {
                    ++hidden_count;
                    hidden_cells += p->size();

                }
                else {
                    ++visible_count;
                    visible_cells += p->size();

                }
            }
            else if (p->is_derived()) {
                ++derived_count;
                derived_cells += p->size();
            }
            else {
                ++fixed_count;
                fixed_cells += p->size();
            }
        }
        int all_count = visible_count + hidden_count + derived_count + fixed_count;
        int all_cells = visible_cells + hidden_cells + derived_cells + fixed_cells;

        rpt << "\n";
        rpt << LT("+----------------------------------+\n");
        rpt << LT("| MODEL INPUT (parameters)         |\n");
        rpt << LT("+---------------+-------+----------+\n");
        rpt << LT("| Kind          | Count |    Cells |\n");
        rpt << LT("+---------------+-------+----------+\n");
        rpt << LT("| Visible       | ") << setw(5) << visible_count << " | " << setw(8) << visible_cells << " |\n";
        rpt << LT("| Hidden        | ") << setw(5) << hidden_count << " | " << setw(8) << hidden_cells << " |\n";
        rpt << LT("| Burned-in     | ") << setw(5) << fixed_count << " | " << setw(8) << fixed_cells << " |\n";
        rpt << LT("| Derived       | ") << setw(5) << derived_count << " | " << setw(8) << derived_cells << " |\n";
        rpt << LT("+---------------+-------+----------+\n");
        rpt << LT("| Total         | ") << setw(5) << all_count << " | " << setw(8) << all_cells << " |\n";
        rpt << LT("+---------------+-------+----------+\n");
        rpt << LT("Note: Burned-in includes fixed and suppressed parameters.\n");
    }

    {
        // block for tables

        int entity_visible_count = 0;
        int entity_visible_cells = 0;
        int entity_hidden_count = 0;
        int entity_hidden_cells = 0;
        int derived_visible_count = 0;
        int derived_visible_cells = 0;
        int derived_hidden_count = 0;
        int derived_hidden_cells = 0;
        for (auto& t : Symbol::pp_all_tables) {
            if (t->is_entity_table()) {
                if (t->is_hidden) {
                    ++entity_hidden_count;
                    entity_hidden_cells += t->cell_count() * t->measure_count();

                }
                else {
                    ++entity_visible_count;
                    entity_visible_cells += t->cell_count() * t->measure_count();
                }
            }
            else { // is a derived table
                if (t->is_hidden) {
                    ++derived_hidden_count;
                    derived_hidden_cells += t->cell_count() * t->measure_count();

                }
                else {
                    ++derived_visible_count;
                    derived_visible_cells += t->cell_count() * t->measure_count();
                }
            }
        }
        int all_count = entity_hidden_count + entity_visible_count
            + derived_hidden_count + derived_visible_count;
        int all_cells = entity_hidden_cells + entity_visible_cells
            + derived_hidden_cells + derived_visible_cells;

        int both_visible_count = entity_visible_count + derived_visible_count;
        int both_visible_cells = entity_visible_cells + derived_visible_cells;
        int both_hidden_count = entity_hidden_count + derived_hidden_count;
        int both_hidden_cells = entity_hidden_cells + derived_hidden_cells;

        rpt << "\n";
        rpt << LT("+----------------------------------+\n");
        rpt << LT("| MODEL OUTPUT (tables)            |\n");
        rpt << LT("+---------------+-------+----------+\n");
        rpt << LT("| Kind          | Count |    Cells |\n");
        rpt << LT("+---------------+-------+----------+\n");
        rpt << LT("| Entity        | ") << setw(5) << "" << " | " << setw(8) << "" << " |\n";
        rpt << LT("|   Visible     | ") << setw(5) << entity_visible_count << " | " << setw(8) << entity_visible_cells << " |\n";
        rpt << LT("|   Hidden      | ") << setw(5) << entity_hidden_count << " | " << setw(8) << entity_hidden_cells << " |\n";
        rpt << LT("| Derived       | ") << setw(5) << "" << " | " << setw(8) << "" << " |\n";
        rpt << LT("|   Visible     | ") << setw(5) << derived_visible_count << " | " << setw(8) << derived_visible_cells << " |\n";
        rpt << LT("|   Hidden      | ") << setw(5) << derived_hidden_count << " | " << setw(8) << derived_hidden_cells << " |\n";
        rpt << LT("| Both          | ") << setw(5) << "" << " | " << setw(8) << "" << " |\n";
        rpt << LT("|   Visible     | ") << setw(5) << both_visible_count << " | " << setw(8) << both_visible_cells << " |\n";
        rpt << LT("|   Hidden      | ") << setw(5) << both_hidden_count << " | " << setw(8) << both_hidden_cells << " |\n";
        rpt << LT("+---------------+-------+----------+\n");
        rpt << LT("| Total         | ") << setw(5) << all_count << " | " << setw(8) << all_cells << " |\n";
        rpt << LT("+---------------+-------+----------+\n");
        rpt << LT("Note: Cells includes margins and expression dimension.\n");
    }

    {
        int language_count = 0;
        int language_labels = 0;
        int language_notes = 0;
        for (auto& s : Symbol::pp_all_languages) {
            ++language_count;
            language_labels += s->is_label_supplied();
            language_notes += s->is_note_supplied();
        }

        int classification_count = 0;
        int classification_labels = 0;
        int classification_notes = 0;
        int range_count = 0;
        int range_labels = 0;
        int range_notes = 0;
        int partition_count = 0;
        int partition_labels = 0;
        int partition_notes = 0;
        for (auto& s : Symbol::pp_all_enumerations) {
            if (s->is_classification()) {
                ++classification_count;
                classification_labels += s->is_label_supplied();
                classification_notes += s->is_note_supplied();
            }
            if (s->is_range()) {
                ++range_count;
                range_labels += s->is_label_supplied();
                range_notes += s->is_note_supplied();
            }
            if (s->is_partition()) {
                ++partition_count;
                partition_labels += s->is_label_supplied();
                partition_notes += s->is_note_supplied();
            }
        }

        int aggregation_count = Symbol::pp_all_aggregations.size();

        int parameter_count = 0;
        int parameter_labels = 0;
        int parameter_notes = 0;
        for (auto& s : Symbol::pp_all_parameters) {
            ++parameter_count;
            parameter_labels += s->is_label_supplied();
            parameter_notes += s->is_note_supplied();
        }

        int table_count = 0;
        int table_labels = 0;
        int table_notes = 0;
        for (auto& s : Symbol::pp_all_tables) {
            ++table_count;
            table_labels += s->is_label_supplied();
            table_notes += s->is_note_supplied();
        }

        int group_count = 0;
        int group_labels = 0;
        int group_notes = 0;
        for (auto& s : Symbol::pp_all_parameter_groups) {
            ++group_count;
            group_labels += s->is_label_supplied();
            group_notes += s->is_note_supplied();
        }
        for (auto& s : Symbol::pp_all_table_groups) {
            ++group_count;
            group_labels += s->is_label_supplied();
            group_notes += s->is_note_supplied();
        }

        int entity_set_count = 0;
        int entity_set_labels = 0;
        int entity_set_notes = 0;
        for (auto& s : Symbol::pp_all_entity_sets) {
            ++entity_set_count;
            entity_set_labels += s->is_label_supplied();
            entity_set_notes += s->is_note_supplied();
        }

        int entity_count = 0;
        int entity_event_count = 0;
        int entity_function_count = 0;
        int entity_rng_count = 0;
        int entity_data_member_count = 0;

        int entity_link_count = 0;
        int entity_link_labels = 0;
        int entity_link_notes = 0;

        int entity_attribute_count = 0;
        int entity_attribute_labels = 0;
        int entity_attribute_notes = 0;

        int entity_data_member_other_count = 0;

        for (auto& e : Symbol::pp_all_entities) {
            ++entity_count;
            entity_event_count += e->pp_events.size();
            entity_function_count += e->pp_functions.size();
            entity_rng_count += e->pp_rng_streams.size();
            entity_link_count += e->pp_link_attributes.size() + e->pp_multilink_members.size();
            for (auto& s : e->pp_data_members) {
                ++entity_data_member_count;
                if (s->is_link_attribute() /* || s->is_multilink()*/) {
                    ++entity_link_count;
                    entity_link_labels += s->is_label_supplied();
                    entity_link_notes += s->is_note_supplied();
                }
                else if (s->is_attribute()) {
                    ++entity_attribute_count;
                    entity_attribute_labels += s->is_label_supplied();
                    entity_attribute_notes += s->is_note_supplied();
                }
                else {
                    ++entity_data_member_other_count;
                }
            }
        }

        string entity_kinds_string;
        if (entity_count > 1) {
            entity_kinds_string = "(" + to_string(entity_count) + LT(" kinds") + ")";
        }

        rpt << "\n";
        rpt << LT("+-------------------------------------------+\n");
        rpt << LT("| OBJECTS (selected)                        |\n");
        rpt << LT("+-------------------+-------+-------+-------+\n");
        rpt << LT("| Description       | Count | Label | Note  |\n");
        rpt << LT("+-------------------+-------+-------+-------+\n");
        rpt << LT("| Language (human)  | ") << setw(5) << language_count << " | " << setw(5) << language_labels << " | " << setw(5) << language_notes << " |\n";
        rpt << LT("| Enumeration       | ") << setw(5) << "" << " | " << setw(5) << "" << " | " << setw(5) << "" << " |\n";
        rpt << LT("|   Classification  | ") << setw(5) << classification_count << " | " << setw(5) << classification_labels << " | " << setw(5) << classification_notes << " |\n";
        rpt << LT("|   Range           | ") << setw(5) << range_count << " | " << setw(5) << range_labels << " | " << setw(5) << range_notes << " |\n";
        rpt << LT("|   Partition       | ") << setw(5) << partition_count << " | " << setw(5) << partition_labels << " | " << setw(5) << partition_notes << " |\n";
        rpt << LT("|   Aggregation     | ") << setw(5) << entity_count << " |\n";
        rpt << LT("| Input/Output      | ") << setw(5) << "" << " | " << setw(5) << "" << " | " << setw(5) << "" << " |\n";
        rpt << LT("|   Parameter       | ") << setw(5) << parameter_count << " | " << setw(5) << parameter_labels << " | " << setw(5) << parameter_notes << " |\n";
        rpt << LT("|   Table           | ") << setw(5) << table_count << " | " << setw(5) << table_labels << " | " << setw(5) << table_notes << " |\n";
        rpt << LT("|   Group           | ") << setw(5) << group_count << " | " << setw(5) << group_labels << " | " << setw(5) << group_notes << " |\n";
        rpt << LT("| Entity ") << setw(10) << std::left << entity_kinds_string << std::right << " | " << setw(5) << "" << " | " << setw(5) << "" << " | " << setw(5) << "" << " |\n";
        rpt << LT("|   Set             | ") << setw(5) << entity_set_count << " | " << setw(5) << entity_set_labels << " | " << setw(5) << entity_set_notes << " |\n";
        rpt << LT("|   Link            | ") << setw(5) << entity_link_count << " | " << setw(5) << entity_link_labels << " | " << setw(5) << entity_link_notes << " |\n";
        rpt << LT("|   Attribute       | ") << setw(5) << entity_attribute_count << " | " << setw(5) << entity_attribute_labels << " | " << setw(5) << entity_attribute_notes << " |\n";
        rpt << LT("|   RNG             | ") << setw(5) << entity_rng_count << " | " << setw(5) << "" << " | " << setw(5) << "" << " |\n";
        rpt << LT("+-------------------+-------+-------+-------+\n");
    }

    {
        // block for maintained dependencies
        int link_dependency_count = 0;
        int attribute_dependency_count = 0;
        int identity_attribute_dependency_count = 0;
        int derived_attribute_dependency_count = 0;
        int multilink_aggregate_dependency_count = 0;
        int table_dependency_count = 0;
        int set_dependency_count = 0;
        int event_dependency_count = 0;
        int all_dependency_count = 0;

        for (auto& e : Symbol::pp_all_entities) {
            for (auto& m : e->pp_data_members) {
                if (m->is_link_attribute() || m->is_multilink()) {
                    ++link_dependency_count; // each link has a maintained reciprocal link
                }
                else if (m->is_maintained_attribute()) {
                    auto ma = dynamic_cast<MaintainedAttributeSymbol*>(m);
                    assert(ma); // logic guarantee
                    auto deps = ma->pp_dependent_attributes.size();
                    attribute_dependency_count += deps;
                    if (ma->is_identity()) {
                        identity_attribute_dependency_count += deps;
                    }
                    else if (ma->is_derived()) {
                        derived_attribute_dependency_count += deps;
                    }
                    else if (ma->is_multilink_aggregate()) {
                        multilink_aggregate_dependency_count += deps;
                    }
                    else {
                        assert(false); // not reached
                    }
                }
            }
            for (auto& t : e->pp_tables) {
                table_dependency_count += t->dimension_count() + (t->filter != nullptr);
            }
            for (auto& s : e->pp_sets) {
                set_dependency_count += s->dimension_count() + (s->filter != nullptr) + (s->pp_order_attribute != nullptr);
            }
            for (auto& evt : e->pp_events) {
                event_dependency_count += evt->pp_attribute_dependencies.size() + evt->pp_linked_attribute_dependencies.size();
            }
        }
        all_dependency_count = link_dependency_count + attribute_dependency_count + table_dependency_count + set_dependency_count + event_dependency_count;

        rpt << "\n";
        rpt << LT("+---------------------------------+\n");
        rpt << LT("| MAINTAINED DEPENDENCIES         |\n");
        rpt << LT("| (in generated C++ runtime code) |\n");
        rpt << LT("+-------------------------+-------+\n");
        rpt << LT("| Dependency              | Count |\n");
        rpt << LT("+-------------------------+-------+\n");
        rpt << LT("| Reciprocal link         | ") << setw(5) << link_dependency_count << " |\n";
        rpt << LT("| Attribute maintenance   | ") << setw(5) << "" << " |\n";
        rpt << LT("|   Identity              | ") << setw(5) << identity_attribute_dependency_count << " |\n";
        rpt << LT("|   Derived               | ") << setw(5) << derived_attribute_dependency_count << " |\n";
        rpt << LT("|   Multilink aggregate   | ") << setw(5) << multilink_aggregate_dependency_count << " |\n";
        rpt << LT("| Table dimension/filter  | ") << setw(5) << table_dependency_count << " |\n";
        rpt << LT("| Set dimension/filter    | ") << setw(5) << set_dependency_count << " |\n";
        rpt << LT("| Event maintenance       | ") << setw(5) << event_dependency_count << " |\n";
        rpt << LT("+-------------------------+-------+\n");
        rpt << LT("| Total                   | ") << setw(5) << all_dependency_count << " |\n";
        rpt << LT("+-------------------------+-------+\n");
    }
    rpt.close();
}
