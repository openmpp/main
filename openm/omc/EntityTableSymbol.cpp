/**
* @file    EntityTableSymbol.cpp
* Definitions for the EntityTableSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <cassert>
#include "EntityTableSymbol.h"
#include "LanguageSymbol.h"
#include "EntitySymbol.h"
#include "EntityIncrementSymbol.h"
#include "EntityInternalSymbol.h"
#include "AttributeSymbol.h"
#include "BuiltinAttributeSymbol.h"
#include "EntityFuncSymbol.h"
#include "GlobalFuncSymbol.h"
#include "IdentityAttributeSymbol.h"
#include "NumericSymbol.h"
#include "BoolSymbol.h"
#include "RangeSymbol.h"
#include "DimensionSymbol.h"
#include "EntityTableAccumulatorSymbol.h"
#include "EntityTableMeasureAttributeSymbol.h"
#include "EntityTableMeasureSymbol.h"
#include "EnumerationSymbol.h"
#include "ModelTypeSymbol.h"
#include "CodeBlock.h"
#include "libopenm/db/metaModelHolder.h"
#include "omc_file.h" // for LTA support


using namespace std;
using namespace openm;
using namespace omc; // for LTA support

void EntityTableSymbol::create_auxiliary_symbols()
{
    {
        assert(!increment); // initialization guarantee
        increment = new EntityIncrementSymbol("om_" + name + "_incr", entity, this);
    }

    {
        assert(!current_cell_fn); // initialization guarantee
        current_cell_fn = new EntityFuncSymbol("om_" + name + "_current_cell", entity, "size_t", "");
        assert(current_cell_fn); // out of memory check
        current_cell_fn->doc_block = doxygen_short("Compute the current cell index of table " + name + " using attributes in the " + entity->name + " entity.");
    }

    {
        assert(!init_increment_fn); // initialization guarantee
        init_increment_fn = new EntityFuncSymbol("om_" + name + "_init_increment", entity, "void", "int pending, big_counter pending_event_counter");
        assert(init_increment_fn); // out of memory check
        init_increment_fn->doc_block = doxygen_short("Initialize the increment for the active table cell in " + name + ".");
    }

    {
        assert(!push_increment_fn); // initialization guarantee
        push_increment_fn = new EntityFuncSymbol("om_" + name + "_push_increment", entity, "void", "size_t cell_in, int pending, big_counter pending_event_counter");
        assert(push_increment_fn); // out of memory check
        push_increment_fn->doc_block = doxygen_short("Finalize the increment and push it to the accumulators in " + name + ".");
    }

    {
        assert(!resource_use_gfn); // initialization guarantee
        resource_use_gfn = new GlobalFuncSymbol("om_" + name + "_resource_use", "auto", "void");
        assert(resource_use_gfn); // out of memory check
        resource_use_gfn->doc_block = doxygen_short("Report resource use of  " + name + ".");
        auto& c = resource_use_gfn->func_body;
        c += "struct result { size_t total_increments; };";
        c += "return result { " + increments_gvn + " }; ";
    }

    {
        assert(!resource_use_reset_gfn); // initialization guarantee
        resource_use_reset_gfn = new GlobalFuncSymbol("om_" + name + "_resource_use_reset", "void", "void");
        assert(resource_use_reset_gfn); // out of memory check
        resource_use_reset_gfn->doc_block = doxygen_short("Reset resource use for  " + name + ".");
        auto& c = resource_use_reset_gfn->func_body;
        c += increments_gvn + " = 0;";
    }
}

void EntityTableSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case eAssignMembers:
    {
        // assign direct pointer to entity for use post-parse
        pp_entity = dynamic_cast<EntitySymbol *> (pp_symbol(entity));
        assert(pp_entity); // parser guarantee

        break;
    }
    case ePopulateCollections:
    {
        if (filter) {
            // Propagate table label to the table filter anonymous identity attribute, if present.
            for (const auto& langSym : Symbol::pp_all_languages) {
                int lang_index = langSym->language_id; // 0-based
                const string& lang = langSym->name; // e.g. "EN" or "FR"
                const string& label = pp_labels[lang_index];
                filter->pp_labels[lang_index] = LTA(lang,"Filter") + " - " + label;
            }
        }

        // add this table to the complete list of entity tables
        pp_all_entity_tables.push_back(this);

        // Add this table to the entity's list of entity tables
        pp_entity->pp_tables.push_back(this);

        // Add the increment for this table to the entity's list of all callback members
        pp_entity->pp_callback_members.push_back(increment);

        // Add this entity table to xref of each attribute used as a dimension
        for (auto d : dimension_list) {
            auto a = d->pp_attribute;
            assert(a); // logic guarantee
            a->pp_entity_tables_using.insert(this);
        }
        // Add this entity table to xref of its filter identity attribute
        if (filter) {
            filter->pp_entity_tables_using.insert(this);
        }

        break;
    }

    case ePopulateDependencies:
    {
        // do not process suppressed table
        if (is_suppressed_table) {
            break;
        }
        // Add this entity table to xref of each attribute used in expressions
        for (auto ma : pp_measure_attributes) {
            auto a = ma->pp_attribute;
            assert(a); // logic guarantee
            a->pp_entity_tables_using.insert(this);
        }
        // Process collections of observations required by accumulators
        for (auto acc : pp_accumulators) {
            if (acc->has_obs_collection) {
                // Search for an accumulator which is already marked to update the same collection.
                EntityTableAccumulatorSymbol *acc_found = nullptr;
                for (auto acc2 : pp_accumulators) {
                    if (acc2->increment == acc->increment
                     && acc2->table_op == acc->table_op
                     && acc2->updates_obs_collection) {
                        // Accumulator found with the same increment, and handling the collection.
                        acc_found = acc2;
                        break;
                    }
                }
                if (acc_found) {
                    // Another accumulator is already updating the collection of observations.
                    // Share the same collection.
                    acc->updates_obs_collection = false;
                    acc->obs_collection_index = acc_found->obs_collection_index;
                }
                else {
                    // This collection of observations is not handled by another accumulator.
                    // This accumulator will update the collection.
                    acc->updates_obs_collection = true;
                    acc->obs_collection_index = n_collections;
                    ++n_collections;
                }
            }
        }

        // construct function bodies
        build_body_current_cell();
        build_body_init_increment();
        build_body_push_increment();

        // Dependency on change in attributes used as dimensions
        for (auto dim : dimension_list) {
            auto av = dim->pp_attribute;
            CodeBlock& c = av->side_effects_fn->func_body;
            c += injection_description();
            c += "// Cell change in " + name;
            c += "if (om_active && " + cxx_instance + ") {";
            c += "// Check and start pending increment in entity table " + name;
            c += "auto cell = " + current_cell_fn->name + "();";
            c += "auto & incr = " + increment->name + ";";
            c += "incr.set_cell(cell);";
            c += "incr.start_pending();";
            c += "}";
        }

        // Dependency on filter
        if (filter) {
            CodeBlock& c = filter->side_effects_fn->func_body;
            c += injection_description();
            c += "// filter change in " + name;
            c += "if (om_active && " + cxx_instance + ") {";
            c += "// Check and start pending increment in entity table " + name;
            c += "auto & incr = " + increment->name + ";";
            c += "incr.set_filter(om_new);";
            c += "incr.start_pending();";
            c += "}";
        }

        // Dependency on change in any attribute in the table
        // to trigger processing of "lazy increments"
        {
            // All attributes used in the table.
            list<AttributeSymbol *> all_attributes;

            // Attributes used as dimensions
            for (auto dim : dimension_list) {
                all_attributes.push_back(dim->pp_attribute);
            }
            // Attributes used in measures
            for (auto ma : pp_measure_attributes) {
                all_attributes.push_back(ma->pp_attribute);
            }
            // Attribute of filter (if present)
            if (filter) {
                all_attributes.push_back(filter);
            }
            
            for (auto attr : all_attributes) {
                CodeBlock& c = attr->notify_fn->func_body;
                c += injection_description();
                c += "// Check for and finish pending increment in entity table " + name;
                c += "if (om_active && " + cxx_instance + ") {";
                c += "auto & incr = " + increment->name + ";";
                c += "incr.finish_pending();";
                c += "}";
            }
        }

        break;
    }

    default:
    break;
    }
}

CodeBlock EntityTableSymbol::cxx_declaration_global()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_global();

    // Perform operations specific to this level in the Symbol hierarchy.
    size_t n_dimensions = dimension_count();
    size_t n_cells = cell_count();
    size_t n_measures = measure_count();
    size_t n_accumulators = accumulator_count();

    string cxx_template;
    if (n_collections == 0) {
        cxx_template = 
            "EntityTable<"
            + to_string(n_dimensions) + ", "
            + to_string(n_cells) + ", "
            + to_string(n_measures) + ", "
            + to_string(n_accumulators)
            + ">";
    }
    else {
        cxx_template = 
            "EntityTableWithObs<"
            + to_string(n_dimensions) + ", "
            + to_string(n_cells) + ", "
            + to_string(n_measures) + ", "
            + to_string(n_accumulators) + ", "
            + to_string(n_collections)
            + ">";
    }
    h += "class " + cxx_class + " final : public " + cxx_template;
    h += "{";
    h += "public:";
    // constructor
    h += cxx_class + "(const char* name, std::initializer_list<int> shape) : " + cxx_template + "(name, shape)";
    h += "{";
    h += "}";
    h += "void initialize_accumulators();";
    h += "void extract_accumulators();";
    h += "void scale_accumulators();";
    h += "void compute_expressions();";
    h += "};";

    h += "typedef " + cxx_class + " " + cxx_type + ";";
    h += "extern thread_local " + cxx_type + " * " + cxx_instance + ";";
    h += "";

    h += "/// Count of increments pushed to table " + name;
    h += "extern thread_local size_t " + increments_gvn + ";";
    h += "";

    return h;
}

CodeBlock EntityTableSymbol::cxx_definition_global()
{
    // Hook into the hierarchical calling chain
    CodeBlock c = super::cxx_definition_global();

    // Perform operations specific to this level in the Symbol hierarchy.
    c += "";
    // table definition
    // E.g. DurationOfLife theDurationOfLife;
    c += "thread_local " + name + " * " + cxx_instance + " = nullptr;";
    c += "thread_local size_t " + increments_gvn + " = 0;";
    c += "";

    // definition of initialize_accumulators()
    c += "void " + name + "::initialize_accumulators()";
    c += "{";
    c += "assert(" + cxx_instance + "); // unitary table must be instantiated";
    c += "";

    for (auto acc : pp_accumulators) {
        string initial_value = "";
        switch (acc->accumulator) {
        case token::TK_unit:
        case token::TK_sum:
            initial_value = "  0.0";
            break;
        case token::TK_minimum:
            initial_value = " std::numeric_limits<double>::infinity()";
            break;
        case token::TK_maximum:
            initial_value = "-std::numeric_limits<double>::infinity()";
            break;
        case token::TK_gini:
        case token::TK_P1:
        case token::TK_P2:
        case token::TK_P5:
        case token::TK_P10:
        case token::TK_P20:
        case token::TK_P25:
        case token::TK_P30:
        case token::TK_P40:
        case token::TK_P50:
        case token::TK_P60:
        case token::TK_P70:
        case token::TK_P75:
        case token::TK_P80:
        case token::TK_P90:
        case token::TK_P95:
        case token::TK_P98:
        case token::TK_P99:
            initial_value = "std::numeric_limits<double>::quiet_NaN()";
            break;
        default:
            // not reached
            assert(0);
        }
        // e.g.  sum(value_in(alive))
        if (acc->accumulator == token::TK_unit) {
            c += "// " + Symbol::token_to_string(acc->accumulator);
        }
        else {
            assert(acc->pp_attribute);
            c += "// " + Symbol::token_to_string(acc->accumulator) + "(" + Symbol::token_to_string(acc->increment) + "(" + acc->pp_attribute->name + "))";
        }
        // e.g. for ( int cell = 0; cell < n_cells; cell++ ) acc[0][cell] =   0.0;
        c += "for ( int cell = 0; cell < n_cells; cell++ ) acc[" + to_string(acc->index) + "][cell] = " + initial_value + ";";
        c += "";

    }
    c += "}";
    c += "";

    // definition of extract_accumulators()
    c += "void " + name + "::extract_accumulators()";
    c += "{";
    c += "assert(" + cxx_instance + "); // unitary table must be instantiated";
    c += "";

    if (n_collections > 0) {
        c += "// process each cell";
        c += "for (int cell = 0; cell < n_cells; ++cell) {";

        c += "";
        c += "// Compute statistics for each observation collection in the cell.";
        c += "double P1[n_collections];";
        c += "double P2[n_collections];";
        c += "double P5[n_collections];";
        c += "double P10[n_collections];";
        c += "double P20[n_collections];";
        c += "double P25[n_collections];";
        c += "double P30[n_collections];";
        c += "double P40[n_collections];";
        c += "double P50[n_collections];";
        c += "double P60[n_collections];";
        c += "double P70[n_collections];";
        c += "double P75[n_collections];";
        c += "double P80[n_collections];";
        c += "double P90[n_collections];";
        c += "double P95[n_collections];";
        c += "double P98[n_collections];";
        c += "double P99[n_collections];";
        c += "double gini[n_collections];";
        c += "for (size_t j = 0; j < n_collections; ++j) {";
        c += "P1[j] = UNDEF_VALUE;";
        c += "P2[j] = UNDEF_VALUE;";
        c += "P5[j] = UNDEF_VALUE;";
        c += "P10[j] = UNDEF_VALUE;";
        c += "P20[j] = UNDEF_VALUE;";
        c += "P25[j] = UNDEF_VALUE;";
        c += "P30[j] = UNDEF_VALUE;";
        c += "P40[j] = UNDEF_VALUE;";
        c += "P50[j] = UNDEF_VALUE;";
        c += "P60[j] = UNDEF_VALUE;";
        c += "P70[j] = UNDEF_VALUE;";
        c += "P75[j] = UNDEF_VALUE;";
        c += "P80[j] = UNDEF_VALUE;";
        c += "P90[j] = UNDEF_VALUE;";
        c += "P95[j] = UNDEF_VALUE;";
        c += "P98[j] = UNDEF_VALUE;";
        c += "P99[j] = UNDEF_VALUE;";
        c += "gini[j] = UNDEF_VALUE;";
        c += "auto &lst = coll[cell][j];";
        c += "lst.sort();";
        c += "double total_count = (double)distance(lst.begin(), lst.end());";
        c += "if (total_count > 0) {";
        c += "double cum_count = 0.0;";
        c += "double cum_count_prev = 0.0;";
        c += "double value_prev = 0.0;";
        c += "double cum_value = 0.0;";
        c += "double cum_value_i = 0.0; // sum of rank*value";
        c += "bool P1_done = false;";
        c += "bool P2_done = false;";
        c += "bool P5_done = false;";
        c += "bool P10_done = false;";
        c += "bool P20_done = false;";
        c += "bool P25_done = false;";
        c += "bool P30_done = false;";
        c += "bool P40_done = false;";
        c += "bool P50_done = false;";
        c += "bool P60_done = false;";
        c += "bool P70_done = false;";
        c += "bool P75_done = false;";
        c += "bool P80_done = false;";
        c += "bool P90_done = false;";
        c += "bool P95_done = false;";
        c += "bool P98_done = false;";
        c += "bool P99_done = false;";
        c += "for (auto &value : lst ) {";
        c += "cum_count += 1.0;";
        c += "cum_value += value;";
        c += "cum_value_i += cum_count * value;";
        c += "if (!P1_done && cum_count >= total_count * 0.01) {";
        c += "P1[j] = value;";
        c += "P1_done = true;";
        c += "}";
        c += "if (!P2_done && cum_count >= total_count * 0.02) {";
        c += "P2[j] = value;";
        c += "P2_done = true;";
        c += "}";
        c += "if (!P5_done && cum_count >= total_count * 0.05) {";
        c += "P5[j] = value;";
        c += "P5_done = true;";
        c += "}";
        c += "if (!P10_done && cum_count >= total_count * 0.10) {";
        c += "P10[j] = value;";
        c += "P10_done = true;";
        c += "}";
        c += "if (!P20_done && cum_count >= total_count * 0.20) {";
        c += "P20[j] = value;";
        c += "P20_done = true;";
        c += "}";
        c += "if (!P25_done && cum_count >= total_count * 0.25) {";
        c += "P25[j] = value;";
        c += "P25_done = true;";
        c += "}";
        c += "if (!P30_done && cum_count >= total_count * 0.30) {";
        c += "P30[j] = value;";
        c += "P30_done = true;";
        c += "}";
        c += "if (!P40_done && cum_count >= total_count * 0.40) {";
        c += "P40[j] = value;";
        c += "P40_done = true;";
        c += "}";
        c += "if (!P50_done && cum_count >= total_count * 0.50) {";
        c += "P50[j] = value;";
        c += "P50_done = true;";
        c += "}";
        c += "if (!P60_done && cum_count >= total_count * 0.60) {";
        c += "P60[j] = value;";
        c += "P60_done = true;";
        c += "}";
        c += "if (!P70_done && cum_count >= total_count * 0.70) {";
        c += "P70[j] = value;";
        c += "P70_done = true;";
        c += "}";
        c += "if (!P75_done && cum_count >= total_count * 0.75) {";
        c += "P75[j] = value;";
        c += "P75_done = true;";
        c += "}";
        c += "if (!P80_done && cum_count >= total_count * 0.80) {";
        c += "P80[j] = value;";
        c += "P80_done = true;";
        c += "}";
        c += "if (!P90_done && cum_count >= total_count * 0.90) {";
        c += "P90[j] = value;";
        c += "P90_done = true;";
        c += "}";
        c += "if (!P95_done && cum_count >= total_count * 0.95) {";
        c += "P95[j] = value;";
        c += "P95_done = true;";
        c += "}";
        c += "if (!P98_done && cum_count >= total_count * 0.98) {";
        c += "P98[j] = value;";
        c += "P98_done = true;";
        c += "}";
        c += "if (!P99_done && cum_count >= total_count * 0.99) {";
        c += "P99[j] = value;";
        c += "P99_done = true;";
        c += "}";
        c += "cum_count_prev = cum_count;";
        c += "value_prev = value;";
        c += "}";
        c += "gini[j] = (2.0 * cum_value_i ) / (total_count * cum_value) - (total_count + 1.0) / total_count;";
        c += "}";
        c += "}";
        c += "";

        for (auto acc : pp_accumulators) {
            if (acc->has_obs_collection) {
                c += "// Assign " + acc->pretty_name();
                string acc_index = to_string(acc->index);
                string obs_index = to_string(acc->obs_collection_index);
                string stat_name = token_to_string(acc->accumulator);
                c += "acc[" + acc_index + "][cell] = " + stat_name + "[" + obs_index + "];";
                c += "";
            }
        }

        c += "}";
        c += "";
    }

    c += "}";
    c += "";

    // definition of scale_accumulators()
    c += "void " + name + "::scale_accumulators()";
    c += "{";
    c += "assert(" + cxx_instance + "); // unitary table must be instantiated";
    c += "";
    c += "double scale_factor = population_scaling_factor();";
    c += "if (scale_factor != 1.0) {";
    for (auto acc : pp_accumulators) {
        if (acc->accumulator == token::TK_sum || acc->accumulator == token::TK_unit ) {
            // e.g.  sum(value_in(alive))
            if (acc->accumulator == token::TK_unit) {
                c += "// " + Symbol::token_to_string(acc->accumulator);
            }
            else {
                assert(acc->pp_attribute);
                c += "// " + Symbol::token_to_string(acc->accumulator) + "(" + Symbol::token_to_string(acc->increment) + "(" + acc->pp_attribute->name + "))";
            }
            // e.g. for ( int cell = 0; cell < n_cells; cell++ ) acc[0][cell] *= scale_factor;
            c += "for (int cell = 0; cell < n_cells; cell++) acc[" + to_string(acc->index) + "][cell] *= scale_factor;";
        }
    }
    c += "}";
    c += "}";
    c += "";

    // definition of compute_expressions()
    // E.g. void DurationOfLife::compute_expressions()
    c += "void " + name + "::compute_expressions()";
    c += "{";
    c += "assert(" + cxx_instance + "); // unitary table must be instantiated";
    c += "";
    for (auto measure : pp_measures) {
        auto etm = dynamic_cast<EntityTableMeasureSymbol *>(measure);
        assert(etm); // logic guarantee
        // construct scale part, e.g. "1.0E-3 * "
        string scale_part;
        if (measure->scale != 0) {
            scale_part = measure->scale_as_factor() + " * ";
        }
        // E.g. for ( int cell = 0; cell < n_cells; cell++ ) measure[0][cell] = acc[0][cell] ;
        c += "for (int cell = 0; cell < n_cells; cell++ ) "
            "measure[" + to_string(etm->index) + "][cell] = " + scale_part + etm->get_expression(etm->root, EntityTableMeasureSymbol::expression_style::cxx) + " ;";
        c += "";
    }
    c += "}";
    c += "";
    return c;
}

string EntityTableSymbol::cxx_initializer() const
{
    string cxx;
    cxx = cxx_shape_initializer_list();
    return cxx;
}

void EntityTableSymbol::build_body_current_cell()
{
    CodeBlock& c = current_cell_fn->func_body;

    c += "assert(" + cxx_instance + "); // unitary table must be instantiated";
    c += "";

    size_t rank = dimension_list.size();

    if (rank == 0) {
        // short version for rank 0
        c += "// only a single cell if rank 0";
        c += "return 0;" ;
        return;
    }

    c += "// Dimensionality of table (does not include measure dimension)";
    c += "const size_t rank = " + to_string(dimension_count()) + ";";
    c += "";
    if (dimension_count() > 1) {
        // suppress declaration of shape (unused) unless rank > 1 to avoid C++ compiler warning
        c += "// Size of each dimension of table (includes margin if present)";
        c += "const std::array<size_t, rank> shape = " + cxx_shape_initializer_list() + ";";
        c += "";
    }
    c += "// Dimension coordinates of current cell";
    c += "const std::array<size_t, rank> coordinates = {";
    for (auto dim : dimension_list) {
        auto av = dim->pp_attribute;
        auto es = dim->pp_enumeration;
        // bail if dimension erroneous (error already reported)
        if (!av || !es) {
            break;
        }
        auto rs = dynamic_cast<RangeSymbol *>(es);
        if (rs && rs->lower_bound != 0) {
            // if type is range adjust to zero-based
            c += "size_t(" + av->name + " - " + to_string(rs->lower_bound) + "), ";
        }
        else {
            c += "size_t(" + av->name + "), ";
        }
    }
    c += "};";
    c += "";
    c += "// Encode the dimension coordinates of the margin cell into flattened index";
    c += "size_t cell = coordinates[0];";
    if (dimension_count() > 1) {
        // suppress do-nothing loop if rank 1 to avoid C++ compiler warning
        c += "for (size_t j = 1; j < rank; ++j) {";
        c += "cell *= shape[j];";
        c += "cell += coordinates[j];";
        c += "}";
    }
    c += "";
    c += "assert(cell >= 0 && cell < " + to_string(cell_count()) + "); // logic guarantee";
    c += "return cell;";
}

void EntityTableSymbol::build_body_init_increment()
{
    CodeBlock& c = init_increment_fn->func_body;

    c += "assert(" + cxx_instance + "); // unitary table must be instantiated";
    c += "";

    for (auto ma : pp_measure_attributes) {
        if (!ma->need_value_in && !ma->need_value_in_event) {
            // value at start of increment not needed for this attribute
            continue;
        }

        if (ma->need_value_in) {
            auto attr = ma->pp_attribute;
            assert(attr); // logic guarantee
            c += "{";
            c += "// interval(" + attr->name +")";
            c += "auto & value_in = " + ma->in_member_name() + ";";
            c += attr->pp_data_type->name + " value_curr = " + attr->name + ";";
            c += "";
            c += "value_in = value_curr;";
            c += "}";
        }
        if (ma->need_value_in_event) {
            auto attr = ma->pp_attribute;
            assert(attr); // logic guarantee
            assert(attr->lagged);
            assert(attr->lagged_event_counter);

            c += "{";
            c += "// event(" + attr->name +")";
            c += "auto& value_in = " + ma->in_event_member_name() + ";";
            c += attr->pp_data_type->name + " value_curr = " + attr->name + ";";
            c += "auto& value_lagged = " + attr->lagged->name + ";";
            c += "auto& value_lagged_counter = " + attr->lagged_event_counter->name + ";";
            c += "";
            c += "if (pending && pending_event_counter == value_lagged_counter) {";
            c += "value_curr = value_lagged;";
            c += "}";
            c += "";
            c += "value_in = value_curr;";
            c += "}";
        }
    }
}

void EntityTableSymbol::build_body_push_increment()
{
    CodeBlock& c = push_increment_fn->func_body;

    c += "if constexpr (om_resource_use_on) {";
    c +=     "++" + increments_gvn + ";";
    c += "}";

    c += "assert(" + cxx_instance + "); // unitary table must be instantiated";
    c += "";

    c += "auto& table = " + cxx_instance + ";";
    bool has_margins = margin_count() > 0;
    if (!has_margins) {
        c += "";
        c += "// Table has no margins";
    }
    else {
        c += "";
        c += "// Table has margins";
        c += "";
        c += "// Dimensionality of table (does not include measure dimension)";
        c += "const size_t rank = " + to_string(dimension_count()) + ";";
        c += "assert(rank > 0);";
        c += "";
        c += "// Size of each dimension of table (includes margin if present)";
        c += "const std::array<size_t, rank> shape = " + cxx_shape_initializer_list() + ";";
        c += "";
        c += "// Number of dimensions with a margin";
        c += "const size_t margin_count = " + to_string(margin_count()) + ";";
        c += "";
        c += "// Dimensions with a margin";
        c += "const std::array<size_t, margin_count> margin_dims = " + cxx_margin_dims_initializer_list() + ";";
        c += "";
        c += "// Number of margin combinations for a cell in the table body";
        c += "// e.g. a table with 3 margins has 2^3-1=7 margin cells associated with a body cell";
        c += "const size_t margin_combos = (1 << margin_count) - 1;";
        c += "";
        c += "// Indexes of table cells to increment (cell in table body and cells in margins)";
        c += "std::array<size_t, 1 + margin_combos> cells_to_increment;";
        c += "";
        c += "// Populate cells_to_increment";
        c += "";
        c += "// The cell in the table body:";
        c += "cells_to_increment[0] = cell_in;";
        c += "";
        c += "// The margin cells";
        c += "{";
        c +=     "// Dimension coordinates of the body cell being incremented";
        c +=     "std::array<size_t, rank> body_coordinates;";
        c +=     "// Decode dimension coordinates of the body cell";
        c +=     "for (size_t j = rank - 1, w = cell_in; ; --j) {";
        c +=         "body_coordinates[j] = w % shape[j];";
        c +=         "if (j == 0) break;";
        c +=         "w /= shape[j];";
        c +=     "}";
        c +=     "// Working coordinates of a margin cell";
        c +=     "std::array<size_t, rank> margin_coordinates;";
        c +=     "for (size_t combo = 1; combo <= margin_combos; ++combo) {";
        c +=         "// The binary bits of combo select the margin dimensions for the combination.";
        c +=         "margin_coordinates = body_coordinates;";
        c +=         "size_t bits = combo;";
        c +=         "for (size_t margin = 0; margin < margin_count; ++margin) {";
        c +=             "// read out the bits in combo";
        c +=             "if (bits & 1) {";
        c +=                 "// low order bit is 1, so replace coordinate from body cell by margin coordinate";
        c +=                 "margin_coordinates[margin_dims[margin]] = shape[margin_dims[margin]] - 1;";
        c +=             "}";
        c +=             "bits >>= 1;";
        c +=         "}";
        c +=         "// Encode the dimension coordinates of the margin cell into flattened index";
        c +=         "size_t margin_cell = margin_coordinates[0];";
        if (dimension_count() > 1) { // suppress do-nothing code which would generate a warning if rank < 2
            c +=     "for (size_t j = 1; j < rank; ++j) {";
            c +=         "margin_cell *= shape[j];";
            c +=         "margin_cell += margin_coordinates[j];";
            c +=     "}";
        }
        c +=         "assert(margin_cell >= 0 && margin_cell < " + to_string(cell_count()) + "); // logic guarantee";
        c +=         "cells_to_increment[combo] = margin_cell;";
        c +=     "}";
        c += "}";
    }

    c += "";
    c += "// Increments and Accumulators:";
    for (auto acc : pp_accumulators) {
        c += "// " + acc->pretty_name();
        c += "{";
        c += "const int acc_index = " + to_string(acc->index) + "; // accumulator index";
        c += "";
        c += "// Compute increment";
        if (acc->accumulator != token::TK_unit) {
            auto attr = acc->pp_attribute;
            assert(attr);
            if (acc->uses_value_out()) {
                c += attr->pp_data_type->name + " value_out = " + attr->name + ";";
            }
            if (acc->uses_value_in()) {
                if (acc->table_op == token::TK_interval) {
                    c += "auto& value_in = " + acc->pp_analysis_attribute->in_member_name() + ";";
                }
                else if (acc->table_op == token::TK_event) {
                    c += "auto& value_in = " + acc->pp_analysis_attribute->in_event_member_name() + ";";
                    assert(attr->lagged);
                    assert(attr->lagged_event_counter);
                    c += "auto& value_lagged = " + attr->lagged->name + ";";
                    c += "auto& value_lagged_counter = " + attr->lagged_event_counter->name + ";";
                    c += "";
                    c += "if (pending && pending_event_counter == value_lagged_counter) {";
                    c += "value_out = value_lagged;";
                    c += "}";
                }
            }
        }
        c += "";

        c += "double dIncrement;";
        switch (acc->increment) {
        case token::TK_value_in:
            c += "dIncrement = value_in;";
            break;
        case token::TK_nz_value_in:
            c += "dIncrement = value_in != 0 ? 1.0 : 0.0;";
            break;
        case token::TK_value_in2:
            c += "dIncrement = (double)value_in * (double)value_in;";
            break;
        case token::TK_value_out:
            c += "dIncrement = value_out;";
            break;
        case token::TK_nz_value_out:
            c += "dIncrement = value_out != 0 ? 1.0 : 0.0;";
            break;
        case token::TK_value_out2:
            c += "dIncrement = (double)value_out * (double)value_out;";
            break;
        case token::TK_delta:
            c += "dIncrement = (double)value_out - (double)value_in;";
            break;
        case token::TK_nz_delta:
            c += "dIncrement = ((double)value_out - (double)value_in) != 0 ? 1.0 : 0.0;";
            break;
        case token::TK_delta2:
            c += "dIncrement = ((double)value_out - (double)value_in) * ((double)value_out - (double)value_in);";
            break;
        case token::TK_unused:
            assert(acc->accumulator == token::TK_unit);
            c += "dIncrement = 1.0;";
            break;
        default:
            assert(false); // parser guarantee
        }

        if (Symbol::option_verify_valid_table_increment && acc->accumulator != token::TK_unit) {
            auto attr = acc->pp_attribute;
            assert(attr);
            c += "";
            if (acc->accumulator == token::TK_sum || acc->accumulator == token::TK_gini) {
                // runtime error if increment is Nan or inf
                c += "// Check increment validity when accumulator is sum or gini";
                c += "if (!std::isfinite(dIncrement)) {";
            }
            else {
                // runtime error if increment is Nan
                c += "// Check increment validity when accumulator is not sum or gini";
                c += "if (std::isnan(dIncrement)) {";
            }
            // arg 1 is table name, arg 2 is name of underlying attribute
            c +=     "handle_invalid_table_increment(dIncrement, \"" + name + "\",\"" + attr->pretty_name() + "\");";
            c += "}";
        }

        c += "";
        if (!has_margins) {
            c += "// Push increment to body cell";
            c += "{";
            c += "size_t cell = cell_in;";
        }
        else {
            c += "// Push increment to body cell and margin cells";
            c += "for (size_t cell : cells_to_increment) {";
        }
        c += "auto& dAccumulator = table->acc[acc_index][cell];";
        switch (acc->accumulator) {
        case token::TK_unit:
            if (Symbol::option_weighted_tabulation) {
                c += "dAccumulator += entity_weight * dIncrement;";
            }
            else {
                c += "dAccumulator += dIncrement;";
            }
            break;
        case token::TK_sum:
            if (Symbol::option_weighted_tabulation) {
                c += "dAccumulator += entity_weight * dIncrement;";
            }
            else {
                c += "dAccumulator += dIncrement;";
            }
            break;
        case token::TK_minimum:
            c += "if ( dIncrement < dAccumulator ) dAccumulator = dIncrement;";
            break;
        case token::TK_maximum:
            c += "if ( dIncrement > dAccumulator ) dAccumulator = dIncrement;";
            break;
        case token::TK_gini:
        case token::TK_P1:
        case token::TK_P2:
        case token::TK_P5:
        case token::TK_P10:
        case token::TK_P20:
        case token::TK_P25:
        case token::TK_P30:
        case token::TK_P40:
        case token::TK_P50:
        case token::TK_P60:
        case token::TK_P70:
        case token::TK_P75:
        case token::TK_P80:
        case token::TK_P90:
        case token::TK_P95:
        case token::TK_P98:
        case token::TK_P99:
        {
            c += "const int obs_index = " + to_string(acc->obs_collection_index) + "; // observation collection index";
            if (acc->updates_obs_collection) {
                c += "auto& obs_coll = table->coll[cell][obs_index];";
                c += "obs_coll.push_front(dIncrement);";
            }
            else {
                c += "// Same increment already being pushed to same collection by another accumulator";
            }
        }
        break;
        default:
            assert(0); // parser guarantee
        }
        if (Symbol::option_event_trace) {
            // inject event trace code into push increment function
            c += "";
            c += "// Code Injection: event trace";
            c += "if (event_trace_on) {";
            c += "double dVal1 = (double)dIncrement;";
            c += "double dVal2 = (double)dAccumulator;";
            c += "std::string cell_pretty = table->cell_formatted(cell);";
            c += "event_trace_msg("
                "\"" + entity->name + "\", "
                "(int)entity_id, "
                "(double)age.direct_get(), "
                "GetCaseSeed(), "
                "cell_pretty.c_str(), " // cstr1 formatted cell coordinates
                + to_string(pp_symbol_id) + ", " // other_id pp_symbol_id of table
                "\"" + name + ".acc" + to_string(acc->index) + "\", " // cstr2 other_name (table name and accumulator)
                "dVal1, " // dbl1 dIncrement
                "dVal2, " // dbl2 dAccumulator
                "(double)BaseEvent::get_global_time(), "
                "BaseEntity::et_msg_type::eTableIncrement);"
                ;
            c += "}";
            c += "";
        }
        c += "}"; // block for cells to increment
        c += "}"; // block for accumulator
    }
}

void EntityTableSymbol::populate_metadata(openm::MetaModelHolder & metaRows)
{
    // Hook into the hierarchical calling chain
    super::populate_metadata(metaRows);

    // Perform operations specific to this level in the Symbol hierarchy.

    int maxAccIndex = 0;    // only to avoid acc->index increment later

    // accumulators for table
    for (auto acc : pp_accumulators) {

        TableAccRow tableAcc;

        tableAcc.tableId = pp_table_id;
        tableAcc.accId = acc->index;
        tableAcc.name = "acc" + to_string(acc->index); // hardcode acc0, acc1, etc. to align with constructed expressions for measures
        tableAcc.accSrc = acc->pretty_name();   // expression is a part of model digest, do we need pretty here?
        tableAcc.isDerived = false;
        metaRows.tableAcc.push_back(tableAcc);

        // Maintain maxAccIndex to use when creating derived accumulators below.
        if (maxAccIndex < tableAcc.accId) {
            maxAccIndex = tableAcc.accId;
        }

        // Labels and notes for accumulators
        for (const auto& langSym : Symbol::pp_all_languages) {
            const string& lang = langSym->name; // e.g. "EN" or "FR"
            TableAccTxtLangRow tableAccTxt;
            tableAccTxt.tableId = pp_table_id;
            tableAccTxt.accId = acc->index;
            tableAccTxt.langCode = lang;
            tableAccTxt.descr = acc->label(*langSym);
            tableAccTxt.note = acc->note(*langSym);
            metaRows.tableAccTxt.push_back(tableAccTxt);
        }
    }

    // Measures for entity table.
    for (auto mIt = pp_measures.begin(); mIt != pp_measures.end(); ++mIt) {

        TableMeasureSymbol * measure = *mIt;

        // make measure db column name: it must be unique, alpanumeric and not longer than 255 chars
        TableMeasureSymbol::to_column_name(name, pp_measures, measure);

        TableExprRow tableExpr;

        auto etm = dynamic_cast<EntityTableMeasureSymbol *>(measure);
        assert(etm); // logic guarantee

        tableExpr.tableId = pp_table_id;
        tableExpr.exprId = etm->index;
        tableExpr.name = etm->short_name;     // Default is Expr0, Expr1, but can be named in model using =>
        tableExpr.decimals = etm->decimals;

        // construct scale part, e.g. "1.0E-3 * "
        string scale_part;
        if (measure->scale != 0) {
            scale_part = measure->scale_as_factor() + " * ";
        }

        // Construct the expression to compute the measure for a single simulation member.
        string measure_expr = etm->get_expression(etm->root, EntityTableMeasureSymbol::expression_style::sql_accumulators);

        // Construct the expression used to compute the measure over simulation members.
        if (measures_are_aggregated) {
            // Aggregate accumulators across simulation members before evaluating the expression for the measure.
            tableExpr.srcExpr =
                scale_part
                + etm->get_expression(etm->root, EntityTableMeasureSymbol::expression_style::sql_aggregated_accumulators);
        }
        else {
            // Average the measure across simulation members.
            tableExpr.srcExpr = scale_part + "OM_AVG(" + measure_expr + ")";
        }

        // save table measure metadata
        metaRows.tableExpr.push_back(tableExpr);

        // Labels and notes for measures
        for (const auto& langSym : Symbol::pp_all_languages) {
            const string& lang = langSym->name; // e.g. "EN" or "FR"
            TableExprTxtLangRow tableExprTxt;
            tableExprTxt.tableId = pp_table_id;
            tableExprTxt.exprId = etm->index;

            tableExprTxt.langCode = lang;

            tableExprTxt.descr = etm->label(*langSym);

            tableExprTxt.note = etm->note(*langSym);
            metaRows.tableExprTxt.push_back(tableExprTxt);
        }

        // Create derived accumulators for the table, one for each measure.
        TableAccRow tableAcc;
        tableAcc.tableId = pp_table_id;
        tableAcc.accId = ++maxAccIndex;
        tableAcc.name = tableExpr.name;
        tableAcc.isDerived = true;
        tableAcc.accSrc = scale_part + measure_expr;
        metaRows.tableAcc.push_back(tableAcc);

        // important:
        // after this point we cannot any longer use pp_accumulators
        // specially acc->index, which is out of sync with reality
        // as result of: tableAcc.accId = ++maxAccIndex;

        // TODO: add description and notes for that accumulator
        // But, these would be identical to the description and notes
        // for the corresponding measure.  So unclear whether
        // there's any point to duplicating that info.
    }
}
