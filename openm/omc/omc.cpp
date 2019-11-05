/**
* @file    omc.cpp
* OpenM++ compiler (omc): main entry point
*  
* @mainpage OpenM++ compiler (omc)
* 
* The OpenM++ compiler produces c++ (.cpp and .h) files and SQL files (.sql) from model source files (.ompp, .mpp) and parameter value files (.dat, .odat). \n
* 
*
* The following command line arguments are supported by omc:
* * -Omc.ModelName      name/of/model/executable, e.g. RiskPaths
* * -Omc.ScenarioName   name/of/base/scenario, e.g. Base
* * -Omc.InputDir       input/dir/to/find/source/files
* * -Omc.OutputDir      output/dir/to/place/compiled/cpp_and_h_and_sql/files
* * -Omc.UseDir         use/dir/with/ompp/files
* * -Omc.ParamDir       input/dir/to/find/parameter/files/for/scenario
* * -Omc.FixedDir       input/dir/to/find/fixed/parameter/files/
* * -Omc.CodePage       code page for converting source files, e.g. windows-1252
* * -Omc.MessageLanguage    language to display output messages, default: user environment settings
* * -Omc.MessageFnc     localized message functions, default: LT,logMsg,logFormatted,WriteLogEntry,WarningMsg,ModelExit
* * -Omc.NoLineDirectives   suppress #line directives in generated cpp files
* * -OpenM.IniFile      path/to/optional/omc.ini
* * -Omc.TraceScanning  detailed tracing from scanner
* * -Omc.TraceParsing   detailed tracing from parser
* * -Omc.SqlDir         sql/script/dir to create SQLite database
* * -Omc.SqlPublishTo   create sql scripts to publish in SQLite,MySQL,PostgreSQL,MSSQL,Oracle,DB2, default: SQLite
* 
* Short form of command line arguments:
* * -m short form of -Omc.ModelName
* * -s short form of -Omc.ScenarioName
* * -i short form of -Omc.InputDir
* * -o short form of -Omc.OutputDir
* * -u short form of -Omc.UseDir
* * -p short form of -Omc.ParamDir
* * -f short form of -Omc.FixedDir
* * -ini short form of -OpenM.IniFile
* 
* Also common OpenM log options supported: 
*   OpenM.LogToConsole, OpenM.LogToFile, OpenM.LogFilePath, 
*   OpenM.LogToStampedFile, OpenM.LogUseTimeStamp, OpenM.LogUsePidStamp, OpenM.LogSql. 
* Please see OpenM++ wiki for more information about log options. \n
* 
* Parameters can be specified on command line or through ini-file. 
* Command line parameters take precedence.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <iostream>
#include <fstream>
#include "Symbol.h"
#include "LanguageSymbol.h"
#include "ParameterSymbol.h"
#include "ScenarioSymbol.h"
#include "Driver.h"
#include "ParseContext.h"
#include "CodeGen.h"
#include "libopenm/common/argReader.h"
#include "libopenm/common/omFile.h"
#include "libopenm/db/modelBuilder.h"
#include "omc_file.h"

using namespace std;
using namespace openm;
using namespace omc;

namespace openm
{
    /** keys for omc options */
    struct OmcArgKey
    {
        /** omc model name */
        static const char * modelName;

        /** omc scenario name */
        static const char * scenarioName;

        /** omc input directory with openM++ source files */
        static const char * inputDir;

        /** omc output directory for compiled files */
        static const char * outputDir;

        /** omc use directory to resolve 'use' statements */
        static const char * useDir;

        /** omc input directory with OpenM++ scenario parameter files */
        static const char * paramDir;

        /** omc input directory with OpenM++ fixed parameter files */
        static const char * fixedDir;

        /**  omc code page: input files encoding name */
        static const char * codePage;

        /** omc suppress #line directives in generated cpp files */
        static const char * noLineDirectives;

        /** omc generate detailed output from parser */
        static const char * traceParsing;

        /** omc generate detailed output from scanner */
        static const char * traceScanning;

        /** omc input directory with sql script to create SQLite model database */
        static const char * sqlDir;

        /** omc list of db-provider names to create sql scripts */
        static const char * dbProviderNames;

        /** language to display output messages */
        static const char * messageLang;

        /** list of functions which produce localized messages */
        static const char * messageFnc;
    };

    /** keys for omc options (short form) */
    struct OmcShortKey
    {
        /** short name for ini file name: -ini fileName.ini */
        static const char * iniFile;

        /** short name for omc model name */
        static const char * modelName;

        /** short name for omc scenario name */
        static const char * scenarioName;

        /** short name for omc input directory  */
        static const char * inputDir;

        /** short name for omc output directory */
        static const char * outputDir;

        /** short name for omc use directory */
        static const char * useDir;

        /** short name for omc param directory */
        static const char * paramDir;

        /** short name for omc fixed directory */
        static const char * fixedDir;
    };

    /** omc model name */
    const char * OmcArgKey::modelName = "Omc.ModelName";

    /** omc model name */
    const char * OmcArgKey::scenarioName = "Omc.ScenarioName";

    /** omc input directory with openM++ source files */
    const char * OmcArgKey::inputDir = "Omc.InputDir";

    /** omc output directory for compiled files */
    const char * OmcArgKey::outputDir = "Omc.OutputDir";

    /** omc use directory to resolve 'use' statements */
    const char * OmcArgKey::useDir = "Omc.UseDir";

    /** omc input directory for OpenM++ parameter files */
    const char * OmcArgKey::paramDir = "Omc.ParamDir";

    /** omc input directory for OpenM++ fixed parameter files */
    const char * OmcArgKey::fixedDir = "Omc.FixedDir";

    /** omc code page: input files encoding name */
    const char * OmcArgKey::codePage = "Omc.CodePage";

    /** omc no #line directives option */
    const char * OmcArgKey::noLineDirectives = "Omc.NoLineDirectives";

    /** omc trace parsing option */
    const char * OmcArgKey::traceParsing = "Omc.TraceParsing";

    /** omc trace scanning option */
    const char * OmcArgKey::traceScanning = "Omc.TraceScanning";

    /** omc input directory with sql script to create SQLite model database */
    const char * OmcArgKey::sqlDir = "Omc.SqlDir";

    /** omc list of db-provider names to create sql scripts */
    const char * OmcArgKey::dbProviderNames = "Omc.SqlPublishTo";

    /** language to display output messages */
    const char * OmcArgKey::messageLang = "Omc.MessageLanguage";

    /** list of functions which produce localized messages */
    const char * OmcArgKey::messageFnc = "Omc.MessageFnc";

    /** short name for ini file name: -ini fileName.ini */
    const char * OmcShortKey::iniFile = "ini";

    /** short name for omc model name  */
    const char * OmcShortKey::modelName = "m";

    /** short name for omc scenario name  */
    const char * OmcShortKey::scenarioName = "s";

    /** short name for omc input directory  */
    const char * OmcShortKey::inputDir = "i";

    /** short name for omc output directory */
    const char * OmcShortKey::outputDir = "o";

    /** short name for omc use directory */
    const char * OmcShortKey::useDir = "u";

    /** short name for omc parm directory */
    const char * OmcShortKey::paramDir = "p";

    /** short name for omc fixed directory */
    const char * OmcShortKey::fixedDir = "f";

    /** array of model run option keys. */
    static const char * runArgKeyArr[] = {
        OmcArgKey::modelName,
        OmcArgKey::scenarioName,
        OmcArgKey::inputDir,
        OmcArgKey::outputDir,
        OmcArgKey::useDir,
        OmcArgKey::paramDir,
        OmcArgKey::fixedDir,
        OmcArgKey::codePage,
        OmcArgKey::noLineDirectives,
        OmcArgKey::traceParsing,
        OmcArgKey::traceScanning,
        OmcArgKey::sqlDir,
        OmcArgKey::dbProviderNames,
        OmcArgKey::messageLang,
        OmcArgKey::messageFnc,
        ArgKey::iniFile,
        ArgKey::logToConsole,
        ArgKey::logToFile,
        ArgKey::logFilePath,
        ArgKey::logToStamped,
        ArgKey::logUseTs,
        ArgKey::logUsePid,
        ArgKey::logNoMsgTime,
        ArgKey::logSql
    };
    static const size_t runArgKeySize = sizeof(runArgKeyArr) / sizeof(const char *);

    /** array of short and full option names, used to find full option name by short. */
    static const pair<const char *, const char *> shortPairArr[] = 
    {
        make_pair(OmcShortKey::iniFile, ArgKey::iniFile),
        make_pair(OmcShortKey::modelName, OmcArgKey::modelName),
        make_pair(OmcShortKey::scenarioName, OmcArgKey::scenarioName),
        make_pair(OmcShortKey::inputDir, OmcArgKey::inputDir),
        make_pair(OmcShortKey::outputDir, OmcArgKey::outputDir),
        make_pair(OmcShortKey::useDir, OmcArgKey::useDir),
        make_pair(OmcShortKey::paramDir, OmcArgKey::paramDir),
        make_pair(OmcShortKey::fixedDir, OmcArgKey::fixedDir),
    };
    static const size_t shortPairSize = sizeof(shortPairArr) / sizeof(const pair<const char *, const char *>);
}

