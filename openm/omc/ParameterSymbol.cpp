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
			// Push the name into the post parse ignore hash for the current pass.
			pp_symbols_ignore.insert(lookup_fn->unique_name);
        }
        break;
    }
    case eAssignLabel:
    {
        // Create default values for parameter value note for all languages
        for (int j = 0; j < LanguageSymbol::number_of_languages(); j++) {
            pp_value_notes.push_back("");
        }

        // Check for a value note specified using NOTE comment, for each language
        for (int j = 0; j < LanguageSymbol::number_of_languages(); j++) {
            auto lang_sym = LanguageSymbol::id_to_sym[j];
            string key = unique_name + "," + lang_sym->name;
            auto search = notes_input.find(key);
            if (search != notes_input.end()) {
                pp_value_notes[j] = search->second;
            }
        }
        break;
    }
    case eAssignMembers:
    {
        if (!is_declared) {
            // parameter was not declared in source code
            // (but was parsed through an initializer in a .dat file)
            pp_error(LT("error : parameter '") + name + LT("'") +
                LT(" initialized but not declared in model"));
        }
        // assign direct pointer to type symbol for use post-parse
        assert(datatype); // grammar guarantee
        assert(!pp_datatype); // no use allowed in parse phase
        pp_datatype = dynamic_cast<TypeSymbol *>(pp_symbol(datatype));
        if (!pp_datatype) {
            pp_error(LT("error : '") + datatype->name + LT("' is not a data-type in parameter '") + name + LT("'"));
        }
        if (datatype2) {
            // redeclaration
            assert(!pp_datatype2); // no use allowed in parse phase
            pp_datatype2 = dynamic_cast<TypeSymbol *>(pp_symbol(datatype2));
            if (!pp_datatype2) {
                pp_error(LT("error : '") + (*datatype2)->name + LT("' is not a data-type in parameter '") + name + LT("'"));
            }
            if (pp_datatype != pp_datatype2) {
                pp_error(LT("error : parameter '") + name + LT("' declared with type '") + pp_datatype->name + LT("' but initialized with type '") + pp_datatype2->name + LT("'"));
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
                pp_error(LT("error : '") + sym->name + LT("' is invalid as a dimension in parameter '") + name + LT("'"));
                continue; // don't insert invalid type in dimension list
            }
            if (es->is_bool()) {
                // bool not allowed as parameter dimension
                pp_error(LT("error : '") + es->name + LT("' is invalid as a dimension in parameter '") + name + LT("'"));
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
                pp_error(LT("error : '") + sym->name + LT("' is invalid as a dimension in parameter '") + name + LT("'"));
                continue; // don't insert invalid type in dimension list
            }
            if (es->is_bool()) {
                // bool not allowed as parameter dimension
                pp_error(LT("error : '") + es->name + LT("' is invalid as a dimension in parameter '") + name + LT("'"));
                continue; // don't insert invalid type in dimension list
            }
            pp_enumeration_list2.push_back(es);
        }
        // clear the parse version to avoid inadvertant use post-parse
        enumeration_list2.clear();

        // check initializer declaration against source declaration
        if (is_declared && (source == fixed_parameter || source == scenario_parameter)) {
            if (pp_enumeration_list.size() != pp_enumeration_list2.size()) {
                // rank check failed
                pp_error(LT("error : parameter '") + name + LT("'") +
                    LT(" declared with rank ") + to_string(pp_enumeration_list.size()) +
                    LT(" but initialized with rank ") + to_string(pp_enumeration_list2.size()));
            }
            else {
                // shape and type dimension type check
                if (!std::equal(pp_enumeration_list.begin(), pp_enumeration_list.end(), pp_enumeration_list2.begin())) {
                    pp_error(LT("error : parameter '") + name + LT("'") +
                        LT(" declaration dimensions differ from initialisation dimensions"));
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
    assert(source == scenario_parameter || source == derived_parameter || source == fixed_parameter || source == missing_parameter);

    if (source == scenario_parameter) {
        if (rank() > 0) {
            // extern thread_local double * om_value_ageSex;
            // #define ageSex (*reinterpret_cast<const double(*)[N_AGE][N_SEX]>(om_value_ageSex))
            h += "extern thread_local " + cxx_type_of_parameter() + " * om_value_" + name + ";";
            h += "#define " +
                name +
                " (*reinterpret_cast<const " + pp_datatype->name + "(*)" + cxx_dimensions() + ">" +
                "(om_value_" + name + ")" +
                ")";
        }
        else {
            // extern thread_local int om_value_startSeed;
            // #define startSeed (static_cast<const int>>(om_value_startSeed))
            h += "extern thread_local " + cxx_type_of_parameter() + " om_value_" + name + ";";
            h += "#define " +
                name +
                " (static_cast<const " + pp_datatype->name + ">" +
                "(om_value_" + name + ")" +
                ")";
        }
    }
    if (source == derived_parameter) {
        if (rank() > 0) {
            // extern thread_local CITY * om_value_NearestCity;
            // #define NearestCity (*reinterpret_cast<CITY(*)[N_CITY]>(om_value_NearestCity))
            h += "extern thread_local " + pp_datatype->name + " * om_value_" + name + ";";
            h += "#define " +
                name +
                " (*reinterpret_cast<" + pp_datatype->name + "(*)" + cxx_dimensions() + ">" +
                "(om_value_" + name + ")" +
                ")";
        }
        else {
            // extern thread_local CITY oneCity;
            h += "extern thread_local " + pp_datatype->name + " " + name + ";";
        }
    }
    if (source == fixed_parameter) {
        h += "extern const "
            + pp_datatype->name + " "
            + name
            + cxx_dimensions() + ";";
    }
    if (source == missing_parameter) {
        h += "extern const "
            + pp_datatype->name + " "
            + name
            + cxx_dimensions() + ";";
    }
    return h;
}

CodeBlock ParameterSymbol::cxx_definition_global()
{
    // Hook into the hierarchical calling chain
    CodeBlock c = super::cxx_definition_global();

    // Perform operations specific to this level in the Symbol hierarchy.
    assert(source == scenario_parameter || source == derived_parameter || source == fixed_parameter || source == missing_parameter);

    if (source == missing_parameter) {
        c += "// WARNING - No data for the following parameter:";
    }

    if (source == scenario_parameter) {
        if (rank() > 0) {
            // static vector<unique_ptr<double[]>> om_param_ageSex;
            // thread_local double * om_value_ageSex = nullptr;
            c += "static vector<unique_ptr<" + cxx_type_of_parameter() + "[]>> " + alternate_name() + ";";
            c += "thread_local " + cxx_type_of_parameter() + " * " + "om_value_" + name + " = nullptr;";
        }
        else {
            // static vector<int> om_param_startSeed;
            // thread_local int om_value_startSeed;
            c += "static vector<" + cxx_type_of_parameter() + "> " + alternate_name() + ";";
            c += "thread_local " + cxx_type_of_parameter() + " om_value_" + name + ";";
        }
    }
    if (source == derived_parameter) {
        if (rank() > 0) {
            // static thread_local CITY om_param_NearestCity[N_CITY];
            // thread_local CITY * om_value_NearestCity = nullptr;
            c += "static thread_local " + pp_datatype->name + " " + alternate_name() + cxx_dimensions() + ";";
            c += "thread_local " + pp_datatype->name + " * " + "om_value_" + name + " = nullptr;";
        }
        else {
            // thread_local CITY oneCity;
            c += "thread_local " + pp_datatype->name + " " + name + ";";
        }
    }
    if (source == fixed_parameter) {
        // An initializer was provided in the model source code.
        assert(!initializer_list.empty());
        // Note that extern is required in the definition (not just the declaration)
        // since in C++ default linkage of 'const' is local, not global.
        c += "extern const "
            + pp_datatype->name + " "
            + name
            + cxx_dimensions() + " = ";
        c += cxx_initializer();
        c += ";";
    }
    if (source == missing_parameter) {
        // Initialize using the default value for a type of this kind.
        c += "extern const "
            + pp_datatype->name + " "
            + name
            + cxx_dimensions()
            + " = { " + pp_datatype->default_initial_value() + " };";
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
        if (haz1rate) {
            c += "// haz1rate parameter - original values transformed";
        }
        int values_in_line = 0;
        string line;
        for (auto k : initializer_list) {
            // output line if values per line limit has been reached
            if (values_in_line == values_per_line) {
                c += line;
                line = "";
                values_in_line = 0;
            }
            string s_value = k->value();
            if (haz1rate) {
                const size_t bufsize = 50;
                char buf[bufsize];
                // perform in-place transformation
                double d_value = atof(s_value.c_str());
                if (d_value >= 1.0 || d_value < 0.0) {
                    // value out of bounds for transformation to hazard
                    pp_fatal(LT("error : value out of bounds for haz1rate conversion in fixed parameter '") + name + LT("'"));
                }
                d_value = -log(1.0 - d_value);
                snprintf(buf, bufsize, "%a", d_value);
                s_value = buf;
            }
            line += s_value + ",";
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
        pp_warning(LT("warning : missing value for parameter '") + name + LT("'"));
        return;
    }

    // Presence check
    if (0 == initializer_list.size()) {
        pp_error(LT("error : missing initializer for parameter '") + name + LT("'"));
        return;
    }

    // Size check
    if (size() != initializer_list.size()) {
        pp_error(redecl_loc, LT("error : initializer for parameter '") + name + LT("' has size ") + to_string(initializer_list.size()) + LT(" - should be ") + to_string(size()));
    }

    // Element check
    for (auto iel : initializer_list) {
        if (!iel->is_valid_constant(*pp_datatype)) {
            string msg = LT("error : '") + iel->value() + LT("' is not a valid '") + pp_datatype->name + LT("' in initializer for parameter '") + name + LT("'");
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
    paramDic.numCumulated = 0; //TODO: not implemented
    metaRows.paramDic.push_back(paramDic);

    // label and note for the parameter
    for (auto lang : Symbol::pp_all_languages) {
        ParamDicTxtLangRow paramTxt;
        paramTxt.paramId = pp_parameter_id;
        paramTxt.langCode = lang->name;
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
            paramDimsTxt.langCode = lang->name;
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
    
    bool adjust = false; // true if parameter values require in-place adjustment
    string c_adjust_comment; // C++ code fragment with explanatory comment about in-place adjustment
    CodeBlock c_adjust_code; // C++ code fragment to do in-place adjustment (using reference named "value")

    if (haz1rate) {
        adjust = true;
        c_adjust_comment = "// Parameter '" + name + "' is declared haz1rate and requires transformation.";
        // Raise model run-time exception if rate is out of bounds for haz1rate conversion
        c_adjust_code += "if (value >= 1.0 || value < 0.0) {";
        c_adjust_code += "// Throw model run-time exception if rate is out of bounds for haz1rate conversion";
        c_adjust_code += "std::string msg;";
        c_adjust_code += "msg = LT(\"error : value out of bounds for haz1rate conversion in parameter '\");";
        c_adjust_code += "msg += \"" + name + "\";";
        c_adjust_code += "msg += LT(\"'\");";
        c_adjust_code += "ModelExit(msg.c_str());";
        c_adjust_code += "}";
        c_adjust_code += "value = - log(1.0 - value);";
    }

    if (rank() > 0) {
        // om_param_ageSex = std::move(read_om_parameter<double>(i_runBase, "ageSex", N_AGE * N_SEX));
        c += alternate_name() + " = std::move("
            "read_om_parameter<" + cxx_type_of_parameter() + ">(i_runBase, \"" + name + "\", " + to_string(size()) + "));";
        if (adjust) {
            c += c_adjust_comment;
            c += "for (auto & p_value : " + alternate_name() + ") {";
            c += "for (size_t j = 0; j < " + to_string(size()) + "; ++j) {";
            c += "auto &value = p_value[j];";
            c += c_adjust_code;
            c += "}";
            c += "}";
        }
    }
    else {
        // om_param_startSeed = std::move(read_om_parameter<int>(i_runBase, "startSeed"));
        c += alternate_name() + " = std::move("
            "read_om_parameter<" + cxx_type_of_parameter() + ">(i_runBase, \"" + name + "\"));";
        if (adjust) {
            c += c_adjust_comment;
            c += "for (auto & p_value : " + alternate_name() + ") {";
            c += "auto &value = p_value;";
            c += c_adjust_code;
            c += "}";
        }
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

string ParameterSymbol::cxx_definition_cumrate()
{
    string result = "";
    if (cumrate) {
        // Definition of the supporting cumrate object used by the generated Lookup_ function.
        // Storage duration is thread_local, except if parameter is fixed (or missing).
        // The cumrate object is only used in generated lookup function defined later in teh same module,
        // so is declared static, and requires no header file declaration.
        string storage_duration = (source == fixed_parameter || source == missing_parameter) ? "" : "thread_local ";
        result = "static "
            + storage_duration
            + "cumrate<" 
            + to_string(conditioning_size()) + ","
            + to_string(distribution_size()) + "> "
            + cumrate_name() + ";";
    }
    return result;
}

string ParameterSymbol::cxx_initialize_cumrate()
{
    string result = "";
    if (cumrate) {
        // N-1 trailing [0] are required to successfully pass argument of type reference_wrapper<...>
        string inds = "";
        for (size_t j = 0; j < rank() - 1; ++j) {
            inds += "[0]";
        }
        //result = cumrate_name() + ".initialize(" + name + inds + ");";
        result = cumrate_name() + ".initialize((const double *)" + name + ");";
    }
    return result;
}

