/**
* @file    omc.cpp
* OpenM++ compiler (omc): main entry point
*  
* @mainpage OpenM++ compiler (omc)
* 
* OpenM++ compiler produce c++ (.cpp and .h) files from .ompp, .mpp or .dat model files. \n
*
* Following command line arguments supported by omc:
* * -Omc.InputDir  input/dir/to/find/source/files
* * -Omc.OutputDir output/dir/to/place/compiled/cpp_and_h/files
* * -Omc.UseDir    use/dir/with/ompp/files
* * -Omc.ParamDir  input/dir/to/find/parameter/files/for/scenario
* * -Omc.FixedDir  input/dir/to/find/fixed/parameter/files/
* * -OpenM.OptionsFile some/optional/omc.ini
* 
* Short form of command line arguments:
* * -i short form of -Omc.InputDir
* * -o short form of -Omc.OutputDir
* * -u short form of -Omc.UseDir
* * -p short form of -Omc.ParamDir
* * -f short form of -Omc.FixedDir
* * -s short form of -OpenM.OptionsFile
* 
* Also common OpenM log options supported: 
*   OpenM.LogToConsole, OpenM.LogToFile, OpenM.LogFilePath, 
*   OpenM.LogToStampedFile, OpenM.LogUseTimeStam, OpenM.LogUsePidStamp, OpenM.LogSql. 
* Please see OpenM++ wiki for more information about log options. \n
* 
* Parameters can be specified on command line or through ini-file. 
* Command line parameters take precedence.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <iostream>
#include <fstream>
#include <list>
#include <cctype>
#include "Symbol.h"
#include "Driver.h"
#include "ParseContext.h"
#include "CodeGen.h"
#include "libopenm/common/omHelper.h"
#include "libopenm/common/argReader.h"
#include "libopenm/db/modelBuilder.h"

// using freeware implementation of dirent.h for VisualStudio because MS stop including it
// it must be replaced with std::filesystem as soon it comes into std
#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <Windows.h>
    #include "dirent/dirent.h"
#else
    #include <dirent.h>
    #include <sys/stat.h>
#endif // _WIN32

using namespace std;

namespace openm
{
    /** keys for omc options */
    struct OmcArgKey
    {
        /** omc input directory with openM++ source files */
        static const char * inputDir;

        /** omc output directory for compiled files */
        static const char * outputDir;

        /** omc use directory to resolve 'use' statements */
        static const char * useDir;

        /** omc input directory with OpenM++ parameter files */
        static const char * paramDir;

        /** omc input directory with OpenM++ fixed parameter files */
        static const char * fixedDir;
    };

    /** keys for omc options (short form) */
    struct OmcShortKey
    {
        /** short name for options file name: -s fileName.ini */
        static const char * optionsFile;

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

    /** short name for options file name: -s fileName.ini */
    const char * OmcShortKey::optionsFile = "s";

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
        OmcArgKey::inputDir,
        OmcArgKey::outputDir,
        OmcArgKey::useDir,
        OmcArgKey::paramDir,
        OmcArgKey::fixedDir,
        ArgKey::optionsFile,
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
        make_pair(OmcShortKey::optionsFile, ArgKey::optionsFile),
        make_pair(OmcShortKey::inputDir, OmcArgKey::inputDir),
        make_pair(OmcShortKey::outputDir, OmcArgKey::outputDir),
        make_pair(OmcShortKey::useDir, OmcArgKey::useDir),
        make_pair(OmcShortKey::paramDir, OmcArgKey::paramDir),
        make_pair(OmcShortKey::fixedDir, OmcArgKey::fixedDir),
    };
    static const size_t shortPairSize = sizeof(shortPairArr) / sizeof(const pair<const char *, const char *>);
}

// get list of files matching extension list from specified directory or current directory if source path is empty
// each file name in result is a relative path and include source directory
static list<string> listSourceFiles(const string & i_srcPath, const list<string> & i_extensions);

// get extension of filename
static string getFileNameExt(const string &file_name);

// get stem of filename
static string getFileNameStem(const string &file_name);

// Parse a list of files
static void parseFiles(list<string> & files, const list<string>::iterator start_it, ParseContext & pc, ofstream *markup_stream);

