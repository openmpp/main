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

#include "maddy/parser.h"

using namespace std;
using namespace openm;
using namespace omc;

void do_model_doc(string& outDir, string& omrootDir, string& model_name, CodeGen& cg)
{

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

    // Language loop
    for (auto lang : Symbol::pp_all_languages) {
        int lid = lang->language_id;
        string langid = lang->name;
        bool isFR = (langid == "FR");
        // In code below, FR strings contain HTML entities for French accented characters.
        // Embedding accented characters in this source code module is problematic,
        // because on WIndows 1252 codepage instead of UTF-8 might be used,
        // which can cause anomalous browser behaviour in a document with a mixture of both.
        // Unfortunately, trailing "some text"u8 on string constants is not yet (as of 2024) supported on clang, so can't use that approach yet.
        // 
        // Symbol Entity    Code
        //  é     &eacute;  &#233;
        //  É     &Eacute;  &#201;
        //  è     &egrave;  &#232
        //  à     &agrave;
        //  À     &Agrave;
        // For a complete list of HTML entities, see https://mateam.net/html-escape-characters/

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

        // Topic: home page (table of contents)
        {
            VersionSymbol* vs = dynamic_cast<VersionSymbol*>(Symbol::find_a_symbol(typeid(VersionSymbol)));
            assert(vs);
            string version_string = to_string(vs->major) + "." + to_string(vs->minor) + "." + to_string(vs->sub_minor) + "." + to_string(vs->sub_sub_minor);

            const size_t ymd_size = 11;
            char ymd[ymd_size];
            std::time_t time = std::time({});
            std::strftime(ymd, ymd_size, "%F", std::localtime(&time));

            if (!isFR) {
                mdStream << "<h1 id=\"" + anchorHomePage + "\">" + model_name + " Model Documentation</h1>\n\n";
                mdStream << "<h2>Model Version " + version_string + ", built " + ymd + "</h2>\n\n";
                mdStream << "\n\n";
                mdStream << "<h3>Table of Contents</h3>\n\n";
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
            }
            else {
                mdStream << "<h1 id=\"home-page\">Documentation du mod&egrave;le " + model_name + "</h1>\n\n";
                mdStream << "* [Param&egrave;tres dans ordre alphab&eacute;tique](#parameters-alphabetic)\n\n";
                //mdStream << "# Document de Model Parameters pour " << tomb_stone << "\n\n";
            }
            mdStream << "\n\n---\n\n"; // topic separator
        }

        // Topic: introduction
        if (flagModelNotePresent) {
            if (!isFR) {
                mdStream << "<h3 id=\"" + anchorModelIntroduction + "\">Intoduction to "+ model_name +"</h3>\n\n";
                mdStream << theModelSymbol->note(*lang);
                mdStream << "\n\n[[Table of Contents](#"+ anchorHomePage +")]\r\n";
            }
            else {
            }
        }

        // Topic: parameters in alphabetic order
        {
            // build line with links to first parameter in alphabetic table with leading letter
            string letterLinks;
            {
                string letterPrev = "";
                for (auto& s : Symbol::pp_all_parameters) {
                    if (!s->is_published()) {
                        // skip unpublished parameter
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

            if (!isFR) {
                mdStream << "<h3 id=\"" + anchorParametersAlphabetic + "\">Parameters in alphabetic order</h3>\n\n";
                mdStream << letterLinks + "\n\n";
                mdStream << "|table>\n"; // maddy-specific begin table
                mdStream << " Name | Label \n";
            }
            else {
                mdStream << "<h3 id=\"parameters-alphabetic\">Param&egrave;tres dans ordre alphab&eacute;tique</h3>" << "\n\n";
                mdStream << "|table>\n"; // maddy-specific begin table
                mdStream << " Nom | &Eacute;tiquette \n";
            }

            mdStream << "- | - | -\n"; // maddy-specific table header separator
            {
                string letterPrev = "";
                for (auto& s : Symbol::pp_all_parameters) {
                    if (!s->is_published()) {
                        // skip unpublished parameter
                        continue;
                    }
                    string letterCurr = s->name.substr(0, 1);
                    string letterLink = "";
                    if (letterCurr != letterPrev) {
                        // anchor looks like A-param, D-param, etc.
                        letterLink += "<div id=\"" + letterCurr + "-param\"/>";
                    }
                    letterPrev = letterCurr;

                    mdStream << letterLink + " [" << s->name << "](#" << s->name << ") | " << s->pp_labels[lid] << "  \n";
                } // end parameter table
            }
            mdStream << "|<table\n"; // maddy-specific end table
            mdStream << "\n\n---\n\n"; // topic separator
            mdStream << "\n\n[[Table of Contents](#" + anchorHomePage + ")]\r\n";
        }

        // Topic for each published parameter
        for (auto& s : Symbol::pp_all_parameters) {
            if (!s->is_published()) {
                // skip parameter if not published
                continue;
            }

            /// is a scalar parameter
            bool isScalar = (s->pp_shape.size() == 0);

            // header line
            mdStream
                // symbol name
                << "<h3 id=\"" << s->name << "\">" << s->name
                // symbol label
                << "<br><span style=\"font-weight:lighter\">&nbsp;&nbsp;&nbsp;&nbsp;" << s->pp_labels[lid] << "</span></h3>\n\n";

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
                mdStream
                    << "**Type:** " << s->pp_datatype->name
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
                        << "[" + dim_enumeration +"](#" + dim_enumeration + ")" << " | "
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
                    // Convert markdown line break (two trailing spaces) to maddy-specifc \r
                    // Maddy documentation says \r\n, but \r seems to be required.
                    string note_out = std::regex_replace(note_in, std::regex("  \n"), "\r"); // maddy-specific line break
                    mdStream << note_out << "\n\n";
                }
            }

            // Group info
            mdStream << "\n ### Belongs to Group(s):\n\n";
            mdStream << "|table>" << "\n\n";
            mdStream << " Name | Label \n";
            mdStream << "- | - | -\n"; // maddy-specific

            for (auto& pg : s->pp_all_parameter_groups) {
                for (auto& pr : pg->pp_symbol_list) {
                    int zz = strcmp(pr->unique_name.c_str(), s->unique_name.c_str());
                    if (zz == 0) {
                        mdStream << "  " << pg->unique_name << " | " << pg->pp_labels[lid] << "\n";
                    }
                }
            }

            mdStream << "|<table" << "\n\n";
            mdStream << "\n\n";
            mdStream << "\n\n---\n\n"; // topic separator
            mdStream << "\n\n[[Table of Contents](#" + anchorHomePage + ")]\r\n";
        } // Topic for each published parameter

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