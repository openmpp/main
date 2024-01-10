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
* @param[in] i_source source message to tarnslate
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

/** LTA translation function: find translated string by language code and source message string. */
#define LTA(lang, sourceMessage) ((getTranslated(lang.c_str(), sourceMessage, allTransaledMsg)))

void do_model_doc(string& outDir, string& omrootDir, string& model_name, CodeGen& cg, const string& i_msgFilePath) {

    /// target folder for HTML output
    string pubDir = outDir + "../bin/io/download/";
    std::filesystem::create_directories(pubDir);

    /// mapped location of pubDir in ompp UI local web server
    string pubURL = "download/";

    /// directory of model sqlite database
    string sqliteDir = outDir + "../bin/";

    // create json file for ompp UI
    {
        string json_name = model_name + ".extra.json";
        string json_path = makeFilePath(sqliteDir.c_str(), json_name.c_str());
        ofstream out(json_path, ios::out | ios::trunc | ios::binary);
        if (out.fail()) {
            string msg = "omc : warning : Unable to open " + json_path + " for writing.";
            theLog->logMsg(msg.c_str());
        }

        // Current version of ompp UI supports a single model documentation file,
        // so for now use the HTML for the model's default language.

        /// The model's default language
        string default_lang = Symbol::pp_all_languages.front()->name;
        out << "{\n";
        // example: "DocLink": "/IDMM.doc.EN.html"
        out << "   \"DocLink\": \"" + pubURL + model_name + ".doc." + default_lang + ".html\"\n";
        out << "}\n";
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
    for (auto lang : Symbol::pp_all_languages) {
        langLst.push_back(lang->name);
    }
    allTransaledMsg = IniFileReader::loadAllMessages(i_msgFilePath.c_str(), langLst);

    // Language loop
    for (auto lang : Symbol::pp_all_languages) {
        int lid = lang->language_id;
        string langid = lang->name;

        // Example: IDMM.doc.EN.html
        string htmlName = model_name + ".doc." + langid + ".html";
        string mdName = model_name + ".doc." + langid + ".md";

        // create the markdown file in outDir (normally 'src')
        string mdPath = makeFilePath(outDir.c_str(), mdName.c_str());
        ofstream mdStream(mdPath, ios::out | ios::trunc | ios::binary);
        if (mdStream.fail()) {
            string msg = "omc : warning : Unable to open " + mdName + " for writing.";
            theLog->logMsg(msg.c_str());
        }

        ModelSymbol* theModelSymbol = dynamic_cast<ModelSymbol*>(Symbol::find_a_symbol(typeid(ModelSymbol)));
        assert(theModelSymbol);
        bool flagModelNotePresent = theModelSymbol->note(*lang).length() > 0;

        // higher-level topics
        string anchorHomePage = "home-page";
        string anchorModelIntroduction = "model-introduction";
        string anchorParametersAlphabetic = "parameters-alphabetic";
        string anchorTablesAlphabetic = "tables-alphabetic";
        string anchorEnumerationsAlphabetic = "enumerations-alphabetic";

        // frgment for topic separator
        string topicSeparator = "\n\n<p style=\"margin-bottom:3cm; break-after:page;\"></p>\n\n";

        // Topic: home page (table of contents)
        {
            VersionSymbol* vs = dynamic_cast<VersionSymbol*>(Symbol::find_a_symbol(typeid(VersionSymbol)));
            assert(vs);
            string version_string = to_string(vs->major) + "." + to_string(vs->minor) + "." + to_string(vs->sub_minor) + "." + to_string(vs->sub_sub_minor);

            const size_t ymd_size = 11;
            char ymd[ymd_size];
            std::time_t time = std::time({});
            std::strftime(ymd, ymd_size, "%F", std::localtime(&time));

            mdStream << "<h1 id=\"" + anchorHomePage + "\">" + model_name + " - " + LTA(langid, "Model Documentation") + "</h1>\n\n";
            mdStream << "<h2>Model Version " + version_string + ", built " + ymd + "</h2>\n\n";
            mdStream << "\n\n";
            mdStream << "<h3>" + LTA(langid, "Table of Contents") + "</h3>\n\n";
            mdStream << "|table>\n"; // maddy-specific begin table
            mdStream << "Topic | Description\n"; // maddy-specific table header separator
            mdStream << "- | - | -\n"; // maddy-specific table header separator
            if (flagModelNotePresent) {
                mdStream << "[Introduction](#" + anchorModelIntroduction + ") | A short overview of the model\n";
            }
            mdStream << "[Parameters](#" + anchorParametersAlphabetic + ") | Model input parameters in alphabetic order\n";
            mdStream << "[Tables](#" + anchorTablesAlphabetic + ") | Model output tables in alphabetic order\n";
            mdStream << "[Enumerations](#" + anchorEnumerationsAlphabetic + ") | Model enumerations (dimensions) in alphabetic order\n";
            mdStream << "|<table\n"; // maddy-specific end table
            mdStream << topicSeparator;
        }

        // Topic: introduction
        if (flagModelNotePresent) {
            mdStream << "<h3 id=\"" + anchorModelIntroduction + "\">Introduction to "+ model_name +"</h3>\n\n";
            mdStream << theModelSymbol->note(*lang);
            mdStream << "\n\n[[Table of Contents](#"+ anchorHomePage +")]\r\n";
            mdStream << topicSeparator;
        }

        // Topic: parameters in alphabetic order
        {
            // build line with links to first parameter in alphabetic table with leading letter
            string letterLinks;
            {
                string letterPrev = "";
                for (auto& s : Symbol::pp_all_parameters) {
                    if (!s->is_published()) {
                        // skip unpublished symbol
                        continue;
                    }
                    string letterCurr = s->name.substr(0, 1);
                    if (letterCurr != letterPrev) {
                        // anchor looks like A-param, D-param, etc.
                        letterLinks += " [" + letterCurr + "](#" + letterCurr + "-param)";
                    }
                    letterPrev = letterCurr;
                }
            }

            mdStream << "<h3 id=\"" + anchorParametersAlphabetic + "\">" + LTA(langid, "Parameters in alphabetic order") + "</h3>\n\n";
            mdStream << letterLinks + "\n\n";
            mdStream << "|table>\n"; // maddy-specific begin table
            mdStream << " Name | Label \n";

            mdStream << "- | - | -\n"; // maddy-specific table header separator
            {
                string letterPrev = "";
                for (auto& s : Symbol::pp_all_parameters) {
                    if (!s->is_published()) {
                        // skip unpublished symbol
                        continue;
                    }
                    string letterCurr = s->name.substr(0, 1);
                    string letterLink = "";
                    if (letterCurr != letterPrev) {
                        // anchor looks like A-param, D-param, etc.
                        letterLink += "<div id=\"" + letterCurr + "-param\"/>";
                    }
                    letterPrev = letterCurr;

                    mdStream << letterLink + " [`" << s->name << "`](#" << s->name << ") | " << s->pp_labels[lid] << "  \n";
                } // end parameter table
            }
            mdStream << "|<table\n"; // maddy-specific end table
            mdStream << "\n\n---\n\n"; // topic separator
            mdStream << "\n\n[[Table of Contents](#" + anchorHomePage + ")]\r\n";
            mdStream << topicSeparator;
        }

        // Topic for each published parameter
        for (auto& s : Symbol::pp_all_parameters) {
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
                << "<span style=\"font-weight:lighter\"> " << s->pp_labels[lid] << "</span></h3>\n\n"
                ;

            // summary line with type and size
            {
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
                    shapeCompact = "scalar";
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
                    << "**Kind:** Parameter"
                    << "\n"
                    << "**Type:** "
                    << "\n"
                    << typeInfo
                    << " **Size:** " << shapeCompact
                    << "\n\n";
            }

            // dimension table with links
            if (!isScalar) {
                mdStream << "**Dimensions:**\n\n";
                mdStream << "|table>\n"; // maddy-specific begin table
                mdStream << " Export Name | Enumeration | Size | Description \n";
                mdStream << "- | - | -\n"; // maddy-specific table header separator
                for (auto& dim : s->dimension_list) {
                    string dim_export_name = dim->short_name;
                    auto e = dim->pp_enumeration;
                    assert(e); // dimensions of parameters are always enumerations
                    string dim_enumeration = e->name;
                    string dim_size = to_string(e->pp_size());
                    string dim_label = dim->pp_labels[lid];
                    mdStream
                        << dim_export_name << " | "
                        << "[`" + dim_enumeration +"`](#" + dim_enumeration + ")" << " | "
                        << dim_size << " | "
                        << dim_label << "\n"
                        ;
                }
                mdStream << "|<table\n"; // maddy-specific end table
            }

            // symbol note if present
            {
                string note_in = s->pp_notes[lid];
                if (note_in.length()) {
                    mdStream << "**Note:**\n\n";
                    // Convert markdown line break (two trailing spaces) to maddy-specifc \r
                    // Maddy documentation says \r\n, but \r seems to be required.
                    string note_out = std::regex_replace(note_in, std::regex("  \n"), "\r"); // maddy-specific line break
                    mdStream << note_out << "\n\n";
                }
            }

            // Group info
            //mdStream << "\n ### Belongs to Group(s):\n\n";
            //mdStream << "|table>" << "\n\n";
            //mdStream << " Name | Label \n";
            //mdStream << "- | - | -\n"; // maddy-specific
            //for (auto& pg : s->pp_all_parameter_groups) {
            //    for (auto& pr : pg->pp_symbol_list) {
            //        int zz = strcmp(pr->unique_name.c_str(), s->unique_name.c_str());
            //        if (zz == 0) {
            //            mdStream << "  " << pg->unique_name << " | " << pg->pp_labels[lid] << "\n";
            //        }
            //    }
            //}
            //mdStream << "|<table" << "\n\n";
            //mdStream << "\n\n";

            mdStream << "\n\n";
            mdStream << "[[Parameters](#" + anchorParametersAlphabetic + ")]";
            mdStream << "[[Table of Contents](#" + anchorHomePage + ")]\r\n";
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

            mdStream << "<h3 id=\"" + anchorEnumerationsAlphabetic + "\">" + LTA(langid, "Enumerations in alphabetic order") + "</h3>\n\n";
            mdStream << letterLinks + "\n\n";
            mdStream << "|table>\n"; // maddy-specific begin table
            mdStream << " Name | Label \n";

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

                    mdStream << letterLink + " [`" << s->name << "`](#" << s->name << ") | " << s->pp_labels[lid] << "  \n";
                } // end enumerations table
            }
            mdStream << "|<table\n"; // maddy-specific end table
            mdStream << "\n\n---\n\n"; // topic separator
            mdStream << "\n\n[[Table of Contents](#" + anchorHomePage + ")]\r\n";
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
                << "<span style=\"font-weight:lighter\"> " << s->pp_labels[lid] << "</span></h3>\n\n"
                ;

            // summary line with type and size
            {
                string kind;
                string values;
                if (s->is_range()) {
                    kind = "range";
                    auto r = dynamic_cast<RangeSymbol*>(s);
                    assert(r);
                    values = "{" + to_string(r->lower_bound) + ",...," + to_string(r->upper_bound) +"}";
                }
                else if (s->is_bool()) {
                    kind = "bool";
                    values = "{0,1}";
                }
                else if (s->is_classification()) {
                    kind = "classification";
                    values = "{0,...," + to_string(s->pp_size() - 1) + "}";
                }
                else if (s->is_partition()) {
                    kind = "partition";
                    values = "{0,...," + to_string(s->pp_size() - 1) + "}";
                }
                else {
                    // not reached
                    assert(false);
                }
                mdStream
                    << "**Kind:** " << kind
                    << " **Size:** " << to_string(s->pp_size())
                    << " **Values:** " << values
                    << "\n\n";
            }

            // table of enumerators
            {
                if (s->is_classification()) {
                    auto c = dynamic_cast<ClassificationSymbol*>(s);
                    assert(c);
                    mdStream << "**Enumerators:**\n\n";
                    mdStream << "|table>\n"; // maddy-specific begin table
                    mdStream << " Export Name | Enumerator | Value | Label \n";
                    mdStream << "- | - | -\n"; // maddy-specific table header separator
                    for (auto enumerator : c->pp_enumerators) {
                        auto ce = dynamic_cast<ClassificationEnumeratorSymbol*>(enumerator);
                        assert(ce); // logic guarantee
                        string export_name = ce->short_name;
                        string name = ce->name;
                        string value = to_string(ce->ordinal);
                        string label = ce->pp_labels[lid];
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
                    mdStream << "**Enumerators:**\n\n";
                    mdStream << "|table>\n"; // maddy-specific begin table
                    mdStream << " Lower | Upper | Value | Label \n";
                    mdStream << "- | - | -\n"; // maddy-specific table header separator
                    for (auto enumerator : p->pp_enumerators) {
                        auto ce = dynamic_cast<PartitionEnumeratorSymbol*>(enumerator);
                        assert(ce); // logic guarantee
                        string lower = ce->lower_split_point;
                        string upper = ce->upper_split_point;
                        string value = to_string(ce->ordinal);
                        string label = ce->label(*lang); // odd that pp_labels differs...
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
                    mdStream << "**Parameters using enumeration:**\n\n";
                    mdStream << "|table>\n"; // maddy-specific begin table
                    mdStream << " Name | Label \n";
                    mdStream << "- | - | -\n"; // maddy-specific table header separator
                    for (auto& name : parameters_used) {
                        auto sym = Symbol::get_symbol(name);
                        assert(sym); // logic guarantee
                        auto label = sym->pp_labels[lid];
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
                    mdStream << "**Tables using enumeration:**\n\n";
                    mdStream << "|table>\n"; // maddy-specific begin table
                    mdStream << " Name | Label \n";
                    mdStream << "- | - | -\n"; // maddy-specific table header separator
                    for (auto& name : tables_used) {
                        auto sym = Symbol::get_symbol(name);
                        assert(sym); // logic guarantee
                        auto label = sym->pp_labels[lid];
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
                string note_in = s->pp_notes[lid];
                if (note_in.length()) {
                    mdStream << "**Note:**\n\n";
                    // Convert markdown line break (two trailing spaces) to maddy-specifc \r
                    // Maddy documentation says \r\n, but \r seems to be required.
                    string note_out = std::regex_replace(note_in, std::regex("  \n"), "\r"); // maddy-specific line break
                    mdStream << note_out << "\n\n";
                }
            }

            mdStream << "\n\n";
            mdStream << "[[Enumerations](#" + anchorEnumerationsAlphabetic + ")]";
            mdStream << "[[Table of Contents](#" + anchorHomePage + ")]\r\n";
            mdStream << topicSeparator;
        } // Topic for each published enumeration


        // Topic: tables in alphabetic order
        {
            // build line with links to first table in alphabetic table with leading letter
            string letterLinks;
            {
                string letterPrev = "";
                for (auto& s : Symbol::pp_all_tables) {
                    if (!s->is_published()) {
                        // skip unpublished symbol
                        continue;
                    }
                    string letterCurr = s->name.substr(0, 1);
                    if (letterCurr != letterPrev) {
                        // anchor looks like A-param, D-param, etc.
                        letterLinks += " [" + letterCurr + "](#" + letterCurr + "-param)";
                    }
                    letterPrev = letterCurr;
                }
            }

            mdStream << "<h3 id=\"" + anchorTablesAlphabetic + "\">" + LTA(langid, "Tables in alphabetic order") + "</h3>\n\n";
            mdStream << letterLinks + "\n\n";
            mdStream << "|table>\n"; // maddy-specific begin table
            mdStream << " Name | Label \n";

            mdStream << "- | - | -\n"; // maddy-specific table header separator
            {
                string letterPrev = "";
                for (auto& s : Symbol::pp_all_tables) {
                    if (!s->is_published()) {
                        // skip unpublished symbol
                        continue;
                    }
                    string letterCurr = s->name.substr(0, 1);
                    string letterLink = "";
                    if (letterCurr != letterPrev) {
                        // anchor looks like A-param, D-param, etc.
                        letterLink += "<div id=\"" + letterCurr + "-param\"/>";
                    }
                    letterPrev = letterCurr;

                    mdStream << letterLink + " [`" << s->name << "`](#" << s->name << ") | " << s->pp_labels[lid] << "  \n";
                } // end parameter table
            }
            mdStream << "|<table\n"; // maddy-specific end table
            mdStream << "\n\n---\n\n"; // topic separator
            mdStream << "\n\n[[Table of Contents](#" + anchorHomePage + ")]\r\n";
            mdStream << topicSeparator;
        } // Topic: tables in alphabetic order

        // all done
        mdStream.close();

        // convert markdown to HTML
        {
            // maddy set-up
            std::shared_ptr<maddy::ParserConfig> config = std::make_shared<maddy::ParserConfig>();
            config->enabledParsers &= ~maddy::types::EMPHASIZED_PARSER;
            config->enabledParsers |= maddy::types::HTML_PARSER;
            std::shared_ptr<maddy::Parser> parser = std::make_shared<maddy::Parser>(config);

            ofstream htmlStream;
            ofstream mdStream;
            string hdpth = makeFilePath(pubDir.c_str(), htmlName.c_str());
            htmlStream.open(hdpth, fstream::out);
            mdStream.open(mdPath, fstream::in);

            std::stringstream markdownInput;
            markdownInput << mdStream.rdbuf();
            string htmlOutput = parser->Parse(markdownInput);
            htmlStream << htmlStyles;
            htmlStream << htmlOutput;
            htmlStream.close();
            mdStream.close();
        }


    } // end language loop


} // end do_model_doc