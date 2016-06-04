/**
* @file    ParameterSymbol.h
* Definitions for the ParameterSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <cassert>
#include <string>
#include <vector>
#include <algorithm>
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
            int rank = enumeration_list.size();
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
            // push the name into the pass #1 ignore hash
            pp_ignore_pass1.insert(lookup_fn->unique_name);
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
            pp_error("error : '" + datatype->name + "' is not a datatype in parameter '" + name + "'");
        }
        if (datatype2) {
            // redeclaration
            assert(!pp_datatype2); // no use allowed in parse phase
            pp_datatype2 = dynamic_cast<TypeSymbol *>(pp_symbol(datatype2));
            if (!pp_datatype2) {
                pp_error("error : '" + (*datatype2)->name + "' is not a datatype in parameter '" + name + "'");
            }
            if (pp_datatype != pp_datatype2) {
                pp_error("error : parameter '" + name + "' declared with type '" + pp_datatype->name + "' but initialized with type '" + pp_datatype2->name + "'");
            }
        }
        break;
    }
    case ePopulateCollections:
    {
        // validate dimension list
        // and populate the post-parse version
        for (auto psym : enumeration_list) {
            assert(psym); // logic guarantee
            auto sym = *psym; // remove one level of indirection
            assert(sym); // grammar guarantee
            auto es = dynamic_cast<EnumerationSymbol *>(sym);
            if (!es) {
                pp_error("error : '" + sym->name + "' is invalid as a dimension in parameter '" + name + "'");
                continue; // don't insert invalid type in dimension list
            }
            if (es->is_bool()) {
                // bool not allowed as parameter dimension
                pp_error("error : '" + es->name + "' is invalid as a dimension in parameter '" + name + "'");
                continue; // don't insert invalid type in dimension list
            }
            // process the dimension into post-parse members
            pp_enumeration_list.push_back(es);
            pp_shape.push_back(es->pp_size());
        }
        // Create auxiliary lists for conditioning and distribution portions
        for (auto es : pp_enumeration_list) {
            if (pp_conditioning_enumeration_list.size() < conditioning_dims()) {
                pp_conditioning_enumeration_list.push_back(es);
                pp_conditioning_shape.push_back(es->pp_size());
            }
            else {
                pp_distribution_enumeration_list.push_back(es);
                pp_distribution_shape.push_back(es->pp_size());
            }
        }
        // clear the parse version to avoid inadvertant use post-parse
        enumeration_list.clear();

        // validate dimension list (redeclaration)
        // and populate the post-parse version (redeclaration version)
        for (auto psym : enumeration_list2) {
            assert(psym); // logic guarantee
            auto sym = *psym; // remove one level of indirection
            assert(sym); // grammar guarantee
            auto es = dynamic_cast<EnumerationSymbol *>(sym);
            if (!es) {
                pp_error("error : '" + sym->name + "' is invalid as a dimension in parameter '" + name + "'");
                continue; // don't insert invalid type in dimension list
            }
            if (es->is_bool()) {
                // bool not allowed as parameter dimension
                pp_error("error : '" + es->name + "' is invalid as a dimension in parameter '" + name + "'");
                continue; // don't insert invalid type in dimension list
            }
            pp_enumeration_list2.push_back(es);
        }
        // clear the parse version to avoid inadvertant use post-parse
        enumeration_list2.clear();

        // check that declaration and initializer dimensions are identical
        if (source == fixed_parameter || source == scenario_parameter) {
            // rank check
            if (pp_enumeration_list.size() != pp_enumeration_list2.size()) {
                pp_error("error : parameter '" + name + "'"
                    " declared with rank " + to_string(pp_enumeration_list.size()) +
                    " but initialized with rank " + to_string(pp_enumeration_list2.size()));
            }
            else {
                if (!std::equal(pp_enumeration_list.begin(), pp_enumeration_list.end(), pp_enumeration_list2.begin())) {
                    pp_error("error : parameter '" + name + "'"
                        " declaration dimensions differ from initializion dimensions");
                }
            }
        }

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
            for (auto es : pp_enumeration_list) {
                es->metadata_needed = true;
            }
        }

        // Generate body of lookup function
        if (cumrate) {
            // Generate body of lookup function
            CodeBlock& c = lookup_fn->func_body;
            c += "// Calculate conditioning index";
            c += "size_t conditioning_index = 0;";
            int cond_index = 0;
            for (auto cond_size : pp_conditioning_shape) {
                if (cond_index > 0) {
                    c += "conditioning_index *= " + to_string(cond_size) + ";";
                }
                c += "conditioning_index += cond" + to_string(cond_index) + ";";
                cond_index++;
            }
            c += "// Determine if distribution is empty";
            c += "bool is_empty = " + cumrate_name() + ".is_degenerate(conditioning_index);";
            c += "size_t distribution_index = 0;";
            c += "if (!is_empty) {";
            c += "// Obtain value from distribution";
            c += "distribution_index = " + cumrate_name() + ".draw(conditioning_index, uniform);";
            c += "}";
            c += "// Calculate values of distribution dimensions";
            int distr_index = distribution_dims() - 1;
            // reverse iterate through the distribution shape
            for (auto it = pp_distribution_shape.rbegin(); it != pp_distribution_shape.rend(); ++it) {
                if (distr_index > 0) {
                    c += "*draw" + to_string(distr_index) + " = distribution_index % " + to_string(*it) + ";";
                    c += "distribution_index /= " + to_string(*it) + ";";
                }
                else {
                    c += "*draw" + to_string(distr_index) + " = distribution_index;";
                }
                distr_index--;
            }
         
            c += "return !is_empty;";
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
    h += "extern " + datatype->name + " " + alternate_name() + cxx_dimensions() + "; // read-write version";
    string cv_qualifier = (source == parameter_source::derived_parameter) ? "" : "const";
    h += "extern " + cv_qualifier + " " + datatype->name + " (&" + name + ")" + cxx_dimensions() + ";";
    if (cumrate) {
        h += "extern cumrate<" + to_string(conditioning_size()) + "," + to_string(distribution_size()) + "> " + cumrate_name() + ";";
    }
    return h;
}

CodeBlock ParameterSymbol::cxx_definition_global()
{
    // Hook into the hierarchical calling chain
    CodeBlock c = super::cxx_definition_global();

    // Perform operations specific to this level in the Symbol hierarchy.

    if (source == missing_parameter) {
        //TODO treat as error?
        c += "// WARNING - No data supplied for the following parameter:";
    }
    
    if (source == fixed_parameter) {
        // Parameter is fixed (an initializer was provided in the model source).
        assert(!initializer_list.empty());
        c += pp_datatype->name + " " + alternate_name() + cxx_dimensions() + " = ";
        c += cxx_initializer();
        c += ";" ;
    }
    else {
        // Initialize using the default value for a type of this kind.
        c += pp_datatype->name + " " + alternate_name() + cxx_dimensions() + " = { " + pp_datatype->default_initial_value() + " };";
    }

    string cv_qualifier = (source == parameter_source::derived_parameter) ? "" : "const";
    // definition of read-only reference for use in model code
    c += cv_qualifier + " " + datatype->name + " (&" + name + ")" + cxx_dimensions() + " = " + alternate_name() + ";";

    if (cumrate) {
        c += "cumrate<" + to_string(conditioning_size()) + "," + to_string(distribution_size()) + "> " + cumrate_name() + ";";
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
            auto es = pp_enumeration_list.back();
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
    if (size() != initializer_list.size()) {
        pp_error(redecl_loc, "error : initializer for parameter '" + name + "' has size " + to_string(initializer_list.size()) + " - should be " + to_string(size()));
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

CodeBlock ParameterSymbol::dat_definition() const
{
    CodeBlock d;

    string lang = LanguageSymbol::default_language();

    d += "/* NOTE ( " + name + ", " + lang + " )";
    d += "*/";
    d += "//" + lang + " " + label();

    assert(pp_datatype);
    string value = pp_datatype->default_initial_value();
    string line1;
    line1 += pp_datatype->name;
    line1 += " " + name;
    for (auto es : pp_enumeration_list) {
        line1 += "[" + es->name + "]";
    }
    line1 += " = ";
    if (rank() == 0) {
        line1 += value + ";";
        d += line1;
    }
    else {
        line1 += "{";
        d += line1;
        d += "(" + to_string(size()) + ") " + value;
        d += "};";
    }
    d += "";

    return d;
}

