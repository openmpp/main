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
* * -OpenM.OptionsFile some/optional/omc.ini
* 
* Short form of command line arguments:
* * -i short form of -Omc.inputDir
* * -o short form of -Omc.outputDir
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
#include "Symbol.h"
#include "Driver.h"
#include "ParseContext.h"
#include "CodeGen.h"
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
#endif // _WIN32

using namespace std;

namespace openm
{
    /** keys for model run options */
    struct OmcArgKey
    {
        /** omc input directory with openM++ source files */
        static const char * inputDir;

        /** omc output directory for compiled files */
        static const char * outputDir;
    };

    /** keys for model run options (short form) */
    struct OmcShortKey
    {
        /** short name for options file name: -s fileName.ini */
        static const char * optionsFile;

        /** short name for omc input directory  */
        static const char * inputDir;

        /** short name for omc output directory */
        static const char * outputDir;
    };

    /** omc input directory with openM++ source files */
    const char * OmcArgKey::inputDir = "Omc.InputDir";

    /** omc output directory for compiled files */
    const char * OmcArgKey::outputDir = "Omc.OutputDir";

    /** short name for options file name: -s fileName.ini */
    const char * OmcShortKey::optionsFile = "s";

    /** short name for omc input directory  */
    const char * OmcShortKey::inputDir = "i";

    /** short name for omc output directory */
    const char * OmcShortKey::outputDir = "o";

    /** array of model run option keys. */
    static const char * runArgKeyArr[] = {
        OmcArgKey::inputDir,
        OmcArgKey::outputDir,
        ArgKey::optionsFile,
        ArgKey::logToConsole,
        ArgKey::logToFile,
        ArgKey::logFilePath,
        ArgKey::logToStamped,
        ArgKey::logUseTs,
        ArgKey::logUsePid,
        ArgKey::logNoTimeConsole,
        ArgKey::logSql
    };
    static const size_t runArgKeySize = sizeof(runArgKeyArr) / sizeof(const char *);

    /** array of short and full option names, used to find full option name by short. */
    static const pair<const char *, const char *> shortPairArr[] = 
    {
        make_pair(OmcShortKey::optionsFile, ArgKey::optionsFile),
        make_pair(OmcShortKey::inputDir, OmcArgKey::inputDir),
        make_pair(OmcShortKey::outputDir, OmcArgKey::outputDir)
    };
    static const size_t shortPairSize = sizeof(shortPairArr) / sizeof(const pair<const char *, const char *>);
}

// get list of source (.mpp .ompp .dat) files from specified directory or current directory if source path is empty
static list<string> listSourceFiles(const string & i_srcDir);

// write string line into new output file
static void writeLinesToFile(const string & i_filePath, const vector<string> & i_lineVec);