int main(int argc, char * argv[])
{
    using namespace openm;
    try {
        // get omc run options from command line and ini-file
        ArgReader argStore(argc, argv, runArgKeySize, runArgKeyArr, shortPairSize, shortPairArr);

        // adjust log file(s) with actual log settings specified on command line or at ini-file
        theLog->init(
            argStore.boolOption(ArgKey::logToConsole) || !argStore.isOptionExist(ArgKey::logToConsole),
            argStore.strOption(ArgKey::logFilePath).c_str(),
            argStore.boolOption(ArgKey::logUseTs),
            argStore.boolOption(ArgKey::logUsePid),
            argStore.boolOption(ArgKey::logNoMsgTime) || !argStore.isOptionExist(ArgKey::logNoMsgTime),
            argStore.boolOption(ArgKey::logSql)
            );
        theLog->logMsg("Start omc");

        // get list of source file names in specified directory or current directory by default
        string inpDir = argStore.strOption(OmcArgKey::inputDir);
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
        bool isToCurrent = outDir == "" || outDir == ".";

        if (!isToCurrent) theLog->logFormatted("Compile source into: %s", outDir.c_str());

        if (isToCurrent) {
            outDir = "";
        }
        else {
            if (outDir.back() != '/' && outDir.back() != '\\') outDir += '/';
        }

        // Obtain location of the 'use' folder and make available to parser.
        if (argStore.isOptionExist(OmcArgKey::useDir)) {
            Symbol::use_folder = argStore.strOption(OmcArgKey::useDir);
        }
        else {
            string omc_exe = argv[0];
            Symbol::use_folder = omc_exe.substr(0, omc_exe.find_last_of("/\\") + 1) + "../use/";
        }

        if (!Symbol::use_folder.empty() && Symbol::use_folder.back() != '/' && Symbol::use_folder.back() != '\\') {
            Symbol::use_folder += '/';
        }

        // Populate symbol table with default symbols
        Symbol::populate_default_symbols();

        // create unique instance of ParseContext
        ParseContext pc;

        // open & prepare pass-through / markup stream om_developer.cpp
        ofstream om_developer_cpp(outDir + "om_developer.cpp", ios_base::out | ios_base::trunc | ios_base::binary);
        exit_guard<ofstream> onExit_om_developer_cpp(&om_developer_cpp, &ofstream::close);   // close on exit
        if (om_developer_cpp.fail()) throw HelperException("Unable to open %s for writing", "om_developer.cpp");

    #if defined(_MSC_VER)
        // UTF-8 BOM for Microsoft compiler
        om_developer_cpp << "\xEF\xBB\xBF";
    #endif
        om_developer_cpp << "/**" << endl;
        om_developer_cpp << " * @file om_developer.cpp" << endl;
        om_developer_cpp << " * Developer-supplied C++ functions" << endl;
        om_developer_cpp << " */" << endl;
        om_developer_cpp << "" << endl;
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
        if (argStore.isOptionExist(OmcArgKey::paramDir)) {
            // -p scenario parameters specified
            string paramDir = argStore.strOption(OmcArgKey::paramDir);
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

        if (Symbol::post_parse_warnings > 0) {
            theLog->logFormatted("%d warnings in post-parse phase", Symbol::post_parse_warnings);
        }
        if (Symbol::post_parse_errors > 0) {
            theLog->logFormatted("%d errors in post-parse phase", Symbol::post_parse_errors);
            throw HelperException("Finish omc");
        }

        theLog->logMsg("Code & meta-data generation");

        // open output streams for generated code
        ofstream om_types0_h(outDir + "om_types0.h", ios_base::out | ios_base::trunc | ios_base::binary);
        exit_guard<ofstream> onExit_om_types0_h(&om_types0_h, &ofstream::close);   // close on exit
        if (om_types0_h.fail()) throw HelperException("Unable to open %s for writing", "om_types0.h");

        ofstream om_types1_h(outDir + "om_types1.h", ios_base::out | ios_base::trunc | ios_base::binary);
        exit_guard<ofstream> onExit_om_types1_h(&om_types1_h, &ofstream::close);   // close on exit
        if (om_types1_h.fail()) throw HelperException("Unable to open %s for writing", "om_types1.h");

        ofstream om_declarations_h(outDir + "om_declarations.h", ios_base::out | ios_base::trunc | ios_base::binary);
        exit_guard<ofstream> onExit_om_declarations_h(&om_declarations_h, &ofstream::close);   // close on exit
        if (om_declarations_h.fail()) throw HelperException("Unable to open %s for writing", "om_declarations.h");

        ofstream om_definitions_cpp(outDir + "om_definitions.cpp", ios_base::out | ios_base::trunc | ios_base::binary);
        exit_guard<ofstream> onExit_om_definitions_cpp(&om_definitions_cpp, &ofstream::close);   // close on exit
        if (om_definitions_cpp.fail()) throw HelperException("Unable to open %s for writing", "om_definitions.cpp");

        ofstream om_fixed_parms_cpp(outDir + "om_fixed_parms.cpp", ios_base::out | ios_base::trunc | ios_base::binary);
        exit_guard<ofstream> onExit_om_fixed_parms_cpp(&om_fixed_parms_cpp, &ofstream::close);   // close on exit
        if (om_fixed_parms_cpp.fail()) throw HelperException("Unable to open %s for writing", "om_fixed_parms.cpp");

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
        unique_ptr<IModelBuilder> builder(IModelBuilder::create(outDir));

        CodeGen cg(&om_types0_h, &om_types1_h, &om_declarations_h, &om_definitions_cpp, &om_fixed_parms_cpp, builder->timeStamp(), metaRows);
        cg.do_all();

        // build model creation sql script
        theLog->logMsg("Meta-data processing");
        builder->build(metaRows);
        
        // create model default working set sql script
        // 
        // builder->beginWorkset(metaRows, metaSet);
        // 
        // for (...all model parameters...) {
        //     ...get parameter value(s) from .dat file...
        //     if (isScalarParameter) {
        //         builder->addWorksetParameter(metaRows, parameter_name, value_as_char_ptr);
        //     }
        //     else {
        //         builder->addWorksetParameter(metaRows, parameter_name, values_vector_of_char_ptrs);
        //     }
        // }
        // 
        // builder->endWorkset();
        // 
//#define TEST_PARAMETERS_SCRIPT 
#ifdef TEST_PARAMETERS_SCRIPT

        MetaSetLangHolder metaSet;  // default working set metadata

        metaSet.worksetRow.name = "my set of default parameters";

        // optional:
        // add description and notes for each model language
        for (ModelDicTxtLangRow mTxtRow : metaRows.modelTxt) {
            WorksetTxtLangRow wsTxt;
            wsTxt.langName = mTxtRow.langName;
            wsTxt.descr = mTxtRow.langName + " some description";
            wsTxt.note = mTxtRow.langName + " some notes";

            metaSet.worksetTxt.push_back(wsTxt);
        }

        // default workset must include ALL model parameters
        for (ParamDicRow paramRow : metaRows.paramDic) {
            WorksetParamRow wsParam;
            wsParam.paramId = paramRow.paramId;

            metaSet.worksetParam.push_back(wsParam);  // add parameter to workset

            // optional:
            // add workset parameter notes for each model language
            for (ModelDicTxtLangRow mTxtRow : metaRows.modelTxt) {
                WorksetParamTxtLangRow paramTxt;
                paramTxt.paramId = paramRow.paramId;
                paramTxt.langName = mTxtRow.langName;
                paramTxt.note = mTxtRow.langName + " parameter value notes for parameter " + to_string(paramRow.paramId);

                metaSet.worksetParamTxt.push_back(paramTxt);
            }
        }

        // start model default working set sql script
        builder->beginWorkset(metaRows, metaSet);

        // add values for ALL model parameters into default working set
        chrono::milliseconds msTotal = chrono::milliseconds(0);
        for (ParamDicRow paramRow : metaRows.paramDic) {
        
            // generate parameter test data:
            // assuming it is coming as result of .dat files parsing
            string parsedValue_10("10");
            string parsedValue_20("20");
            string parsedValue_30("30");
            string parsedValue_scalar("1");
            string parsedValue_file("some/path/to/file.pa'ra'meter");

            // generate parameter test data:
            // check is this a file parameter
            // code below is for test only, binary search used in production
            bool isFileParam = false;
            for (TypeDicRow typeRow : metaRows.typeDic) {
                if (typeRow.typeId == paramRow.typeId) { 
                    isFileParam = equalNoCase(typeRow.name.c_str(), "file");
                    break;
                }
            }

            // add parameter values to default workset
            if (paramRow.rank <= 0) {   // scalar parameter

                // generate parameter test data:
                const string & paramValue = isFileParam ? parsedValue_file : parsedValue_scalar;

                // add scalar parameter value to default workset
                builder->addWorksetParameter(metaRows, paramRow.paramName.c_str(), paramValue);
            }
            else {
                // generate parameter test data:
                // calculte size of parameter
                // code below is for test only, binary search used in production
                size_t totalSize = 1;
                for (ParamDimsRow dimRow : metaRows.paramDims) {
                    if (dimRow.paramId != paramRow.paramId) continue;

                    size_t dimSize = 0;
                    for (TypeEnumLstRow enumRow : metaRows.typeEnum) {
                        if (enumRow.typeId == dimRow.typeId) dimSize++;
                    }
                    if (dimSize > 0) totalSize *= dimSize;
                }

                // generate parameter test data:
                list<string *> valueLst;
                for (size_t k = 0; k < totalSize; k++) {
                    valueLst.push_back(
                        (k % 3) ? ((k % 2) ? &parsedValue_10 : &parsedValue_20) : &parsedValue_30
                        );
                }

                chrono::system_clock::time_point tStart = chrono::system_clock::now();

                // add array parameter value to default workset
                builder->addWorksetParameter(metaRows, paramRow.paramName.c_str(), valueLst);
                
                chrono::system_clock::time_point tStop = chrono::system_clock::now();
                chrono::seconds sec = chrono::duration_cast<chrono::seconds>(tStop - tStart);
                chrono::milliseconds ms = chrono::duration_cast<chrono::milliseconds>(tStop - tStart);
                msTotal = msTotal + ms;
                theLog->logFormatted("%5d %40s %8ld %8lld : %5lld", paramRow.paramId, paramRow.paramName.c_str(), totalSize, ms.count(), sec.count());
            }
        }
        chrono::seconds secTotal = chrono::duration_cast<chrono::seconds>(msTotal);
        theLog->logFormatted("total: %5d = %20lld : %8lld seconds", metaRows.paramDic.size(), msTotal.count(), secTotal.count());

        // complete model default working set sql script
        builder->endWorkset();

#endif  // TEST_PARAMETERS_SCRIPT

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
            *markup_stream << "#line 1 \"" << normalized_full_name << "\"" << endl;

            // create new instance of parser-scanner driver for each source file
            Driver drv( pc );
            drv.trace_scanning = false; // set to true to see detailed scanning actions
            drv.trace_parsing = false; // set to true to see detailed parsing actions
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
        throw HelperException("Finish omc");
    }
}

// get list of files matching extension list from specified directory or current directory if source path is empty
// each file name in result is a relative path and include source directory
static list<string> listSourceFiles(const string & i_srcPath, const list<string> & i_extensions)
{
    using namespace openm;
    list<string> pathLst;
    
    // open source directory or current directory if source path is empty
    string srcPath = !i_srcPath.empty() ? i_srcPath : ".";
    DIR * dir = opendir(srcPath.c_str());
    if (dir == NULL) throw HelperException("Can not open source directory: %s", srcPath.c_str());

    // collect list of .mpp, .ompp or .dat files
    try {
        dirent * ent;
        struct stat fileStat;
        string path;

        string basePath = i_srcPath;
        if (!basePath.empty() && basePath.back() != '/' && basePath.back() != '\\') basePath += '/';

        while ((ent = readdir(dir)) != NULL) {

            if (ent->d_name == NULL) continue;  // skip file name errors

            path = basePath + ent->d_name;     // include directory into result

            // skip directories and special files
            if (ent->d_type != DT_REG) {
                if (ent->d_type != DT_UNKNOWN) continue;            // skip directories and special files
                if (stat(path.c_str(), &fileStat) != 0) continue;   // we can't get file type
                if (!S_ISREG(fileStat.st_mode)) continue;           // skip directories and special files
            }

            for (auto ext : i_extensions) {
                if (endWithNoCase(path, ext.c_str())) {
                    pathLst.push_back(path);
                    break;
                }
            }
        }
    }
    catch(...) {
        closedir(dir);  // close directory on error
        throw;
    }
    
    // sort source files in alphabetical order for reproducibility
    pathLst.sort();
    return pathLst;
}

// get extension of filename
static string getFileNameExt(const string &file_name)
{
    // length of optional path part, including final trailing slash
    int in_path_len = file_name.find_last_of("/\\");
    if (in_path_len == -1) in_path_len = 0;
    if (in_path_len > 0) in_path_len++;

    // position of stem part
    int in_stem_pos = in_path_len;

    // position of extension (0 if no extension)
    int in_ext_pos = file_name.find_last_of(".");
    if (in_ext_pos < in_path_len) in_ext_pos = 0; // ignore final . if in path portion

    // length of extension
    int in_ext_len = (in_ext_pos == 0) ? 0 : file_name.length() - in_ext_pos;

    // length of stem
    int in_stem_len = file_name.length() - in_path_len - in_ext_len;

    string in_path = file_name.substr(0, in_path_len);
    string in_stem = file_name.substr(in_stem_pos, in_stem_len);
    string in_ext = file_name.substr(in_ext_pos, in_ext_len);
    openm::toLower(in_ext);

    return in_ext;
}

// get stem of filename
static string getFileNameStem(const string &file_name)
{
    // length of optional path part, including final trailing slash
    int in_path_len = file_name.find_last_of("/\\");
    if (in_path_len == -1) in_path_len = 0;
    if (in_path_len > 0) in_path_len++;

    // position of stem part
    int in_stem_pos = in_path_len;

    // position of extension (0 if no extension)
    int in_ext_pos = file_name.find_last_of(".");
    if (in_ext_pos < in_path_len) in_ext_pos = 0; // ignore final . if in path portion

    // length of extension
    int in_ext_len = (in_ext_pos == 0) ? 0 : file_name.length() - in_ext_pos;

    // length of stem
    int in_stem_len = file_name.length() - in_path_len - in_ext_len;

    string in_path = file_name.substr(0, in_path_len);
    string in_stem = file_name.substr(in_stem_pos, in_stem_len);
    string in_ext = file_name.substr(in_ext_pos, in_ext_len);
    openm::toLower(in_ext);

    return in_stem;
}
