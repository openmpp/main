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

    // Language loop
    for (auto lang : Symbol::pp_all_languages) {
        int lid = lang->language_id;
        string langid = lang->name;
        bool isFR = (langid == "FR");
        // In code below, FR strings contain HTML entities for French accented characters.
        // Embedding accented characters in this source code module is problematic,
        // because 1252 codepage instead of UTF might be used on Windows,
        // which can cause anomalous browser behaviour in a document with a mixture of both.
        // Unfortunately, trailing u8 on string constants is not yet (as of 2024) supported on clang, so can't use that approach yet.
        // 
        // Symbol Entity    Code
        //  é     &eacute;  &#233;
        //  É     &Eacute;  &#201;
        //  è     &egrave;  &#232
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

        // Topic: home page (table of contents)
        {
            if (!isFR) {
                mdStream << "<h1 id=\"home-page\">" + model_name + " Model Documentation</h1>\n\n";
                mdStream << "* [Parameters in alphabetic order](#parameters-alphabetic)\n\n";
            }
            else {
                mdStream << "<h1 id=\"home-page\">Documentation du mod&egrave;le " + model_name + "</h1>\n\n";
                mdStream << "* [Param&egrave;tres dans ordre alphab&eacute;tique](#parameters-alphabetic)\n\n";
                //mdStream << "# Document de Model Parameters pour " << tomb_stone << "\n\n";
            }
            mdStream << "\n\n---\n\n"; // topic separator
        }

        // Topic: parameters in alphabetic order
        {
            if (!isFR) {
                mdStream << "<h3 id=\"parameters-alphabetic\">Parameters in alphabetic order</h3>\n\n";
                mdStream << "|table>\n"; // maddy-specific begin table
                mdStream << " Name | Label \n";
            }
            else {
                mdStream << "<h3 id=\"parameters-alphabetic\">Param&egrave;tres dans ordre alphab&eacute;tique</h3>" << "\n\n";
                mdStream << "|table>\n"; // maddy-specific end table
                mdStream << " Nom | &Eacute;tiquette \n";
            }

            mdStream << "- | - | -\n"; // maddy-specific table header separator

            for (auto& s : Symbol::pp_all_parameters) {
                if (!s->is_published()) {
                    // skip unpublished parameter
                    continue;
                }
                //mdStream << "| " << s->name << "  |  " << s->pp_labels[lid] << " |\n";
                mdStream << " [" << s->name << "](#" << s->name << ") | " << s->pp_labels[lid] << "  \n";


            } // end parameter table
            mdStream << "|<table\n"; // maddy-specific end table
            mdStream << "\n\n";
            mdStream << "---" << "\n\n"; // topic separator
        }

        // Topic for each published parameter
        for (auto& s : Symbol::pp_all_parameters) {
            if (!s->is_published()) {
                // skip parameter if not published
                continue;
            }
            //mdStream << "## " << s->name << "\n\n";
            mdStream << "<h3 id=\"" << s->name << "\">" << s->name << "</h3>\n\n";
            mdStream << "  - Label: " << s->pp_labels[lid] << "\n\n";
            //mdStream << "  - Note: " << s->pp_notes[lid] << "\n\n";
            string note_in = s->pp_notes[lid];
            if (note_in.length()) {
                // Convert markdown line break (two trailing spaces) to maddy-specifc \r
                // Maddy documentation says \r\n, but \r seems to be required.
                string note_out = std::regex_replace(note_in, std::regex("  \n"), "\r"); // maddy-specific line break
                mdStream << note_out << "\n\n";
            }
            mdStream << "  - Type: " << s->pp_datatype->name << "\n\n";

            // Dimension and shape
            for (auto& dl : s->dimension_list) {
                //mdStream << "    - " << dl->short_name << "\n";
                mdStream << "    * " << dl->unique_name << "\n\n";
            }
            mdStream << "\n    * Size[ ";
            int tt = 0;
            for (int ps : s->pp_shape) {
                mdStream << ps << " ";
                tt = tt + ps;
            }
            mdStream << " ] total:" << tt << " \n\n";

            tt = 0;

            // Dimension and shape another way
            mdStream << "|table>" << "\n\n";
            mdStream << " Name | Label | Size \n";
            mdStream << "- | - | -\n"; // maddy-specific


            auto dli = s->dimension_list.begin();
            auto psi = s->pp_shape.begin();

            while ( dli != s->dimension_list.end() ) {

                int xxx = 0;

                mdStream << (*dli)->short_name << " | " << (*dli)->pp_labels[lid] << " | " << (*psi) << "\n";
                tt = tt + (*psi);

                dli = std::next(dli, 1);
                psi = std::next(psi, 1);
            } // end while

            mdStream << " Total | | " << tt << "\n";



            mdStream << "|<table" << "\n\n";
            mdStream << "\n\n";

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
            mdStream << "---" << "\n\n"; // topic separator
        } // Topic for each published parameter

        mdStream.close();

        {
            // maddy set-up
            std::stringstream markdownInput("");
            std::shared_ptr<maddy::ParserConfig> config = std::make_shared<maddy::ParserConfig>();
            config->enabledParsers &= ~maddy::types::EMPHASIZED_PARSER;
            config->enabledParsers |= maddy::types::HTML_PARSER;
            std::shared_ptr<maddy::Parser> parser = std::make_shared<maddy::Parser>(config);

            ofstream htmlStream;
            ofstream mdStream;
            string hdpth = makeFilePath(pubDir.c_str(), htmlName.c_str());
            htmlStream.open(hdpth, fstream::out);
            mdStream.open(mdPath, fstream::in);

            markdownInput << mdStream.rdbuf();
            string htmlOutput = parser->Parse(markdownInput);
            htmlStream << htmlStyles;
            htmlStream << htmlOutput;
            htmlStream.close();
            mdStream.close();
        }


    } // end language loop


} // end do_model_doc