// write text string into new output file
static void writeToFile(const string & i_filePath, const string & i_fileContent);

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
            argStore.boolOption(ArgKey::logNoTimeConsole) || !argStore.isOptionExist(ArgKey::logNoTimeConsole),
            argStore.boolOption(ArgKey::logSql)
            );
        theLog->logMsg("Start omc");

        // get list of source file names in specified directory or current directory by default
        string inpDir = argStore.strOption(OmcArgKey::inputDir);
        bool isFromCurrent = inpDir == "" || inpDir == ".";

        if (!isFromCurrent) theLog->logFormatted("Compile from: %s", inpDir.c_str());

        list<string> source_files = listSourceFiles(inpDir);     // list of source file names

        if (source_files.empty()) {
            theLog->logMsg("No source files found, nothing to compile at current directory");
            return EXIT_FAILURE;
        }

        // "normalize" input and output directories: 
        // use empty "" if it is current directory else make sure it is end with /
        if (isFromCurrent) {
            inpDir = "";
        }
        else {
            if (inpDir.back() != '/' && inpDir.back() != '\\') inpDir += '/';
        }

        string outDir = argStore.strOption(OmcArgKey::outputDir);
        bool isToCurrent = outDir == "" || outDir == ".";

        if (!isToCurrent) theLog->logFormatted("Compile into: %s", outDir.c_str());

        if (isToCurrent) {
            outDir = "";
        }
        else {
            if (outDir.back() != '/' && outDir.back() != '\\') outDir += '/';
        }

        // Obtain location of the 'use' folder.
        // TODO: normalize path when std::filesystem available, using
        //    boost::filesystem::system_complete(argv[0]);
        //    and split into components using std::filesystem.
        string omc_exe = argv[0];
        Symbol::use_folder = omc_exe.substr(0, omc_exe.find_last_of("/\\") + 1) + "../use/";

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
        for (const string & name : source_files) {
            Symbol::all_source_files.push_back(inpDir + name);
        }

        for (string & name : Symbol::all_source_files) {
            try {
                theLog->logFormatted("Parsing %s", name.c_str());

                // create new instance of parser-scanner driver for each source file
                Driver drv( pc );

                // set to true to see detailed scanning actions
                drv.trace_scanning = false;

                // set to true to see detailed parsing actions
                drv.trace_parsing = false;

                // split source file name into path, stem, and extension (lower-case normalized)
                // at some point use std::filesystem
                
                // length of optional path part, including final trailing slash
                int in_path_len = name.find_last_of("/\\");
                if (in_path_len > 0) in_path_len++;

                // position of stem part
                int in_stem_pos = in_path_len;

                // position of extension (0 if no extension)
                int in_ext_pos = name.find_last_of(".");
                if (in_ext_pos < in_path_len) in_ext_pos = 0; // ignore final . if in path portion

                // length of extension
                int in_ext_len = (in_ext_pos == 0) ? 0 : name.length() - in_ext_pos;

                // length of stem
                int in_stem_len = name.length() - in_path_len - in_ext_len;

                string in_path = name.substr(0, in_path_len);
                string in_stem = name.substr(in_stem_pos, in_stem_len);
                string in_ext = name.substr(in_ext_pos, in_ext_len);
                openm::toLower(in_ext);

                drv.parse(&name, in_stem + in_ext, in_stem, &om_developer_cpp);
            }
            catch(exception & ex) {
                theLog->logErr(ex);
                if ( pc.parse_errors == 0 ) {
                    pc.parse_errors = 1;
                }
                // continue parsing any remaining source code modules to detect additional syntax errors (possibly non-spurious)
            }
        }

        if ( pc.parse_errors > 0 ) {
            theLog->logFormatted("%d syntax errors in parse phase", pc.parse_errors);
            throw HelperException("Finish omc");
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

        if (Symbol::post_parse_errors > 0) {
            theLog->logFormatted("%d semantic errors in post-parse phase", Symbol::post_parse_errors);
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

#if defined(_MSC_VER)
        // UTF-8 BOM for Microsoft compiler
        om_types0_h << "\xEF\xBB\xBF";
        om_declarations_h << "\xEF\xBB\xBF";
        om_definitions_cpp << "\xEF\xBB\xBF";
#endif
        // collect model metadata during code generation
        MetaModelHolder metaRows;
        unique_ptr<IModelBuilder> builder(IModelBuilder::create());

        CodeGen cg(&om_types0_h, &om_types1_h, &om_declarations_h, &om_definitions_cpp, builder->timeStamp(), metaRows);
        cg.do_all();

        // build model creation script and save it
        theLog->logMsg("Meta-data processing");
        vector<string> scriptLines = builder->build(metaRows);
        writeLinesToFile(outDir + metaRows.modelDic.name + "_create_model.sql", scriptLines);
        
        // build Modgen views creation script and save
        scriptLines = builder->buildCompatibilityViews(metaRows);
        writeLinesToFile(outDir + metaRows.modelDic.name + "_optional_views.sql", scriptLines);

        // debug only: create model default parameters script from template
        string srcInsertParameters = metaRows.modelDic.name + "_insert_parameters.sql_template";

        string scriptContent = builder->buildInsertParameters(metaRows, inpDir + srcInsertParameters);
        if (!scriptContent.empty()) {
            writeToFile(outDir + metaRows.modelDic.name + "_insert_parameters.sql", scriptContent);
        }
        else {
            theLog->logFormatted("Insert parameters sql template file not found (or empty): %s", srcInsertParameters.c_str());
        }
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

// get list of source (.mpp .ompp .dat) files from specified directory or current directory if source path is empty
list<string> listSourceFiles(const string & i_srcPath)
{
    using namespace openm;
    list<string> nameLst;
    
    // open source directory or current directory if source path is empty
    string srcPath = !i_srcPath.empty() ? i_srcPath : ".";
    DIR * dir = opendir(srcPath.c_str());
    if (dir == NULL) throw HelperException("Can not open source directory: %s", srcPath.c_str());

    // collect list of .mpp, .ompp or .dat files
    try {
        dirent * ent;

        while ((ent = readdir (dir)) != NULL) {

            if (ent->d_type != DT_REG || ent->d_name == NULL) continue; // skip directories, special files and file name errors

            string name = ent->d_name;
            if (endWithNoCase(name, ".mpp") || endWithNoCase(name, ".ompp") || endWithNoCase(name, ".dat")) {
                nameLst.push_back(name);
            }
        }
    }
    catch(...) {
        closedir(dir);  // close directory on error
        throw;
    }
    
    // sort source files in alphabetical order for reproducibility
    nameLst.sort();
    return nameLst;
}

// write string line into new output file
void writeLinesToFile(const string & i_filePath, const vector<string> & i_lineVec)
{
    using namespace openm;

    ofstream ost;
    exit_guard<ofstream> onExit(&ost, &ofstream::close);   // close on exit

    ost.open(i_filePath, ios_base::out | ios_base::trunc);
    if(ost.fail()) throw HelperException("Failed to create file: %s", i_filePath.c_str());

    for (const string & line : i_lineVec) {
        ost << line << '\n';
        if(ost.fail()) throw HelperException("Failed to write into file: %s", i_filePath.c_str());
    }
}

// write text string into new output file
void writeToFile(const string & i_filePath, const string & i_fileContent)
{
    using namespace openm;

    ofstream ost;
    exit_guard<ofstream> onExit(&ost, &ofstream::close);   // close on exit

    ost.open(i_filePath, ios_base::out | ios_base::trunc | ios_base::binary);
    if(ost.fail()) throw HelperException("Failed to create file: %s", i_filePath.c_str());

    ost << i_fileContent;
    if(ost.fail()) throw HelperException("Failed to write into file: %s", i_filePath.c_str());
}
