/**
* @file    ParameterSymbol.h
* Definitions for the ParameterSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include <string>
#include "ParameterSymbol.h"
#include "LanguageSymbol.h"
#include "NumericSymbol.h"
#include "ClassificationSymbol.h"
#include "RangeSymbol.h"
#include "PartitionSymbol.h"
#include "EnumerationSymbol.h"
#include "TimeSymbol.h"
#include "GlobalFuncSymbol.h"
#include "ConstantSymbol.h"
#include "CodeBlock.h"
#include "libopenm/db/metaModelHolder.h"

using namespace std;
using namespace openm;

void ParameterSymbol::post_parse(int pass)
{
    // Hook into the hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case eCreateMissingSymbols:
    {
        if (cumrate) {
            // create lookup function for cumrate parameter
            
            // construct the argument list
            string args = "double uniform";
            int rank = dimension_list.size();
            int conditional_dims = rank - cumrate_dims;
            int lookup_dims = cumrate_dims;
            for (int j = 0; j < conditional_dims; j++) {
                args += ", int cond" + to_string(j);
            }
            for (int j = 0; j < lookup_dims; j++) {
                args += ", int * draw" + to_string(j);
            }

            lookup_fn = new GlobalFuncSymbol("Lookup_" + name, "bool", args);
            lookup_fn->doc_block = doxygen_short("Draw from discrete probability distribution - " + name);
            CodeBlock& c = lookup_fn->func_body;
            c += "// TODO";
            c += "return true;";
        }
        break;
    }
    case eAssignMembers:
    {
        // assign direct pointer to type symbol for use post-parse
        assert(datatype); // grammar guarantee
        assert(!pp_datatype); // no use allowed in parse phase
        pp_datatype = dynamic_cast<TypeSymbol *>(pp_symbol(datatype));
        if (!pp_datatype) {
            pp_error("'" + datatype->name + "' is not a datatype in parameter '" + name + "'");
        }
        break;
    }
    case ePopulateCollections:
    {
        // validate dimension list
        // and populate the post-parse version
        for (auto psym : dimension_list) {
            assert(psym); // logic guarantee
            auto sym = *psym; // remove one level of indirection
            assert(sym); // grammar guarantee
            auto es = dynamic_cast<EnumerationSymbol *>(sym);
            if (!es) {
                pp_error("'" + sym->name + "' is invalid as a dimension in parameter '" + name + "'");
                continue; // don't insert invalid type in dimension list
            }
            if (es->is_bool()) {
                // bool not allowed as parameter dimension
                pp_error("'" + es->name + "' is invalid as a dimension in parameter '" + name + "'");
                continue; // don't insert invalid type in dimension list
            }
            pp_dimension_list.push_back(es);
        }
        // clear the parse version to avoid inadvertant use post-parse
        dimension_list.clear();

        // validate dimension list (redeclaration)
        // and populate the post-parse version (redeclaration version)
        for (auto psym : dimension_list2) {
            assert(psym); // logic guarantee
            auto sym = *psym; // remove one level of indirection
            assert(sym); // grammar guarantee
            auto es = dynamic_cast<EnumerationSymbol *>(sym);
            if (!es) {
                pp_error("'" + sym->name + "' is invalid as a dimension in parameter '" + name + "'");
                continue; // don't insert invalid type in dimension list
            }
            if (es->is_bool()) {
                // bool not allowed as parameter dimension
                pp_error("'" + es->name + "' is invalid as a dimension in parameter '" + name + "'");
                continue; // don't insert invalid type in dimension list
            }
            pp_dimension_list2.push_back(es);
        }
        // clear the parse version to avoid inadvertant use post-parse
        dimension_list2.clear();

        // add this parameter to the complete list of parameters
        pp_all_parameters.push_back(this);
        break;
    }
    case ePopulateDependencies:
    {
        // Mark enumerations required for metadata support for this parameter
        if (source == scenario_parameter) {
            // The storage type if an enumeration
            if (pp_datatype->is_enumeration()) {
                auto es = dynamic_cast<EnumerationSymbol *>(pp_datatype);
                assert(es); // compiler guarantee
                es->metadata_needed = true;
            }
            // The enumeration of each dimension
            for (auto es : pp_dimension_list) {
                es->metadata_needed = true;
            }
        }
        break;
    }

    default:
        break;
    }
}

CodeBlock ParameterSymbol::cxx_declaration_global()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_global();

    // Perform operations specific to this level in the Symbol hierarchy.
    h += "extern " + datatype->name + " " + cxx_name_and_dimensions() + ";";
    return h;
}

CodeBlock ParameterSymbol::cxx_definition_global()
{
    // Hook into the hierarchical calling chain
    CodeBlock c = super::cxx_definition_global();

    // Perform operations specific to this level in the Symbol hierarchy.

    if (source == missing_parameter) {
        c += "// WARNING - No data supplied for the following parameter:";
    }
    
    if (source != fixed_parameter || initializer_list.empty()) {
        // Parameter is not internal, or (if internal), no initializer was provided in the model source.
        // For these situations, just use the default value for a type of this kind.
        c += pp_datatype->name + " " + cxx_name_and_dimensions() + " = { " + pp_datatype->default_initial_value() + " };";
    }
    else {
        // Parameter is internal and an initializer was provided in the model source.
        c += pp_datatype->name + " " + cxx_name_and_dimensions() + " = ";
        c += cxx_initializer();
        c += ";" ;
    }
    return c;
}

CodeBlock ParameterSymbol::cxx_initializer()
{
    CodeBlock c;
    if (!initializer_list.empty()) {
        int values_per_line = 1; // number of initializer values to place on each line
        if (rank() >= 1) {
            // number of values per line is size of trailing dimension
            auto es = pp_dimension_list.back();
            values_per_line = es->pp_size();
        }
        c += "{";
        int values_in_line = 0;
        string line;
        for (auto k : initializer_list) {
            // output line if values per line limit has been reached
            if (values_in_line == values_per_line) {
                c += line;
                line = "";
                values_in_line = 0;
            }
            line += k->value() + ",";
            values_in_line++;
        }
        // output final partial line
        if (values_in_line > 0) {
            c += line;
        }
        c += "}";
    }
    return c;
}

void ParameterSymbol::validate_initializer()
{
    // Nothing to do for derived parameters
    if (source == derived_parameter) return;

    // warn about parameters with missing values
    if (source == missing_parameter) {
        pp_warning("warning : missing value for parameter '" + name + "'");
        return;
    }

    // Presence check
    if (0 == initializer_list.size()) {
        pp_error("error : missing initializer for parameter '" + name + "'");
        return;
    }

    // Size check
    if (cells() != initializer_list.size()) {
        pp_error(redecl_loc, "error : initializer for parameter '" + name + "' has size " + to_string(initializer_list.size()) + " - should be " + to_string(cells()));
    }

    // Element check
    for (auto iel : initializer_list) {
        if (!iel->is_valid_constant(*pp_datatype)) {
            string msg = "error : '" + iel->value() + "' is not a valid '" + pp_datatype->name + "' in initializer for parameter '" + name + "'";
            pp_error(iel->decl_loc, msg);
        }
    }
}

list<string> ParameterSymbol::initializer_for_storage()
{
    list<string> values;
    for (auto k : initializer_list) {
        values.push_back(k->format_for_storage(*pp_datatype));
    }
    return values;
}


void ParameterSymbol::populate_metadata(openm::MetaModelHolder & metaRows)
{
    // Only external parameters have metadata
    if (source != scenario_parameter) return;

    // Hook into the hierarchical calling chain
    super::populate_metadata( metaRows );

    // Perform operations specific to this level in the Symbol hierarchy.
 
    ParamDicRow paramDic;

    // basic information aboput the parameter

    paramDic.paramId = pp_parameter_id;
    paramDic.paramName = name;  // must be valid database view name, if we want to use compatibility views
    paramDic.rank = rank();
    paramDic.typeId = pp_datatype->type_id;
    paramDic.isHidden = false; // TODO: not implemented
    paramDic.isGenerated = false; // TODO: not implemented
    paramDic.numCumulated = 0; //TODO: not implemented
    metaRows.paramDic.push_back(paramDic);

    // label and note for the parameter
    for (auto lang : Symbol::pp_all_languages) {
        ParamDicTxtLangRow paramTxt;
        paramTxt.paramId = pp_parameter_id;
        paramTxt.langName = lang->name;
        paramTxt.descr = label(*lang);
        paramTxt.note = note(*lang);
        metaRows.paramTxt.push_back(paramTxt);
    }

    int dimension = 0;
    for (auto es : pp_dimension_list) {
        ParamDimsRow paramDims;
        paramDims.paramId = pp_parameter_id;
        paramDims.name = "dim" + to_string(dimension);
        paramDims.pos = dimension;
        paramDims.typeId = es->type_id;
        metaRows.paramDims.push_back(paramDims);
        ++dimension;
    }
}

string ParameterSymbol::cxx_name_and_dimensions(bool use_zero)
{
    string result = name;
    for (auto es : pp_dimension_list) {
        result += "[" + (use_zero ? "0" : to_string(es->pp_size())) + "]";
    }
    return result;
}

unsigned long ParameterSymbol::rank()
{
    return (unsigned long)pp_dimension_list.size();
}


unsigned long ParameterSymbol::cells()
{
    unsigned long result = 1;

    for (auto es : pp_dimension_list) {
        result *= es->pp_size();
    }
    return result;
}

CodeBlock ParameterSymbol::cxx_read_parameter()
{
    CodeBlock c;
    string typ; // storage type
    if (pp_datatype->is_time()) {
        // For Time, the type is wrapped
        auto ts = dynamic_cast<TimeSymbol *>(pp_datatype);
        assert(ts); // grammar guarantee
        typ = Symbol::token_to_string(ts->time_type);
    }
    else if (pp_datatype->is_numeric_or_bool()) {
        // For fundamental types (and bool), the name of the symbol is the name of the type
        typ = pp_datatype->name;
    }
    else {
        // for parameters of type classification, range, or partition
        // get the underlying storage type
        auto ens = dynamic_cast<EnumerationSymbol *>(pp_datatype);
        assert(ens); // grammar guarantee
        typ = Symbol::token_to_string(ens->storage_type);
    }

    // i_model is an argument (local variable) of the global function ModelStartup
    if (pp_datatype->is_range()) {
        auto rng = dynamic_cast<RangeSymbol *>(pp_datatype);
        assert(rng);
        if (rng->lower_bound != 0) {
            string cell_count = to_string(cells());
            c += "{";
            c += "// Parameter '" + name + "' has range type '" + pp_datatype->name + "' and requires transformation from ordinal -> value";
            c += "long work[" + cell_count + "];";
            c += "i_model->readParameter(\"" + name + "\", typeid(long), " + cell_count + ", &work);";
            c += typ + " *parm = (" + typ + " *) &" + name + ";";
            c += "for (size_t j = 0; j < " + cell_count + "; ++j) parm[j] = (" + typ + ") (work[j] + " + to_string(rng->lower_bound) + ");";
            c += "}";
        }
        else {
            // range starts at 0 so requires no transformation
            c += "i_model->readParameter(\"" + name + "\", typeid(" + typ + "), " + to_string(cells()) + ", &" + name + ");";
        }
    }
    else {
        c += "i_model->readParameter(\"" + name + "\", typeid(" + typ + "), " + to_string(cells()) + ", &" + name + ");";
    }

    return c;
}

string ParameterSymbol::cxx_assert_sanity()
{
    string typ; // storage type
    if (pp_datatype->is_time()) {
        // For Time, the type is wrapped
        auto ts = dynamic_cast<TimeSymbol *>(pp_datatype);
        assert(ts); // grammar guarantee
        typ = Symbol::token_to_string(ts->time_type);
    }
    else if (pp_datatype->is_numeric_or_bool()) {
        // For fundamental types (and bool), the name of the symbol is the name of the type
        typ = pp_datatype->name;
    }
    else {
        // for parameters of type classification, range, or partition
        // get the underlying storage type
        auto ens = dynamic_cast<EnumerationSymbol *>(pp_datatype);
        assert(ens); // grammar guarantee
        typ = Symbol::token_to_string(ens->storage_type);
    }

    string result = "assert(sizeof(" + cxx_name_and_dimensions(true) + ") == sizeof(" + typ + "));" ;
    return result;
}