void ParameterSymbol::populate_metadata(openm::MetaModelHolder & metaRows)
{
    // Only external parameters have metadata
    if (source != scenario_parameter) return;

    // Hook into the hierarchical calling chain
    super::populate_metadata( metaRows );

    // Perform operations specific to this level in the Symbol hierarchy.
 
    ParamDicRow paramDic;

    // basic information about the parameter

    paramDic.paramId = pp_parameter_id;
    paramDic.paramName = name;  // must be valid database view name, if we want to use compatibility views
    paramDic.rank = rank();
    paramDic.typeId = pp_datatype->type_id;
    paramDic.isHidden = is_hidden;
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

    // The dimensions of the parameter
    for (auto dim : dimension_list ) {

        ParamDimsRow paramDims;

        auto es = dim->pp_enumeration;
        assert(es); // logic guarantee
        paramDims.paramId = pp_parameter_id;
        paramDims.dimId = dim->index;
        //paramDims.name = "dim" + to_string(dim->index);
        paramDims.typeId = es->type_id;
        metaRows.paramDims.push_back(paramDims);

        // Labels and notes for the dimensions of the parameter
        for (auto lang : Symbol::pp_all_languages) {

            ParamDimsTxtLangRow paramDimsTxt;

            paramDimsTxt.paramId = pp_parameter_id;
            paramDimsTxt.dimId = dim->index;
            paramDimsTxt.langName = lang->name;
            paramDimsTxt.descr = dim->label(*lang);
            paramDimsTxt.note = dim->note(*lang);
            metaRows.paramDimsTxt.push_back(paramDimsTxt);
        }
    }
}

