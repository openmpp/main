/**
* @file    omc.cpp
* OpenM++ compiler (omc): main entry point
*  
* @mainpage OpenM++ compiler (omc)
* 
* The OpenM++ compiler produces c++ (.cpp and .h) files and SQL files (.sql) from model source files (.ompp, .mpp) and parameter value files (.dat, .odat, .csv, .tsv). \n
* 
*
* The following command line arguments are supported by omc:
* * -Omc.ModelName      name/of/model/executable, e.g. RiskPaths
* * -Omc.ScenarioName   name/of/base/scenario, e.g. Base (can be list of names if there is more than one parameter directory specified)
* * -Omc.InputDir       input/dir/to/find/source/files
* * -Omc.OutputDir      output/dir/to/place/compiled/cpp_and_h_and_sql/files
* * -Omc.UseDir         use/dir/with/ompp/files
* * -Omc.ParamDir       input/dir/to/find/parameter/files/for/scenario (can be list of directories)
* * -Omc.FixedDir       input/dir/to/find/fixed/parameter/files/
* * -Omc.DocDir         input/dir/to/find/authored/model/documentation/files/
* * -Omc.ModelDoc       if true then generate model documentation (user version)
* * -Omc.ModelDevDoc    if true then generate model documentation (model dev version)
* * -Omc.SqlDir         input sql/script/dir to create SQLite database
* * -Omc.SqliteDir      output directory to create SQLite model database
* * -Omc.SqlPublishTo   create sql scripts to publish in SQLite,MySQL,PostgreSQL,MSSQL,Oracle,DB2, default: SQLite
* * -Omc.CodePage       code page for converting source files, e.g. windows-1252
* * -Omc.MessageLanguage    language to display output messages, default: user environment settings
* * -Omc.MessageFnc     localized message functions, default: LT,logMsg,logFormatted,WriteLogEntry,WarningMsg,ModelExit
* * -Omc.NoLineDirectives   suppress #line directives in generated cpp files
* * -Omc.NoMetadata     suppress production of model metadata (model cannot be run), default: false
* * -Omc.TraceScanning  detailed tracing from scanner
* * -Omc.TraceParsing   detailed tracing from parser
* * -OpenM.IniFile      path/to/optional/omc.ini
*
* Short form of command line arguments:
* * -m short form of -Omc.ModelName
* * -s short form of -Omc.ScenarioName
* * -i short form of -Omc.InputDir
* * -o short form of -Omc.OutputDir
* * -u short form of -Omc.UseDir
* * -p short form of -Omc.ParamDir
* * -f short form of -Omc.FixedDir
* * -d short form of -Omc.DocDir
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
#include <filesystem>
#include "Symbol.h"
#include "LanguageSymbol.h"
#include "ParameterSymbol.h"
#include "ScenarioSymbol.h"
#include "AnonGroupSymbol.h"
#include "Driver.h"
#include "ParseContext.h"
#include "CodeGen.h"
#include "libopenm/common/argReader.h"
#include "libopenm/common/omFile.h"
#include "libopenm/db/modelBuilder.h"
#include "omc_file.h"
#include "omc_csv.h"
#include "omc_model_metrics.h"
#include "omc_missing_documentation.h"
#include "omc_model_doc.h"

using namespace std;
using namespace openm;
using namespace omc;

namespace openm
{
    /** keys for omc options */
    struct OmcArgKey
    {
        /** omc model name */
        static constexpr const char * modelName = "Omc.ModelName";

        /** omc scenario name (or list of names) */
        static constexpr const char * scenarioName = "Omc.ScenarioName";

        /** omc input directory with openM++ source files */
        static constexpr const char * inputDir = "Omc.InputDir";

        /** omc output directory for compiled files */
        static constexpr const char * outputDir = "Omc.OutputDir";

        /** omc use directory to resolve 'use' statements */
        static constexpr const char * useDir = "Omc.UseDir";

        /** omc input directory with OpenM++ scenario parameter files (or list of directories) */
        static constexpr const char * paramDir = "Omc.ParamDir";

        /** omc input directory with OpenM++ fixed parameter files */
        static constexpr const char * fixedDir = "Omc.FixedDir";

        /** omc input directory with openM++ authored documentation files */
        static constexpr const char* docDir = "Omc.DocDir";

        /**  omc code page: input files encoding name */
        static constexpr const char * codePage = "Omc.CodePage";

        /** omc suppress #line directives in generated cpp files */
        static constexpr const char * noLineDirectives = "Omc.NoLineDirectives";

        /** omc suppress metadata publishing (model cannot be run) */
        static constexpr const char* noMetadata = "Omc.NoMetadata";

        /** omc generate model documentation (user version) */
        static constexpr const char* modelDoc = "Omc.ModelDoc";

        /** omc generate model documentation (model dev version) */
        static constexpr const char* modelDevDoc = "Omc.ModelDevDoc";

        /** omc generate detailed output from parser */
        static constexpr const char * traceParsing = "Omc.TraceParsing";

        /** omc generate detailed output from scanner */
        static constexpr const char * traceScanning = "Omc.TraceScanning";

        /** omc input directory with sql script to create SQLite model database */
        static constexpr const char * sqlDir = "Omc.SqlDir";

        /** omc output directory to create SQLite model database */
        static constexpr const char * sqliteDir = "Omc.SqliteDir";

        /** omc list of db-provider names to create sql scripts */
        static constexpr const char * dbProviderNames = "Omc.SqlPublishTo";

        /** language to display output messages */
        static constexpr const char * messageLang = "Omc.MessageLanguage";

        /** list of functions which produce localized messages */
        static constexpr const char * messageFnc = "Omc.MessageFnc";
    };

    /** keys for omc options (short form) */
    struct OmcShortKey
    {
        /** short name for ini file name: -ini fileName.ini */
        static constexpr const char * iniFile = "ini";

        /** short name for omc model name */
        static constexpr const char * modelName = "m";

        /** short name for omc scenario name */
        static constexpr const char * scenarioName = "s";

        /** short name for omc input directory  */
        static constexpr const char * inputDir = "i";

        /** short name for omc output directory */
        static constexpr const char * outputDir = "o";

        /** short name for omc use directory */
        static constexpr const char * useDir = "u";

        /** short name for omc param directory */
        static constexpr const char * paramDir = "p";

        /** short name for omc fixed directory */
        static constexpr const char * fixedDir = "f";

        /** short name for omc doc directory */
        static constexpr const char* docDir = "d";
    };

    /** array of model run option keys. */
    static const char * runArgKeyArr[] = {
        OmcArgKey::modelName,
        OmcArgKey::scenarioName,
        OmcArgKey::inputDir,
        OmcArgKey::outputDir,
        OmcArgKey::useDir,
        OmcArgKey::paramDir,
        OmcArgKey::fixedDir,
        OmcArgKey::docDir,
        OmcArgKey::codePage,
        OmcArgKey::noLineDirectives,
        OmcArgKey::noMetadata,
        OmcArgKey::modelDoc,
        OmcArgKey::modelDevDoc,
        OmcArgKey::traceParsing,
        OmcArgKey::traceScanning,
        OmcArgKey::sqlDir,
        OmcArgKey::sqliteDir,
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
        make_pair(OmcShortKey::docDir, OmcArgKey::docDir),
    };
    static const size_t shortPairSize = sizeof(shortPairArr) / sizeof(const pair<const char *, const char *>);
}

