/**
* @file    omc_model_doc.cpp
*
* Function to create Model doc
*
*/
// Copyright (c) 2023-2024 OpenM++ Contributors (see AUTHORS.txt)
// This code is licensed under the MIT license (see LICENSE.txt)

#include <iostream>
#include <fstream>
#include <regex>

#include "Symbol.h"
#include "VersionSymbol.h"
#include "ModelSymbol.h"
#include "LanguageSymbol.h"
#include "ParameterSymbol.h"
#include "TableSymbol.h"
#include "DimensionSymbol.h"
#include "ParameterGroupSymbol.h"
#include "TableGroupSymbol.h"
#include "ImportSymbol.h"
#include "EntitySymbol.h"
#include "EntityDataMemberSymbol.h"
#include "EnumerationSymbol.h"
#include "EnumerationWithEnumeratorsSymbol.h"
#include "EnumeratorSymbol.h"
#include "ClassificationEnumeratorSymbol.h"
#include "ClassificationSymbol.h"
#include "RangeSymbol.h"
#include "PartitionSymbol.h"
#include "PartitionEnumeratorSymbol.h"
#include "AggregationSymbol.h"
#include "MaintainedAttributeSymbol.h"
#include "EntitySetSymbol.h"
#include "EntityTableSymbol.h"
#include "EntityEventSymbol.h"
#include "IdentityAttributeSymbol.h"
#include "GlobalFuncSymbol.h"
#include "ParseContext.h"
#include "omc_file.h"
#include "omc_model_metrics.h"

#include "maddy/parser.h"

using namespace std;
using namespace openm;
using namespace omc;

/** find language-specific message by source non-translated message and language
*
* @param[in] i_lang   language to translate into, e.g.: fr-CA
* @param[in] i_source source message to translate
* @param[in] i_allMsg all translated strings form omc.message.ini
*/
const string getTranslated(const char* i_lang, const char* i_source, const list<pair<string, unordered_map<string, string>>>& i_allMsg) noexcept
{
    try {
        if (i_lang == nullptr || i_source == nullptr) return "";    // empty result if language code empty or source on null

        // if language exist in message.ini
        auto msgMapIt = std::find_if(
            i_allMsg.cbegin(),
            i_allMsg.cend(),
            [&i_lang](const pair<string, unordered_map<string, string>>& i_msgPair) -> bool { return equalNoCase(i_lang, i_msgPair.first.c_str()); }
        );
        if (msgMapIt == i_allMsg.cend()) return i_source;   // language not found

        // if translation exist then return copy of translated message else return original message (no translation)
        const unordered_map<string, string>::const_iterator msgIt = msgMapIt->second.find(i_source);
        return
            (msgIt != msgMapIt->second.cend()) ? msgIt->second.c_str() : i_source;
    }
    catch (...) {
        return "";
    }
}

// translated strings for all model languages from omc.message.ini
static list<pair<string, unordered_map<string, string>>> allTransaledMsg;

/**
 * @def LTA(lang, sourceMessage)
 *
 * @brief   LTA translation function: find translated string by language code and source message
 *          string.
 *
 * @param   lang            The language.
 * @param   sourceMessage   Message describing the source.
 */
#define LTA(lang, sourceMessage) ((getTranslated(lang.c_str(), sourceMessage, allTransaledMsg)))

/**
 * @fn  static string bread_crumb_hierarchy(string lang, int lang_index, Symbol* s)
 *
 * @brief   helper function to construct bread crumb hierarchy for symbol s
 *
 * @param           lang        The language.
 * @param           lang_index  Zero-based index of the language.
 * @param           s           The Symbol to process.
 *
 * @returns A single-line string which is a self-contained HTML fragment
 */
static string bread_crumb_hierarchy(string lang, int lang_index, Symbol* s)
{
    string bread_crumbs;
    if (s->pp_parent_group) {
        /// group hierarchy of s ordered highest to lowest
        list<GroupSymbol*> grp_list;
        {
            auto grp = s->pp_parent_group;
            while (grp) {
                grp_list.push_front(grp);
                grp = grp->pp_parent_group;
            }
        }
        /// HTML fragment for breadcrumbs (escaped from maddy)
        bread_crumbs = "<p>";
        bread_crumbs += "<b>" + LTA(lang, "Hierarchy") + ":</b><br>";
        string indent = "";
        for (auto grp : grp_list) {
            string grp_name = grp->name;
            string grp_label = grp->pp_labels[lang_index];
            bread_crumbs += indent + "<a href=\"#" + grp_name + "\"><b>" + grp_label + "</b></a><br>";
            indent += "&nbsp;&nbsp;&nbsp;&nbsp;";
        }
        bread_crumbs += indent + "<code><b>" + s->name + "</b></code>";
        bread_crumbs += "</p>\n\n";

    }
    return bread_crumbs;
}

static string expand_group(int lang_index, const GroupSymbol* g, int depth, int max_depth, bool summary_mode)
{
    string result;
    if (g) {
        string indent;
        for (int i = 0; i < depth; ++i) {
            indent += "&nbsp;&nbsp;&nbsp;&nbsp;";
        }
        if (summary_mode) {
            // link to group target using group label
            result = indent
                + "<a href=\"#" + g->name + "\">"
                + "<b>" + g->pp_labels[lang_index] + "</b>"
                + "</a><br>";
        }
        else {
            // group link target and label
            result = indent
                + "<span id=\"" + g->name + "\"/>"
                + "<b>" + g->pp_labels[lang_index] + "</b>"
                + "<br>";
        }
        indent += "&nbsp;&nbsp;&nbsp;&nbsp;";
        for (auto s : g->pp_symbol_list) {
            /// subgroup
            auto sg = dynamic_cast<GroupSymbol*>(s);
            if (sg && ((max_depth != -1) && (depth < max_depth))) {
                // expand recursively to next level down in hierarchy
                result += expand_group(lang_index, sg, depth + 1, max_depth, summary_mode);
            }
            else if (!summary_mode) {
                // symbol hyperlink, name, and label
                result += indent
                    + "<a href=\"#" + s->name + "\"><code>" + s->name + "</code></a> "
                    + s->pp_labels[lang_index]
                    + "<br>";
            }
        }
    }
    return result;
}