string ParameterSymbol::cxx_dimensions(bool use_zero) const
{
    string result;
    for (auto es : pp_enumeration_list) {
        result += "[" + (use_zero ? "0" : to_string(es->pp_size())) + "]";
    }
    return result;
}

CodeBlock ParameterSymbol::cxx_read_parameter()
{
    CodeBlock c;

    // i_runBase is an argument (local variable) of the global function RunInit
    if (pp_datatype->is_range()) {
        auto rng = dynamic_cast<RangeSymbol *>(pp_datatype);
        assert(rng);
        if (rng->lower_bound != 0) {
            string typ = cxx_type_of_parameter();   // storage type
            string cell_count = to_string(size());
            c += "{";
            c += "// Parameter '" + name + "' has range type '" + pp_datatype->name + "' and requires transformation from ordinal -> value";
            c += "long work[" + cell_count + "];";
            c += "i_runBase->readParameter(" + cxx_parameter_name_type_size() + ", &work);";
            c += typ + " *parm = (" + typ + " *) &" + alternate_name() + ";";
            c += "for (size_t j = 0; j < " + cell_count + "; ++j) parm[j] = (" + typ + ") (work[j] + " + to_string(rng->lower_bound) + ");";
            c += "}";
        }
        else {
            // range starts at 0 so requires no transformation
            c += "i_runBase->readParameter(" + cxx_parameter_name_type_size() + ", &" + alternate_name() + ");";
        }
    }
    else {
        c += "i_runBase->readParameter(" + cxx_parameter_name_type_size() + ", &" + alternate_name() + ");";
    }

    return c;
}

const string ParameterSymbol::cxx_parameter_name_type_size(void) const
{
    string result;

    if (pp_datatype->is_range()) {
        auto rng = dynamic_cast<RangeSymbol *>(pp_datatype);
        assert(rng);
        if (rng->lower_bound != 0) {
            result = "\"" + name + "\", typeid(long), " + to_string(size());
        }
        else {
            // range starts at 0 so requires no transformation
            result = "\"" + name + "\", typeid(" + cxx_type_of_parameter() + "), " + to_string(size());
        }
    }
    else {
        result = "\"" + name + "\", typeid(" + cxx_type_of_parameter() + "), " + to_string(size());
    }
    return result;
}

const string ParameterSymbol::cxx_type_of_parameter(void) const
{
    string typ;

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
    else if (pp_datatype->is_string()) {
        // For the string type, the type is "string"
        typ = "string";
    }
    else {
        // for parameters of type classification, range, or partition
        // get the underlying storage type
        auto ens = dynamic_cast<EnumerationSymbol *>(pp_datatype);
        assert(ens); // grammar guarantee
        typ = Symbol::token_to_string(ens->storage_type);
    }
    return typ;
}

string ParameterSymbol::cxx_initialize_cumrate()
{
    string result = "";
    if (cumrate) {
        result = cumrate_name() + ".initialize((double *)" + name + ");";
    }
    return result;
}

CodeBlock ParameterSymbol::cxx_transform_haz1rate()
{
    CodeBlock cxx;
    if (haz1rate) {
        cxx += "{";
        cxx += "// Perform haz1rate transformation on contents of " + name;
        cxx += "double *pd = (double *)&" + name + ";";
        cxx += "for (size_t j = 0; j < " + to_string(size()) + "; ++j) {";
        cxx += "pd[j] = - log(1.0 - pd[j]);";
        cxx += "}";
        cxx += "}";
    }
    return cxx;
}

string ParameterSymbol::cxx_type_check()
{
    if (pp_datatype->is_string()) {
        // For string type parameters, skip sanity check
        return "";
    }

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

    string result = "static_assert(sizeof(" + alternate_name() + cxx_dimensions(true) + ") == sizeof(" + typ + "), \"Incoherence in datatype of " + name + "\");" ;
    return result;
}
