/**
* @file    omc_model_doc.cpp
*
* Function to create Model doc
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

#include "maddy/parser.h"

using namespace std;
using namespace openm;
using namespace omc;

void do_model_doc(string& pubDir, string& outDir, string& model_name, CodeGen& cg)
{
    // maddy set-up
    std::stringstream markdownInput("");
    std::shared_ptr<maddy::ParserConfig> config = std::make_shared<maddy::ParserConfig>();
    config->enabledParsers &= ~maddy::types::EMPHASIZED_PARSER;
    config->enabledParsers |= maddy::types::HTML_PARSER;

    std::shared_ptr<maddy::Parser> parser = std::make_shared<maddy::Parser>(config);
    //std::string htmlOutput = parser->Parse(markdownInput);
    
    string ModelDocs_md_name = "TOC.md";
    string ModelDocs_html_name = "TOC.html";

    std::string htmlOutput;

    string flpth = makeFilePath(pubDir.c_str(), ModelDocs_md_name.c_str());
    if (!std::filesystem::exists(pubDir)) {
        std::filesystem::create_directory(pubDir);
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

        // Example: IDMM.doc.EN.html
        ModelDocs_html_name = model_name + ".doc." + langid + ".html";
        ModelDocs_md_name = model_name + ".doc." + langid + ".md";

        // create the markdown file in outDir (normally 'src')
        string ldpth = makeFilePath(outDir.c_str(), ModelDocs_md_name.c_str());
        ofstream rpt(ldpth, ios::out | ios::trunc | ios::binary);
        if (rpt.fail()) {
            string msg = "omc : warning : Unable to open " + ModelDocs_md_name + " for writing.";
            theLog->logMsg(msg.c_str());
        }


        if (lid == 0) {
            rpt << LT("# Model Parameters Documents for ") << tomb_stone << "\n\n";
        }
        else if (lid == 1) {
            rpt << LT("# Document de Model Parameters pour ") << tomb_stone << "\n\n";
        }

        rpt << "---" << "\n\n"; // seperator

        if (lid == 0) {
            rpt << "## Parameters" << "\n\n";
            rpt << "|table>" << "\n\n";
            rpt << " Name | Label \n";
        }
        else if (lid == 1) {
            rpt << "## Parameters" << "\n\n";
            rpt << "|table>" << "\n\n";
            rpt << " Name | Label \n";
        }


        rpt << " --- | --- \n";

        for (auto& s : Symbol::pp_all_parameters) {
            if (!s->is_published()) {
                // skip parameter groups which are not published
                continue;
            }
            //rpt << "| " << s->name << "  |  " << s->pp_labels[lid] << " |\n";
            rpt << " [" << s->name << "](#" << s->name << ") | " << s->pp_labels[lid] << "  \n";


        } // end parameter table
        rpt << "|<table" << "\n\n";
        rpt << "\n\n";
        rpt << "---" << "\n\n"; // seperator

        for (auto& s : Symbol::pp_all_parameters) {
            if (!s->is_published()) {
                // skip parameter groups which are not published
                continue;
            }
            //rpt << "## " << s->name << "\n\n";
            rpt << "<h2 id=\"" << s->name << "\">" << s->name << "</h2>\n\n";
            rpt << "  - Label: " << s->pp_labels[lid] << "\n\n";
            rpt << "  - Note: " << s->pp_notes[lid] << "\n\n";
            rpt << "  - Type: " << s->pp_datatype->name << "\n\n";

            // Dimension and shape
            for (auto& dl : s->dimension_list) {
                //rpt << "    - " << dl->short_name << "\n";
                rpt << "    * " << dl->unique_name << "\n\n";
            }
            rpt << "\n    * Size[ ";
            int tt = 0;
            for (int ps : s->pp_shape) {
                rpt << ps << " ";
                tt = tt + ps;
            }
            rpt << " ] total:" << tt << " \n\n";

            tt = 0;

            // Dimension and shape another way
            rpt << "|table>" << "\n\n";
            rpt << " Name | Label | Size \n";
            rpt << " --- | --- | --- \n";

            auto dli = s->dimension_list.begin();
            auto psi = s->pp_shape.begin();

            while ( dli != s->dimension_list.end() ) {

                int xxx = 0;

                rpt << (*dli)->short_name << " | " << (*dli)->pp_labels[lid] << " | " << (*psi) << "\n";
                tt = tt + (*psi);

                dli = std::next(dli, 1);
                psi = std::next(psi, 1);
            } // end while

            rpt << " Total | | " << tt << "\n";



            rpt << "|<table" << "\n\n";
            rpt << "\n\n";

            // Group info
            rpt << "\n ### Belongs to Group(s):\n\n";
            rpt << "|table>" << "\n\n";
            rpt << " Name | Label \n";
            rpt << " --- | --- \n";

            for (auto& pg : s->pp_all_parameter_groups) {
                for (auto& pr : pg->pp_symbol_list) {
                    int zz = strcmp(pr->unique_name.c_str(), s->unique_name.c_str());
                    if (zz == 0) {
                        rpt << "  " << pg->unique_name << " | " << pg->pp_labels[lid] << "\n";
                    }
                }

            }

            rpt << "|<table" << "\n\n";
            rpt << "\n\n";
        } // end all_parameters

        //markdownInput << rpt.rdbuf();
        //htmlOutput = parser->Parse(markdownInput);


        rpt.close();

        //ldpth;
        //std::string htmlOutput = parser->Parse(markdownInput);


        ofstream filePtr, filePtr2;
        string hdpth = makeFilePath(pubDir.c_str(), ModelDocs_html_name.c_str());
        filePtr.open(hdpth, fstream::out);
        filePtr2.open(ldpth, fstream::in);

        markdownInput << filePtr2.rdbuf();
        htmlOutput = parser->Parse(markdownInput);
        filePtr << htmlOutput.c_str();


        filePtr.close();
        filePtr2.close();
        //rpt.close();

    } // end language loop


} // end do_model_doc