static bool parseBoolOption(const char * i_key, const ArgReader & i_argStore);
static void parseFiles(list<string> & files, const list<string>::iterator start_it, ParseContext & pc, ofstream *markup_stream);
static void processExtraParamDir(const string & i_paramDir, const string & i_scenarioName, const MetaModelHolder & i_metaRows, IModelBuilder * i_builder);
static void createWorkset(const MetaModelHolder & i_metaRows, MetaSetLangHolder & io_metaSet, IModelBuilder * i_builder);

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

        string omc_exe = argv[0];   /// path of omc.exe

        // read language specific messages from path/to/exe/omc.message.ini
        string msgFilePath = makeFilePath(baseDirOf(omc_exe).c_str(), "omc.message.ini");

        IniFileReader::loadMessages(
            msgFilePath.c_str(),
            argStore.strOption(OmcArgKey::messageLang)
        );
        theLog->logMsg("Start omc");

        // load translated messages for all languages from path/to/exe/omc.message.ini
        theAllTranslated->load(msgFilePath.c_str());

        // get model name
        string model_name = argStore.strOption(OmcArgKey::modelName);
        if (model_name.empty()) {
            model_name = "Model";
            theLog->logMsg("Model name not specified - using default name 'Model'. Use -m option to specify model name.");
        }

        // list of scenario(s) names
        list<string> scNameLst = splitCsv(argStore.strOption(OmcArgKey::scenarioName), ",;");

        // default scenario name
        string scenario_name;
        if (scNameLst.size() > 0) scenario_name = scNameLst.front();

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

        // Get 'doc' folder containing authored model documentation files.
        if (argStore.isOptionExist(OmcArgKey::docDir)) {
            string docDir = argStore.strOption(OmcArgKey::docDir);
            // "normalize" input doc directory: 
            // use empty "" if it is current directory else make sure it ends with /
            std::replace(docDir.begin(), docDir.end(), '\\', '/');
            bool isToCurrent = ((docDir == "") || (docDir == "."));
            if (isToCurrent) {
                docDir = "";
            }
            else {
                if (docDir.back() != '/') docDir += '/';
            }

            if (std::filesystem::is_directory(docDir)) {
                list<string> doc_extensions = { ".md" };
                Symbol::in_doc_dir = docDir;
                auto in_doc_paths = listSourceFiles(docDir, doc_extensions);
                if (in_doc_paths.size() > 0) {
                    Symbol::in_doc_active = true;
                    for (auto s : in_doc_paths) {
                        // remove leading path portion
                        {
                            auto p = s.find_last_of('/');
                            if (p != s.npos) {
                                s = s.substr(p + 1);
                            }
                        }
                        // remove extension
                        {
                            auto p = s.find_last_of('.');
                            if (p != s.npos) {
                                s = s.substr(0, p);
                            }
                        }
                        Symbol::in_doc_stems.insert(s);
                    }
                    theLog->logFormatted("Authored input model documentation from: %s", docDir.c_str());
                }
            }
        }

        // Obtain information on code page name, default: empty "" name (= system default code page)
        Symbol::code_page = argStore.strOption(OmcArgKey::codePage);

        // Obtain information on generation of #line directives, default: false
        Symbol::no_line_directives = parseBoolOption(OmcArgKey::noLineDirectives, argStore);

        // Obtain information on disabling metadata publishing, default: false
        Symbol::no_metadata = parseBoolOption(OmcArgKey::noMetadata, argStore);

        // Obtain information on generating model documentation (user version), default: false
        Symbol::model_doc = parseBoolOption(OmcArgKey::modelDoc, argStore);

        // Obtain information on generating model documentation (model dev version), default: false
        Symbol::model_devdoc = parseBoolOption(OmcArgKey::modelDevDoc, argStore);

        // Obtain information on detailed parsing option, default: false
        Symbol::trace_parsing = parseBoolOption(OmcArgKey::traceParsing, argStore);

        // Obtain information on detailed scanning option, default: false
        Symbol::trace_scanning = parseBoolOption(OmcArgKey::traceScanning, argStore);

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
        Symbol::mpp_source_files = source_files;
        list<string>::iterator start_it = Symbol::all_source_files.begin();
        parseFiles(Symbol::all_source_files, start_it, pc, &om_developer_cpp);

        // Parse parameter scenario directory if specified
        list<string> paramDirLst = splitCsv(argStore.strOption(OmcArgKey::paramDir), ",;");

        string paramDir;
        if (paramDirLst.size() > 0) paramDir = paramDirLst.front();

        if (!paramDir.empty()) {
            // -p scenario parameters specified
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
                Symbol::dat_source_files = param_files;
                size_t count_prev = Symbol::all_source_files.size();
                Symbol::all_source_files.splice(Symbol::all_source_files.end(), param_files);
                auto start_it = Symbol::all_source_files.begin();
                advance(start_it, count_prev);
                parseFiles(Symbol::all_source_files, start_it, pc, &om_developer_cpp);
            }
        }

        // Parse fixed parameter directory if specified
        string fixedDir = argStore.strOption(OmcArgKey::fixedDir);

        if (argStore.isOptionExist(OmcArgKey::fixedDir)) {
            // -f fixed parameters specified
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
                Symbol::dat_source_files.insert(Symbol::dat_source_files.end(), parm_files.begin(), parm_files.end());
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
            // An error count of zero means something went seriously wrong in the post-parse phase
            // and an exception was thrown without first incrementing the error count and log message.
            if (Symbol::post_parse_errors == 0) Symbol::post_parse_errors = 1;
        }

        // load parameters data from .csv or .tsv files
        forward_list<unique_ptr<Constant> > cpLst;

        if (!paramDir.empty()) {
            readParameterCsvFiles(false, paramDir, cpLst);
        }
        if (argStore.isOptionExist(OmcArgKey::fixedDir)) {
            readParameterCsvFiles(true, fixedDir, cpLst);
        }

        // Change source of parameters if parameters_suppress used
        if (Symbol::any_parameters_suppress) {
            for (auto grp : Symbol::pp_all_anon_groups) {
                if (grp->anon_kind == AnonGroupSymbol::eKind::parameters_suppress) {
                    for (auto sym : grp->expanded_list()) {
                        auto symbol_name = sym->name;
                        auto ps = dynamic_cast<ParameterSymbol*>(sym);
                        if (ps) {
                            if (ps->source == ParameterSymbol::parameter_source::scenario_parameter) {
                                // indicate that this scenario parameter is to be burned into the executable instead of published
                                ps->source = ParameterSymbol::parameter_source::fixed_parameter;
                            }
                        }
                    }
                }
            }
        }

        // Change source of parameters if parameters_retain used
        if (Symbol::any_parameters_retain) {
            for (auto param : Symbol::pp_all_parameters) {
                // mark all scenario parameters as burned in unless explicitly retained
                if (param->source == ParameterSymbol::parameter_source::scenario_parameter) {
                    param->source = ParameterSymbol::parameter_source::fixed_parameter;
                }
            }
            for (auto grp : Symbol::pp_all_anon_groups) {
                if (grp->anon_kind == AnonGroupSymbol::eKind::parameters_retain) {
                    for (auto sym : grp->expanded_list()) {
                        auto symbol_name = sym->name;
                        auto ps = dynamic_cast<ParameterSymbol*>(sym);
                        if (ps) {
                            if (ps->source == ParameterSymbol::parameter_source::fixed_parameter) {
                                // indicate that this parameter is to be published
                                ps->source = ParameterSymbol::parameter_source::scenario_parameter;
                            }
                        }
                    }
                }
            }
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
        unique_ptr<IModelBuilder> builder(IModelBuilder::create(sqlProviders, sqlDir, outDir, argStore.strOption(OmcArgKey::sqliteDir)));
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

        // block for creation of file with missing parameter definitions
        {
            const string Missing_dat_name = "MissingParameters.dat";
            if (missing_param_defs.size() > 0) {
                // Some generated output for one or more missing parameters.
                // Open output stream for generated place-holder definitions for missing parameters.
                ofstream Missing_dat(makeFilePath(outDir.c_str(), Missing_dat_name.c_str()), ios::out | ios::trunc | ios::binary);
                exit_guard<ofstream> onExit_Missing_dat(&Missing_dat, &ofstream::close);   // close on exit
                if (Missing_dat.fail()) {
                    string msg = "warning : Unable to open " + Missing_dat_name + " for writing.";
                    theLog->logMsg(msg.c_str());
                }
                else {
                    Missing_dat << missing_param_defs;
                    Missing_dat.close();
                    theLog->logFormatted("note : Missing parameters written to '%sMissingParameters.dat'", outDir.c_str());
                }
            }
            else {
                // Model contains no missing parameters, so delete obsolete MissingParameters.dat if present.
                remove(makeFilePath(outDir.c_str(), Missing_dat_name.c_str()).c_str());
            }
        }

        // block for creation of file with generated short names
        {
            CodeBlock generated_names_code = Symbol::build_NAME_code();
            const string GeneratedNames_ompp_name = "GeneratedNames.ompp";
            if (generated_names_code.size() > 0) {
                // There are one or more generated short names.
                // open output stream for //NAME statements for generated names
                ofstream GeneratedNames_ompp(makeFilePath(outDir.c_str(), GeneratedNames_ompp_name.c_str()), ios::out | ios::trunc | ios::binary);
                exit_guard<ofstream> onExit_Missing_dat(&GeneratedNames_ompp, &ofstream::close);   // close on exit
                if (GeneratedNames_ompp.fail()) {
                    string msg = "omc : warning : Unable to open " + GeneratedNames_ompp_name + " for writing.";
                    theLog->logMsg(msg.c_str());
                }
                else {
                    GeneratedNames_ompp << generated_names_code;
                    GeneratedNames_ompp.close();
                }
            }
            else {
                // Model contains no generated names, so delete obsolete GeneratedNames.ompp if present
                remove(makeFilePath(inpDir.c_str(), GeneratedNames_ompp_name.c_str()).c_str());
            }
        }

        // block to create file containing generated event dependencies
        {
            std::list<std::string> event_dependencies_csv_content = Symbol::build_event_dependencies_csv();
            const string EventDependencies_csv_name = "EventDependencies.csv";
            if (!event_dependencies_csv_content.empty()) {
                // There are one or more event dependencies.
                // Open output stream for csv
                ofstream EventDependencies_csv(makeFilePath(outDir.c_str(), EventDependencies_csv_name.c_str()), ios::out | ios::trunc | ios::binary);
                exit_guard<ofstream> onExit_Missing_dat(&EventDependencies_csv, &ofstream::close);   // close on exit
                if (EventDependencies_csv.fail()) {
                    string msg = "omc : warning : Unable to open " + EventDependencies_csv_name + " for writing.";
                    theLog->logMsg(msg.c_str());
                }
                else {
                    for (auto& line : event_dependencies_csv_content) {
                        EventDependencies_csv << line + "\n";
                    }
                    EventDependencies_csv.close();
                }
            }
            else {
                // Model contains no generated names, so delete obsolete EventDependencies.csv if present
                remove(makeFilePath(inpDir.c_str(), EventDependencies_csv_name.c_str()).c_str());
            }
        }

        // block for creation of file with import information in csv format
        {
            auto content = Symbol::build_imports_csv();
            const string model_imports_csv_name = model_name + ".imports.csv";
            if (content.size() > 0) {
                // There are one or more imports.
                // open output stream for csv
                ofstream model_imports_csv(makeFilePath(outDir.c_str(), model_imports_csv_name.c_str()), ios::out | ios::trunc | ios::binary);
                exit_guard<ofstream> onExit_imports_csv(&model_imports_csv, &ofstream::close);   // close on exit
                if (model_imports_csv.fail()) {
                    string msg = "omc : warning : Unable to open " + model_imports_csv_name + " for writing.";
                    theLog->logMsg(msg.c_str());
                }
                else {
                    model_imports_csv << content;
                    model_imports_csv.close();
                }
            }
            else {
                // Model contains no imports, so delete obsolete model_imports.csv if present
                remove(makeFilePath(inpDir.c_str(), model_imports_csv_name.c_str()).c_str());
            }
        }

        // create model.message.ini file
        theLog->logMsg("Meta-data processing");
        buildMessageIni(metaRows, inpDir, outDir, Symbol::code_page.c_str(), Symbol::tran_strings);

        if (Symbol::no_metadata) {
            theLog->logMsg("warning : Meta-data publishing is disabled with Omc.NoMetadata");
        }

        if (!Symbol::no_metadata) {
            // build model creation sql script and model.sqlite database
            builder->build(metaRows);
        }
        
        // Create working set for published scenario
        theLog->logFormatted("Scenario processing: %s", scenario_name.c_str());

        // Get the scenario symbol
        ScenarioSymbol *scenario_symbol = dynamic_cast<ScenarioSymbol *>(Symbol::find_a_symbol(typeid(ScenarioSymbol)));
        assert(scenario_symbol);

        MetaSetLangHolder metaSet;    // metadata for working set
        metaSet.worksetRow.name = scenario_name;
        for (const auto& langSym : Symbol::pp_all_languages) {
            const string& lang = langSym->name; // e.g. "EN" or "FR"
            WorksetTxtLangRow worksetTxt;
            worksetTxt.langCode = lang;
            worksetTxt.descr = scenario_symbol->label(*langSym);
            worksetTxt.note = scenario_symbol->note(*langSym);
            metaSet.worksetTxt.push_back(worksetTxt);
        }

        int scenario_parameters_count = 0;
        for (auto param : Symbol::pp_all_parameters) {
            if (param->source != ParameterSymbol::scenario_parameter) continue;

            scenario_parameters_count++;
            WorksetParamRow wsParam;
            wsParam.paramId = param->pp_parameter_id;
            wsParam.subCount = param->sub_count();          // number of parameter sub-values in the scenario
            wsParam.defaultSubId = param->default_sub_id;   // sub-value id to be used by default for that parameter
            metaSet.worksetParam.push_back(wsParam);        // add parameter to workset

            // value notes for the parameter
            for (const auto& langSym : Symbol::pp_all_languages) {
                int lang_index = langSym->language_id; // 0-based
                const string& lang = langSym->name; // e.g. "EN" or "FR"
                WorksetParamTxtLangRow worksetParamTxt;
                assert(lang_index < (int)param->pp_value_notes.size()); // logic guarantee
                string value_note = param->pp_value_notes[lang_index];
                if (value_note.length() > 0) {
                    worksetParamTxt.paramId = wsParam.paramId;
                    worksetParamTxt.langCode = lang;
                    worksetParamTxt.note = value_note;
                    metaSet.worksetParamTxt.push_back(worksetParamTxt);
                }
            }
        }

        // insert model workset into SQLite database
        if (!builder->isSqliteDb()) {
            theLog->logMsg("warning : model SQLite database not created");
        }
        else {
            if (!Symbol::no_metadata) {
                // create new workset in model database from scenario parameters
                createWorkset(metaRows, metaSet, builder.get());
            }
        }
        if (!Symbol::no_metadata) {
            // build Modgen compatibilty views sql script
            builder->buildCompatibilityViews(metaRows);
        }

        // cleanup literals created from csv files
        for (auto & cp : cpLst) {
            if (cp->literal != nullptr) delete cp->literal;
        }

        // process additional scenario parameter directories
        if (paramDirLst.size() > 1) {

            if (Symbol::any_parameters_suppress || Symbol::any_parameters_retain) {
                throw HelperException(LT("Multiple scenarios are incompatible with parameters_suppress or parameters_retain"));
            }

            auto snIt = scNameLst.cbegin();
            if (snIt != scNameLst.cend()) ++snIt;

            for (auto pdIt = ++paramDirLst.cbegin(); pdIt != paramDirLst.cend(); ++pdIt) {

                // if scenario name supplied then use it else by default name is equal to directory stem
                string scName = "";
                if (snIt != scNameLst.cend()) scName = *snIt++;

                if (!Symbol::no_metadata) {
                    processExtraParamDir(*pdIt, scName, metaRows, builder.get());
                }
            }
        }

        // create the Model Metrics Report
        do_model_metrics_report(outDir, model_name, cg);

        // generate model documentation
        if (Symbol::model_doc || Symbol::model_devdoc) {
            theLog->logMsg("Generate model documentation - start");

            string omrootDir = omc_exe.substr(0, omc_exe.find_last_of("/\\") + 1) + "../";

            if (Symbol::model_doc) {
                // create doc version for users
                do_model_doc(false, outDir, omrootDir, model_name, cg);
            }
            if (Symbol::model_devdoc) {
                // create doc version for model developers
                do_model_doc(true, outDir, omrootDir, model_name, cg);
            }
            theLog->logMsg("Generate model documentation - finish");
        }

        // issue warnings for missing labels,notes,translations
        do_missing_documentation();
        
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

// check boolean option for correct value and return true, false or exception
// return true if option value is "true", "1", "yes" (case insensitive) or empty "" value
// return false if option is not specified or one of: "false", "0", "no"
// raise exception otherwise
static bool parseBoolOption(const char * i_key, const ArgReader & i_argStore)
{
    int nOpt = i_argStore.boolOptionToInt(i_key);
    if (nOpt < -1) {
        string sOpt = i_argStore.strOption(i_key);
        if (nOpt < 0) throw HelperException(LT("Invalid option value %s %s"), i_key, sOpt.c_str());
    }
    return nOpt > 0;
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
            // record the line count of the parsed file
            Symbol::source_files_line_count.insert({ full_name, pc.all_line_count });
            // record the syntactic island line count of the parsed file
            Symbol::source_files_island_line_count.insert({ full_name, pc.island_line_count });
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

// process extra parameter directory: 
// if any parameter csv files exist then read it and create additional (not default) workset in model database
static void processExtraParamDir(const string & i_paramDir, const string & i_scenarioName, const MetaModelHolder & i_metaRows, IModelBuilder * i_builder)
{
    theLog->logFormatted("Compile scenario parameters from: %s", i_paramDir.c_str());

    // only .csv or .tsv files allowed in additional parameters directory
    list<string> dat_files = listSourceFiles(i_paramDir, { ".dat", ".odat" });
    if (dat_files.size() > 0) {
        theLog->logFormatted("warning : skip %zd .dat or .odat parameter file(s) in %s", dat_files.size(), i_paramDir.c_str());
    }

    // clean previous parameter data
    for (const auto param : Symbol::pp_all_parameters) {
        if (param->source != ParameterSymbol::scenario_parameter && param->source != ParameterSymbol::missing_parameter) continue;

        param->source = ParameterSymbol::missing_parameter;
        param->sub_initial_list.resize(1);
        param->sub_initial_list.front().first = 0;
        param->sub_initial_list.front().second.clear();
    }

    // load parameters data from .csv or .tsv files
    forward_list<unique_ptr<Constant> > cpLst;

    readParameterCsvFiles(false, i_paramDir, cpLst);

    // Create working set for published scenario
    MetaSetLangHolder metaSet;
    metaSet.worksetRow.name = !i_scenarioName.empty() ? i_scenarioName : getFileNameStem(i_paramDir);

    for (auto param : Symbol::pp_all_parameters) {
        if (param->source != ParameterSymbol::scenario_parameter) continue; // write into db only scenario parameters

        WorksetParamRow wsParam;
        wsParam.paramId = param->pp_parameter_id;
        wsParam.subCount = param->sub_count();          // number of parameter sub-values in the scenario
        wsParam.defaultSubId = param->default_sub_id;   // sub-value id to be used by default for that parameter
        metaSet.worksetParam.push_back(wsParam);        // add parameter to workset

        // value notes for the parameter
        for (const auto& langSym : Symbol::pp_all_languages) {
            int lang_index = langSym->language_id; // 0-based
            const string& lang = langSym->name; // e.g. "EN" or "FR"
            WorksetParamTxtLangRow worksetParamTxt;
            assert(lang_index < (int)param->pp_value_notes.size()); // logic guarantee
            string value_note = param->pp_value_notes[lang_index];
            if (value_note.length() > 0) {
                worksetParamTxt.paramId = wsParam.paramId;
                worksetParamTxt.langCode = lang;
                worksetParamTxt.note = value_note;
                metaSet.worksetParamTxt.push_back(worksetParamTxt);
            }
        }
    }

    // if any scenario parameters found in create new workset in model database from scenario parameters
    if (metaSet.worksetParam.size() > 0) {
        theLog->logFormatted("Scenario processing: %s", metaSet.worksetRow.name.c_str());
        createWorkset(i_metaRows, metaSet, i_builder);
    }
    else {
        theLog->logFormatted("No scenario parameters found in: %s", i_paramDir.c_str());
    }

    // cleanup literals created from csv files
    for (auto & cp : cpLst) {
        if (cp->literal != nullptr) delete cp->literal;
    }
}

// create new workset in the model database from scenario parameters
static void createWorkset(const MetaModelHolder & i_metaRows, MetaSetLangHolder & io_metaSet, IModelBuilder * i_builder)
{
    // start model working set
    i_builder->beginWorkset(i_metaRows, io_metaSet);

    // add values for all scenario model parameters into working set
    size_t scenario_parameters_count = io_metaSet.worksetParam.size();
    int scenario_parameters_done = 0;

    chrono::system_clock::time_point start = chrono::system_clock::now();
    for (auto param : Symbol::pp_all_parameters) {

        if (param->source != ParameterSymbol::scenario_parameter) continue; // write into db only scenario parameters

        // insert all parameter sub-values, each parameter has atleast one (default) sub-value
        for (int k = 0; k < param->sub_count(); k++) {
            pair< int, list<string> > subValue = param->initializer_for_storage(k);
            i_builder->addWorksetParameter(i_metaRows, io_metaSet, param->name, subValue.first, subValue.second); // insert parameter sub-value(s)
        }
        scenario_parameters_done++;

        // report not more often than every 3 seconds
        chrono::system_clock::time_point now = chrono::system_clock::now();
        if (chrono::duration_cast<chrono::seconds>(now - start).count() >= 3) {
            start = now;
            theLog->logFormatted("%d of %zd parameters processed (%s)", scenario_parameters_done, scenario_parameters_count, param->name.c_str());
        }
    }
    theLog->logFormatted("%d of %zd parameters processed", scenario_parameters_done, scenario_parameters_count);

    // complete model working set
    theLog->logFormatted("Finalize scenario processing: %s", io_metaSet.worksetRow.name.c_str());
    i_builder->endWorkset(i_metaRows, io_metaSet);
}

