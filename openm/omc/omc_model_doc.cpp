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

void do_model_doc(string& outDir, string& model_name, CodeGen& cg)
{
    // maddy set-up
    std::stringstream markdownInput("");
    std::shared_ptr<maddy::ParserConfig> config = std::make_shared<maddy::ParserConfig>();
    config->enabledParsers &= ~maddy::types::EMPHASIZED_PARSER;
    config->enabledParsers |= maddy::types::HTML_PARSER;

    std::shared_ptr<maddy::Parser> parser = std::make_shared<maddy::Parser>(config);
    std::string htmlOutput = parser->Parse(markdownInput);

    string ModelDocs_md_name = "TOC.md";
    string flpth = makeFilePath(outDir.c_str(), ModelDocs_md_name.c_str());
    if (!std::filesystem::exists(outDir)) {
        std::filesystem::create_directory(outDir);
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

        // create lang dir here EN & FR
        ModelDocs_md_name = "TOC_" + langid + ".md";

        string ldir = langid + string("/");

        //create lang sub-dir
        string flpth = makeFilePath(outDir.c_str(), ldir.c_str());
        if (!std::filesystem::exists(flpth)) {
            std::filesystem::create_directory(flpth);
        }

        // create the markdown file
        string ldpth = makeFilePath(flpth.c_str(), ModelDocs_md_name.c_str());
        ofstream rpt(ldpth, ios::out | ios::trunc | ios::binary);
        if (rpt.fail()) {
            string msg = "omc : warning : Unable to open " + ModelDocs_md_name + " for writing.";
            theLog->logMsg(msg.c_str());
        }


        if (lid == 0) {
            rpt << LT("# Model Parameters Documents for ") << tomb_stone << "\n";
        }
        else if (lid == 1) {
            rpt << LT("# Document de Model Parameters pour ") << tomb_stone << "\n";
        }

        rpt << "-----" << "\n"; // seperator

        if (lid == 0) {
            rpt << "## Parameters" << "\n\n";
            rpt << "| Name | Label |\n";
        }
        else if (lid == 1) {
            rpt << "## Parameters" << "\n\n";
            rpt << "| Name | Label |\n";
        }


        rpt << "| --- | --- |\n";

        for (auto& s : Symbol::pp_all_parameters) {
            if (!s->is_published()) {
                // skip parameter groups which are not published
                continue;
            }
            //rpt << "| " << s->name << "  |  " << s->pp_labels[lid] << " |\n";
            rpt << "| [" << s->name << "](#" << s->name << ") | " << s->pp_labels[lid] << " | \n";


        } // end parameter table
        rpt << "\n\n";
        rpt << "-----" << "\n"; // seperator
        for (auto& s : Symbol::pp_all_parameters) {
            if (!s->is_published()) {
                // skip parameter groups which are not published
                continue;
            }
            rpt << "# " << s->name << "\n";
            rpt << "  - Label: " << s->pp_labels[lid] << "\n";
            rpt << "  - Note: " << s->pp_notes[lid] << "\n";
            rpt << "  - Type: " << s->pp_datatype->name << "\n";
            rpt << "\n\n";
        }




        rpt.close();

    } // end language loop


} // end do_model_doc