static string preprocess_markdown(string& md_in)
{
    string md_out;

    // Convert markdown line break (two trailing spaces) to maddy-specifc \r
    // Maddy documentation says \r\n, but \r seems to be required.
    md_out = std::regex_replace(md_in, std::regex("  \n"), "\r"); // maddy-specific line break
    
    return md_out;
}


string short_loc(location& loc)
{
    string result = *loc.begin.filename;
    auto p = result.find_last_of("/");
    if ((p != result.npos) && (p < result.length()) ) {
        result = result.substr(p + 1);
    }
    result +=
        + "["
        + to_string(loc.begin.line)
        + "]";
    return result;
}

void do_model_doc(bool devMode, string& outDir, string& omrootDir, string& model_name, CodeGen& cg, const string& i_msgFilePath) {

    /// target folder for HTML output
    string pubDir = outDir + "../bin/io/download/";
    std::filesystem::create_directories(pubDir);

    /// mapped location of pubDir in ompp UI local web server
    string pubURL = "download/";

    /// directory of model sqlite database
    string sqliteDir = outDir + "../bin/";

    // create json file for ompp UI:
    /*
    {
    "ModelDoc": [{
            "LangCode": "EN",
            "Link": "download/RiskPaths.doc.EN.html"
        }, {
            "LangCode": "FR",
            "Link": "download/RiskPaths.doc.FR.html"
        }
    ]
    }
    */
    if (!devMode) {
        string json_name = model_name + ".extra.json";
        string json_path = makeFilePath(sqliteDir.c_str(), json_name.c_str());
        ofstream out(json_path, ios::out | ios::trunc | ios::binary);
        if (out.fail()) {
            string msg = "omc : warning : Unable to open " + json_path + " for writing.";
            theLog->logMsg(msg.c_str());
        }

        out << "{\n\"ModelDoc\": [\n";
        size_t n = 0;

        for (auto lang : Symbol::pp_all_languages) {
            out <<
                "   {\n" <<
                "   \"LangCode\": \"" << lang->name << "\",\n" <<
                "   \"Link\": \"" << pubURL + model_name + ".doc." + lang->name + ".html\"\n" <<
                "   }";
            if (++n < Symbol::pp_all_languages.size()) out << ",";
            out << "\n";
        }
        out << "]\n}\n";
        out.close();
    }

    /// styles for HTML output
    string htmlStyles;
    {
        /// styles file (hard-coded)
        string stylesPath = omrootDir + "props/styles/doc-style-portion.html";
        ofstream stylesStream;
        stylesStream.open(stylesPath, fstream::in);
        if (stylesStream.fail()) {
            string msg = "omc : warning : Unable to open " + stylesPath + " for reading.";
            theLog->logMsg(msg.c_str());
        }
        else {
            stringstream ssStyles;
            ssStyles << stylesStream.rdbuf();
            htmlStyles = ssStyles.str();
            stylesStream.close();
        }
    }

    // read translated strings for all model languages from dir/of/omc.exe/omc.message.ini
    list<string> langLst;
    for (auto langSym : Symbol::pp_all_languages) {
        langLst.push_back(langSym->name);
    }
    allTransaledMsg = IniFileReader::loadAllMessages(i_msgFilePath.c_str(), langLst);

    // Language loop
    for (auto langSym : Symbol::pp_all_languages) {
        int lang_index = langSym->language_id;
        string lang = langSym->name;

        // skip model's secondary languages for modeldev version
        if (devMode && lang_index > 0) {
            continue;
        }

        // Example: IDMM.doc.EN.html
        string htmlName = model_name + ".doc." + lang + ".html";
        string mdName = model_name + ".doc." + lang + ".md";
        if (devMode) {
            htmlName = model_name + ".devdoc." + lang + ".html";
            mdName = model_name + ".devdoc." + lang + ".md";
        }

        // create the markdown file in outDir (normally 'src')
        string mdPath = makeFilePath(outDir.c_str(), mdName.c_str());
        ofstream mdStream(mdPath, ios::out | ios::trunc | ios::binary);
        if (mdStream.fail()) {
            string msg = "omc : warning : Unable to open " + mdName + " for writing.";
            theLog->logMsg(msg.c_str());
        }

        ModelSymbol* theModelSymbol = dynamic_cast<ModelSymbol*>(Symbol::find_a_symbol(typeid(ModelSymbol)));
        assert(theModelSymbol);
        bool flagModelNotePresent = theModelSymbol->note(*langSym).length() > 0;

        // higher-level topics
        string anchorTableOfContents = "table-of-contents";
        string anchorModelIntroduction = "model-introduction";
        string anchorParametersAlphabetic = "parameters-alphabetic";
        string anchorParameterHierarchy = "parameter-hierarchy";
        string anchorParameterMajorGroups = "parameter-major_groups";
        string anchorTablesAlphabetic = "tables-alphabetic";
        string anchorTableHierarchy = "table-hierarchy";
        string anchorTableMajorGroups = "table-major-groups";
        string anchorEnumerationsAlphabetic = "enumerations-alphabetic";

        bool do_parameter_hierarchy = false;
        for (auto s : Symbol::pp_all_parameter_groups) {
            if (s->is_published()) {
                do_parameter_hierarchy = true;
                break;
            }
        }

        bool do_table_hierarchy = false;
        for (auto s : Symbol::pp_all_table_groups) {
            if (s->is_published()) {
                do_table_hierarchy = true;
                break;
            }
        }

        // HTML fragment for topic separator
        string topicSeparator = "\n\n<p style=\"margin-bottom:3cm; break-after:page;\"></p>\n\n";

        // Topic: Table of contents
        {
            VersionSymbol* vs = dynamic_cast<VersionSymbol*>(Symbol::find_a_symbol(typeid(VersionSymbol)));
            assert(vs);
            string version_string = to_string(vs->major) + "." + to_string(vs->minor) + "." + to_string(vs->sub_minor) + "." + to_string(vs->sub_sub_minor);

            const size_t ymd_size = 50;
            char ymd[ymd_size];
            std::time_t time = std::time({});
            std::strftime(ymd, ymd_size, "%F %T", std::localtime(&time));

            string mainTitle = LTA(lang, "Model Documentation");
            if (devMode) {
                mainTitle += " (" + LTA(lang, "Developer Edition") + ")";
            }

            mdStream << "<h1 id=\"" + anchorTableOfContents + "\">" + model_name + " - " + mainTitle + "</h1>\n\n";
            mdStream << "<h2>" + LTA(lang, "Version") + " " + version_string + " " + LTA(lang,"built on") + " " + ymd + "</h2>\n\n";
            mdStream << "\n\n";
            mdStream << "<h3>" + LTA(lang, "Table of Contents") + "</h3>\n\n";
            mdStream << "|table>\n"; // maddy-specific begin table
            mdStream << LTA(lang, "Topic") + " | " + LTA(lang, "Description") + "\n"; // maddy-specific table header separator
            mdStream << "- | - | -\n"; // maddy-specific table header separator
            if (flagModelNotePresent) {
                mdStream << "[" + LTA(lang, "Introduction") + "](#" + anchorModelIntroduction + ") | " + LTA(lang, "Overview of the model") + "\n";
            }
            if (do_parameter_hierarchy || do_table_hierarchy) {
                mdStream << "**" + LTA(lang, "Major Groups") + "** | \n";
            }
            if (do_parameter_hierarchy) {
                mdStream << "&nbsp;&nbsp;[" + LTA(lang, "Parameters") + "](#" + anchorParameterMajorGroups + ") | " + LTA(lang, "Major groups of input parameters") + "\n";
            }
            if (do_table_hierarchy) {
                mdStream << "&nbsp;&nbsp;[" + LTA(lang, "Tables") + "](#" + anchorTableMajorGroups + ") | " + LTA(lang, "Major groups of output tables") + "\n";
            }
            if (do_parameter_hierarchy || do_table_hierarchy) {
                mdStream << "**" + LTA(lang, "Hierarchies") + "** | \n";
            }
            if (do_parameter_hierarchy) {
                mdStream << "&nbsp;&nbsp;[" + LTA(lang, "Parameters") + "](#" + anchorParameterHierarchy + ") | " + LTA(lang, "Input parameters arranged hierarchically") + "\n";
            }
            if (do_table_hierarchy) {
                mdStream << "&nbsp;&nbsp;[" + LTA(lang, "Tables") + "](#" + anchorTableHierarchy + ") | " + LTA(lang, "Output tables arranged hierarchically") + "\n";
            }
            mdStream << "**" + LTA(lang, "Lists") + "** | \n";
            mdStream << "&nbsp;&nbsp;[" + LTA(lang, "Parameters") + "](#" + anchorParametersAlphabetic + ") | " + LTA(lang,"Input parameters in alphabetic order") + "\n";
            mdStream << "&nbsp;&nbsp;[" + LTA(lang, "Tables") + "](#" + anchorTablesAlphabetic + ") | " + LTA(lang, "Output tables in alphabetic order") + "\n";
            mdStream << "&nbsp;&nbsp;[" + LTA(lang, "Enumerations") + "](#" + anchorEnumerationsAlphabetic + ") | " + LTA(lang, "Enumerations in alphabetic order") + "\n";
            mdStream << "|<table\n"; // maddy-specific end table
            mdStream << topicSeparator;
        }

        // Topic: introduction
        if (flagModelNotePresent) {
            mdStream << "<h3 id=\"" + anchorModelIntroduction + "\">" + LTA(lang, "Introduction to") + " " + model_name + "</h3>\n\n";
            mdStream << theModelSymbol->note(*langSym);
            mdStream << "\n\n[[" + LTA(lang, "Table of Contents") + "](#"+ anchorTableOfContents +")]\r\n";
            mdStream << topicSeparator;
        }

        // Table-like symbols - published tables merged with derived parameters published as tables
        list<Symbol*> table_like;
        {
            for (auto t : Symbol::pp_all_tables) {
                if (t->is_published()) {
                    table_like.push_back(t);
                }
            }
            for (auto p : Symbol::pp_all_parameters) {
                if (p->publish_as_table && p->is_published()) {
                    table_like.push_back(p);
                }
            }
            table_like.sort([](Symbol* a, Symbol* b) { return a->name < b->name; });
        }

        // Parameter-like symbols - published parameters excluding derived parameters published as tables
        list<Symbol*> parameter_like;
        {
            for (auto p : Symbol::pp_all_parameters) {
                if (!p->publish_as_table && p->is_published()) {
                    parameter_like.push_back(p);
                }
            }
            // no need to sort because pp_all_parameters is already sorted
        }

        // Topic: parameters in alphabetic order
        {
            // build line with links to first parameter in alphabetic table with leading letter
            string letterLinks;
            {
                string letterPrev = "";
                for (auto& s : parameter_like) {
                    string letterCurr = s->name.substr(0, 1);
                    if (letterCurr != letterPrev) {
                        // anchor looks like A-param, D-param, etc.
                        letterLinks += " [" + letterCurr + "](#" + letterCurr + "-param)";
                    }
                    letterPrev = letterCurr;
                }
            }

            mdStream << "<h3 id=\"" + anchorParametersAlphabetic + "\">" + LTA(lang, "Parameters in alphabetic order") + "</h3>\n\n";
            mdStream << letterLinks + "\n\n";
            mdStream << "|table>\n"; // maddy-specific begin table
            mdStream << " " + LTA(lang, "Name") + " | " + LTA(lang, "Label") + " \n";

            mdStream << "- | - | -\n"; // maddy-specific table header separator
            {
                string letterPrev = "";
                for (auto& s : parameter_like) {
                    string letterCurr = s->name.substr(0, 1);
                    string letterLink = "";
                    if (letterCurr != letterPrev) {
                        // anchor looks like A-param, D-param, etc.
                        letterLink += "<div id=\"" + letterCurr + "-param\"/>";
                    }
                    letterPrev = letterCurr;

                    mdStream << letterLink + " [`" << s->name << "`](#" << s->name << ") | " << s->pp_labels[lang_index] << "  \n";
                } // end parameter table
            }
            mdStream << "|<table\n"; // maddy-specific end table
            mdStream << "\n\n[[" + LTA(lang, "Table of Contents") + "](#" + anchorTableOfContents + ")]\r\n";
            mdStream << topicSeparator;
        }

        // Topic: parameter major groups
        {
            mdStream << "<h3 id=\"" + anchorParameterMajorGroups + "\">" + LTA(lang, "Parameter Major Groups") + "</h3>\n\n";

            mdStream << "<p>";
            // top-level groups
            for (auto g : Symbol::pp_all_parameter_groups) {
                if (g->pp_parent_group || !g->is_published()) {
                    // skip group if not top-level or not published.
                    continue;
                }
                mdStream << expand_group(lang_index, g, 0, 1, true);
            }
            mdStream << "</p>\n\n";
            mdStream << "\n\n[[" + LTA(lang, "Table of Contents") + "](#" + anchorTableOfContents + ")]\r\n";
            mdStream << topicSeparator;
        }

        // Topic: parameter hierarchy
        {
            mdStream << "<h3 id=\"" + anchorParameterHierarchy + "\">" + LTA(lang, "Parameter hierarchy") + "</h3>\n\n";

            // top-level groups
            for (auto g : Symbol::pp_all_parameter_groups) {
                if (g->pp_parent_group || !g->is_published()) {
                    // skip group if not top-level or not published.
                    continue;
                }
                mdStream << "<p>" + expand_group(lang_index, g, 0, 1000, false) + "</p>\n\n";
            }
            // orphan symbols
            {
                string orphan_fragment;
                bool first_orphan = true;
                for (auto s : parameter_like) {
                    if (s->pp_parent_group) {
                        // skip symbol if in a group.
                        continue;
                    }
                    if (first_orphan) {
                        first_orphan = false;
                        orphan_fragment = "<b>" + LTA(lang, "Orphan parameters") + "</b><br>";
                    }
                    // symbol hyperlink, name, and label
                    orphan_fragment += 
                        + "<a href=\"#" + s->name + "\"><code>" + s->name + "</code></a> "
                        + s->pp_labels[lang_index]
                        + "<br>";
                }
                if (orphan_fragment.length() > 0) {
                    mdStream << "<p>" + orphan_fragment + "</p>\n\n";
                }
            }
            mdStream << "\n\n[[" + LTA(lang, "Table of Contents") + "](#" + anchorTableOfContents + ")]\r\n";
            mdStream << topicSeparator;
        }

        // Topic for each published parameter
        for (auto s : Symbol::pp_all_parameters) {
            if (!s->is_published()) {
                // skip parameter if not published
                continue;
            }

            /// is a scalar parameter
            bool isScalar = (s->pp_shape.size() == 0);

            // topic header line
            mdStream
                // symbol name
                << "<h3 id=\"" << s->name << "\">" << " <code>" + s->name + "</code>"
                // symbol label
                << "<span style=\"font-weight:lighter\"> " << s->pp_labels[lang_index] << "</span></h3>\n\n"
                ;

            // summary line with type, size, etc.
            {
                string kindInfo;
                {
                    if (s->is_derived()) {
                        kindInfo += LTA(lang, "Derived Parameter");
                    }
                    else {
                        kindInfo = LTA(lang, "Parameter");
                    }
                    if (s->is_hidden) {
                        kindInfo += "(" + LTA(lang, "hidden") + ")";
                    }
                }
                string shapeCompact;
                if (!isScalar) {
                    bool isFirst = true;
                    int cells = 1;
                    for (int ps : s->pp_shape) {
                        shapeCompact += (!isFirst ? ", " : "") + to_string(ps);
                        isFirst = false;
                        cells *= ps;
                    }
                    shapeCompact = "[ " + shapeCompact + " ] = " + to_string(cells);
                }
                else {
                    shapeCompact = LTA(lang, "scalar");
                }
                string defaultValue = "";
                if (isScalar && (s->source == ParameterSymbol::parameter_source::scenario_parameter)) {
                    /// for sub=0
                    auto& constants = s->sub_initial_list.front().second;
                    auto& constant = constants.front();
                    if (constant->is_literal) {
                        defaultValue = constant->literal->value();
                    }
                    else if (constant->is_enumerator) {
                        /// pointer to EnumeratorSymbol
                        auto e = *(constant->enumerator);
                        defaultValue = e->name;
                        // Show short name instead if a classification enumerator, since that's what user sees.
                        auto ce = dynamic_cast<ClassificationEnumeratorSymbol*>(e);
                        if (ce) {
                            defaultValue = ce->short_name;
                        }
                    }
                    else {
                        assert(false); // not reached
                    }
                }
                string typeInfo;
                {
                    if (s->pp_datatype->is_enumeration()) {
                        // hyperlink to enumeration
                        typeInfo = "[`" + s->pp_datatype->name + "`](#" + s->pp_datatype->name + ")";
                    }
                    else {
                        // just show the name of the type, e.g. "double"
                        typeInfo = s->pp_datatype->name;
                    }
                }
                mdStream
                    << "**" + LTA(lang, "Kind") + ":** " + kindInfo
                    << "\n"
                    << "**" + LTA(lang, "Type") + ":** "
                    << "\n"
                    << typeInfo
                    << " **" + LTA(lang, "Size") + ":** " << shapeCompact;
                if (defaultValue.length() > 0) {
                    mdStream << "\n**" + LTA(lang, "Default") + ":** " + defaultValue;
                }
                mdStream << "\n\n";
            }

            // bread crumb hierachy (possibly empty)
            mdStream << bread_crumb_hierarchy(lang, lang_index, s);

            // dimension table with links
            if (!isScalar) {
                mdStream << "**"+ LTA(lang, "Dimensions") + ":**\n\n";
                mdStream << "|table>\n"; // maddy-specific begin table
                mdStream << " " + LTA(lang, "External Name") + " | " + LTA(lang, "Enumeration") + " | " + LTA(lang, "Size") + " | " + LTA(lang, "Label") + " \n";
                mdStream << "- | - | -\n"; // maddy-specific table header separator
                for (auto& dim : s->dimension_list) {
                    string dim_export_name = dim->short_name;
                    auto e = dim->pp_enumeration;
                    assert(e); // dimensions of parameters are always enumerations
                    string dim_enumeration = e->name;
                    string dim_size = to_string(e->pp_size());
                    string dim_label = dim->pp_labels[lang_index];
                    mdStream
                        << dim_export_name << " | "
                        << "[`" + dim_enumeration +"`](#" + dim_enumeration + ")" << " | "
                        << dim_size << " | "
                        << dim_label << "\n"
                        ;
                }
                mdStream << "|<table\n"; // maddy-specific end table
            }

            // global functions using this parameter
            if (devMode && s->pp_global_funcs_using.size() > 0) {
                mdStream << "**" + LTA(lang, "Referenced in Global Functions") + ":**\n\n";
                mdStream << "|table>\n"; // maddy-specific begin table
                mdStream << " " + LTA(lang, "Function") + " | " + LTA(lang, "Module") + " | " + LTA(lang, "Label") + " \n";
                mdStream << "- | - | -\n"; // maddy-specific table header separator
                for (auto& f : s->pp_global_funcs_using) {
                    string name = f->name;
                    string module = short_loc(f->decl_loc);
                    string label = f->pp_labels[lang_index];
                    mdStream
                        << name << " | "
                        << module << " | "
                        << label << "\n"
                        ;
                }
                mdStream << "|<table\n"; // maddy-specific end table
            }

            // entity functions using this parameter
            if (devMode && s->pp_entity_funcs_using.size() > 0) {
                mdStream << "**" + LTA(lang, "Referenced in Entity Functions") + ":**\n\n";
                mdStream << "|table>\n"; // maddy-specific begin table
                mdStream << " " + LTA(lang, "Entity") + " | " + LTA(lang, "Function") + " | " + LTA(lang, "Module") + " | " + LTA(lang, "Label") + " \n";
                mdStream << "- | - | -\n"; // maddy-specific table header separator
                for (auto& m : s->pp_entity_funcs_using) {
                    string entity = m->pp_entity->name;
                    string member = m->name;
                    string module = short_loc(m->defn_loc);
                    string label = m->pp_labels[lang_index];
                    mdStream
                        << entity << " | "
                        << member << " | "
                        << module << " | "
                        << label << "\n"
                        ;
                }
                mdStream << "|<table\n"; // maddy-specific end table
            }

            // identity attributes using this parameter
            if (devMode && s->pp_identity_attributes_using.size() > 0) {
                mdStream << "**" + LTA(lang, "Referenced in Identity Attributes") + ":**\n\n";
                mdStream << "|table>\n"; // maddy-specific begin table
                mdStream << " " + LTA(lang, "Entity") + " | " + LTA(lang, "Attribute") + " | " + LTA(lang, "Module") + " | " + LTA(lang, "Label") + " \n";
                mdStream << "- | - | -\n"; // maddy-specific table header separator
                for (auto& m : s->pp_identity_attributes_using) {
                    string entity = m->pp_entity->name;
                    string member = m->name;
                    string module = short_loc(m->decl_loc);
                    string label = m->pp_labels[lang_index];
                    mdStream
                        << entity << " | "
                        << member << " | "
                        << module << " | "
                        << label << "\n"
                        ;
                }
                mdStream << "|<table\n"; // maddy-specific end table
            }

            // symbol note if present
            {
                string note_in = s->pp_notes[lang_index];
                if (note_in.length()) {
                    mdStream << "**" + LTA(lang, "Note") + ":**\n\n";
                    string note_out = preprocess_markdown(note_in);
                    mdStream << note_out << "\n\n";
                }
            }

            // parameter value note if present
            {
                string note_in = s->pp_value_notes[lang_index];
                if (note_in.length()) {
                    mdStream << "**" + LTA(lang, "Default Value Note") + ":**\n\n";
                    string note_out = preprocess_markdown(note_in);
                    mdStream << note_out << "\n\n";
                }
            }

            mdStream << "\n\n";
            mdStream << "[[" + LTA(lang, "Table of Contents") + "](#" + anchorTableOfContents + ")]\r\n";
            mdStream << topicSeparator;
        } // Topic for each published parameter

        // Topic: enumerations in alphabetic order
        {
            // build line with links to first symbol in alphabetic table with leading letter
            string letterLinks;
            {
                string letterPrev = "";
                for (auto& s : Symbol::pp_all_enumerations) {
                    if (!s->is_published()) {
                        // skip unpublished symbol
                        continue;
                    }
                    string letterCurr = s->name.substr(0, 1);
                    if (letterCurr != letterPrev) {
                        // anchor looks like A-enum, D-enum, etc.
                        letterLinks += " [" + letterCurr + "](#" + letterCurr + "-enum)";
                    }
                    letterPrev = letterCurr;
                }
            }

            mdStream << "<h3 id=\"" + anchorEnumerationsAlphabetic + "\">" + LTA(lang, "Enumerations in alphabetic order") + "</h3>\n\n";
            mdStream << letterLinks + "\n\n";
            mdStream << "|table>\n"; // maddy-specific begin table
            mdStream << " " + LTA(lang, "Name") + " | " + LTA(lang, "Label") + " \n";

            mdStream << "- | - | -\n"; // maddy-specific table header separator
            {
                string letterPrev = "";
                for (auto& s : Symbol::pp_all_enumerations) {
                    if (!s->is_published()) {
                        // skip unpublished symbol
                        continue;
                    }
                    string letterCurr = s->name.substr(0, 1);
                    string letterLink = "";
                    if (letterCurr != letterPrev) {
                        // anchor looks like A-enum, D-enum, etc.
                        letterLink += "<div id=\"" + letterCurr + "-enum\"/>";
                    }
                    letterPrev = letterCurr;

                    mdStream << letterLink + " [`" << s->name << "`](#" << s->name << ") | " << s->pp_labels[lang_index] << "  \n";
                } // end enumerations table
            }
            mdStream << "|<table\n"; // maddy-specific end table
            mdStream << "\n\n[[" + LTA(lang, "Table of Contents") + "](#" + anchorTableOfContents + ")]\r\n";
            mdStream << topicSeparator;
        } // Topic: enumerations in alphabetic order


        // Topic for each published enumeration
        for (auto& s : Symbol::pp_all_enumerations) {
            if (!s->is_published()) {
                // skip parameter if not published
                continue;
            }

            // topic header line
            mdStream
                // symbol name
                << "<h3 id=\"" << s->name << "\">" << "<code>" + s->name + "</code>"
                // symbol label
                << "<span style=\"font-weight:lighter\"> " << s->pp_labels[lang_index] << "</span></h3>\n\n"
                ;

            // summary line with type and size
            {
                string kind;
                string values;
                if (s->is_range()) {
                    kind = LTA(lang, "range");
                    auto r = dynamic_cast<RangeSymbol*>(s);
                    assert(r);
                    values = "{" + to_string(r->lower_bound) + ",...," + to_string(r->upper_bound) +"}";
                }
                else if (s->is_bool()) {
                    kind = LTA(lang, "bool");
                    values = "{0,1}";
                }
                else if (s->is_classification()) {
                    kind = LTA(lang, "classification");
                    values = "{0,...," + to_string(s->pp_size() - 1) + "}";
                }
                else if (s->is_partition()) {
                    kind = LTA(lang, "partition");
                    values = "{0,...," + to_string(s->pp_size() - 1) + "}";
                }
                else {
                    // not reached
                    assert(false);
                }
                mdStream
                    << "**" + LTA(lang, "Kind") + ":** " << kind
                    << " **" + LTA(lang, "Size") + ":** " << to_string(s->pp_size())
                    << " **" + LTA(lang, "Values") + ":** " << values
                    << "\n\n";
            }

            // table of enumerators of enumeration
            {
                if (s->is_classification()) {
                    auto c = dynamic_cast<ClassificationSymbol*>(s);
                    assert(c);
                    mdStream << "**" + LTA(lang, "Enumerators") + ":**\n\n";
                    mdStream << "|table>\n"; // maddy-specific begin table
                    mdStream << " " + LTA(lang, "External Name") + " | " + LTA(lang, "Enumerator") + " | " + LTA(lang, "Value") + " | " + LTA(lang, "Label") +" \n";
                    mdStream << "- | - | -\n"; // maddy-specific table header separator
                    for (auto enumerator : c->pp_enumerators) {
                        auto ce = dynamic_cast<ClassificationEnumeratorSymbol*>(enumerator);
                        assert(ce); // logic guarantee
                        string export_name = ce->short_name;
                        string name = ce->name;
                        string value = to_string(ce->ordinal);
                        string label = ce->pp_labels[lang_index];
                        mdStream
                            << export_name << " | "
                            << name << " | "
                            << value << " | "
                            << label << "\n"
                            ;
                    }
                    mdStream << "|<table\n"; // maddy-specific end table
                }
                else if (s->is_partition()) {
                    auto p = dynamic_cast<PartitionSymbol*>(s);
                    assert(p);
                    mdStream << "**" + LTA(lang, "Enumerators") + ":**\n\n";
                    mdStream << "|table>\n"; // maddy-specific begin table
                    mdStream << " " + LTA(lang, "Lower") + " | " + LTA(lang, "Upper") + " | " + LTA(lang, "Value") + " | " + LTA(lang, "Label") + " \n";
                    mdStream << "- | - | -\n"; // maddy-specific table header separator
                    for (auto enumerator : p->pp_enumerators) {
                        auto ce = dynamic_cast<PartitionEnumeratorSymbol*>(enumerator);
                        assert(ce); // logic guarantee
                        string lower = ce->lower_split_point;
                        string upper = ce->upper_split_point;
                        string value = to_string(ce->ordinal);
                        string label = ce->label(*langSym); // odd that pp_labels differs...
                        mdStream
                            << lower << " | "
                            << upper << " | "
                            << value << " | "
                            << label << "\n"
                            ;
                    }
                    mdStream << "|<table\n"; // maddy-specific end table
                }
            }

            // table of parameters using this enumeration
            {
                set<string> parameters_used;
                for (auto& p : Symbol::pp_all_parameters) {
                    if (!p->is_published()) {
                        // skip unpublished parameter
                        continue;
                    }
                    // examine enumeration of each parameter dimension
                    for (auto pe : p->pp_enumeration_list) {
                        // s is the current enumeration, pe is an enumeration of parameter p
                        if (s == pe) {
                            parameters_used.insert(p->name);
                        }
                    }
                    // examine datatype of parameter
                    if (p->pp_datatype->is_enumeration()) {
                        if (s == p->pp_datatype) {
                            parameters_used.insert(p->name);
                        }
                    }
                }
                if (parameters_used.size() > 0) {
                    mdStream << "**" + LTA(lang, "Parameters using enumeration") + ":**\n\n";
                    mdStream << "|table>\n"; // maddy-specific begin table
                    mdStream << " " + LTA(lang, "Name") + " | " + LTA(lang, "Label") + " \n";
                    mdStream << "- | - | -\n"; // maddy-specific table header separator
                    for (auto& name : parameters_used) {
                        auto sym = Symbol::get_symbol(name);
                        assert(sym); // logic guarantee
                        auto label = sym->pp_labels[lang_index];
                        mdStream
                            << "[`" + name + "`](#" + name + ")" << " | "
                            << label << "\n"
                            ;
                    }
                    mdStream << "|<table\n"; // maddy-specific end table
                }
            }

            // table of tables using this enumeration
            {
                set<string> tables_used;
                for (auto t : Symbol::pp_all_tables) {
                    if (!t->is_published()) {
                        // skip unpublished table
                        continue;
                    }
                    for (auto d : t->dimension_list) {
                        // s is the current enumeration, te is an enumeration used in table t
                        auto te = d->pp_enumeration;
                        if (s == te) {
                            tables_used.insert(t->name);
                        }
                    }
                }
                if (tables_used.size() > 0) {
                    mdStream << "**" + LTA(lang, "Tables using enumeration") + ":**\n\n";
                    mdStream << "|table>\n"; // maddy-specific begin table
                    mdStream << " " + LTA(lang, "Name") + " | " + LTA(lang, "Label") + " \n";
                    mdStream << "- | - | -\n"; // maddy-specific table header separator
                    for (auto& name : tables_used) {
                        auto sym = Symbol::get_symbol(name);
                        assert(sym); // logic guarantee
                        auto label = sym->pp_labels[lang_index];
                        mdStream
                            << "[`" + name + "`](#" + name + ")" << " | "
                            << label << "\n"
                            ;
                    }
                    mdStream << "|<table\n"; // maddy-specific end table
                }
            }

            // symbol note if present
            {
                string note_in = s->pp_notes[lang_index];
                if (note_in.length()) {
                    mdStream << "**" + LTA(lang, "Note") + ":**\n\n";
                    string note_out = preprocess_markdown(note_in);
                    mdStream << note_out << "\n\n";
                }
            }

            mdStream << "\n\n";
            mdStream << "[[" + LTA(lang, "Table of Contents") + "](#" + anchorTableOfContents + ")]\r\n";
            mdStream << topicSeparator;
        } // Topic for each published enumeration

        // Topic: tables in alphabetic order
        {
            // build line with links to first table in alphabetic table with leading letter
            string letterLinks;
            {
                string letterPrev = "";
                for (auto& s : table_like) {
                    string letterCurr = s->name.substr(0, 1);
                    if (letterCurr != letterPrev) {
                        // anchor looks like A-param, D-param, etc.
                        letterLinks += " [" + letterCurr + "](#" + letterCurr + "-param)";
                    }
                    letterPrev = letterCurr;
                }
            }

            mdStream << "<h3 id=\"" + anchorTablesAlphabetic + "\">" + LTA(lang, "Tables in alphabetic order") + "</h3>\n\n";
            mdStream << letterLinks + "\n\n";
            mdStream << "|table>\n"; // maddy-specific begin table
            mdStream << " " + LTA(lang, "Name") + " | " + LTA(lang, "Label") + " \n";

            mdStream << "- | - | -\n"; // maddy-specific table header separator
            {
                string letterPrev = "";
                for (auto& s : table_like) {
                    string letterCurr = s->name.substr(0, 1);
                    string letterLink = "";
                    if (letterCurr != letterPrev) {
                        // anchor looks like A-param, D-param, etc.
                        letterLink += "<div id=\"" + letterCurr + "-param\"/>";
                    }
                    letterPrev = letterCurr;

                    mdStream << letterLink + " [`" << s->name << "`](#" << s->name << ") | " << s->pp_labels[lang_index] << "  \n";
                } // end parameter table
            }
            mdStream << "|<table\n"; // maddy-specific end table
            mdStream << "\n\n[[" + LTA(lang, "Table of Contents") + "](#" + anchorTableOfContents + ")]\r\n";
            mdStream << topicSeparator;
        } // Topic: tables in alphabetic order

        // Topic: table major groups
        {
            mdStream << "<h3 id=\"" + anchorTableMajorGroups + "\">" + LTA(lang, "Table Major Groups") + "</h3>\n\n";

            mdStream << "<p>";
            // top-level groups
            for (auto g : Symbol::pp_all_table_groups) {
                if (g->pp_parent_group || !g->is_published()) {
                    // skip group if not top-level or not published.
                    continue;
                }
                mdStream << expand_group(lang_index, g, 0, 1, true);
            }
            mdStream << "</p>\n\n";
            mdStream << "\n\n[[" + LTA(lang, "Table of Contents") + "](#" + anchorTableOfContents + ")]\r\n";
            mdStream << topicSeparator;
        }

        // Topic: table hierarchy
        {
            mdStream << "<h3 id=\"" + anchorTableHierarchy + "\">" + LTA(lang, "Table hierarchy") + "</h3>\n\n";

            // top-level groups
            for (auto g : Symbol::pp_all_table_groups) {
                if (g->pp_parent_group || !g->is_published()) {
                    // skip group if not top-level or not published.
                    continue;
                }
                mdStream << "<p>" + expand_group(lang_index, g, 0, 1000, false) + "</p>\n\n";
            }
            // orphan symbols
            {
                string orphan_fragment;
                bool first_orphan = true;
                for (auto s : table_like) {
                    bool is_orphan = !s->pp_parent_group;
                    // Handle special case of a derived parameter published as a table
                    // whose parent is a parameter group.
                    if (!is_orphan) {
                        bool is_derived_parameter = dynamic_cast<ParameterSymbol*>(s);
                        bool parent_is_parameter_group = dynamic_cast<ParameterGroupSymbol*>(s->pp_parent_group);
                        if (is_derived_parameter && parent_is_parameter_group) {
                            is_orphan = true;
                        }
                    }
                    if (!is_orphan) {
                        continue;
                    }
                    if (first_orphan) {
                        first_orphan = false;
                        orphan_fragment = "<b>" + LTA(lang, "Orphan tables") + "</b><br>";
                    }
                    // symbol hyperlink, name, and label
                    orphan_fragment +=
                        +"<a href=\"#" + s->name + "\"><code>" + s->name + "</code></a> "
                        + s->pp_labels[lang_index]
                        + "<br>";
                }
                if (orphan_fragment.length() > 0) {
                    mdStream << "<p>" + orphan_fragment + "</p>\n\n";
                }
            }

            mdStream << "\n\n[[" + LTA(lang, "Table of Contents") + "](#" + anchorTableOfContents + ")]\r\n";
            mdStream << topicSeparator;
        }

        // Topic for each published table
        for (auto& s : Symbol::pp_all_tables) {
            if (!s->is_published()) {
                // skip table if not published
                continue;
            }

            /// is a scalar table
            bool isScalar = (s->dimension_list.size() == 0);

            // topic header line
            mdStream
                // symbol name
                << "<h3 id=\"" << s->name << "\">" << " <code>" + s->name + "</code>"
                // symbol label
                << "<span style=\"font-weight:lighter\"> " << s->pp_labels[lang_index] << "</span></h3>\n\n"
                ;

            // summary line with type and size
            {
                string kindInfo;
                {
                    kindInfo = LTA(lang, "Table");
                    if (s->is_hidden) {
                        kindInfo += "(" + LTA(lang, "hidden") + ")";
                    }
                }
                string shapeCompact;
                if (!isScalar) {
                    bool isFirst = true;
                    int cells = 1;
                    for (auto dim : s->dimension_list) {
                        auto e = dim->pp_enumeration;
                        assert(e);
                        auto dim_size = e->pp_size() + dim->has_margin;
                        shapeCompact += (!isFirst ? ", " : "") + to_string(dim_size);
                        isFirst = false;
                        cells *= dim_size;
                    }
                    shapeCompact = "[ " + shapeCompact + " ] = " + to_string(cells);
                }
                else {
                    shapeCompact = "1";
                }
                string measures = to_string(s->measure_count());
                mdStream
                    << "**" + LTA(lang, "Kind") + ":** " + kindInfo
                    << "\n"
                    << " **" + LTA(lang, "Cells") + ":** " << shapeCompact
                    << "\n"
                    << " **" + LTA(lang, "Measures") + ":** " << measures
                    << "\n\n";
            }

            // bread crumb hierarchy (possibly empty)
            mdStream << bread_crumb_hierarchy(lang, lang_index, s);

            // dimension table with links
            if (!isScalar) {
                mdStream << "**" + LTA(lang, "Dimensions") + ":**\n\n";
                mdStream << "|table>\n"; // maddy-specific begin table
                mdStream << " " + LTA(lang, "External Name") + " | " + LTA(lang, "Enumeration") + " | " + LTA(lang, "Size") + " | " + LTA(lang, "Margin") + " | " + LTA(lang, "Label") + " \n";
                mdStream << "- | - | -\n"; // maddy-specific table header separator
                for (auto& dim : s->dimension_list) {
                    string dim_export_name = dim->short_name;
                    auto e = dim->pp_enumeration;
                    assert(e); // dimensions of parameters are always enumerations
                    string dim_enumeration = e->name;
                    string dim_size = to_string(e->pp_size());
                    string dim_label = dim->pp_labels[lang_index];
                    mdStream
                        << dim_export_name << " | "
                        << "[`" + dim_enumeration + "`](#" + dim_enumeration + ")" << " | "
                        << dim_size << " | "
                        << (dim->has_margin ? "**X**" : "") << " | "
                        << dim_label << "\n"
                        ;
                }
                mdStream << "|<table\n"; // maddy-specific end table
            }

            // measure table
            {
                mdStream << "**" + LTA(lang, "Measures") + ":**\n\n";
                mdStream << "|table>\n"; // maddy-specific begin table
                mdStream << " " + LTA(lang, "External Name") + " | " + LTA(lang, "Label") + " \n";
                mdStream << "- | - | -\n"; // maddy-specific table header separator
                for (auto& m : s->pp_measures) {
                    string m_export_name = m->short_name;
                    string m_label = m->pp_labels[lang_index];
                    mdStream
                        << m_export_name << " | "
                        << m_label << "\n"
                        ;
                }
                mdStream << "|<table\n"; // maddy-specific end table
            }

            // symbol note if present
            {
                string note_in = s->pp_notes[lang_index];
                if (note_in.length()) {
                    mdStream << "**" + LTA(lang, "Note") + ":**\n\n";
                    string note_out = preprocess_markdown(note_in);
                    mdStream << note_out << "\n\n";
                }
            }

            mdStream << "\n\n";
            mdStream << "[[" + LTA(lang, "Table of Contents") + "](#" + anchorTableOfContents + ")]\r\n";
            mdStream << topicSeparator;
        } // Topic for each published table

        // all done
        mdStream.close();

        // convert markdown to HTML
        {
            // maddy set-up
            std::shared_ptr<maddy::ParserConfig> config = std::make_shared<maddy::ParserConfig>();
            config->enabledParsers &= ~maddy::types::EMPHASIZED_PARSER;
            config->enabledParsers |= maddy::types::HTML_PARSER;
            config->enabledParsers |= maddy::types::LATEX_BLOCK_PARSER;
            std::shared_ptr<maddy::Parser> parser = std::make_shared<maddy::Parser>(config);

            ofstream htmlStream;
            ofstream mdStream;
            string hdpth = makeFilePath(pubDir.c_str(), htmlName.c_str());
            htmlStream.open(hdpth, fstream::out);
            mdStream.open(mdPath, fstream::in);

            std::stringstream markdownInput;
            markdownInput << mdStream.rdbuf();
            string htmlOutput = parser->Parse(markdownInput);
            htmlStream << "<!DOCTYPE html>\n";
            htmlStream << "<html>\n";
            htmlStream << "<head>\n";

            // experimental: include LaTeX(MathJax) support 
            htmlStream << "<meta charset=\"utf-8\">\n";
            htmlStream << "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
            htmlStream << "<script src=\"https://polyfill.io/v3/polyfill.min.js?features=es6\"></script>\n";
            htmlStream << "<script id=\"MathJax-script\" async \n"
                            "src=\"https://cdn.jsdelivr.net/npm/mathjax@3/es5/tex-mml-chtml.js\">";
            htmlStream << "</script>\n";

            htmlStream << htmlStyles;

            htmlStream << "</head>\n";

            htmlStream << "<body>\n";
            htmlStream << htmlOutput;
            htmlStream << "</body>\n";
            htmlStream << "</html>\n";
            htmlStream.close();
            mdStream.close();
        }


    } // end language loop


} // end do_model_doc
