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
#include "EntityTableMeasureSymbol.h"
#include "EntityTableMeasureAttributeSymbol.h"
#include "EntityTableAccumulatorSymbol.h"
#include "EntityEventSymbol.h"
#include "IdentityAttributeSymbol.h"
#include "DerivedAttributeSymbol.h"
#include "MaintainedAttributeSymbol.h"
#include "SimpleAttributeSymbol.h"
#include "GlobalFuncSymbol.h"
#include "ModuleSymbol.h"
#include "ParseContext.h"
#include "omc_file.h"
#include "omc_model_metrics.h"

#include "maddy/parser.h"

using namespace std;
using namespace openm;
using namespace omc;

/**
 * Find all paths to top of hierarchy from a symbol
 *
 * Called recursively.
 * 
 * @param [in,out]  sym    Symbol at top of path.
 * @param [in,out]  path   Path being extended.
 * @param [out]     result Resulting list of paths in the hierarcy.
 */
static void all_paths(Symbol *sym, list<Symbol*> *path, list<list<Symbol*>*> &result)
{
    // extend the path with current symbol
    path->push_front(sym);
    auto parents = sym->pp_parent_groups;
    if (parents.size() == 0) {
        // terminal at top of hierarchy.
        // add completed path to the list of paths
        result.push_back(path);
        // that's all
        return;
    }
    for (auto parent : parents) {
        // create a deep copy of the current path
        auto *new_path = new list<Symbol*>;
        for (auto& e : *path) {
            new_path->push_back(e);
        }
        // and extend it upwards
        all_paths(parent, new_path, result);
    }
    // cleanup of path
    // - not the multiple 'new_path' created above
    // - not the 'path' which terminates at the top of the hierarchy (note 'return' above).
    path->clear();
    delete path;
}

/**
 * helper function to construct bread crumb hierarchy for symbol s
 *
 * @param           lang         The language.
 * @param           lang_index   Zero-based index of the language.
 * @param [in,out]  sym_terminal The Symbol to process.
 *
 * @returns A single-line string which is a self-contained HTML fragment.
 */
static string bread_crumb_hierarchy(string lang, int lang_index, Symbol* sym_terminal)
{
    string bread_crumbs;
    if (sym_terminal->has_parent_group()) {
        // prime the pump for the breadth-first traversal of all paths up the hierarchy from s
        // A list of lists.  Each list is a path in the hierarchy
        list<list<Symbol*>*> result;
        
        // first path, empty
        auto the_path = new list<Symbol*>;
        all_paths(sym_terminal, the_path, result);

        bread_crumbs = "<p>";
        bread_crumbs += "<b>" + LTA(lang, "Hierarchy") + ":</b><br>";

        for (auto path : result) {
            string indent = "";
            size_t steps = 0;
            for (auto s : *path) {
                /// HTML fragment for breadcrumbs (escaped from maddy)
                string name = s->name;
                string label = s->pp_labels[lang_index];
                if (steps < (path->size() - 1)) {
                    // group in hierarchy
                    bread_crumbs += indent + "<a href=\"#" + name + "\"><b>" + label + "</b></a><br>";
                }
                else {
                    // terminal
                    bread_crumbs += indent + "<code><b>" + s->name + "</b></code>";
                }
                ++steps;
                indent += "&nbsp;&nbsp;&nbsp;&nbsp;";
            }
            bread_crumbs += "<br>"; // separator between paths
        }
        bread_crumbs += "</p>\n\n";

        // clean up result
        for (auto& path : result) {
            path->clear();
            delete path;
        }
    }
    return bread_crumbs;
}

/**
 * Expand group
 * 
 * Called recursively.
 *
 * @param   lang_index     Zero-based index of the language.
 * @param   g              A GroupSymbol to process.
 * @param   depth          The depth.
 * @param   max_depth      The maximum depth.
 * @param   summary_mode   True to enable summary mode, false to disable it.
 * @param   do_unpublished True if do unpublished.
 *
 * @returns A string.
 */
