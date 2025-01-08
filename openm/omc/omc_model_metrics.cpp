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
#include "VersionSymbol.h"
#include "LanguageSymbol.h"
#include "ParameterSymbol.h"
#include "TableSymbol.h"
#include "ParameterGroupSymbol.h"
#include "TableGroupSymbol.h"
#include "ImportSymbol.h"
#include "EntitySymbol.h"
#include "EntityDataMemberSymbol.h"
#include "EnumerationSymbol.h"
#include "EnumeratorSymbol.h"
#include "ClassificationSymbol.h"
#include "AggregationSymbol.h"
#include "MaintainedAttributeSymbol.h"
#include "EntitySetSymbol.h"
#include "EntityTableSymbol.h"
#include "EntityEventSymbol.h"
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
        // year-month-day and model version # for tombstone
        const size_t ymd_size = 11;
        char ymd[ymd_size];
        std::time_t time = std::time({});
        std::strftime(ymd, ymd_size, "%F", std::localtime(&time));
        // Get the version symbol
        VersionSymbol* vs = dynamic_cast<VersionSymbol*>(Symbol::find_a_symbol(typeid(VersionSymbol)));
        assert(vs);
        string version_string = to_string(vs->major) + "." + to_string(vs->minor) + "." + to_string(vs->sub_minor) + "." + to_string(vs->sub_sub_minor);
        tomb_stone = model_name + " " + version_string + " built " + ymd;
    }

    {
        // block for report title
        rpt << LT("Model Metrics for ") << tomb_stone << "\n";
    }

    {
        // block for CODE INPUT table
        int mpp_source_files_count = (int)Symbol::mpp_source_files.size();
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

        int use_source_files_count = (int)Symbol::use_source_files.size();
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
        // block for CODE OUTPUT table
        int decl_cpp_lines = (int)(cg.h.size() + cg.t0.size() + cg.t1.size());
        int defn_cpp_lines = (int)(cg.c.size());
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
        // block for MODEL INPUT table
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
                    hidden_cells += (int)p->size();

                }
                else {
                    ++visible_count;
                    visible_cells += (int)p->size();

                }
            }
            else if (p->is_derived()) {
                ++derived_count;
                derived_cells += (int)p->size();
            }
            else {
                ++fixed_count;
                fixed_cells += (int)p->size();
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
        // block for MODEL OUTPUT table
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
                    entity_hidden_cells += (int)(t->cell_count() * t->measure_count());

                }
                else {
                    ++entity_visible_count;
                    entity_visible_cells += (int)(t->cell_count() * t->measure_count());
                }
            }
            else { // is a derived table
                if (t->is_hidden) {
                    ++derived_hidden_count;
                    derived_hidden_cells += (int)(t->cell_count() * t->measure_count());

                }
                else {
                    ++derived_visible_count;
                    derived_visible_cells += (int)(t->cell_count() * t->measure_count());
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

    //list<pair<string, omc::location>> missing_labels; // for appendix of unlabelled symbols
    {
        // block for MODEL SYMBOLS table
        int language_count = 0;
        int language_labels = 0;
        int language_notes = 0;
        for (const auto& langSym : Symbol::pp_all_languages) {
            ++language_count;
            language_labels += langSym->is_label_supplied();
            language_notes += langSym->is_note_supplied();
        }

        int classification_count = 0;
        int classification_labels = 0;
        int classification_notes = 0;
        int classification_level_count = 0;
        int classification_level_labels = 0;
        int classification_level_notes = 0;
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
                //if (!s->is_label_supplied()) {
                //    missing_labels.push_back({ "classification " + s->name, s->decl_loc });
                //}
                auto c = dynamic_cast<ClassificationSymbol*>(s);
                assert(c); // logic guarantee
                for (auto& l : c->pp_enumerators) {
                    ++classification_level_count;
                    classification_level_labels += l->is_label_supplied();
                    classification_level_notes += l->is_note_supplied();
                }
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

        int aggregation_count = 0;
        int aggregation_labels = 0;
        int aggregation_notes = 0;
        for (auto& s : Symbol::pp_all_aggregations) {
            ++aggregation_count;
            aggregation_labels += s->is_label_supplied();
            aggregation_notes += s->is_note_supplied();
        }

        int parameter_count = 0;
        int parameter_labels = 0;
        int parameter_notes = 0;
        int parameter_dimension_count = 0;
        int parameter_dimension_labels = 0;
        int parameter_dimension_notes = 0;
        for (auto& s : Symbol::pp_all_parameters) {
            ++parameter_count;
            parameter_labels += s->is_label_supplied();
            parameter_notes += s->is_note_supplied();
            for (auto& d : s->pp_enumeration_list) {
                ++parameter_dimension_count;
                parameter_dimension_labels += d->is_label_supplied();
                parameter_dimension_notes += d->is_note_supplied();
            }
        }

        int table_count = 0;
        int table_labels = 0;
        int table_notes = 0;
        int table_dimension_count = 0;
        int table_dimension_labels = 0;
        int table_dimension_notes = 0;
        int table_expression_count = 0;
        int table_expression_labels = 0;
        int table_expression_notes = 0;
        for (auto& s : Symbol::pp_all_tables) {
            ++table_count;
            table_labels += s->is_label_supplied();
            table_notes += s->is_note_supplied();
            for (auto& d : s->dimension_list) {
                ++table_dimension_count;
                table_dimension_labels += d->is_label_supplied();
                table_dimension_notes += d->is_note_supplied();
            }
            {
                // count the single measure dimension, which is always present
                ++table_dimension_count;
                auto& d = s->measure_dimension;
                table_dimension_labels += d->is_label_supplied();
                table_dimension_notes += d->is_note_supplied();
            }
            for (auto& e : s->pp_measures) {
                ++table_expression_count;
                table_expression_labels += e->is_label_supplied();
                table_expression_notes += e->is_note_supplied();
            }
        }

        int parameter_group_count = 0;
        int parameter_group_labels = 0;
        int parameter_group_notes = 0;
        for (auto& s : Symbol::pp_all_parameter_groups) {
            ++parameter_group_count;
            parameter_group_labels += s->is_label_supplied();
            parameter_group_notes += s->is_note_supplied();
        }

        int table_group_count = 0;
        int table_group_labels = 0;
        int table_group_notes = 0;
        for (auto& s : Symbol::pp_all_table_groups) {
            ++table_group_count;
            table_group_labels += s->is_label_supplied();
            table_group_notes += s->is_note_supplied();
        }

        int import_count = 0;
        int import_labels = 0;
        int import_notes = 0;
        for (auto& s : Symbol::pp_all_imports) {
            ++import_count;
            import_labels += s->is_label_supplied();
            import_notes += s->is_note_supplied();
        }

        int entity_set_count = 0;
        int entity_set_labels = 0;
        int entity_set_notes = 0;
        int entity_set_dimension_count = 0;
        int entity_set_dimension_labels = 0;
        int entity_set_dimension_notes = 0;
        for (auto& s : Symbol::pp_all_entity_sets) {
            ++entity_set_count;
            entity_set_labels += s->is_label_supplied();
            entity_set_notes += s->is_note_supplied();
            for (auto& d : s->dimension_list) {
                ++entity_set_dimension_count;
                entity_set_dimension_labels += d->is_label_supplied();
                entity_set_dimension_notes += d->is_note_supplied();
            }
        }

        int entity_kind_count = 0;
        int entity_kind_labels = 0;
        int entity_kind_notes = 0;

        int entity_event_count = 0;
        int entity_event_labels = 0;
        int entity_event_notes = 0;

        int entity_function_count = 0;
        int entity_function_labels = 0;
        int entity_function_notes = 0;

        int entity_rng_count = 0;
        int entity_eligible_microdata_count = 0;
        int entity_data_member_count = 0;

        int entity_multilink_count = 0;
        int entity_multilink_labels = 0;
        int entity_multilink_notes = 0;

        int entity_array_count = 0;
        int entity_array_labels = 0;
        int entity_array_notes = 0;

        int entity_foreign_count = 0;
        int entity_foreign_labels = 0;
        int entity_foreign_notes = 0;

        int attribute_count = 0;
        int attribute_labels = 0;
        int attribute_notes = 0;

        int builtin_attribute_count = 0;
        int builtin_attribute_labels = 0;
        int builtin_attribute_notes = 0;

        int simple_attribute_count = 0;
        int simple_attribute_labels = 0;
        int simple_attribute_notes = 0;

        int identity_attribute_count = 0;
        int identity_attribute_labels = 0;
        int identity_attribute_notes = 0;

        int derived_attribute_count = 0;

        int link_attribute_count = 0;
        int link_attribute_labels = 0;
        int link_attribute_notes = 0;

        int multilink_aggregate_attribute_count = 0;

        int entity_data_member_other_count = 0;

        for (auto& e : Symbol::pp_all_entities) {
            ++entity_kind_count;
            entity_kind_labels += e->is_label_supplied();
            entity_kind_notes += e->is_note_supplied();

            for (auto& evt : e->pp_events) {
                if (!evt->is_developer_supplied) {
                    // skip self-scheduling event (if present)
                    continue;
                }
                ++entity_event_count;
                entity_event_labels += evt->is_label_supplied();
                entity_event_notes += evt->is_note_supplied();
            }

            for (auto& f : e->pp_functions) {
                if (!f->is_developer_supplied) {
                    // only count entity functions declared in model code.
                    continue;
                }
                ++entity_function_count;
                entity_function_labels += f->is_label_supplied();
                entity_function_notes += f->is_note_supplied();
            }

            entity_rng_count += (int)e->pp_rng_streams.size();

            for (auto& s : e->pp_data_members) {
                ++entity_data_member_count;
                if (s->is_multilink()) {
                    ++entity_multilink_count;
                    entity_multilink_labels += s->is_label_supplied();
                    entity_multilink_notes += s->is_note_supplied();
                }
                else if (s->is_array()) {
                    ++entity_array_count;
                    entity_array_labels += s->is_label_supplied();
                    entity_array_notes += s->is_note_supplied();
                }
                else if (s->is_foreign()) {
                    ++entity_foreign_count;
                    entity_foreign_labels += s->is_label_supplied();
                    entity_foreign_notes += s->is_note_supplied();
                }
                else if (s->is_attribute()) {
                    ++attribute_count;
                    attribute_labels += s->is_label_supplied();
                    attribute_notes += s->is_note_supplied();
                    if (s->is_builtin_attribute()) {
                        ++builtin_attribute_count;
                        builtin_attribute_labels += s->is_label_supplied();
                        builtin_attribute_notes += s->is_note_supplied();
                    }
                    else if (s->is_simple_attribute()) {
                        ++simple_attribute_count;
                        simple_attribute_labels += s->is_label_supplied();
                        simple_attribute_notes += s->is_note_supplied();
                    }
                    else if (s->is_identity_attribute()) {
                        ++identity_attribute_count;
                        identity_attribute_labels += s->is_label_supplied();
                        identity_attribute_notes += s->is_note_supplied();
                    }
                    else if (s->is_link_attribute()) {
                        ++link_attribute_count;
                        link_attribute_labels += s->is_label_supplied();
                        link_attribute_notes += s->is_note_supplied();
                    }
                    else if (s->is_derived_attribute()) {
                        ++derived_attribute_count;
                    }
                    else if (s->is_multilink_aggregate_attribute()) {
                        ++multilink_aggregate_attribute_count;
                    }
                }
                else {
                    ++entity_data_member_other_count;
                }
                if (s->is_eligible_microdata()) {
                    ++entity_eligible_microdata_count;
                }
            }
        }

        int total_count =
              language_count
            + classification_count
            + classification_level_count
            + range_count
            + partition_count
            + aggregation_count
            + parameter_count
            + parameter_dimension_count
            + table_count
            + table_dimension_count
            + table_expression_count
            + parameter_group_count
            + table_group_count
            + import_count
            + entity_kind_count
            + entity_event_count
            + attribute_count
            + entity_function_count
            + entity_multilink_count
            + entity_array_count
            + entity_foreign_count
            + entity_set_count
            + entity_set_dimension_count
            ;
        int total_labels = 
              language_labels
            + classification_labels
            + classification_level_labels
            + range_labels
            + partition_labels
            + aggregation_labels
            + parameter_labels
            + parameter_dimension_labels
            + table_labels
            + table_dimension_labels
            + table_expression_labels
            + parameter_group_labels
            + table_group_labels
            + import_labels
            + entity_kind_labels
            + entity_event_labels
            + attribute_labels
            + entity_function_labels
            + entity_multilink_labels
            + entity_array_labels
            + entity_foreign_labels
            + entity_set_labels
            + entity_set_dimension_labels
            ;
        int total_notes = 
              language_notes
            + classification_notes
            + classification_level_notes
            + range_notes
            + partition_notes
            + aggregation_notes
            + parameter_notes
            + parameter_dimension_notes
            + table_notes
            + table_dimension_notes
            + table_expression_notes
            + parameter_group_notes
            + table_group_notes
            + import_notes
            + entity_kind_notes
            + entity_event_notes
            + attribute_notes
            + entity_function_notes
            + entity_multilink_notes
            + entity_array_notes
            + entity_foreign_notes
            + entity_set_notes
            + entity_set_dimension_notes
            ;

        string entity_kinds_string; // for row title
        if (entity_kind_count > 1) {
            entity_kinds_string = string("(") + to_string(entity_kind_count) + LT(" kinds") + ")";
        }

        rpt << "\n";
        rpt << LT("+-------------------------------------------------+\n");
        rpt << LT("| MODEL SYMBOLS                                   |\n");
        rpt << LT("+-------------------------+-------+-------+-------+\n");
        rpt << LT("| Description             | Count | Label |  Note |\n");
        rpt << LT("+-------------------------+-------+-------+-------+\n");
        rpt << LT("| Language (human)        | ") << setw(5) << language_count << " | " << setw(5) << language_labels << " | " << setw(5) << language_notes << " |\n";
        rpt << LT("| Enumeration             | ") << setw(5) << "" << " | " << setw(5) << "" << " | " << setw(5) << "" << " |\n";
        rpt << LT("|   Classification        | ") << setw(5) << classification_count << " | " << setw(5) << classification_labels << " | " << setw(5) << classification_notes << " |\n";
        rpt << LT("|     Level               | ") << setw(5) << classification_level_count << " | " << setw(5) << classification_level_labels << " | " << setw(5) << classification_level_notes << " |\n";
        rpt << LT("|   Range                 | ") << setw(5) << range_count << " | " << setw(5) << range_labels << " | " << setw(5) << range_notes << " |\n";
        rpt << LT("|   Partition             | ") << setw(5) << partition_count << " | " << setw(5) << partition_labels << " | " << setw(5) << partition_notes << " |\n";
        rpt << LT("|   Aggregation           | ") << setw(5) << aggregation_count << " | " << setw(5) << aggregation_labels << " | " << setw(5) << aggregation_notes << " |\n";
        rpt << LT("| Input/Output            | ") << setw(5) << "" << " | " << setw(5) << "" << " | " << setw(5) << "" << " |\n";
        rpt << LT("|   Parameter             | ") << setw(5) << parameter_count << " | " << setw(5) << parameter_labels << " | " << setw(5) << parameter_notes << " |\n";
        rpt << LT("|     Dimension           | ") << setw(5) << parameter_dimension_count << " | " << setw(5) << parameter_dimension_labels << " | " << setw(5) << parameter_dimension_notes << " |\n";
        rpt << LT("|     Group               | ") << setw(5) << parameter_group_count << " | " << setw(5) << parameter_group_labels << " | " << setw(5) << parameter_group_notes << " |\n";
        rpt << LT("|   Table                 | ") << setw(5) << table_count << " | " << setw(5) << table_labels << " | " << setw(5) << table_notes << " |\n";
        rpt << LT("|     Dimension           | ") << setw(5) << table_dimension_count << " | " << setw(5) << table_dimension_labels << " | " << setw(5) << table_dimension_notes << " |\n";
        rpt << LT("|     Expression          | ") << setw(5) << table_expression_count << " | " << setw(5) << table_expression_labels << " | " << setw(5) << table_expression_notes << " |\n";
        rpt << LT("|     Group               | ") << setw(5) << table_group_count << " | " << setw(5) << table_group_labels << " | " << setw(5) << table_group_notes << " |\n";
        rpt << LT("|   Import                | ") << setw(5) << import_count << " | " << setw(5) << import_labels << " | " << setw(5) << import_notes << " |\n";
        rpt << LT("| Entity ") << setw(16) << std::left << entity_kinds_string << std::right << " | " << setw(5) << "" << " | " << setw(5) << "" << " | " << setw(5) << "" << " |\n";
        rpt << LT("|   Kind                  | ") << setw(5) << entity_kind_count << " | " << setw(5) << entity_kind_labels << " | " << setw(5) << entity_kind_notes << " |\n";
        rpt << LT("|   Event                 | ") << setw(5) << entity_event_count << " | " << setw(5) << entity_event_labels << " | " << setw(5) << entity_event_notes << " |\n";
        rpt << LT("|   Attribute             | ") << setw(5) << attribute_count << " | " << setw(5) << attribute_labels << " | " << setw(5) << attribute_notes << " |\n";
        rpt << LT("|     Built-in            | ") << setw(5) << builtin_attribute_count << " | " << setw(5) << builtin_attribute_labels << " | " << setw(5) << builtin_attribute_notes << " |\n";
        rpt << LT("|     Simple              | ") << setw(5) << simple_attribute_count << " | " << setw(5) << simple_attribute_labels << " | " << setw(5) << simple_attribute_notes << " |\n";
        rpt << LT("|     Identity            | ") << setw(5) << identity_attribute_count << " | " << setw(5) << identity_attribute_labels << " | " << setw(5) << identity_attribute_notes << " |\n";
        rpt << LT("|     Derived             | ") << setw(5) << derived_attribute_count << " | " << setw(5) << "" << " | " << setw(5) << "" << " |\n";
        rpt << LT("|     Link                | ") << setw(5) << link_attribute_count << " | " << setw(5) << link_attribute_labels << " | " << setw(5) << link_attribute_notes << " |\n";
        rpt << LT("|     Multilink aggregate | ") << setw(5) << multilink_aggregate_attribute_count << " | " << setw(5) << "" << " | " << setw(5) << "" << " |\n";
        rpt << LT("|   Function              | ") << setw(5) << entity_function_count << " | " << setw(5) << entity_function_labels << " | " << setw(5) << entity_function_notes << " |\n";
        rpt << LT("|   Multilink             | ") << setw(5) << entity_multilink_count << " | " << setw(5) << entity_multilink_labels << " | " << setw(5) << entity_multilink_notes << " |\n";
        rpt << LT("|   Array                 | ") << setw(5) << entity_array_count << " | " << setw(5) << entity_array_labels << " | " << setw(5) << entity_array_notes << " |\n";
        rpt << LT("|   Foreign               | ") << setw(5) << entity_foreign_count << " | " << setw(5) << entity_foreign_labels << " | " << setw(5) << entity_foreign_notes << " |\n";
        rpt << LT("| Entity set              | ") << setw(5) << entity_set_count << " | " << setw(5) << entity_set_labels << " | " << setw(5) << entity_set_notes << " |\n";
        rpt << LT("|   Dimension             | ") << setw(5) << entity_set_dimension_count << " | " << setw(5) << entity_set_dimension_labels << " | " << setw(5) << entity_set_dimension_notes << " |\n";
        rpt << LT("+-------------------------+-------+-------+-------+\n");
        rpt << LT("| Total                   | ") << setw(5) << total_count << " | " << setw(5) << total_labels << " | " << setw(5) << total_notes << " |\n";
        rpt << LT("+-------------------------+-------+-------+-------+\n");
        rpt << LT("| Supplementary Info      | ") << setw(5) << "" << " | " << setw(5) << "" << " | " << setw(5) << "" << " |\n";
        rpt << LT("|   Random streams        | ") << setw(5) << entity_rng_count << " | " << setw(5) << "" << " | " << setw(5) << "" << " |\n";
        rpt << LT("|   Eligible microdata    | ") << setw(5) << entity_eligible_microdata_count << " | " << setw(5) << "" << " | " << setw(5) << "" << " |\n";
        rpt << LT("+-------------------------+-------+-------+-------+\n");
        rpt << LT("Notes: Parameter includes derived parameters.\n");
        rpt << LT("       Table > Dimension includes the expression dimension.\n");
        rpt << LT("       Entity > Attribute > Identity includes those generated from filters.\n");
        rpt << LT("       Entity > Function does not include event time and implement functions.\n");
    }

    {
        // block for PUBLISHED SYMBOLS table
        int language_count = 0;
        int language_labels = 0;
        int language_notes = 0;
        for (const auto& langSym : Symbol::pp_all_languages) {
            ++language_count;
            language_labels += langSym->is_label_supplied();
            language_notes += langSym->is_note_supplied();
        }

        int classification_count = 0;
        int classification_labels = 0;
        int classification_notes = 0;
        int classification_level_count = 0;
        int classification_level_labels = 0;
        int classification_level_notes = 0;
        int range_count = 0;
        int range_labels = 0;
        int range_notes = 0;
        int partition_count = 0;
        int partition_labels = 0;
        int partition_notes = 0;
        for (auto& s : Symbol::pp_all_enumerations) {
            if (!s->metadata_needed) {
                // skip enumerations which are not published
                continue;
            }
            if (s->is_classification()) {
                ++classification_count;
                classification_labels += s->is_label_supplied();
                classification_notes += s->is_note_supplied();
                auto c = dynamic_cast<ClassificationSymbol*>(s);
                assert(c); // logic guarantee
                for (auto l : c->pp_enumerators) {
                    ++classification_level_count;
                    classification_level_labels += l->is_label_supplied();
                    classification_level_notes += l->is_note_supplied();
                }
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

        int parameter_count = 0;
        int parameter_labels = 0;
        int parameter_notes = 0;
        int parameter_dimension_count = 0;
        int parameter_dimension_labels = 0;
        int parameter_dimension_notes = 0;
        for (auto& s : Symbol::pp_all_parameters) {
            if (s->source != ParameterSymbol::parameter_source::scenario_parameter && !s->metadata_as_table) {
                // skip parameters which are not published
                continue;
            }
            ++parameter_count;
            parameter_labels += s->is_label_supplied();
            parameter_notes += s->is_note_supplied();
            for (auto& d : s->pp_enumeration_list) {
                ++parameter_dimension_count;
                parameter_dimension_labels += d->is_label_supplied();
                parameter_dimension_notes += d->is_note_supplied();
            }
        }

        int table_count = 0;
        int table_labels = 0;
        int table_notes = 0;
        int table_dimension_count = 0;
        int table_dimension_labels = 0;
        int table_dimension_notes = 0;
        int table_expression_count = 0;
        int table_expression_labels = 0;
        int table_expression_notes = 0;
        for (auto& s : Symbol::pp_all_tables) {
            if (s->is_internal) {
                // skip tables which are not published
                continue;
            }
            ++table_count;
            table_labels += s->is_label_supplied();
            table_notes += s->is_note_supplied();
            for (auto& d : s->dimension_list) {
                ++table_dimension_count;
                table_dimension_labels += d->is_label_supplied();
                table_dimension_notes += d->is_note_supplied();
            }
            {
                // count the single measure dimension, which is always present
                ++table_dimension_count;
                auto& d = s->measure_dimension;
                table_dimension_labels += d->is_label_supplied();
                table_dimension_notes += d->is_note_supplied();
            }
            for (auto& e : s->pp_measures) {
                ++table_expression_count;
                table_expression_labels += e->is_label_supplied();
                table_expression_notes += e->is_note_supplied();
            }
        }

        int parameter_group_count = 0;
        int parameter_group_labels = 0;
        int parameter_group_notes = 0;
        for (auto& s : Symbol::pp_all_parameter_groups) {
            if (!s->contains_scenario_parameter()) {
                // skip parameter groups which are not published
                continue;
            }
            ++parameter_group_count;
            parameter_group_labels += s->is_label_supplied();
            parameter_group_notes += s->is_note_supplied();
        }

        int table_group_count = 0;
        int table_group_labels = 0;
        int table_group_notes = 0;
        for (auto& s : Symbol::pp_all_table_groups) {
            if (!s->contains_published_table()) {
                // skip table groups which are not published
                continue;
            }
            ++table_group_count;
            table_group_labels += s->is_label_supplied();
            table_group_notes += s->is_note_supplied();
        }

        int import_count = 0;
        int import_labels = 0;
        int import_notes = 0;
        for (auto& s : Symbol::pp_all_imports) {
            ++import_count;
            import_labels += s->is_label_supplied();
            import_notes += s->is_note_supplied();
        }

        int total_count =
            language_count
            + classification_count
            + classification_level_count
            + range_count
            + partition_count
            + parameter_count
            + parameter_dimension_count
            + table_count
            + table_dimension_count
            + table_expression_count
            + parameter_group_count
            + table_group_count
            + import_count
            ;
        int total_labels =
            language_labels
            + classification_labels
            + classification_level_labels
            + range_labels
            + partition_labels
            + parameter_labels
            + parameter_dimension_labels
            + table_labels
            + table_dimension_labels
            + table_expression_labels
            + parameter_group_labels
            + table_group_labels
            + import_labels
            ;
        int total_notes =
            language_notes
            + classification_notes
            + classification_level_notes
            + range_notes
            + partition_notes
            + parameter_notes
            + parameter_dimension_notes
            + table_notes
            + table_dimension_notes
            + table_expression_notes
            + parameter_group_notes
            + table_group_notes
            + import_notes
            ;

        rpt << "\n";
        rpt << LT("+-------------------------------------------+\n");
        rpt << LT("| PUBLISHED SYMBOLS                         |\n");
        rpt << LT("+-------------------+-------+-------+-------+\n");
        rpt << LT("| Description       | Count | Label |  Note |\n");
        rpt << LT("+-------------------+-------+-------+-------+\n");
        rpt << LT("| Language (human)  | ") << setw(5) << language_count << " | " << setw(5) << language_labels << " | " << setw(5) << language_notes << " |\n";
        rpt << LT("| Enumeration       | ") << setw(5) << "" << " | " << setw(5) << "" << " | " << setw(5) << "" << " |\n";
        rpt << LT("|   Classification  | ") << setw(5) << classification_count << " | " << setw(5) << classification_labels << " | " << setw(5) << classification_notes << " |\n";
        rpt << LT("|     Level         | ") << setw(5) << classification_level_count << " | " << setw(5) << classification_level_labels << " | " << setw(5) << classification_level_notes << " |\n";
        rpt << LT("|   Range           | ") << setw(5) << range_count << " | " << setw(5) << range_labels << " | " << setw(5) << range_notes << " |\n";
        rpt << LT("|   Partition       | ") << setw(5) << partition_count << " | " << setw(5) << partition_labels << " | " << setw(5) << partition_notes << " |\n";
        rpt << LT("| Input/Output      | ") << setw(5) << "" << " | " << setw(5) << "" << " | " << setw(5) << "" << " |\n";
        rpt << LT("|   Parameter       | ") << setw(5) << parameter_count << " | " << setw(5) << parameter_labels << " | " << setw(5) << parameter_notes << " |\n";
        rpt << LT("|     Dimension     | ") << setw(5) << parameter_dimension_count << " | " << setw(5) << parameter_dimension_labels << " | " << setw(5) << parameter_dimension_notes << " |\n";
        rpt << LT("|     Group         | ") << setw(5) << parameter_group_count << " | " << setw(5) << parameter_group_labels << " | " << setw(5) << parameter_group_notes << " |\n";
        rpt << LT("|   Table           | ") << setw(5) << table_count << " | " << setw(5) << table_labels << " | " << setw(5) << table_notes << " |\n";
        rpt << LT("|     Dimension     | ") << setw(5) << table_dimension_count << " | " << setw(5) << table_dimension_labels << " | " << setw(5) << table_dimension_notes << " |\n";
        rpt << LT("|     Expression    | ") << setw(5) << table_expression_count << " | " << setw(5) << table_expression_labels << " | " << setw(5) << table_expression_notes << " |\n";
        rpt << LT("|     Group         | ") << setw(5) << table_group_count << " | " << setw(5) << table_group_labels << " | " << setw(5) << table_group_notes << " |\n";
        rpt << LT("|   Import          | ") << setw(5) << import_count << " | " << setw(5) << import_labels << " | " << setw(5) << import_notes << " |\n";
        rpt << LT("+-------------------+-------+-------+-------+\n");
        rpt << LT("| Total             | ") << setw(5) << total_count << " | " << setw(5) << total_labels << " | " << setw(5) << total_notes << " |\n";
        rpt << LT("+-------------------+-------+-------+-------+\n");
        rpt << LT("Note: Table > Dimension includes the expression dimension.\n");
    }

    {
        // block for MAINTAINED DEPENDENCIES table
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
                    auto deps = (int)ma->pp_dependent_attributes.size();
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
                table_dependency_count += (int)t->dimension_count() + (t->filter != nullptr);
            }
            for (auto& s : e->pp_sets) {
                set_dependency_count += (int)s->dimension_count() + (s->filter != nullptr) + (s->pp_order_attribute != nullptr);
            }
            for (auto& evt : e->pp_events) {
                event_dependency_count += (int)evt->pp_attribute_dependencies.size() + (int)evt->pp_linked_attribute_dependencies.size();
            }
        }
        all_dependency_count = link_dependency_count + attribute_dependency_count + table_dependency_count + set_dependency_count + event_dependency_count;

        rpt << "\n";
        rpt << LT("+----------------------------+-------+\n");
        rpt << LT("| MAINTAINED DEPENDENCIES            |\n");
        rpt << LT("| (in generated C++ runtime code)    |\n");
        rpt << LT("+----------------------------+-------+\n");
        rpt << LT("| Dependency                 | Count |\n");
        rpt << LT("+----------------------------+-------+\n");
        rpt << LT("| Reciprocal link            | ") << setw(5) << link_dependency_count << " |\n";
        rpt << LT("| Attribute maintenance      | ") << setw(5) << "" << " |\n";
        rpt << LT("|   Identity                 | ") << setw(5) << identity_attribute_dependency_count << " |\n";
        rpt << LT("|   Derived                  | ") << setw(5) << derived_attribute_dependency_count << " |\n";
        rpt << LT("|   Multilink aggregate      | ") << setw(5) << multilink_aggregate_dependency_count << " |\n";
        rpt << LT("| Table dimension/filter     | ") << setw(5) << table_dependency_count << " |\n";
        rpt << LT("| Set dimension/filter/order | ") << setw(5) << set_dependency_count << " |\n";
        rpt << LT("| Event maintenance          | ") << setw(5) << event_dependency_count << " |\n";
        rpt << LT("+----------------------------+-------+\n");
        rpt << LT("| Total                      | ") << setw(5) << all_dependency_count << " |\n";
        rpt << LT("+----------------------------+-------+\n");
    }

    rpt.close();
}