// Parse a list of files
static void parseFiles(list<string> & files, const list<string>::iterator start_it, ParseContext & pc, ofstream *markup_stream);


int main(int argc, char * argv[])
{
    try {
        // get omc run options from command line and ini-file
        ArgReader argStore(argc, argv, runArgKeySize, runArgKeyArr, shortPairSize, shortPairArr);

        // adjust log file(s) with actual log settings specified on command line or at ini-file
        theLog->init(
            argStore.boolOption(ArgKey::logToConsole) || !argStore.isOptionExist(ArgKey::logToConsole),
            argStore.strOption(ArgKey::logFilePath).c_str(),
            argStore.boolOption(ArgKey::logToFile),
            argStore.boolOption(ArgKey::logUseTs),
            argStore.boolOption(ArgKey::logUsePid),
            argStore.boolOption(ArgKey::logNoMsgTime) || !argStore.isOptionExist(ArgKey::logNoMsgTime),
            argStore.boolOption(ArgKey::logSql)
            );

        // read language specific messages from path/to/exe/omc.message.ini
        IniFileReader::loadMessages(
            makeFilePath(baseDirOf((argc > 0 ? argv[0] : "")).c_str(), "omc.message.ini").c_str(),
            argStore.strOption(OmcArgKey::messageLang)
        );
        theLog->logMsg("Start omc");

        // get model name
        string model_name = argStore.strOption(OmcArgKey::modelName);
        if (model_name.empty()) {
            model_name = "Model";
            theLog->logMsg("Model name not specified - using default name 'Model'. Use -m option to specify model name.");
        }

        // get scenario name
        string scenario_name = argStore.strOption(OmcArgKey::scenarioName);
        if (scenario_name.empty()) {
            scenario_name = "Default";
            theLog->logMsg("Scenario name not specified - using default name 'Default'. Use -s option to specify scenario name.");
        }

        // get list of source file names in specified directory or current directory by default
        string inpDir = argStore.strOption(OmcArgKey::inputDir);
        // normalize path separators (to avoid escaping \ in #line directives)
        std::replace(inpDir.begin(), inpDir.end(), '\\', '/'); 
        bool isFromCurrent = inpDir == "" || inpDir == ".";

        if (!isFromCurrent) theLog->logFormatted("Compile source from: %s", inpDir.c_str());

        list<string> source_extensions = { ".mpp", ".ompp" };
        list<string> source_files = listSourceFiles(inpDir, source_extensions);

        if (source_files.empty()) {
            theLog->logMsg("No source files found, nothing to compile at current directory");
            return EXIT_FAILURE;
        }

        // "normalize" input and output directories: 
        // use empty "" if it is current directory else make sure it ends with /
        if (isFromCurrent) {
            inpDir = "";
        }
        else {
            if (inpDir.back() != '/' && inpDir.back() != '\\') inpDir += '/';
        }

        string outDir = argStore.strOption(OmcArgKey::outputDir);
        // normalize path separators (to avoid escaping \ in #line directives)
        std::replace( outDir.begin(), outDir.end(), '\\', '/'); 
        bool isToCurrent = outDir == "" || outDir == ".";

        if (!isToCurrent) theLog->logFormatted("Compile source into: %s", outDir.c_str());

        if (isToCurrent) {
            outDir = "";
        }
        else {
            if (outDir.back() != '/' && outDir.back() != '\\') outDir += '/';
        }

        // Obtain locations of 'use' folders to make available to parser.
		string omc_exe = argv[0];
		{
			string use_folders;
			if (argStore.isOptionExist(OmcArgKey::useDir)) {
				use_folders = argStore.strOption(OmcArgKey::useDir);
			}
			else {
				// default value is sister directory of omc.exe directory, named 'use'
				use_folders = omc_exe.substr(0, omc_exe.find_last_of("/\\") + 1) + "../use/";
			}

			// split and iterate the delimited use folders - either ; or , are valid delimiters
			{
				const string delimiters = ";,";
				size_t last_pos = use_folders.find_first_not_of(delimiters, 0);
				size_t pos = use_folders.find_first_of(delimiters, last_pos);
				while (string::npos != pos || string::npos != last_pos) {
					string use_folder = use_folders.substr(last_pos, pos - last_pos);
					// 'normalize' to have trailing /
					if (!use_folder.empty() && use_folder.back() != '/' && use_folder.back() != '\\') {
						use_folder += '/';
					}
					Symbol::use_folders.push_back(use_folder);
					theLog->logFormatted("use folder: %s", use_folder.c_str());
					// advance to next token
					last_pos = use_folders.find_first_not_of(delimiters, pos);
					pos = use_folders.find_first_of(delimiters, last_pos);
				}
			}
		}

        // Obtain information on code page name, default: empty "" name (= system default code page)
        Symbol::code_page = argStore.strOption(OmcArgKey::codePage);

        // Obtain information on generation of #line directives, default: false
        Symbol::no_line_directives = argStore.boolOption(OmcArgKey::noLineDirectives);

        // Obtain information on detailed parsing option, default: false
        Symbol::trace_parsing = argStore.boolOption(OmcArgKey::traceParsing);

        // Obtain information on detailed scanning option, default: false
        Symbol::trace_scanning = argStore.boolOption(OmcArgKey::traceScanning);

        // get sql script directory where SQLite create_db.sql is located
        string sqlDir;
        if (argStore.isOptionExist(OmcArgKey::sqlDir)) {
            sqlDir = argStore.strOption(OmcArgKey::sqlDir);
        }
        else {
            // default sql directory: /om_root/bin/omc.exe => /om_root/sql
            sqlDir = makeFilePath(baseDirOf(baseDirOf(omc_exe)).c_str(), "sql");
        }

        // get comma- or semicolon- separated list of SQL provider names to create model sql scripts
        // use SQLite as default
        string sqlProviders = argStore.strOption(OmcArgKey::dbProviderNames, openm::SQLITE_DB_PROVIDER);

        // Populate symbol table with default symbols
        Symbol::populate_default_symbols(model_name, scenario_name);

        // Create set of functions taking translatable string literal as first argument
        list<string> mf = splitCsv(
            argStore.strOption(OmcArgKey::messageFnc, "LT,logMsg,logFormatted,WriteLogEntry,WarningMsg,ModelExit"),
            ",;");
        for (const string & f : mf) {
            Symbol::tran_funcs.insert(f);
        }

        // create unique instance of ParseContext
        ParseContext pc;

        // open & prepare pass-through / markup stream om_developer.cpp
        ofstream om_developer_cpp(outDir + "om_developer.cpp", ios::out | ios::trunc | ios::binary);
        exit_guard<ofstream> onExit_om_developer_cpp(&om_developer_cpp, &ofstream::close);   // close on exit
        if (om_developer_cpp.fail()) throw HelperException(LT("error : unable to open %s for writing"), "om_developer.cpp");

    #if defined(_MSC_VER)
        // UTF-8 BOM for Microsoft compiler
        om_developer_cpp << "\xEF\xBB\xBF";
    #endif
        om_developer_cpp << "/**" << endl;
        om_developer_cpp << " * @file om_developer.cpp" << endl;
        om_developer_cpp << " * Developer-supplied C++ functions" << endl;
        om_developer_cpp << " */" << endl;
        om_developer_cpp << "" << endl;
        om_developer_cpp << "#include \"omc/omPch.h\"" << endl;
        om_developer_cpp << "#include \"omc/omSimulation.h\"" << endl;
		om_developer_cpp << "using namespace openm;" << endl;

        // Populate starting contents of the list of all source files to be parsed.
        // An additional file is appended to this list during parsing
        // each time a 'use' statement is encountered in a source code file.
        //Symbol::all_source_files.swap(source_files);
        // deep copy required because file names must be maintained (code locations use pointer to file name)
        Symbol::all_source_files = source_files;
        list<string>::iterator start_it = Symbol::all_source_files.begin();
        parseFiles(Symbol::all_source_files, start_it, pc, &om_developer_cpp);

        // Parse parameter scenario directory if specified
        string paramDir; // make visible for possible later use for Missing.dat
        if (argStore.isOptionExist(OmcArgKey::paramDir)) {
            // -p scenario parameters specified
            paramDir = argStore.strOption(OmcArgKey::paramDir);
            pc.is_scenario_parameter_value = true;
            pc.is_fixed_parameter_value = false;
            if (paramDir != ".") theLog->logFormatted("Compile scenario parameters from: %s", paramDir.c_str());
            if (paramDir.back() != '/' && paramDir.back() != '\\') paramDir += '/';
            list<string> param_extensions = { ".dat", ".odat" };
            list<string> param_files = listSourceFiles(paramDir, param_extensions);     // list of parameter file names
            if (param_files.empty()) {
                theLog->logMsg("No parameter files found, nothing to compile at current directory");
            }
            else {
                size_t count_prev = Symbol::all_source_files.size();
                Symbol::all_source_files.splice(Symbol::all_source_files.end(), param_files);
                auto start_it = Symbol::all_source_files.begin();
                advance(start_it, count_prev);
                parseFiles(Symbol::all_source_files, start_it, pc, &om_developer_cpp);
            }
        }

        // Parse fixed parameter directory if specified
        if (argStore.isOptionExist(OmcArgKey::fixedDir)) {
            // -f fixed parameters specified
            string fixedDir = argStore.strOption(OmcArgKey::fixedDir);
            pc.is_scenario_parameter_value = false;
            pc.is_fixed_parameter_value = true;
            if (fixedDir != ".") theLog->logFormatted("Compile fixed parameters from: %s", fixedDir.c_str());
            if (fixedDir.back() != '/' && fixedDir.back() != '\\') fixedDir += '/';
            list<string> parm_extensions = { ".dat", ".odat" };
            list<string> parm_files = listSourceFiles(fixedDir, parm_extensions);     // list of parameter file names
            if (parm_files.empty()) {
                theLog->logMsg("No parameter files found at current directory");
            }
            else {
                size_t count_prev = Symbol::all_source_files.size();
                Symbol::all_source_files.splice(Symbol::all_source_files.end(), parm_files);
                auto start_it = Symbol::all_source_files.begin();
                advance(start_it, count_prev);
                parseFiles(Symbol::all_source_files, start_it, pc, &om_developer_cpp);
            }
        }

        if (Symbol::tran_strings.size() > 0 && LanguageSymbol::number_of_languages() > 1) {
            theLog->logFormatted("Identified %d translatable C++ strings in model source", Symbol::tran_strings.size());
        }

        try {
            theLog->logMsg("Post-parse processing");
            // perform all post-parse multi-pass processing
            Symbol::post_parse_all();
        }
        catch(exception & ex) {
            theLog->logErr(ex);
            // An error count of zero means something went seriously worng in the post-parse phase
            // and an exception was thrown without first incrementing the error count and log message.
            if (Symbol::post_parse_errors == 0) Symbol::post_parse_errors = 1;
        }

        // validate parameter initializers
        for (auto param : Symbol::pp_all_parameters) {
            param->validate_initializer();
        }

        if (Symbol::post_parse_warnings > 0) {
            theLog->logFormatted("%d warnings in post-parse phase", Symbol::post_parse_warnings);
        }
        if (Symbol::post_parse_errors > 0) {
            theLog->logFormatted("%d errors in post-parse phase", Symbol::post_parse_errors);
            throw HelperException(LT("Finish omc"));
        }

        theLog->logMsg("Code & meta-data generation");

        // open output streams for generated code
        ofstream om_types0_h(outDir + "om_types0.h", ios::out | ios::trunc | ios::binary);
        exit_guard<ofstream> onExit_om_types0_h(&om_types0_h, &ofstream::close);   // close on exit
        if (om_types0_h.fail()) throw HelperException(LT("error : unable to open %s for writing"), "om_types0.h");

        ofstream om_types1_h(outDir + "om_types1.h", ios::out | ios::trunc | ios::binary);
        exit_guard<ofstream> onExit_om_types1_h(&om_types1_h, &ofstream::close);   // close on exit
        if (om_types1_h.fail()) throw HelperException(LT("error : unable to open %s for writing"), "om_types1.h");

        ofstream om_declarations_h(outDir + "om_declarations.h", ios::out | ios::trunc | ios::binary);
        exit_guard<ofstream> onExit_om_declarations_h(&om_declarations_h, &ofstream::close);   // close on exit
        if (om_declarations_h.fail()) throw HelperException(LT("error : unable to open %s for writing"), "om_declarations.h");

        string om_definitions_cpp_fname = outDir + "om_definitions.cpp";
        ofstream om_definitions_cpp(om_definitions_cpp_fname, ios::out | ios::trunc | ios::binary);
        exit_guard<ofstream> onExit_om_definitions_cpp(&om_definitions_cpp, &ofstream::close);   // close on exit
        if (om_definitions_cpp.fail()) throw HelperException(LT("error : unable to open %s for writing"), "om_definitions.cpp");

        ofstream om_fixed_parms_cpp(outDir + "om_fixed_parms.cpp", ios::out | ios::trunc | ios::binary);
        exit_guard<ofstream> onExit_om_fixed_parms_cpp(&om_fixed_parms_cpp, &ofstream::close);   // close on exit
        if (om_fixed_parms_cpp.fail()) throw HelperException(LT("error : unable to open %s for writing"), "om_fixed_parms.cpp");

#if defined(_MSC_VER)
        // UTF-8 BOM for Microsoft compiler
        om_types0_h << "\xEF\xBB\xBF";
        om_types1_h << "\xEF\xBB\xBF";
        om_declarations_h << "\xEF\xBB\xBF";
        om_definitions_cpp << "\xEF\xBB\xBF";
        om_fixed_parms_cpp << "\xEF\xBB\xBF";
#endif
        // collect model metadata during code generation
        MetaModelHolder metaRows;
        unique_ptr<IModelBuilder> builder(IModelBuilder::create(sqlProviders, sqlDir, outDir));
        if (!builder->isSqliteDb())
            theLog->logMsg("warning : model SQLite database not created");

        CodeBlock missing_param_defs; // Generated definitions for missing parameters
        CodeGen cg(
            &om_types0_h,
            &om_types1_h,
            &om_declarations_h,
            &om_definitions_cpp,
            &om_fixed_parms_cpp,
            missing_param_defs,
            builder.get(),
            Symbol::no_line_directives,
            om_definitions_cpp_fname,
            metaRows
            );
        cg.do_all();

        string Missing_dat_name = "Missing.dat.tmp";
        if (missing_param_defs.size() > 0) {
            // Some generated output for one or more missing parameters present.
            if (argStore.isOptionExist(OmcArgKey::paramDir)) {
                // open output stream for generated definitions for missing parameters
                ofstream Missing_dat(paramDir + Missing_dat_name, ios::out | ios::trunc | ios::binary);
                exit_guard<ofstream> onExit_Missing_dat(&Missing_dat, &ofstream::close);   // close on exit
                if (Missing_dat.fail()) throw HelperException(LT("error : unable to open %s for writing"), "Missing.dat.tmp");
                Missing_dat << missing_param_defs;
                Missing_dat.close();
            }
            else {
                string msg = "omc : warning : Unable to write missing parameters to " + Missing_dat_name + " - no input parameter directory was specified.";
                theLog->logMsg(msg.c_str());
            }
        }
        else {
            // Model contains no missing parameters, so delete obsolete Missing.dat.tmp if present
            if (argStore.isOptionExist(OmcArgKey::paramDir)) {
                string full_name = paramDir + Missing_dat_name;
                remove(full_name.c_str());
            }
        }

        // create model.message.ini file
        theLog->logMsg("Meta-data processing");
        buildMessageIni(metaRows, inpDir, outDir, Symbol::code_page.c_str(), Symbol::tran_strings);

        // build model creation sql script and model.sqlite database
        builder->build(metaRows);
        
        // Create working set for published scenario
        theLog->logMsg("Scenario processing");

        // Get the scenario symbol
        ScenarioSymbol *scenario_symbol = dynamic_cast<ScenarioSymbol *>(Symbol::find_a_symbol(typeid(ScenarioSymbol)));
        assert(scenario_symbol);

        MetaSetLangHolder metaSet;    // metadata for working set
        metaSet.worksetRow.name = scenario_name;
        for (auto lang : Symbol::pp_all_languages) {
            WorksetTxtLangRow worksetTxt;
            auto lang_name = lang->name;
            worksetTxt.langCode = lang_name;
            worksetTxt.descr = scenario_symbol->label(*lang);
            worksetTxt.note = scenario_symbol->note(*lang);
            metaSet.worksetTxt.push_back(worksetTxt);
        }

        int scenario_parameters_count = 0;
        for (auto param : Symbol::pp_all_parameters) {
            if (param->source != ParameterSymbol::scenario_parameter) continue;
            scenario_parameters_count++;
            WorksetParamRow wsParam;
            wsParam.paramId = param->pp_parameter_id;
            metaSet.worksetParam.push_back(wsParam);  // add parameter to workset
            // value notes for the parameter
            for (auto lang : Symbol::pp_all_languages) {
                WorksetParamTxtLangRow worksetParamTxt;
                auto lang_id = lang->language_id;
                auto lang_name = lang->name;
                assert(lang_id < (int)param->pp_value_notes.size()); // logic guarantee
                string value_note = param->pp_value_notes[lang_id];
                if (value_note.length() > 0) {
                    worksetParamTxt.paramId = wsParam.paramId;
                    worksetParamTxt.langCode = lang_name;
                    worksetParamTxt.note = value_note;
                    metaSet.worksetParamTxt.push_back(worksetParamTxt);
                }
            }
        }

        // insert model workset int SQLite database
        if (!builder->isSqliteDb()) {
            theLog->logMsg("warning : model SQLite database not created");
        }
        else {
            // start model default working set
            builder->beginWorkset(metaRows, metaSet);

            // add values for all scenario model parameters into default working set
            int scenario_parameters_done = 0;
            chrono::system_clock::time_point start = chrono::system_clock::now();
            for (auto param : Symbol::pp_all_parameters) {
                if (param->source != ParameterSymbol::scenario_parameter) continue;
                auto lst = param->initializer_for_storage();
                builder->addWorksetParameter(metaRows, metaSet, param->name, lst);
                scenario_parameters_done++;
                chrono::system_clock::time_point now = chrono::system_clock::now();
                if (chrono::duration_cast<chrono::seconds>(now - start).count() >= 3) {     // report not more often than every 3 seconds
                    start = now;
                    theLog->logFormatted("%d of %d parameters processed (%s)", scenario_parameters_done, scenario_parameters_count, param->name.c_str());
                }
            }
            theLog->logFormatted("%d of %d parameters processed", scenario_parameters_count, scenario_parameters_count);

            // complete model default working set
            theLog->logMsg("Finalize scenario processing");
            builder->endWorkset(metaRows, metaSet);
        }

        // build Modgen compatibilty views sql script
        builder->buildCompatibilityViews(metaRows);
    }
    catch(DbException & ex) {
        theLog->logErr(ex, "DB error");
        return EXIT_FAILURE;
    }
    catch(exception & ex) {
        theLog->logErr(ex);
        return EXIT_FAILURE;
    }

    theLog->logMsg("Finish omc");
    return EXIT_SUCCESS;
}