static string expand_group(int lang_index, const GroupSymbol* g, int depth, int max_depth, bool summary_mode, bool do_unpublished)
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
            if (sg && (sg->is_published() || do_unpublished) && ((max_depth != -1) && (depth < max_depth))) {
                // expand recursively to next level down in hierarchy
                result += expand_group(lang_index, sg, depth + 1, max_depth, summary_mode, do_unpublished);
            }
            else if (!summary_mode && (s->is_published() || do_unpublished)) {
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

/**
 * Applies pre-processing to the markdown for maddy
 *
 * @param   md_in The input markdown.
 *
 * @returns The pre-preocessed markdown.
 */
static string preprocess_markdown(const string& md_in)
{
    string md_out;

    // Convert markdown line break (two or more trailing spaces) to maddy-specifc \r\n
    md_out = std::regex_replace(md_in, std::regex("  +\\n"), "\r\n"); // maddy-specific line break
    
    return md_out;
}

/**
 * Construct a maddy markdown link for a symbol
 *
 * @param   name The name to display for the symbol and the target of the link.
 *
 * @returns A markdwon fragment.
 */
static string maddy_link(const string& name)
{
    return "[`" + name + "`](#" + name + ")";
}

/**
 * Construct a maddy markdown link for a symbol
 *
 * @param   name The name to display for the symbol
 * @param   link The target of the link.
 *
 * @returns A markdwon fragment.
 */
static string maddy_link(const string& name, const string& link)
{
    return "[`" + name + "`](#" + link + ")";
}

/**
 * Construct a maddy markdown name for a symbol
 *
 * @param   name The name to display for the symbol.
 *
 * @returns A markdwon fragment.
 */
static string maddy_symbol(const string& name)
{
    return "`" + name + "`";
}

/**
 * Escape markdown characters in a generated label
 *
 * @param   s          The label.
 * @param   lang_index Zero-based index of the language.
 *
 * @returns A markdwon fragment.
 */
static string escape_generated_label(const Symbol *s, int lang_index)
{
    string lbl = s->pp_labels[lang_index];
    if (s->pp_labels_explicit[lang_index]) {
        // label is not generated
        return lbl;
    }
    else {
        // Replace "|" by HTML entity
        lbl = std::regex_replace(lbl, std::regex("\\|"), "&vert;");
        // markdown as code
        return "`" + lbl + "`";
    }
}

/**
 * Construct the cross reference content for a symbol
 *
 * @param           lang       The language.
 * @param           lang_index Zero-based index of the language.
 * @param [in,out]  s          If non-null, a Symbol to process.
 * @param           name       The name to use for the symbol in output.
 * @param [in,out]  mdStream   The md stream.
 *
 * @returns True if it succeeds, false if it fails.
 */
static bool do_xref(string lang, int lang_index, Symbol* s, string name, std::stringstream &mdStream)
{
    bool any_xref = false;
    // global functions using this symbol
    if (s->pp_global_funcs_using.size() > 0) {
        any_xref = true;
        mdStream << "<strong>" + LTA(lang, "Global functions using") + " <code>" + name + "</code>:</strong>\n\n";
        mdStream << "|table>\n"; // maddy-specific begin table
        mdStream << " " + LTA(lang, "Function") + " | " + LTA(lang, "Module") + " | " + LTA(lang, "Label") + " \n";
        mdStream << "- | - | -\n"; // maddy-specific table header separator
        for (auto& f : s->pp_global_funcs_using) {
            string name = maddy_symbol(f->name);
            string module = f->pp_module ? maddy_link(f->pp_module->name) : "";
            string label = f->pp_labels[lang_index];
            mdStream
                << name << " | "
                << module << " | "
                << label << "\n"
                ;
        }
        mdStream << "|<table\n"; // maddy-specific end table
    }

    // entity functions using this symbol
    if (s->pp_entity_funcs_using.size() > 0) {
        any_xref = true;
        mdStream << "<strong>" + LTA(lang, "Entity functions using") + " <code>" + name + "</code>:</strong>\n\n";
        mdStream << "|table>\n"; // maddy-specific begin table
        mdStream << " " + LTA(lang, "Entity") + " | " + LTA(lang, "Function") + " | " + LTA(lang, "Module") + " | " + LTA(lang, "Label") + " \n";
        mdStream << "- | - | -\n"; // maddy-specific table header separator
        for (auto& m : s->pp_entity_funcs_using) {
            string entity = maddy_symbol(m->pp_entity->name);
            string member = maddy_symbol(m->name);
            string module = m->pp_module ? maddy_link(m->pp_module->name) : "";
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

    // identity attributes using this symbol
    if (s->pp_identity_attributes_using.size() > 0) {
        any_xref = true;
        mdStream << "<strong>" + LTA(lang, "Identity attributes using") + " <code>" + name + "</code>:</strong>\n\n";
        mdStream << "|table>\n"; // maddy-specific begin table
        mdStream << " " + LTA(lang, "Entity") + " | " + LTA(lang, "Attribute") + " | " + LTA(lang, "Module") + " | " + LTA(lang, "Label") + " \n";
        mdStream << "- | - | -\n"; // maddy-specific table header separator
        for (auto& m : s->pp_identity_attributes_using) {
            string entity = maddy_symbol(m->pp_entity->name);
            string member = maddy_link(m->name, m->dot_name());
            string module = m->pp_module ? maddy_link(m->pp_module->name) : "";
            string label = escape_generated_label(m, lang_index);
            mdStream
                << entity << " | "
                << member << " | "
                << module << " | "
                << label << "\n"
                ;
        }
        mdStream << "|<table\n"; // maddy-specific end table
    }

    // derived attributes using this symbol
    if (s->pp_derived_attributes_using.size() > 0) {
        any_xref = true;
        mdStream << "<strong>" + LTA(lang, "Derived attributes using") + " <code>" + name + "</code>:</strong>\n\n";
        mdStream << "|table>\n"; // maddy-specific begin table
        mdStream << " " + LTA(lang, "Entity") + " | " + LTA(lang, "Attribute") + " \n";
        mdStream << "- | - | -\n"; // maddy-specific table header separator
        for (auto& m : s->pp_derived_attributes_using) {
            string entity = maddy_symbol(m->pp_entity->name);
            string member = maddy_link(m->pretty_name(), m->dot_name());
            mdStream
                << entity << " | "
                << member << " | "
                << "\n"
                ;
        }
        mdStream << "|<table\n"; // maddy-specific end table
    }

    // entity tables using this symbol
    if (s->pp_entity_tables_using.size() > 0) {
        any_xref = true;
        mdStream << "<strong>" + LTA(lang, "Entity tables using") + " <code>" + name + "</code>:</strong>\n\n";
        mdStream << "|table>\n"; // maddy-specific begin table
        mdStream << " " + LTA(lang, "Table") + " | " + LTA(lang, "Module") + " | " + LTA(lang, "Label") + " \n";
        mdStream << "- | - | -\n"; // maddy-specific table header separator
        for (auto& m : s->pp_entity_tables_using) {
            string symbol = maddy_link(m->name);
            string module = m->pp_module ? maddy_link(m->pp_module->name) : "";
            string label = m->pp_labels[lang_index];
            mdStream
                << symbol << " | "
                << module << " | "
                << label << "\n"
                ;
        }
        mdStream << "|<table\n"; // maddy-specific end table
    }

    // entity sets using this symbol
    if (s->pp_entity_sets_using.size() > 0) {
        any_xref = true;
        mdStream << "<strong>" + LTA(lang, "Entity sets using") + " <code>" + name + "</code>:</strong>\n\n";
        mdStream << "|table>\n"; // maddy-specific begin table
        mdStream << " " + LTA(lang, "Entity set") + " | " + LTA(lang, "Module") + " | " + LTA(lang, "Label") + " \n";
        mdStream << "- | - | -\n"; // maddy-specific table header separator
        for (auto& m : s->pp_entity_sets_using) {
            string symbol = maddy_link(m->name);
            string module = m->pp_module ? maddy_link(m->pp_module->name) : "";
            string label = m->pp_labels[lang_index];
            mdStream
                << symbol << " | "
                << module << " | "
                << label << "\n"
                ;
        }
        mdStream << "|<table\n"; // maddy-specific end table
    }
    return any_xref;
}

/**
 * create model user documentation and model developer documentation.
 *
 * @param   model_name Name of the model.
 * @param   docOutDir  The document out dir.
 * @param   mdOutDir   The md out dir.
 * @param   omrootDir  The omroot dir.
 * @param   cg         The cg.
 */
void do_model_doc(
    const string& model_name, const string& docOutDir, const string& mdOutDir, const string& omrootDir, const CodeGen& cg
) {

    // create json file for ompp UI:
    /*
    {
    "ModelDoc": [{
            "LangCode": "EN",
            "Link": "RiskPaths.doc.EN.html"
        }, {
            "LangCode": "FR",
            "Link": "RiskPaths.doc.FR.html"
        }
    ]
    }
    */
    {
        string json_name = model_name + ".extra.json";
        string json_path = makeFilePath(baseDirOf(docOutDir).c_str(), json_name.c_str());
        ofstream json_stream(json_path, ios::out | ios::trunc | ios::binary);
        if (json_stream.fail()) {
            theLog->logFormatted("omc : error : Unable to open '%s' for writing.", json_path.c_str());
            throw HelperException(LT("fatal error - no model documentation"));
        }

        json_stream << "{\n\"ModelDoc\": [\n";
        size_t n = 0;

        for (const auto& langSym : Symbol::pp_all_languages) {
            const string& lang = langSym->name; // e.g. "EN" or "FR"
            json_stream <<
                "   {\n" <<
                "   \"LangCode\": \"" << lang << "\",\n" <<
                "   \"Link\": \"" << model_name + ".doc." + lang + ".html\"\n" <<
                "   }";
            if (++n < Symbol::pp_all_languages.size()) json_stream << ",";
            json_stream << "\n";
        }
        json_stream << "]\n}\n";
        json_stream.close();
    }

    /// styles for HTML output
    string htmlStyles;
    {
        /// styles file (hard-coded)
        string stylesPath = makeFilePath(omrootDir.c_str(), "props/styles/doc-style-portion.html");
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

    //
    // CONTENT CONTROL OPTIONS
    // 

    /// Show generated content
    const bool& do_generated_content = Symbol::option_generated_documentation;

    /// Show authored stand-alone content
    const bool& do_authored_content = Symbol::option_authored_documentation;

    /// Show content for Developer Edition
    const bool& do_developer_edition = Symbol::option_symref_developer_edition;

    /// Show unpublished symbols
    bool do_unpublished = false;
    if (do_developer_edition) {
        // allow unpublished symbols to be turned off in Developer Edition
        do_unpublished = Symbol::option_symref_unpublished_symbols;
    }
    else {
        // unpublished symbols are always excluded in User Edition
        do_unpublished = false;
    }

    /// Show authored NOTEs
    const bool& do_NOTEs = Symbol::option_symref_topic_notes;

    /// Show authored NOTEs early in indiviudal topics
    const bool& do_NOTEs_early = Symbol::option_symref_topic_notes_early;

    /// Show the Note: heading in indiviudal topics
    const bool& do_NOTE_heading = Symbol::option_symref_topic_note_heading;

    /// Show main topic
    bool do_main_topic = Symbol::option_symref_main_topic;

    /// Show topic for model symbol (provided it has a note in the language)
    bool do_model_symbol = Symbol::option_symref_model_symbol;

    /// Show the parameter hierarchy topic
    bool do_parameter_hierarchy = false;
    for (auto s : Symbol::pp_all_parameter_groups) {
        if (s->is_published() || do_unpublished) {
            // hierarchy exists
            do_parameter_hierarchy = true;
            break;
        }
    }
    if (!Symbol::option_symref_parameter_hierarchy) {
        // turn it off
        do_parameter_hierarchy = false;
    }

    /// Show the table hierarchy topic
    bool do_table_hierarchy = false;
    for (auto s : Symbol::pp_all_table_groups) {
        if (s->is_published() || do_unpublished) {
            // hierarchy exists
            do_table_hierarchy = true;
            break;
        }
    }
    if (!Symbol::option_symref_table_hierarchy) {
        // turn it off
        do_table_hierarchy = false;
    }

    /// Show the parameter major groups topic
    bool do_parameter_major_groups = do_parameter_hierarchy;
    if (!Symbol::option_symref_parameter_major_groups) {
        // turn it off
        do_parameter_major_groups = false;
    }

    /// Show the table major groups topic
    bool do_table_major_groups = do_table_hierarchy;
    if (!Symbol::option_symref_table_major_groups) {
        // turn it off
        do_table_major_groups = false;
    }

    /// Show the parameters alphabetic list topic
    bool do_parameters_alphabetic_topic = Symbol::option_symref_parameters_alphabetic;

    /// Show the tables alphabetic list topic
    bool do_tables_alphabetic_topic = Symbol::option_symref_tables_alphabetic;

    /// Show the enumerations alphabetic list topic
    bool do_enumerations_alphabetic_topic = Symbol::option_symref_enumerations_alphabetic;

    /// Show the attributes alphabetic list topic
    bool do_attributes_alphabetic_topic = Symbol::option_symref_attributes_alphabetic;

    /// Show the individual parameter topics
    bool do_parameter_topics = Symbol::option_symref_topic_parameters;

    /// Show the individual table topics
    bool do_table_topics = Symbol::option_symref_topic_tables;

    /// Show the individual enumeration topics
    bool do_enumeration_topics = Symbol::option_symref_topic_enumerations;

    /// Show the individual module topics
    bool do_module_topics = Symbol::option_symref_topic_modules;

    /// Show the individual attribute topics
    bool do_attribute_topics = Symbol::option_symref_topic_attributes;

    /// Show the individual entity set topics
    bool do_entity_set_topics = Symbol::option_symref_topic_entity_sets;
    if (Symbol::pp_all_entity_sets.size() == 0) {
        do_entity_set_topics = false;
    }

    /// Show the individual module topics for use modules
    bool do_module_use_topics = false;
    if (do_module_topics) {
        // Use modules are normally off, but they can be turned on here
        do_module_use_topics = Symbol::option_symref_topic_modules_use;
    }

    /// Show the symbols declared table in individual module topics
    bool do_module_symbols_declared = Symbol::option_symref_topic_modules_symbols_declared;

    /// Show the modules alphabetic list topic
    bool do_modules_alphabetic_topic = false;
    if (do_module_topics) {
        do_modules_alphabetic_topic = true;
    }

    /// Show the entity sets alphabetic list topic
    bool do_entity_sets_alphabetic_topic = false;
    if (do_entity_set_topics) {
        do_entity_sets_alphabetic_topic = true;
    }

    if (do_developer_edition) {
        // turn on/off flags for Developer Edition independent of options
        do_attribute_topics = true;
    }
    else {
        // turn on/off flags for User Edition independent of options
        do_module_topics = false;
        do_modules_alphabetic_topic = false;
        do_entity_set_topics = false;
        do_entity_sets_alphabetic_topic = false;
    }

    if (!do_generated_content) {
        // turn off all parts of generated content
        do_main_topic = false;
        do_model_symbol = false;

        do_parameter_hierarchy = false;
        do_table_hierarchy = false;

        do_parameter_major_groups = false;
        do_table_major_groups = false;

        do_parameters_alphabetic_topic = false;
        do_tables_alphabetic_topic = false;
        do_attributes_alphabetic_topic = false;
        do_enumerations_alphabetic_topic = false;
        do_entity_sets_alphabetic_topic = false;
        do_modules_alphabetic_topic = false;

        do_parameter_topics = false;
        do_table_topics = false;
        do_enumeration_topics = false;
        do_module_topics = false;
        do_entity_set_topics = false;
        do_attribute_topics = false;
        // TODO add more as they are implemented
    }

    // Table-like symbols - published tables merged with derived parameters published as tables
    list<Symbol*> table_like;
    {
        for (auto t : Symbol::pp_all_tables) {
            if (t->is_published() || do_unpublished) {
                table_like.push_back(t);
            }
        }
        for (auto p : Symbol::pp_all_parameters) {
            if (p->metadata_as_table && (p->is_published() || do_unpublished)) {
                table_like.push_back(p);
            }
        }
        table_like.sort([](Symbol* a, Symbol* b) { return a->name < b->name; });
    }

    // Parameter-like symbols - published parameters excluding derived parameters published as tables
    list<Symbol*> parameter_like;
    {
        for (auto p : Symbol::pp_all_parameters) {
            if (!p->metadata_as_table && (p->is_published() || do_unpublished)) {
                parameter_like.push_back(p);
            }
        }
        // no need to sort because pp_all_parameters is already sorted
    }

    // Attribute symbols - published attributes
    list<AttributeSymbol*> attributes;
    {
        for (auto e : Symbol::pp_all_entities) {
            for (auto dm : e->pp_data_members) {
                if (dm->is_attribute()) {
                    auto a = dynamic_cast<AttributeSymbol*>(dm);
                    assert(a); // logic guarantee
                    if ((a->is_published() || do_unpublished)) {
                        if (!a->is_generated || Symbol::option_symref_topic_attributes_internal) {
                            // suppress generated attributes, including derived attributes, if requested.
                            attributes.push_back(a);
                        }
                    }
                }
            }
        }
        // sort by attributes name, and if tied, by entity name
        attributes.sort([](AttributeSymbol* a, AttributeSymbol* b) { return (a->pretty_name() + "|" + a->entity->name) < (b->pretty_name() + "|" + b->entity->name); });
        // if no attributes to show, suppress alphabetic list of attributes
        if (attributes.size() == 0) {
            do_attributes_alphabetic_topic = false;
        }
    }

    // Language loop
    for (const auto& langSym : Symbol::pp_all_languages) {
        int lang_index = langSym->language_id; // 0-based
        const string& lang = langSym->name; // e.g. "EN" or "FR"

        // markdown content
        stringstream mdStream;

        // HTML fragment for topic separator
        string fragmentTopicSeparator = "\n\n<p style=\"margin-bottom:3cm; break-after:page;\"></p>\n\n";

        // no content!
        if (!do_generated_content && !do_authored_content) {
            mdStream << "**" + LTA(lang, "This page intentionally left blank.") + "**\n\n";
        }

        /// Name of the Home topic
        const string homeTopic = "Home";

        /// Anchor of the Home topic
        const string anchorHome = "Home";

        /// Is there a stand-alone authored Home topic?
        bool authoredHomeTopicPresent = Symbol::in_doc_active && (Symbol::in_doc_stems_md.count(homeTopic + "." + lang) > 0);

        /// list of all stand-alone authored topics
        list<string> authoredTopics;
        for (auto& s : Symbol::in_doc_stems_md) {
            if ((s.length() >= 5) && s.substr(0, 5) == "NOTE.") {  // Use starts_with() in C++20
                // skip NOTE topics
                continue;
            }
            if ((s.length() >= (lang.length() + 1))
                && s.substr(s.length() - lang.length() - 1, lang.length() + 1) != "." + lang) { // Use ends_with() in C++20
                // skip topics for other languages
                continue;
            }
            string topic = s.substr(0, s.length() - lang.length() - 1);
            authoredTopics.push_back(topic);
        }

        // The authored stand-alone Home topic is the first topic in the document.
        if (authoredHomeTopicPresent && do_authored_content) {
            string topic = homeTopic;
            mdStream << "<span id=\"" + topic + "\"/>\n\n"; // topic destination anchor
            mdStream << preprocess_markdown(Symbol::slurp_doc_file_md(topic + "." + lang));
            mdStream << fragmentTopicSeparator;
        }

        if (do_authored_content) {
            // all other authored stand-alone topics
            for (auto& topic : authoredTopics) {
                if (topic == homeTopic) {
                    // already done
                    continue;
                }
                mdStream << "<span id=\"" + topic + "\"></span>\n\n"; // topic destination anchor
                mdStream << preprocess_markdown(Symbol::slurp_doc_file_md(topic + "." + lang));
                mdStream << fragmentTopicSeparator;
            }
        }

        /// anchor target of each higher-level topic

        string anchorSymbolReference = "symbol-reference";
        string anchorModelSymbol = "model";
        string anchorParametersAlphabetic = "parameters-alphabetic";
        string anchorParameterHierarchy = "parameter-hierarchy";
        string anchorParameterMajorGroups = "parameter-major_groups";
        string anchorTablesAlphabetic = "tables-alphabetic";
        string anchorTableHierarchy = "table-hierarchy";
        string anchorTableMajorGroups = "table-major-groups";
        string anchorEnumerationsAlphabetic = "enumerations-alphabetic";
        string anchorModulesAlphabetic = "modules-alphabetic";
        string anchorAttributesAlphabetic = "attributes-alphabetic";
        string anchorEntitySetsAlphabetic = "entity-sets-alphabetic";

        // Fragment for back navigation
        string fragmentReturnLinks = "\n\n<a href=\"#" + anchorSymbolReference + "\">[" + LTA(lang, "Symbol Reference") + "]</a>";
        if (authoredHomeTopicPresent && do_authored_content) {
            fragmentReturnLinks += "<br><a href=\"#" + anchorHome + "\">[" + LTA(lang, "Home") + "]</a>";
        }
        fragmentReturnLinks += "\n\n";

        /// The unique 'model' symbol
        ModelSymbol* theModelSymbol = dynamic_cast<ModelSymbol*>(Symbol::find_a_symbol(typeid(ModelSymbol)));
        assert(theModelSymbol);
        /// The unique 'model' symbol has a note in the current language
        bool model_symbol_has_note = theModelSymbol->note(*langSym).length() > 0;

        // Topic: Symbol Reference Main Topic
        if (do_main_topic) {
            VersionSymbol* vs = dynamic_cast<VersionSymbol*>(Symbol::find_a_symbol(typeid(VersionSymbol)));
            assert(vs);
            string version_string = to_string(vs->major) + "." + to_string(vs->minor) + "." + to_string(vs->sub_minor) + "." + to_string(vs->sub_sub_minor);

            const size_t ymd_size = 50;
            char ymd[ymd_size];
            std::time_t time = std::time({});
            std::strftime(ymd, ymd_size, "%F %T", std::localtime(&time));

            string mainTitle = LTA(lang, "Symbol Reference");
            if (do_developer_edition) {
                mainTitle += " (" + LTA(lang, "Developer Edition") + ")";
            }

            mdStream << "<h1 id=\"" + anchorSymbolReference + "\">" + model_name + " - " + mainTitle + "</h1>\n\n";
            mdStream << "<h2>" + LTA(lang, "Version") + " " + version_string + " " + LTA(lang,"built on") + " " + ymd + "</h2>\n\n";
            mdStream << "\n\n";
            mdStream << "<h3>" + LTA(lang, "Navigation Aids") + "</h3>\n\n";
            mdStream << "|table>\n"; // maddy-specific begin table
            mdStream << LTA(lang, "Topic") + " | " + LTA(lang, "Description") + "\n"; // maddy-specific table header separator
            mdStream << "- | - | -\n"; // maddy-specific table header separator
            if (do_model_symbol && model_symbol_has_note) {
                mdStream << "[`model`](#" + anchorModelSymbol + ") | " + LTA(lang, "The unique `model` symbol") + "\n";
            }
            if (do_parameter_major_groups || do_table_major_groups) {
                mdStream << "**" + LTA(lang, "Major Groups") + "** | \n";
                if (do_parameter_major_groups) {
                    mdStream << "&nbsp;&nbsp;[" + LTA(lang, "Parameters") + "](#" + anchorParameterMajorGroups + ") | " + LTA(lang, "Major groups of input parameters") + "\n";
                }
                if (do_table_major_groups) {
                    mdStream << "&nbsp;&nbsp;[" + LTA(lang, "Tables") + "](#" + anchorTableMajorGroups + ") | " + LTA(lang, "Major groups of output tables") + "\n";
                }
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
            if (do_parameters_alphabetic_topic || do_tables_alphabetic_topic || do_enumerations_alphabetic_topic || do_attributes_alphabetic_topic || do_modules_alphabetic_topic || do_entity_sets_alphabetic_topic) {
                mdStream << "**" + LTA(lang, "Lists") + "** | \n";
                if (do_parameters_alphabetic_topic) {
                    mdStream << "&nbsp;&nbsp;[" + LTA(lang, "Parameters") + "](#" + anchorParametersAlphabetic + ") | " + LTA(lang, "Input parameters in alphabetic order") + "\n";
                }
                if (do_tables_alphabetic_topic) {
                    mdStream << "&nbsp;&nbsp;[" + LTA(lang, "Tables") + "](#" + anchorTablesAlphabetic + ") | " + LTA(lang, "Output tables in alphabetic order") + "\n";
                }
                if (do_attributes_alphabetic_topic) {
                    mdStream << "&nbsp;&nbsp;[" + LTA(lang, "Attributes") + "](#" + anchorAttributesAlphabetic + ") | " + LTA(lang, "Attributes in alphabetic order") + "\n";
                }
                if (do_enumerations_alphabetic_topic) {
                    mdStream << "&nbsp;&nbsp;[" + LTA(lang, "Enumerations") + "](#" + anchorEnumerationsAlphabetic + ") | " + LTA(lang, "Enumerations in alphabetic order") + "\n";
                }
                if (do_entity_sets_alphabetic_topic) {
                    mdStream << "&nbsp;&nbsp;[" + LTA(lang, "Entity Sets") + "](#" + anchorEntitySetsAlphabetic + ") | " + LTA(lang, "Entity sets in alphabetic order") + "\n";
                }
                if (do_modules_alphabetic_topic) {
                    mdStream << "&nbsp;&nbsp;[" + LTA(lang, "Modules") + "](#" + anchorModulesAlphabetic + ") | " + LTA(lang, "Modules in alphabetic order") + "\n";
                }
            }
            mdStream << "|<table\n"; // maddy-specific end table
            if (authoredHomeTopicPresent && do_authored_content) {
                mdStream << "\n\n<br><a href=\"#" + anchorHome + "\">[" + LTA(lang, "Home") + "]</a>";
            }
            mdStream << fragmentTopicSeparator;
        }

        // Topic: the unique symbol 'model'
        if (do_model_symbol) {
            if (model_symbol_has_note) {
                string modelLabel = theModelSymbol->label(*langSym);
                string modelNote = theModelSymbol->note(*langSym);
                mdStream
                    // symbol name
                    << "<h3 id=\"" + anchorModelSymbol + "\"><code>model</code>"
                    // symbol label
                    << "<span style=\"font-weight:lighter\"> " << modelLabel << "</span></h3>\n\n"
                    ;
                mdStream << modelNote;
                mdStream << fragmentReturnLinks;
                mdStream << fragmentTopicSeparator;
            }
        }

        // Topic: parameters in alphabetic order
        if (do_parameters_alphabetic_topic) {
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
            mdStream << fragmentReturnLinks;
            mdStream << fragmentTopicSeparator;
        }

        // Topic: parameter major groups
        if (do_parameter_major_groups) {
            mdStream << "<h3 id=\"" + anchorParameterMajorGroups + "\">" + LTA(lang, "Parameter Major Groups") + "</h3>\n\n";

            mdStream << "<p>";
            // top-level groups
            for (auto g : Symbol::pp_all_parameter_groups) {
                if (g->has_parent_group() || !(g->is_published() || do_unpublished)) {
                    // skip group if not top-level or not published.
                    continue;
                }
                mdStream << expand_group(lang_index, g, 0, 1, true, do_unpublished);
            }
            mdStream << "</p>\n\n";
            mdStream << fragmentReturnLinks;
            mdStream << fragmentTopicSeparator;
        }

        // Topic: parameter hierarchy
        if (do_parameter_hierarchy) {
            mdStream << "<h3 id=\"" + anchorParameterHierarchy + "\">" + LTA(lang, "Parameter hierarchy") + "</h3>\n\n";

            // top-level groups
            for (auto g : Symbol::pp_all_parameter_groups) {
                if (g->has_parent_group() || !(g->is_published() || do_unpublished)) {
                    // skip group if not top-level or not published.
                    continue;
                }
                mdStream << "<p>" + expand_group(lang_index, g, 0, 1000, false, do_unpublished) + "</p>\n\n";
            }
            // orphan symbols
            {
                string orphan_fragment;
                bool first_orphan = true;
                for (auto s : parameter_like) {
                    if (s->has_parent_group()) {
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
            mdStream << fragmentReturnLinks;
            mdStream << fragmentTopicSeparator;
        }

        // Topic for each parameter
        if (do_parameter_topics) {
            for (auto s : Symbol::pp_all_parameters) {
                if (!(s->is_published() || do_unpublished)) {
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
                        size_t cells = 1;
                        for (size_t ps : s->pp_shape) {
                            shapeCompact += (!isFirst ? ", " : "") + to_string(ps);
                            isFirst = false;
                            cells *= ps;
                        }
                        shapeCompact = "[ " + shapeCompact + " ] = " + to_string(cells);
                    }
                    else {
                        shapeCompact = LTA(lang, "scalar");
                    }
                    string moduleInfo = "";
                    if (do_module_topics) {
                        if (s->pp_module) {
                            moduleInfo = "[`" + s->pp_module->name + "`](#" + s->pp_module->name + ")";
                        }
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
                    if (moduleInfo.length() > 0) {
                        mdStream << "\n**" + LTA(lang, "Module") + ":** \n" + moduleInfo;
                    }
                    mdStream << "\n\n";
                }

                // bread crumb hierarchy (possibly empty)
                if (do_parameter_hierarchy) {
                    mdStream << bread_crumb_hierarchy(lang, lang_index, s);
                }

                // symbol notes if present
                if (do_NOTEs && do_NOTEs_early) {
                    // symbol note if present
                    {
                        string note_in = s->pp_notes[lang_index];
                        if (note_in.length()) {
                            if (do_NOTE_heading) {
                                mdStream << "**" + LTA(lang, "Note") + ":**\n\n";
                            }
                            mdStream << "\n\n";
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
                }

                // dimension table with links
                if (!isScalar) {
                    mdStream << "**" + LTA(lang, "Dimensions") + ":**\n\n";
                    mdStream << "|table>\n"; // maddy-specific begin table
                    mdStream << " " + LTA(lang, "External Name") + " | " + LTA(lang, "Enumeration") + " | " + LTA(lang, "Size") + " | " + LTA(lang, "Label") + " \n";
                    mdStream << "- | - | -\n"; // maddy-specific table header separator
                    for (auto& dim : s->dimension_list) {
                        string dim_external_name = maddy_symbol(dim->short_name);
                        auto e = dim->pp_enumeration;
                        assert(e); // dimensions of parameters are always enumerations
                        string dim_enumeration = e->name;
                        string dim_size = to_string(e->pp_size());
                        string dim_label = dim->pp_labels[lang_index];
                        mdStream
                            << dim_external_name << " | "
                            << "[`" + dim_enumeration + "`](#" + dim_enumeration + ")" << " | "
                            << dim_size << " | "
                            << dim_label << "\n"
                            ;
                    }
                    mdStream << "|<table\n"; // maddy-specific end table
                }

                // x-reference section
                if (do_developer_edition) {
                    do_xref(lang, lang_index, s, s->name, mdStream);
                }

                // symbol notes if present
                if (do_NOTEs && !do_NOTEs_early) {
                    // symbol note if present
                    {
                        string note_in = s->pp_notes[lang_index];
                        if (note_in.length()) {
                            if (do_NOTE_heading) {
                                mdStream << "**" + LTA(lang, "Note") + ":**\n\n";
                            }
                            mdStream << "\n\n";
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
                }

                mdStream << fragmentReturnLinks;
                mdStream << fragmentTopicSeparator;
            } // Topic for each parameter
        }

        // Topic: enumerations in alphabetic order
        if (do_enumerations_alphabetic_topic) {
            // build line with links to first symbol in alphabetic table with leading letter
            string letterLinks;
            {
                string letterPrev = "";
                for (auto& s : Symbol::pp_all_enumerations) {
                    if (!(s->is_published() || do_unpublished)) {
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
            mdStream << " " + LTA(lang, "Name") + " | " + (do_developer_edition ? (LTA(lang, "Kind") + " | ") : "") + LTA(lang, "Label") + " \n";

            mdStream << "- | - | -\n"; // maddy-specific table header separator
            {
                string letterPrev = "";
                for (auto& s : Symbol::pp_all_enumerations) {
                    if (!(s->is_published() || do_unpublished)) {
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

                    string kind;
                    if (s->is_classification()) {
                        kind = LTA(lang, "classification");
                    }
                    else if (s->is_range()) {
                        kind = LTA(lang, "range");
                    }
                    else if (s->is_partition()) {
                        kind = LTA(lang, "partition");
                    }
                    else if (s->is_bool()) {
                        kind = LTA(lang, "classification");
                    }
                    else {
                        kind = LTA(lang, "other");
                    }

                    mdStream << letterLink + " [`" << s->name << "`](#" << s->name << ") | " << (do_developer_edition? (kind + " | ") : "") << s->pp_labels[lang_index] << "  \n";
                } // end enumerations table
            }
            mdStream << "|<table\n"; // maddy-specific end table
            mdStream << fragmentReturnLinks;
            mdStream << fragmentTopicSeparator;
        } // Topic: enumerations in alphabetic order

        // Topic for each enumeration
        if (do_enumeration_topics) {
            for (auto& s : Symbol::pp_all_enumerations) {
                if (!(s->is_published() || do_unpublished)) {
                    // skip if not published
                    continue;
                }

                // topic header line
                mdStream
                    // symbol name
                    << "<h3 id=\"" << s->name << "\">" << "<code>" + s->name + "</code>"
                    // symbol label
                    << "<span style=\"font-weight:lighter\"> " << s->pp_labels[lang_index] << "</span></h3>\n\n"
                    ;

                // summary line with type, size, etc.
                {
                    string kind;
                    string values;
                    if (s->is_range()) {
                        kind = LTA(lang, "range");
                        auto r = dynamic_cast<RangeSymbol*>(s);
                        assert(r);
                        values = "{" + to_string(r->lower_bound) + ",...," + to_string(r->upper_bound) + "}";
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
                    string moduleInfo = "";
                    if (do_module_topics) {
                        if (s->pp_module) {
                            moduleInfo = "[`" + s->pp_module->name + "`](#" + s->pp_module->name + ")";
                            //moduleInfo = "Hello";
                        }
                    }
                    mdStream
                        << "**" + LTA(lang, "Kind") + ":** " << kind
                        << " **" + LTA(lang, "Size") + ":** " << to_string(s->pp_size())
                        << " **" + LTA(lang, "Values") + ":** " << values;
                    if (moduleInfo.length() > 0) {
                        mdStream << "\n**" + LTA(lang, "Module") + ":** \n" + moduleInfo;
                    }
                    mdStream << "\n\n";
                }

                // symbol note if present
                if (do_NOTEs && do_NOTEs_early) {
                    string note_in = s->pp_notes[lang_index];
                    if (note_in.length()) {
                        if (do_NOTE_heading) {
                            mdStream << "**" + LTA(lang, "Note") + ":**\n\n";
                        }
                        mdStream << "\n\n";
                        string note_out = preprocess_markdown(note_in);
                        mdStream << note_out << "\n\n";
                    }
                }

                // table of enumerators of enumeration
                {
                    if (s->is_classification()) {
                        auto c = dynamic_cast<ClassificationSymbol*>(s);
                        assert(c);
                        mdStream << "<strong>" + LTA(lang, "Enumerators of") + " <code>" + s->name + "</code>:</strong>\n\n";
                        mdStream << "|table>\n"; // maddy-specific begin table
                        mdStream << " " + LTA(lang, "External Name") + " | " + LTA(lang, "Enumerator") + " | " + LTA(lang, "Value") + " | " + LTA(lang, "Label") + " \n";
                        mdStream << "- | - | -\n"; // maddy-specific table header separator
                        for (auto enumerator : c->pp_enumerators) {
                            auto ce = dynamic_cast<ClassificationEnumeratorSymbol*>(enumerator);
                            assert(ce); // logic guarantee
                            string export_name = maddy_symbol(ce->short_name);
                            string name = ce->name;
                            string value = to_string(ce->ordinal);
                            string label = ce->pp_labels[lang_index];
                            mdStream
                                << export_name << " | "
                                << ("`" + name + "`") << " | "
                                << value << " | "
                                << label << "\n"
                                ;
                        }
                        mdStream << "|<table\n"; // maddy-specific end table
                    }
                    else if (s->is_partition()) {
                        auto p = dynamic_cast<PartitionSymbol*>(s);
                        assert(p);
                        mdStream << "<strong>" + LTA(lang, "Enumerators of") + " <code>" + s->name + "</code>:</strong>\n\n";
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

                // sections showing aggregation if this classification is an aggregation of another
                if (s->is_classification()) {
                    auto c = dynamic_cast<ClassificationSymbol*>(s);
                    assert(c);
                    /// aggregations to this classification
                    std::list<AggregationSymbol*> alist;
                    AggregationSymbol* aggr = nullptr;
                    for (auto a : Symbol::pp_all_aggregations) {
                        if (c == a->pp_to) {
                            alist.push_back(a);
                        }
                    }
                    // sort matching aggregations by name of 'from' classification
                    alist.sort([](AggregationSymbol* a, AggregationSymbol* b) { return a->pp_from->name < b->pp_from->name; });
                    for (auto& aggr : alist) { // alist is possibly empty
                        auto c_from = aggr->pp_from;
                        assert(c_from);
                        // read the aggregation and turn it into a list of rows
                        struct row {
                            int    from_ord;
                            string from_name;
                            string from_label;
                            int    to_ord;
                            string to_name;
                            string to_label;
                        };
                        std::list<row> theTable;
                        for (auto& it : aggr->pp_enumerator_map) {
                            auto from = it.first;
                            auto to = it.second;
                            row r;
                            r.from_ord   = from->ordinal;
                            r.from_name  = from->name;
                            r.from_label = from->pp_labels[lang_index];
                            r.to_ord     = to->ordinal;
                            r.to_name    = to->name;
                            r.to_label   = to->pp_labels[lang_index];
                            theTable.push_back(r);
                        }
                        theTable.sort([](row a, row b)
                            { return (a.to_ord == b.to_ord) ?
                            (a.from_ord < b.from_ord) :
                            (a.to_ord < b.to_ord);
                            }
                        );
                        mdStream <<
                            "<strong>"
                            + LTA(lang, "Aggregation from")
                            + " <a href=\"#" + c_from->name + "\"><code>" + c_from->name + "</code></a> "
                            + LTA(lang, "to")
                            + " <code>" + s->name + "</code>: "
                            "</strong>\n\n";
                        mdStream << "|table>\n"; // maddy-specific begin table
                        mdStream << 
                            LTA(lang, "Enumerator of") + "<br>&nbsp;&nbsp;" + maddy_symbol(c->name)
                            + " | " 
                            + LTA(lang, "Enumerator of") + "<br>&nbsp;&nbsp;" + maddy_link(c_from->name)
                            + " | "
                            + LTA(lang, "Enumerator label") + "<br>&nbsp;&nbsp;" + maddy_symbol(c->name)
                            + " | " 
                            + LTA(lang, "Enumerator label") + "<br>&nbsp;&nbsp;" + maddy_link(c_from->name)
                            + " \n";
                        mdStream << "- | - | -\n"; // maddy-specific table header separator
                        int prev = -1;
                        for (auto& r : theTable) {
                            /// row has same 'to' classification as previous row
                            bool repeat = (prev == r.to_ord);
                            string to = (prev == r.to_ord) ? "" : maddy_symbol(r.to_name);
                            mdStream <<
                                (repeat ? "" : maddy_symbol(r.to_name))
                                + " | "
                                + maddy_symbol(r.from_name)
                                + " | "
                                + (repeat ? "" : r.to_label)
                                + " | "
                                + r.from_label
                                + "\n";
                            prev = r.to_ord;
                        }
                        mdStream << "|<table\n"; // maddy-specific end table
                        theTable.clear();
                    }
                    alist.clear();
                }

                // table showing aggregations of this classification
                if (s->is_classification()) {
                    auto c = dynamic_cast<ClassificationSymbol*>(s);
                    assert(c);
                    /// aggregations of this classification
                    std::list<ClassificationSymbol *> clist;
                    for (auto a : Symbol::pp_all_aggregations) {
                        if (c == a->pp_from) {
                            clist.push_back(a->pp_to);
                        }
                    }
                    // sort classification list by name of 'to' classification
                    clist.sort([](ClassificationSymbol* a, ClassificationSymbol* b) { return a->name < b->name; });
                    if (clist.size() > 0) {
                        mdStream << "<strong>" + LTA(lang, "Aggregations of") + " <code>" + s->name + "</code>:</strong>\n\n";
                        mdStream << "|table>\n"; // maddy-specific begin table
                        mdStream << " " + LTA(lang, "Classification") + " | " + LTA(lang, "Label") + " \n";
                        mdStream << "- | - | -\n"; // maddy-specific table header separator
                        for (auto sym : clist) {
                            auto label = sym->pp_labels[lang_index];
                            mdStream << maddy_link(sym->name) + " | " + label + "\n";
                        }
                        mdStream << "|<table\n"; // maddy-specific end table
                        clist.clear();
                    }
                }

                // table of parameters using this enumeration as type
                {
                    set<string> parameters_used;
                    for (auto& p : Symbol::pp_all_parameters) {
                        if (!(p->is_published() || do_unpublished)) {
                            // skip unpublished parameter
                            continue;
                        }
                        // examine datatype of parameter
                        if (p->pp_datatype->is_enumeration()) {
                            if (s == p->pp_datatype) {
                                parameters_used.insert(p->name);
                            }
                        }
                    }
                    if (parameters_used.size() > 0) {
                        mdStream << "<strong>" + LTA(lang, "Parameters of type") + " <code>" + s->name + "</code>:</strong>\n\n";
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

                // table of attributes using this enumeration as type
                {
                    set<string> attributes_used;
                    for (auto a : attributes) {
                        if (a->pp_data_type->is_enumeration()) {
                            auto t = dynamic_cast<EnumerationSymbol*>(a->pp_data_type);
                            assert(t); // logic guarantee
                            if (s == t) {
                                attributes_used.insert(a->unique_name);
                            }
                        }
                    }
                    if (attributes_used.size() > 0) {
                        mdStream << "<strong>" + LTA(lang, "Attributes of type") + " <code>" + s->name + "</code>:</strong>\n\n";
                        mdStream << "|table>\n"; // maddy-specific begin table
                        mdStream << " " + LTA(lang, "Entity") + " | " + LTA(lang, "Name") + " | " + LTA(lang, "Label") + " \n";
                        mdStream << "- | - | -\n"; // maddy-specific table header separator
                        for (auto& au : attributes_used) {
                            auto sym = Symbol::get_symbol(au);
                            assert(sym); // logic guarantee
                            auto a = dynamic_cast<AttributeSymbol*>(sym);
                            assert(a); // logic guarantee
                            auto name = a->pretty_name();
                            auto entity = a->pp_entity->name;
                            auto label = escape_generated_label(a, lang_index);
                            if (a->is_derived_attribute()) {
                                label = ""; // is already in pretty_name
                            }
                            mdStream
                                << maddy_symbol(entity) + " | "
                                << maddy_link(name, a->dot_name()) << " | "
                                << label << "\n"
                                ;
                        }
                        mdStream << "|<table\n"; // maddy-specific end table
                    }
                }

                // table of parameters using this enumeration as dimension
                {
                    set<string> parameters_used;
                    for (auto& p : Symbol::pp_all_parameters) {
                        if (!(p->is_published() || do_unpublished)) {
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
                    }
                    if (parameters_used.size() > 0) {
                        mdStream << "<strong>" + LTA(lang, "Parameters with dimension") + " <code>" + s->name + "</code>:</strong>\n\n";
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

                // table of tables using this enumeration as a dimension
                {
                    set<string> tables_used;
                    for (auto t : Symbol::pp_all_tables) {
                        if (!(t->is_published() || do_unpublished)) {
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
                        mdStream << "<strong>" + LTA(lang, "Tables with dimension") + " <code>" + s->name + "</code>:</strong>\n\n";
                        mdStream << "|table>\n"; // maddy-specific begin table
                        mdStream << " " + LTA(lang, "Name") + " | " + LTA(lang, "Label") + " \n";
                        mdStream << "- | - | -\n"; // maddy-specific table header separator
                        for (auto& name : tables_used) {
                            auto sym = Symbol::get_symbol(name);
                            assert(sym); // logic guarantee
                            auto label = sym->pp_labels[lang_index];
                            mdStream
                                << maddy_link(name) << " | "
                                << label << "\n"
                                ;
                        }
                        mdStream << "|<table\n"; // maddy-specific end table
                    }
                }

                // table of entity sets using this enumeration as a dimension
                {
                    set<string> entity_sets_used;
                    for (auto es : Symbol::pp_all_entity_sets) {
                        for (auto d : es->dimension_list) {
                            // s is the current enumeration, te is an enumeration used in t
                            auto te = d->pp_enumeration;
                            if (s == te) {
                                entity_sets_used.insert(es->name);
                            }
                        }
                    }
                    if (entity_sets_used.size() > 0) {
                        mdStream << "<strong>" + LTA(lang, "Entity sets with dimension") + " <code>" + s->name + "</code>:</strong>\n\n";
                        mdStream << "|table>\n"; // maddy-specific begin table
                        mdStream << " " + LTA(lang, "Name") + " | " + LTA(lang, "Label") + " \n";
                        mdStream << "- | - | -\n"; // maddy-specific table header separator
                        for (auto& name : entity_sets_used) {
                            auto sym = Symbol::get_symbol(name);
                            assert(sym); // logic guarantee
                            auto label = sym->pp_labels[lang_index];
                            mdStream
                                << maddy_link(name) << " | "
                                << label << "\n"
                                ;
                        }
                        mdStream << "|<table\n"; // maddy-specific end table
                    }
                }

                // x-reference section
                if (do_developer_edition) {
                    do_xref(lang, lang_index, s, s->name, mdStream);
                    if (s->is_classification()) {
                        auto c = dynamic_cast<ClassificationSymbol*>(s);
                        for (auto enumerator : c->pp_enumerators) {
                            auto ce = dynamic_cast<ClassificationEnumeratorSymbol*>(enumerator);
                            assert(ce); // logic guarantee
                            string qualified_name = s->name + "::" + ce->name;
                            do_xref(lang, lang_index, ce, qualified_name, mdStream);
                        }
                    }
                }

                // symbol note if present
                if (do_NOTEs && !do_NOTEs_early) {
                    string note_in = s->pp_notes[lang_index];
                    if (note_in.length()) {
                        if (do_NOTE_heading) {
                            mdStream << "**" + LTA(lang, "Note") + ":**\n\n";
                        }
                        mdStream << "\n\n";
                        string note_out = preprocess_markdown(note_in);
                        mdStream << note_out << "\n\n";
                    }
                }

                mdStream << fragmentReturnLinks;
                mdStream << fragmentTopicSeparator;
            } // Topic for each enumeration
        }

        // Topic: list of tables in alphabetic order
        if (do_tables_alphabetic_topic) {
            // build line with links to first table in alphabetic table with leading letter
            string letterLinks;
            {
                string letterPrev = "";
                for (auto& s : table_like) {
                    string letterCurr = s->name.substr(0, 1);
                    if (letterCurr != letterPrev) {
                        // anchor looks like A-table, D-table, etc.
                        letterLinks += " [" + letterCurr + "](#" + letterCurr + "-table)";
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
                        // anchor looks like A-table, D-table, etc.
                        letterLink += "<div id=\"" + letterCurr + "-table\"/>";
                    }
                    letterPrev = letterCurr;

                    mdStream << letterLink + " [`" << s->name << "`](#" << s->name << ") | " << s->pp_labels[lang_index] << "  \n";
                } // end tables table
            }
            mdStream << "|<table\n"; // maddy-specific end table
            mdStream << fragmentReturnLinks;
            mdStream << fragmentTopicSeparator;
        } // Topic: tables in alphabetic order

        // Topic: table major groups
        if (do_table_major_groups) {
            mdStream << "<h3 id=\"" + anchorTableMajorGroups + "\">" + LTA(lang, "Table Major Groups") + "</h3>\n\n";

            mdStream << "<p>";
            // top-level groups
            for (auto g : Symbol::pp_all_table_groups) {
                if (g->has_parent_group() || !(g->is_published() || do_unpublished)) {
                    // skip group if not top-level or not published.
                    continue;
                }
                mdStream << expand_group(lang_index, g, 0, 1, true, do_unpublished);
            }
            mdStream << "</p>\n\n";
            mdStream << fragmentReturnLinks;
            mdStream << fragmentTopicSeparator;
        }

        // Topic: table hierarchy
        if (do_table_hierarchy) {
            mdStream << "<h3 id=\"" + anchorTableHierarchy + "\">" + LTA(lang, "Table hierarchy") + "</h3>\n\n";

            // top-level groups
            for (auto g : Symbol::pp_all_table_groups) {
                if (g->has_parent_group() || !(g->is_published() || do_unpublished)) {
                    // skip group if not top-level or not published.
                    continue;
                }
                mdStream << "<p>" + expand_group(lang_index, g, 0, 1000, false, do_unpublished) + "</p>\n\n";
            }
            // orphan symbols
            {
                string orphan_fragment;
                bool first_orphan = true;
                for (auto s : table_like) {
                    bool is_orphan = !s->has_parent_group();
                    if (!is_orphan) {
                        // Handle special case of a derived parameter published as a table
                        // which has a parameter group parent but no table group parent.
                        bool is_derived_parameter = dynamic_cast<ParameterSymbol*>(s);
                        if (is_derived_parameter) {
                            bool has_table_group_parent = false;
                            for (auto pg : s->pp_parent_groups) {
                                if (dynamic_cast<TableGroupSymbol*>(pg)) {
                                    has_table_group_parent = true;
                                    break;
                                }
                            }
                            if (!has_table_group_parent) {
                                is_orphan = true;
                            }
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

            mdStream << fragmentReturnLinks;
            mdStream << fragmentTopicSeparator;
        }

        if (do_table_topics) {
            // Topic for each table
            for (auto& s : Symbol::pp_all_tables) {
                if (!(s->is_published() || do_unpublished)) {
                    // skip table
                    continue;
                }

                /// is a scalar table
                bool isScalar = (s->dimension_list.size() == 0);

                /// is an entity table
                bool isEntityTable = s->is_entity_table();
                auto et = dynamic_cast<EntityTableSymbol*>(s);

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
                        if (do_developer_edition) {
                            string extraInfo;
                            if (isEntityTable) {
                                assert(et);
                                kindInfo = LTA(lang, "Entity Table");
                                if (et->is_untransformed) {
                                    extraInfo += (extraInfo.length() > 0) ? "," : "";
                                    extraInfo += LTA(lang, "untransformed");
                                }
                                extraInfo += (extraInfo.length() > 0) ? "," : "";
                                extraInfo += LTA(lang, et->kind_as_string()); // possibilities: classic, duration, snapshot
                            }
                            else {
                                kindInfo = LTA(lang, "Derived Table");
                            }
                            if (s->is_hidden) {
                                extraInfo += (extraInfo.length() > 0) ? "," : "";
                                extraInfo += LTA(lang, "hidden");
                            }
                            if (extraInfo.length() > 0) {
                                kindInfo += " (" + extraInfo + ")";
                            }
                        }
                        else {
                            kindInfo = LTA(lang, "Table");
                        }
                    }
                    string entityInfo;
                    if (isEntityTable) {
                        assert(et); // logic guarantee
                        entityInfo = maddy_symbol(et->pp_entity->name);
                    }
                    string shapeCompact;
                    if (!isScalar) {
                        bool isFirst = true;
                        size_t cells = 1;
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
                    string moduleInfo = "";
                    if (do_module_topics) {
                        if (s->pp_module) {
                            moduleInfo = "[`" + s->pp_module->name + "`](#" + s->pp_module->name + ")";
                        }
                    }
                    mdStream
                        << "**" + LTA(lang, "Kind") + ":** " + kindInfo
                        << "\n";
                    if (do_developer_edition && isEntityTable) {
                        mdStream
                            << "**" + LTA(lang, "Entity") + ":** "
                            << "\n"
                            << entityInfo
                            << "\n";
                    }
                    mdStream << " **" + LTA(lang, "Cells") + ":** " << shapeCompact
                        << "\n"
                        << " **" + LTA(lang, "Measures") + ":** " << measures;
                    if (moduleInfo.length() > 0) {
                        mdStream << "\n**" + LTA(lang, "Module") + ":** \n" + moduleInfo;
                    }
                    mdStream << "\n\n";
                }

                // bread crumb hierarchy (possibly empty)
                if (do_table_hierarchy) {
                    mdStream << bread_crumb_hierarchy(lang, lang_index, s);
                }

                // symbol note if present
                if (do_NOTEs && do_NOTEs_early) {
                    string note_in = s->pp_notes[lang_index];
                    if (note_in.length()) {
                        if (do_NOTE_heading) {
                            mdStream << "**" + LTA(lang, "Note") + ":**\n\n";
                        }
                        mdStream << "\n\n";
                        string note_out = preprocess_markdown(note_in);
                        mdStream << note_out << "\n\n";
                    }
                }

                // filter if present
                if (do_developer_edition && isEntityTable && et->filter) {
                    auto ia = et->filter;
                    mdStream << "**" + LTA(lang, "Filter:") + "** \n";
                    mdStream << maddy_link(ia->name, ia->dot_name());
                    mdStream << "\n\n";
                    // enclose declaration of associated identity attribute in c++ code block
                    mdStream << "```cpp\n";
                    // TODO: find a way to get a string containing the original model code for the identity attribute instead.
                    mdStream << ia->cxx_expression(ia->root, true);  // true means use_pretty_name
                    mdStream << "\n```\n\n";
                }

                // dimensions table with links
                if (!isScalar) {
                    mdStream << "**" + LTA(lang, "Dimensions") + ":**\n\n";
                    mdStream << "|table>\n"; // maddy-specific begin table
                    mdStream << " " + LTA(lang, "External Name");
                    mdStream << " | " + LTA(lang, "Enumeration");
                    if (do_developer_edition && isEntityTable) {
                        mdStream << " | " + LTA(lang, "Attribute");
                    }
                    mdStream << " | " + LTA(lang, "Size");
                    mdStream << " | " + LTA(lang, "Margin");
                    mdStream << " | " + LTA(lang, "Label");
                    mdStream << " \n";
                    mdStream << "- | - | -\n"; // maddy-specific table header separator
                    for (auto& dim : s->dimension_list) {
                        string dim_external_name = maddy_symbol(dim->short_name);
                        auto e = dim->pp_enumeration;
                        assert(e); // dimensions of parameters are always enumerations
                        string dim_enumeration = maddy_link(e->name);
                        string dim_size = to_string(e->pp_size());
                        string dim_label = dim->pp_labels[lang_index];
                        mdStream << dim_external_name;
                        mdStream << " | " + dim_enumeration;
                        if (do_developer_edition && isEntityTable) {
                            auto a = dim->pp_attribute;
                            assert(a); // logic guarantee
                            mdStream << " | " + maddy_link(a->pretty_name(), a->dot_name());
                        }
                        mdStream << " | " + dim_size;
                        mdStream << " | " << (dim->has_margin ? "**&nbsp;&nbsp;&nbsp;&nbsp;X**" : "");
                        mdStream << " | " + dim_label;
                        mdStream << "\n";
                    }
                    mdStream << "|<table\n"; // maddy-specific end table
                }

                // measures table
                {
                    auto mdim = s->measure_dimension;
                    assert(mdim); // logic guarantee
                    mdStream << "**" + LTA(lang, "Measures") + ":**";
                    mdStream << "\n" + mdim->pp_labels[lang_index];
                    mdStream << "\n\n";
                    mdStream << "|table>\n"; // maddy-specific begin table
                    mdStream << " " + LTA(lang, "External Name");
                    if (do_developer_edition && isEntityTable) {
                        mdStream << " | " + LTA(lang, "Expression");
                    }
                    mdStream << " | " + LTA(lang, "Label");
                    mdStream << " \n";
                    mdStream << "- | - | -\n"; // maddy-specific table header separator
                    for (auto& m : s->pp_measures) {
                        string m_external_name = maddy_symbol(m->short_name);
                        string m_label = m->pp_labels[lang_index];
                        mdStream << m_external_name;
                        if (do_developer_edition && isEntityTable) {
                            auto etms = dynamic_cast<EntityTableMeasureSymbol*>(m);
                            assert(etms); // logic guarantee
                            auto expr = etms->get_expression(etms->root, EntityTableMeasureSymbol::expression_style::table_syntax);
                            mdStream << " | ```" + expr + "```";
                        }
                        mdStream << " | " + m_label;
                        mdStream << "\n";
                    }
                    mdStream << "|<table\n"; // maddy-specific end table
                }

                // accumulators table
                if (do_developer_edition && isEntityTable) {
                    mdStream << "**" + LTA(lang, "Accumulators") + ":**";
                    mdStream << "\n\n";
                    mdStream << "|table>\n"; // maddy-specific begin table
                    mdStream << " " + LTA(lang, "Name");
                    mdStream << " | " + LTA(lang, "Statistic");
                    mdStream << " | " + LTA(lang, "Increment");
                    mdStream << " | " + LTA(lang, "Tabop");
                    mdStream << " | " + LTA(lang, "Assembled");
                    mdStream << " | " + LTA(lang, "Attribute");
                    mdStream << " | " + LTA(lang, "Attribute Label");
                    mdStream << " \n";
                    mdStream << "- | - | -\n"; // maddy-specific table header separator
                    assert(et);
                    for (auto& acc : et->pp_accumulators) {
                        string name = "acc" + std::to_string(acc->index);
                        string stat = maddy_symbol(Symbol::token_to_string(acc->statistic));
                        string incr;
                        string tabop;
                        if (acc->statistic != token::TK_unit) {
                            incr = maddy_symbol(Symbol::token_to_string(acc->increment));
                            tabop = maddy_symbol(Symbol::token_to_string(acc->tabop));
                        }
                        string assembled = maddy_symbol(acc->declaration(false, true));
                        auto attr = acc->pp_attribute;
                        string attr_name;
                        string attr_label;
                        if (attr) {
                            attr_name = maddy_link(attr->pretty_name(), attr->dot_name());
                            if (attr->is_derived_attribute()) {
                                // use empty label for derived attributes because is same as pretty_name()
                                attr_label = "";
                            }
                            else {
                                attr_label = attr->pp_labels[lang_index];
                            }
                        }
                        mdStream << maddy_symbol(name);
                        mdStream << " | " + stat;
                        mdStream << " | " + incr;
                        mdStream << " | " + tabop;
                        mdStream << " | " + assembled;
                        mdStream << " | " + attr_name;
                        mdStream << " | " + attr_label;
                        mdStream << "\n";
                    }
                    mdStream << "|<table\n"; // maddy-specific end table
                }

                // symbol note if present
                if (do_NOTEs && !do_NOTEs_early) {
                    string note_in = s->pp_notes[lang_index];
                    if (note_in.length()) {
                        if (do_NOTE_heading) {
                            mdStream << "**" + LTA(lang, "Note") + ":**\n\n";
                        }
                        mdStream << "\n\n";
                        string note_out = preprocess_markdown(note_in);
                        mdStream << note_out << "\n\n";
                    }
                }

                mdStream << fragmentReturnLinks;
                mdStream << fragmentTopicSeparator;
            } // Topic for each table
        }

        // Topic: list of attributes in alphabetic order
        if (do_attributes_alphabetic_topic) {
            // build line with links to first row in alphabetic table with leading letter
            string letterLinks;
            {
                string letterPrev = "";
                for (auto& s : attributes) {
                    string letterCurr = (s->pretty_name()).substr(0, 1);
                    if (letterCurr != letterPrev) {
                        // anchor looks like A-table, D-table, etc.
                        letterLinks += " [" + letterCurr + "](#" + letterCurr + "-attribute)";
                    }
                    letterPrev = letterCurr;
                }
            }

            mdStream << "<h3 id=\"" + anchorAttributesAlphabetic + "\">" + LTA(lang, "Attributes in alphabetic order") + "</h3>\n\n";
            mdStream << letterLinks + "\n\n";
            mdStream << "|table>\n"; // maddy-specific begin table
            mdStream << " " + LTA(lang, "Entity") + " | " + LTA(lang, "Name") + " | " + LTA(lang, "Label") + " \n";

            mdStream << "- | - | -\n"; // maddy-specific table header separator
            {
                string letterPrev = "";
                for (auto& s : attributes) {
                    string letterCurr = (s->pretty_name()).substr(0, 1);
                    string letterLink = "";
                    if (letterCurr != letterPrev) {
                        // anchor looks like A-table, D-table, etc.
                        letterLink += "<div id=\"" + letterCurr + "-attribute\"/>";
                    }
                    letterPrev = letterCurr;

                    mdStream
                        << letterLink
                        << maddy_symbol(s->pp_entity->name) + " | "
                        << maddy_link(s->pretty_name(), s->dot_name()) + " | "
                        << escape_generated_label(s, lang_index)
                        << "  \n";
                }
            }
            mdStream << "|<table\n"; // maddy-specific end table
            mdStream << fragmentReturnLinks;
            mdStream << fragmentTopicSeparator;
        } // Topic: attributes in alphabetic order

        // Topic for each attribute
        if (do_attribute_topics) {
            for (auto& s : attributes) {
                // topic header line
                // symbol name
                mdStream << "<h3 id=\"" << s->dot_name() << "\">" << " <code>" + s->pretty_name() + "</code>";
                string label = s->pp_labels[lang_index];
                if (s->is_derived_attribute() || s->is_multilink_aggregate_attribute()) {
                    // use empty label for derived attributes and multilink aggregate attributes because is same as pretty_name()
                    label = "";
                }
                if (s->is_identity_attribute() && s->is_generated) {
                    // use empty label for generated identity attributes because is same as expression shown immediately below
                    label = "";
                }
                // symbol label
                mdStream << "<span style=\"font-weight:lighter\"> " << label << "</span></h3>\n\n";
                // summary line with type, size, etc.
                {
                    string entityInfo = maddy_symbol(s->pp_entity->name);
                    //string entityInfo = s->pp_entity->name;

                    string kindInfo;
                    {
                        kindInfo += LTA(lang, "Attribute");
                        kindInfo += " (";

                        if (s->is_builtin_attribute()) {
                            kindInfo += LTA(lang, "built-in");
                        }
                        else if (s->is_link_attribute()) {
                            kindInfo += LTA(lang, "link");
                        }
                        else if (s->is_identity_attribute()) {
                            kindInfo += LTA(lang, "identity");
                        }
                        else if (s->is_derived_attribute()) {
                            kindInfo += LTA(lang, "derived");
                        }
                        else if (s->is_multilink_aggregate_attribute()) {
                            kindInfo += LTA(lang, "multilink aggregate");
                        }
                        else if (s->is_simple_attribute()) {
                            kindInfo += LTA(lang, "simple");
                        }
                        else {
                            assert(false); // should not be reached
                            kindInfo += LTA(lang, "other");
                        }
                        if (s->is_generated) {
                            kindInfo += "," + LTA(lang, "generated");
                        }
                        if (s->is_time_like) {
                            kindInfo += "," + LTA(lang, "time-like");
                        }
                        kindInfo += ")";
                    }

                    string moduleInfo = "";
                    if (do_module_topics) {
                        if (s->pp_module) {
                            moduleInfo = "[`" + s->pp_module->name + "`](#" + s->pp_module->name + ")";
                        }
                    }
                    string typeInfo;
                    {
                        if (s->pp_data_type->is_enumeration()) {
                            // hyperlink to enumeration
                            typeInfo = "[`" + s->pp_data_type->name + "`](#" + s->pp_data_type->name + ")";
                        }
                        else {
                            // just show the name of the type, e.g. "double"
                            typeInfo = s->pp_data_type->name;
                        }
                    }
                    mdStream
                        << " **" + LTA(lang, "Kind") + ":** "
                        << "\n"
                        << kindInfo
                        << " **" + LTA(lang, "Type") + ":** "
                        << "\n"
                        << typeInfo
                        << " **" + LTA(lang, "Entity") + ":** "
                        << "\n"
                        << entityInfo
                        ;
                    if (moduleInfo.length() > 0) {
                        mdStream << "\n**" + LTA(lang, "Module") + ":** \n" + moduleInfo;
                    }
                    mdStream << "\n\n";
                }

                // symbol note if present
                if (do_NOTEs && do_NOTEs_early) {
                    string note_in = s->pp_notes[lang_index];
                    if (note_in.length()) {
                        if (do_NOTE_heading) {
                            mdStream << "**" + LTA(lang, "Note") + ":**\n\n";
                        }
                        mdStream << "\n\n";
                        string note_out = preprocess_markdown(note_in);
                        mdStream << note_out << "\n\n";
                    }
                }

                // declaration section
                if (do_developer_edition) {
                    if (s->is_identity_attribute()) {
                        auto ia = dynamic_cast<IdentityAttributeSymbol*>(s);
                        assert(ia); // logic guarantee
                        mdStream << "**" + LTA(lang, "Declaration:") + "** \n";
                        mdStream << "\n";
                        // enclose declaration in c++ code block
                        mdStream << "```cpp\n";
                        // TODO: consider finding a way to get a string containing the original model code for the identity attribute,
                        // to preserve the lin breaks in highly complex identity attributes.
                        string decl = ia->pp_data_type->name + " " + ia->name + " = ";
                        decl += ia->cxx_expression(ia->root, true); // true means use_pretty_name
                        mdStream << decl;
                        mdStream << "\n```\n\n";
                    }
                    if (s->is_derived_attribute() || s->is_multilink_aggregate_attribute()) {
                        mdStream << "**" + LTA(lang, "Name:") + "** \n";
                        mdStream << maddy_symbol(s->name) + "\n";
                        mdStream << "\n\n";
                    }
                    if (s->is_maintained_attribute()) {
                        auto ma = dynamic_cast<MaintainedAttributeSymbol*>(s);
                        assert(ma); // logic guarantee
                        if (ma->pp_dependent_attributes.size()) {
                            mdStream << "<strong>" + LTA(lang, "Attributes used by") + " <code>" + ma->pretty_name() + "</code>:</strong>\n\n";
                            mdStream << "|table>\n"; // maddy-specific begin table
                            mdStream << " " + LTA(lang, "Entity") + " | " + LTA(lang, "Attribute") + " | " + LTA(lang, "Module") + " | " + LTA(lang, "Label") + " \n";
                            mdStream << "- | - | -\n"; // maddy-specific table header separator
                            for (auto& dep : ma->pp_dependent_attributes) {
                                string entity = maddy_symbol(ma->pp_entity->name);
                                string member = maddy_link(dep->pretty_name(),dep->dot_name());
                                string module = dep->pp_module ? maddy_link(dep->pp_module->name) : "";
                                string label = dep->pp_labels[lang_index];
                                mdStream
                                    << entity << " | "
                                    << member << " | "
                                    << module << " | "
                                    << label << "\n"
                                    ;
                            }
                            mdStream << "|<table\n"; // maddy-specific end table
                        }
                    }
                }

                // x-reference section
                if (do_developer_edition) {
                    do_xref(lang, lang_index, s, s->pretty_name(), mdStream);
                }

                // symbol note if present
                if (do_NOTEs && !do_NOTEs_early) {
                    string note_in = s->pp_notes[lang_index];
                    if (note_in.length()) {
                        if (do_NOTE_heading) {
                            mdStream << "**" + LTA(lang, "Note") + ":**\n\n";
                        }
                        mdStream << "\n\n";
                        string note_out = preprocess_markdown(note_in);
                        mdStream << note_out << "\n\n";
                    }
                }

                mdStream << fragmentReturnLinks;
                mdStream << fragmentTopicSeparator;
            } // Topic for each attribute
        }

        // Topic: entity sets in alphabetic order
        if (do_entity_sets_alphabetic_topic) {
            mdStream << "<h3 id=\"" + anchorEntitySetsAlphabetic + "\">" + LTA(lang, "Entity sets in alphabetic order") + "</h3>\n\n";
            mdStream << "\n\n";
            mdStream << "|table>\n"; // maddy-specific begin table
            mdStream << " " + LTA(lang, "Name") + " | " + LTA(lang, "Label") + " \n";

            mdStream << "- | - | -\n"; // maddy-specific table header separator
            {
                for (auto& s : Symbol::pp_all_entity_sets) {
                    mdStream << maddy_link(s->name);
                    mdStream << " | ";
                    mdStream << s->pp_labels[lang_index];
                    mdStream << "\n";
                }
            }
            mdStream << "|<table\n"; // maddy-specific end table
            mdStream << fragmentReturnLinks;
            mdStream << fragmentTopicSeparator;
        } // Topic: entity sets in alphabetic order

        if (do_entity_set_topics) {
            // Topic for each entity set
            for (auto& s : Symbol::pp_all_entity_sets) {

                /// is a scalar
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
                    string kindInfo = LTA(lang, "Entity Set");
                    string entityInfo = maddy_symbol(s->pp_entity->name);
                    string orderInfo;
                    if (s->pp_order_attribute) {
                        orderInfo = maddy_link(s->pp_order_attribute->name, s->pp_order_attribute->dot_name());
                    }
                    string shapeCompact;
                    if (!isScalar) {
                        bool isFirst = true;
                        size_t cells = 1;
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
                    string moduleInfo = "";
                    if (do_module_topics) {
                        if (s->pp_module) {
                            moduleInfo = "[`" + s->pp_module->name + "`](#" + s->pp_module->name + ")";
                        }
                    }
                    mdStream
                        << "**" + LTA(lang, "Kind") + ":** " + kindInfo
                        << "\n";
                    mdStream
                        << "**" + LTA(lang, "Entity") + ":** "
                        << "\n"
                        << entityInfo
                        << "\n";
                    if (orderInfo.length() > 0) {
                        mdStream
                            << "**" + LTA(lang, "Order:") + "** "
                            << "\n"
                            << orderInfo
                            << "\n";
                    }
                    mdStream << " **" + LTA(lang, "Cells") + ":** " << shapeCompact
                        << "\n";
                    if (moduleInfo.length() > 0) {
                        mdStream << "**" + LTA(lang, "Module") + ":** \n" + moduleInfo;
                    }
                    mdStream << "\n\n";
                }

                // symbol note if present
                if (do_NOTEs && do_NOTEs_early) {
                    string note_in = s->pp_notes[lang_index];
                    if (note_in.length()) {
                        if (do_NOTE_heading) {
                            mdStream << "**" + LTA(lang, "Note") + ":**\n\n";
                        }
                        mdStream << "\n\n";
                        string note_out = preprocess_markdown(note_in);
                        mdStream << note_out << "\n\n";
                    }
                }

                // filter if present
                if (s->filter) {
                    auto ia = s->filter;
                    mdStream << "**" + LTA(lang, "Filter:") + "** \n";
                    mdStream << maddy_link(ia->name, ia->dot_name());
                    mdStream << "\n\n";
                    // enclose declaration of associated identity attribute in c++ code block
                    mdStream << "```cpp\n";
                    // TODO: find a way to get a string containing the original model code for the identity attribute instead.
                    mdStream << ia->cxx_expression(ia->root, true);  // true means use_pretty_name
                    mdStream << "\n```\n\n";
                }

                // dimensions table with links
                if (!isScalar) {
                    mdStream << "**" + LTA(lang, "Dimensions") + ":**\n\n";
                    mdStream << "|table>\n"; // maddy-specific begin table
                    mdStream << " " + LTA(lang, "Enumeration");
                    mdStream << " | " + LTA(lang, "Attribute");
                    mdStream << " | " + LTA(lang, "Size");
                    mdStream << " | " + LTA(lang, "Label");
                    mdStream << " \n";
                    mdStream << "- | - | -\n"; // maddy-specific table header separator
                    for (auto& dim : s->dimension_list) {
                        auto e = dim->pp_enumeration;
                        assert(e);
                        string dim_enumeration = maddy_link(e->name);
                        string dim_size = to_string(e->pp_size());
                        string dim_label = dim->pp_labels[lang_index];
                        mdStream << " " + dim_enumeration;
                        auto a = dim->pp_attribute;
                        assert(a); // logic guarantee
                        mdStream << " | " + maddy_link(a->pretty_name(), a->dot_name());
                        mdStream << " | " + dim_size;
                        mdStream << " | " + dim_label;
                        mdStream << "\n";
                    }
                    mdStream << "|<table\n"; // maddy-specific end table
                }

                // x-reference section
                do_xref(lang, lang_index, s, s->name, mdStream);

                // symbol note if present
                if (do_NOTEs && !do_NOTEs_early) {
                    string note_in = s->pp_notes[lang_index];
                    if (note_in.length()) {
                        if (do_NOTE_heading) {
                            mdStream << "**" + LTA(lang, "Note") + ":**\n\n";
                        }
                        mdStream << "\n\n";
                        string note_out = preprocess_markdown(note_in);
                        mdStream << note_out << "\n\n";
                    }
                }

                mdStream << fragmentReturnLinks;
                mdStream << fragmentTopicSeparator;
            } // Topic for each table
        }

        // Topic: modules in alphabetic order
        if (do_modules_alphabetic_topic) {
            // build line with links to first symbol in alphabetic table with leading letter
            string letterLinks;
            {
                string letterPrev = "";
                for (auto& s : Symbol::pp_all_modules) {
                    if ((s->provenance == ModuleSymbol::module_provenance::from_dat) || (s->provenance == ModuleSymbol::module_provenance::from_use && !do_module_use_topics)) {
                        continue;
                    }
                    string letterCurr = s->name.substr(0, 1);
                    if (letterCurr != letterPrev) {
                        // anchor looks like A-module, D-module, etc.
                        letterLinks += " [" + letterCurr + "](#" + letterCurr + "-module)";
                    }
                    letterPrev = letterCurr;
                }
            }

            mdStream << "<h3 id=\"" + anchorModulesAlphabetic + "\">" + LTA(lang, "Modules in alphabetic order") + "</h3>\n\n";
            mdStream << letterLinks + "\n\n";
            mdStream << "|table>\n"; // maddy-specific begin table
            mdStream << " " + LTA(lang, "Name") + " | " + LTA(lang, "Label") + " \n";

            mdStream << "- | - | -\n"; // maddy-specific table header separator
            {
                string letterPrev = "";
                for (auto& s : Symbol::pp_all_modules) {
                    if ((s->provenance == ModuleSymbol::module_provenance::from_dat) || (s->provenance == ModuleSymbol::module_provenance::from_use && !do_module_use_topics)) {
                        continue;
                    }
                    string letterCurr = s->name.substr(0, 1);
                    string letterLink = "";
                    if (letterCurr != letterPrev) {
                        // anchor looks like A-module, D-module, etc.
                        letterLink += "<div id=\"" + letterCurr + "-module\"/>";
                    }
                    letterPrev = letterCurr;

                    mdStream << letterLink + " [`" << s->name << "`](#" << s->name << ") | " << s->pp_labels[lang_index] << "  \n";
                } // end modules table
            }
            mdStream << "|<table\n"; // maddy-specific end table
            mdStream << fragmentReturnLinks;
            mdStream << fragmentTopicSeparator;
        } // Topic: modules in alphabetic order

        if (do_module_topics) {
            for (auto& s : Symbol::pp_all_modules) {
                if ((s->provenance == ModuleSymbol::module_provenance::from_dat) || (s->provenance == ModuleSymbol::module_provenance::from_use && !do_module_use_topics)) {
                    continue;
                }

                // topic header line
                mdStream
                    // symbol name
                    << "<h3 id=\"" << s->name << "\">" << "<code>" + s->name + "</code>"
                    // symbol label
                    << "<span style=\"font-weight:lighter\"> " << s->pp_labels[lang_index] << "</span></h3>\n\n"
                    ;

                // summary line with type, size, etc.
                {
                    //mdStream
                    //    << "**" + LTA(lang, "Kind") + ":** " << kind
                    //    << " **" + LTA(lang, "Size") + ":** " << to_string(s->pp_size())
                    //    << " **" + LTA(lang, "Values") + ":** " << values;
                    //if (moduleInfo.length() > 0) {
                    //    mdStream << "\n**" + LTA(lang, "Module") + ":** \n" + moduleInfo;
                    //}
                    mdStream << "\n\n";
                }

                // symbol note if present
                if (do_NOTEs && do_NOTEs_early) {
                    string note_in = s->pp_notes[lang_index];
                    if (note_in.length()) {
                        if (do_NOTE_heading) {
                            mdStream << "**" + LTA(lang, "Note") + ":**\n\n";
                        }
                        mdStream << "\n\n";
                        string note_out = preprocess_markdown(note_in);
                        mdStream << note_out << "\n\n";
                    }
                }

                // table of symbols declared in this module
                if (do_module_symbols_declared) {
                    mdStream << "<strong>" + LTA(lang, "Symbols declared in") + " <code>" + s->name + "</code>:</strong>\n\n";
                    mdStream << "|table>\n"; // maddy-specific begin table
                    mdStream << " " + LTA(lang, "Kind") + " | " + LTA(lang, "Name") + " | " + LTA(lang, "Entity") +  + " | " + LTA(lang, "Label") + " \n";
                    mdStream << "- | - | -\n"; // maddy-specific table header separator
                    // struct for a single row of the table
                    struct row {
                        string kind;
                        string name;
                        string entity;
                        string label;
                    };
                    std::list<row> theTable;
                    for (auto d : s->pp_symbols_declared) {
                        row r;
                        r.name = "";
                        r.kind = "";
                        r.entity = "";
                        r.label = d->pp_labels[lang_index];
                        if (auto p = dynamic_cast<ParameterSymbol*>(d)) {
                            r.name = maddy_link(p->name);
                            r.kind = LTA(lang, "parameter");
                        }
                        else if (auto t = dynamic_cast<TableSymbol*>(d)) {
                            r.name = maddy_link(t->name);
                            r.kind = LTA(lang, "table");
                        }
                        else if (auto c = dynamic_cast<ClassificationSymbol*>(d)) {
                            r.name = maddy_link(c->name);
                            r.kind = LTA(lang, "enumeration - classification");
                        }
                        else if (auto rng = dynamic_cast<RangeSymbol*>(d)) {
                            r.name = maddy_link(rng->name);
                            r.kind = LTA(lang, "enumeration - range");
                        }
                        else if (auto part = dynamic_cast<PartitionSymbol*>(d)) {
                            r.name = maddy_link(part->name);
                            r.kind = LTA(lang, "enumeration - partition");
                        }
                        else if (auto ef = dynamic_cast<EntityFuncSymbol*>(d)) {
                            r.name = maddy_symbol(ef->name);
                            r.entity = maddy_symbol(ef->pp_entity->name);
                            r.kind = LTA(lang, "entity function");
                            if (auto ee = ef->associated_event) {
                                if (ef == ee->time_func_original) {
                                    r.kind = LTA(lang, "event - time function");
                                }
                                else if (ef == ee->implement_func_original) {
                                    r.kind = LTA(lang, "event - implement function");
                                }
                                else {
                                    assert(false); // internal logic error
                                }
                            }
                        }
                        else if (auto a = dynamic_cast<IdentityAttributeSymbol*>(d)) {
                            r.name = maddy_link(a->name, a->dot_name());
                            r.entity = maddy_symbol(a->pp_entity->name);
                            r.kind = LTA(lang, "attribute - identity");
                            r.label = escape_generated_label(a, lang_index);  // replace label by escaped version
                        }
                        else if (auto a = dynamic_cast<SimpleAttributeSymbol*>(d)) {
                            r.name = maddy_link(a->name, a->dot_name());
                            r.entity = maddy_symbol(a->pp_entity->name);
                            r.kind = LTA(lang, "attribute - simple");
                        }
                        else if (auto a = dynamic_cast<LinkAttributeSymbol*>(d)) {
                            r.name = maddy_link(a->name, a->dot_name());
                            r.entity = maddy_symbol(a->pp_entity->name);
                            r.kind = LTA(lang, "attribute - link");
                        }
                        else if (auto es = dynamic_cast<EntitySetSymbol*>(d)) {
                            r.name = maddy_link(es->name);
                            r.kind = LTA(lang, "entity set");
                        }
                        else if (auto gf = dynamic_cast<GlobalFuncSymbol*>(d)) {
                            r.name = maddy_symbol(d->name);
                            r.kind = LTA(lang, "global function");
                        }
                        else {
                            r.name = maddy_symbol(d->name);
                            r.kind = ""; // odds and sods
                        }
                        if (r.kind != "") {
                            // ignore odds and sods
                            theTable.push_back(r);
                        }
                    }
                    // sort the rows of theTable into the desired order, by kind and within kind by name
                    theTable.sort([](row a, row b)
                        { return (a.kind == b.kind) ?
                        (a.name < b.name) :
                        (a.kind < b.kind);
                        }
                    );
                    // display the rows
                    for (auto& r : theTable) {
                        mdStream << r.kind;
                        mdStream << " | " + r.name;
                        mdStream << " | " + r.entity;
                        mdStream << " | " + r.label + "\n";
                    }
                    // cleanup
                    theTable.clear();
                    mdStream << "|<table\n"; // maddy-specific end table
                }

                // symbol note if present
                if (do_NOTEs && !do_NOTEs_early) {
                    string note_in = s->pp_notes[lang_index];
                    if (note_in.length()) {
                        if (do_NOTE_heading) {
                            mdStream << "**" + LTA(lang, "Note") + ":**\n\n";
                        }
                        mdStream << "\n\n";
                        string note_out = preprocess_markdown(note_in);
                        mdStream << note_out << "\n\n";
                    }
                }

                mdStream << fragmentReturnLinks;
                mdStream << fragmentTopicSeparator;
            } // Topic for each module
        }

        // finished markdown creation

        // convert markdown to HTML
        {
            // maddy set-up
            std::shared_ptr<maddy::ParserConfig> config = std::make_shared<maddy::ParserConfig>();
            config->enabledParsers &= ~maddy::types::EMPHASIZED_PARSER;
            config->enabledParsers |= maddy::types::HTML_PARSER;
            config->enabledParsers |= maddy::types::LATEX_BLOCK_PARSER;
            std::shared_ptr<maddy::Parser> parser = std::make_shared<maddy::Parser>(config);

            // maddy html output
            string htmlOutput = parser->Parse(mdStream);

            /// HTML output name.  Example: RiskPaths.doc.EN.html
            string htmlName = model_name + ".doc." + lang + ".html";
            string htmlPath = makeFilePath(docOutDir.c_str(), htmlName.c_str());
            ofstream htmlStream(htmlPath, ios::out | ios::trunc | ios::binary);
            if (htmlStream.fail()) {
                theLog->logFormatted("omc : error : Unable to open '%s' for writing.", htmlPath.c_str());
                throw HelperException(LT("fatal error - no model documentation"));
            }

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
        }


    } // end language loop


} // end do_model_doc