// Parse a list of files
static void parseFiles(list<string> & files, const list<string>::iterator start_it, ParseContext & pc, ofstream *markup_stream)
{
    using namespace openm;
    for (auto it = start_it; it != files.cend(); it++) {
        string full_name = *it;
        try {
            string file_ext = getFileNameExt(full_name);
            string file_stem = getFileNameStem(full_name);
            string file_name = file_stem + file_ext;
            // As a special case, ignore file starting with "modgen_".
            // This mechanism allows a single model code base to support compilation
            // by either the Modgen or OpenM++ compilers.
            string file_name_lc = file_name;
            openm::toLower(file_name_lc);

            if (openm::equalNoCase(file_name.c_str(), "modgen_", 7)) {
                theLog->logFormatted("Skipping %s", full_name.c_str());
                continue;
            }

            theLog->logFormatted("Parsing %s", full_name.c_str());
            string normalized_full_name = replaceAll(full_name, "\\", "/");
            *markup_stream << endl; // required in case last line of previous file had no trailing newline
            string line_start = Symbol::no_line_directives ? "//#line " : "#line ";
            *markup_stream << line_start << "1 \"" << normalized_full_name << "\"" << endl;

            // create new instance of parser-scanner driver for each source file
            Driver drv( pc );
            drv.trace_scanning = Symbol::trace_scanning;
            drv.trace_parsing = Symbol::trace_parsing;
            // must pass non-transient pointer to string as first argument to drv.parse, since used in location objects
            drv.parse(&*it, file_name, file_stem, markup_stream);
        }
        catch(exception & ex) {
            theLog->logErr(ex);
            if ( pc.parse_errors == 0 ) {
                pc.parse_errors = 1;
            }
            // continue parsing any remaining source code modules to detect additional syntax errors (possibly non-spurious)
        }
    }
    if ( pc.parse_warnings > 0 ) {
        theLog->logFormatted("%d warnings in parse phase", pc.parse_warnings);
    }
    if ( pc.parse_errors > 0 ) {
        theLog->logFormatted("%d syntax errors in parse phase", pc.parse_errors);
        throw HelperException(LT("Finish omc"));
    }
}

