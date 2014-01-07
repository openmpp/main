/**
* @file    omc.cpp
* OpenM++ compiler (omc): main entry point
*  
* @mainpage OpenM++ compiler (omc)
* 
* OpenM++ compiler produce c++ (.cpp and .h) files from .ompp or .mpp model files. \n
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
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <iostream>
#include <fstream>
#include <list>
#include "Driver.h"
#include "ParseContext.h"
#include "CodeGen.h"
#include "libopenm/common/argReader.h"
#include "libopenm/db/modelBuilder.h"

// using freeware implementation of dirent.h for VisualStudio because MS stop including it
// it must be replaced with std::filesystem as soon it comes into std
#ifdef _WIN32
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

// get list of source (.mpp or .ompp) files from specified directory or current directory if source path is empty
static list<string> listSourceFiles(const string & i_srcDir);

// write string line into new output file
static void writeLinesToFile(const string & i_filePath, const vector<string> & i_lineVec);

// write text string into new output file
static void writeToFile(const string & i_filePath, const string & i_fileContent);

// debug only
// add test metadata to produce at least some output
static void addTestMetadata(openm::MetaModelHolder & io_metaRows);

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
            nullptr,
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

        // Populate symbol table with default symbols
        Symbol::default_symbols();

        // create unique instance of ParseContext
        ParseContext pc;

        // open & prepare pass-through / markup stream om_outside.cpp
        ofstream om_outside_cpp;
        exit_guard<ofstream> onExit_om_outside_cpp(&om_outside_cpp, &ofstream::close);   // close on exit
        om_outside_cpp.open(outDir + "om_outside.cpp", ios_base::out | ios_base::trunc | ios_base::binary);

    #if defined(_MSC_VER)
        // UTF-8 BOM for Microsoft compiler
        om_outside_cpp << "\xEF\xBB\xBF";
    #endif
        om_outside_cpp << "/**" << endl;
        om_outside_cpp << " * @file om_outside.cpp" << endl;
        om_outside_cpp << " * Developer-supplied C++ functions" << endl;
        om_outside_cpp << " */" << endl;
        om_outside_cpp << "" << endl;
        om_outside_cpp << "#include \"omc/omSimulation.h\"" << endl;
		om_outside_cpp << "using namespace openm;" << endl;
        om_outside_cpp << "namespace mm {" << endl;

        for (const string & name : source_files) {
            try {
                theLog->logFormatted("Parsing %s", name.c_str());

                // create new instance of parser-scanner driver for each source file
                Driver drv( pc );

                // set to true to see detailed scanning actions
                drv.trace_scanning = false;

                // set to true to see detailed parsing actions
                drv.trace_parsing = false;

                string in_stem = name;
                if (endWithNoCase(name, ".mpp")) in_stem = name.substr(0, name.length() - 4);
                if (endWithNoCase(name, ".ompp")) in_stem = name.substr(0, name.length() - 5);

                drv.parse( inpDir + name, in_stem, &om_outside_cpp);
            }
            catch(exception & ex) {
                theLog->logErr(ex);
                if ( pc.parse_errors == 0 ) {
                    pc.parse_errors = 1;
                }
                // continue parsing any remaining source code modules to detect additional syntax errors (possibly non-spurious)
            }
        }

        om_outside_cpp << "} //namespace mm" << endl;

        if ( pc.parse_errors > 0 ) {
            theLog->logFormatted("Syntax errors (%d) in parse phase", pc.parse_errors);
            throw HelperException("Finish omc");
        }

        try {
            theLog->logMsg("Post-parse processing");
            Symbol::post_parse_all();
        }
        catch(exception & ex) {
            theLog->logErr(ex);
            if ( pc.post_parse_errors == 0 ) {
                pc.post_parse_errors = 1;
            }
            theLog->logFormatted("Syntax errors (%d) in post-parse phase", pc.post_parse_errors);
            throw HelperException("Finish omc");
        }

        theLog->logMsg("Code generation");

        // open output streams for generated code
        ofstream om_agents_t;
        exit_guard<ofstream> onExit_om_agents_t(&om_agents_t, &ofstream::close);   // close on exit
        om_agents_t.open(outDir + "om_types.h", ios_base::out | ios_base::trunc | ios_base::binary);

        ofstream om_agents_h;
        exit_guard<ofstream> onExit_om_agents_h(&om_agents_h, &ofstream::close);   // close on exit
        om_agents_h.open(outDir + "om_agents.h", ios_base::out | ios_base::trunc | ios_base::binary);

        ofstream om_agents_cpp;
        exit_guard<ofstream> onExit_om_agents_cpp(&om_agents_cpp, &ofstream::close);   // close on exit
        om_agents_cpp.open(outDir + "om_agents.cpp", ios_base::out | ios_base::trunc | ios_base::binary);

#if defined(_MSC_VER)
        // UTF-8 BOM for Microsoft compiler
        om_agents_t << "\xEF\xBB\xBF";
        om_agents_h << "\xEF\xBB\xBF";
        om_agents_cpp << "\xEF\xBB\xBF";
#endif
        // collect model metadata during code generation
        MetaModelHolder metaRows;
        unique_ptr<IModelBuilder> builder(IModelBuilder::create());
        
        CodeGen cg( &om_agents_t, &om_agents_h, &om_agents_cpp, builder->timeStamp(), metaRows );
        cg.do_all();

        // debug only: add test metadata to produce at least some output
        addTestMetadata(metaRows);

        // build model creation script and save it
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

// get list of source (.mpp or .ompp) files from specified directory or current directory if source path is empty
list<string> listSourceFiles(const string & i_srcPath)
{
    using namespace openm;
    list<string> nameLst;

    // open source directory or current directory if source path is empty
    string srcPath = !i_srcPath.empty() ? i_srcPath : ".";
    DIR * dir = opendir(srcPath.c_str());
    if (dir == NULL) throw HelperException("Can not open source directory: %s", srcPath.c_str());

    // collect list of .mpp or .ompp files
    try {
        dirent * ent;

        while ((ent = readdir (dir)) != NULL) {

            if (ent->d_type != DT_REG || ent->d_name == NULL) continue; // skip directories, special files and file name errors

            string name = ent->d_name;
            if (endWithNoCase(name, ".mpp") || endWithNoCase(name, ".ompp")) nameLst.push_back(name);
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

// debug only
// add test metadata to produce at least some output
// we can introduce constructors for every xxxxRow struct below
// to avoid multiline assignments, here is just a test
// 
// All strings expected to be UTF-8, it can be converted to UTF-8 as:
//   str = toUtf8(source);
// or better:
//   fileContent = fileToUtf8(path);
void addTestMetadata(openm::MetaModelHolder & io_metaRows)
{
    using namespace openm;

    // model_dic table
    //io_metaRows.modelDic.type = ModelType::caseBased;
    io_metaRows.modelDic.version = "test";

    // model_dic_txt table
    ModelDicTxtLangRow modelTxt;
    modelTxt.langName = "EN";
    modelTxt.descr = "model description (EN)";
    modelTxt.note = "model notes  (EN)";
    io_metaRows.modelTxt.push_back(modelTxt);

    modelTxt.langName = "FR";
    modelTxt.descr = "model description (FR)";
    modelTxt.note = "model notes (FR)";
    io_metaRows.modelTxt.push_back(modelTxt);

    // built-in types: simple types
    TypeDicRow typeDic;
    typeDic.typeId = 0;
    typeDic.name = "int";
    typeDic.dicId = 0;          // simple types
    typeDic.totalEnumId = 1;
    io_metaRows.typeDic.push_back(typeDic);

    typeDic.typeId = 1;
    typeDic.name = "char";
    io_metaRows.typeDic.push_back(typeDic);

    typeDic.typeId = 2;
    typeDic.name = "short";
    io_metaRows.typeDic.push_back(typeDic);

    typeDic.typeId = 3;
    typeDic.name = "long";
    io_metaRows.typeDic.push_back(typeDic);

    typeDic.typeId = 4;
    typeDic.name = "uint";
    io_metaRows.typeDic.push_back(typeDic);

    typeDic.typeId = 5;
    typeDic.name = "uchar";
    io_metaRows.typeDic.push_back(typeDic);

    typeDic.typeId = 6;
    typeDic.name = "ushort";
    io_metaRows.typeDic.push_back(typeDic);

    typeDic.typeId = 7;
    typeDic.name = "ulong";
    io_metaRows.typeDic.push_back(typeDic);

    typeDic.typeId = 8;
    typeDic.name = "integer";
    io_metaRows.typeDic.push_back(typeDic);

    typeDic.typeId = 9;
    typeDic.name = "counter";
    io_metaRows.typeDic.push_back(typeDic);

    typeDic.typeId = 10;
    typeDic.name = "real";
    io_metaRows.typeDic.push_back(typeDic);

    typeDic.typeId = 11;
    typeDic.name = "float";
    io_metaRows.typeDic.push_back(typeDic);

    typeDic.typeId = 12;
    typeDic.name = "double";
    io_metaRows.typeDic.push_back(typeDic);

    typeDic.typeId = 13;
    typeDic.name = "TIME";
    io_metaRows.typeDic.push_back(typeDic);

    typeDic.typeId = 14;
    typeDic.name = "rate";
    io_metaRows.typeDic.push_back(typeDic);

    typeDic.typeId = 15;
    typeDic.name = "cumrate";
    io_metaRows.typeDic.push_back(typeDic);

    typeDic.typeId = 16;
    typeDic.name = "haz1rate";
    io_metaRows.typeDic.push_back(typeDic);

    typeDic.typeId = 17;
    typeDic.name = "haz2rate";
    io_metaRows.typeDic.push_back(typeDic);

    typeDic.typeId = 18;
    typeDic.name = "piece_linear";
    io_metaRows.typeDic.push_back(typeDic);

    typeDic.typeId = 19;
    typeDic.name = "file";
    io_metaRows.typeDic.push_back(typeDic);

    // built-in types: logical type
    TypeDicRow typeRow;
    typeDic.typeId = 20;
    typeDic.name = "logical";
    typeDic.dicId = 1;          // logical type
    typeDic.totalEnumId = 2;
    io_metaRows.typeDic.push_back(typeDic);

    TypeDicTxtLangRow typeTxt;
    typeTxt.typeId = 20;
    typeTxt.langName = "EN";
    typeTxt.descr = "logical type";
    typeTxt.note = "logical type notes";
    io_metaRows.typeTxt.push_back(typeTxt);

    typeTxt.langName = "FR";
    typeTxt.descr = "(FR) logical type";
    typeTxt.note = "(FR) logical type notes";
    io_metaRows.typeTxt.push_back(typeTxt);

    TypeEnumLstRow typeEnum;
    typeEnum.typeId = 20;
    typeEnum.enumId = 0;
    typeEnum.name = "FALSE";
    io_metaRows.typeEnum.push_back(typeEnum);

    typeEnum.enumId = 1;
    typeEnum.name = "TRUE";
    io_metaRows.typeEnum.push_back(typeEnum);

    TypeEnumTxtLangRow typeEnumTxt;
    typeEnumTxt.typeId = 20;
    typeEnumTxt.enumId = 0;
    typeEnumTxt.langName = "EN";
    typeEnumTxt.descr = "False";
    typeEnumTxt.note = "";
    io_metaRows.typeEnumTxt.push_back(typeEnumTxt);
    
    typeEnumTxt.enumId = 0;
    typeEnumTxt.langName = "FR";
    typeEnumTxt.descr = "Faux";
    io_metaRows.typeEnumTxt.push_back(typeEnumTxt);

    typeEnumTxt.enumId = 1;
    typeEnumTxt.langName = "EN";
    typeEnumTxt.descr = "True";
    io_metaRows.typeEnumTxt.push_back(typeEnumTxt);

    typeEnumTxt.enumId = 1;
    typeEnumTxt.langName = "FR";
    typeEnumTxt.descr = "Vrai";
    io_metaRows.typeEnumTxt.push_back(typeEnumTxt);

    // classification types: age
    typeDic.typeId = 21;
    typeDic.name = "age";
    typeDic.dicId = 2;          // classifcation type
    typeDic.totalEnumId = 4;
    io_metaRows.typeDic.push_back(typeDic);

    typeTxt.typeId = 21;
    typeTxt.langName = "EN";
    typeTxt.descr = "Age";
    typeTxt.note = "age type notes";
    io_metaRows.typeTxt.push_back(typeTxt);

    typeEnum.typeId = 21;
    typeEnum.enumId = 0;
    typeEnum.name = "10";
    io_metaRows.typeEnum.push_back(typeEnum);

    typeEnum.enumId = 1;
    typeEnum.name = "20";
    io_metaRows.typeEnum.push_back(typeEnum);

    typeEnum.enumId = 2;
    typeEnum.name = "30";
    io_metaRows.typeEnum.push_back(typeEnum);

    typeEnum.enumId = 3;
    typeEnum.name = "40";
    io_metaRows.typeEnum.push_back(typeEnum);

    typeEnumTxt.typeId = 21;
    typeEnumTxt.enumId = 0;
    typeEnumTxt.langName = "EN";
    typeEnumTxt.descr = "age 10";
    typeEnumTxt.note = "";
    io_metaRows.typeEnumTxt.push_back(typeEnumTxt);

    typeEnumTxt.enumId = 1;
    typeEnumTxt.descr = "age 20";
    typeEnumTxt.note = "";
    io_metaRows.typeEnumTxt.push_back(typeEnumTxt);

    typeEnumTxt.enumId = 2;
    typeEnumTxt.descr = "age 30";
    typeEnumTxt.note = "age 20 notes";
    io_metaRows.typeEnumTxt.push_back(typeEnumTxt);

    typeEnumTxt.enumId = 3;
    typeEnumTxt.descr = "age 40";
    typeEnumTxt.note = "";
    io_metaRows.typeEnumTxt.push_back(typeEnumTxt);

    // classification types: sex
    typeDic.typeId = 22;
    typeDic.name = "sex";
    typeDic.dicId = 2;          // classifcation type
    typeDic.totalEnumId = 2;
    io_metaRows.typeDic.push_back(typeDic);

    typeTxt.typeId = 22;
    typeTxt.langName = "EN";
    typeTxt.descr = "Sex";
    typeTxt.note = "";
    io_metaRows.typeTxt.push_back(typeTxt);

    typeEnum.typeId = 22;
    typeEnum.enumId = 0;
    typeEnum.name = "M";
    io_metaRows.typeEnum.push_back(typeEnum);

    typeEnum.enumId = 1;
    typeEnum.name = "F";
    io_metaRows.typeEnum.push_back(typeEnum);

    typeEnumTxt.typeId = 22;
    typeEnumTxt.enumId = 0;
    typeEnumTxt.langName = "EN";
    typeEnumTxt.descr = "Male";
    typeEnumTxt.note = "";
    io_metaRows.typeEnumTxt.push_back(typeEnumTxt);

    typeEnumTxt.enumId = 1;
    typeEnumTxt.descr = "Female";
    typeEnumTxt.note = "";
    io_metaRows.typeEnumTxt.push_back(typeEnumTxt);

    // classification types: salary
    typeDic.typeId = 23;
    typeDic.name = "salary";
    typeDic.dicId = 2;          // classifcation type
    typeDic.totalEnumId = 3;
    io_metaRows.typeDic.push_back(typeDic);

    typeTxt.typeId = 23;
    typeTxt.langName = "EN";
    typeTxt.descr = "Salary level";
    typeTxt.note = "salary type notes";
    io_metaRows.typeTxt.push_back(typeTxt);

    typeEnum.typeId = 23;
    typeEnum.enumId = 0;
    typeEnum.name = "L";
    io_metaRows.typeEnum.push_back(typeEnum);

    typeEnum.enumId = 1;
    typeEnum.name = "M";
    io_metaRows.typeEnum.push_back(typeEnum);

    typeEnum.enumId = 2;
    typeEnum.name = "H";
    io_metaRows.typeEnum.push_back(typeEnum);

    typeEnumTxt.typeId = 23;
    typeEnumTxt.enumId = 0;
    typeEnumTxt.langName = "EN";
    typeEnumTxt.descr = "Low";
    typeEnumTxt.note = "";
    io_metaRows.typeEnumTxt.push_back(typeEnumTxt);

    typeEnumTxt.enumId = 1;
    typeEnumTxt.descr = "Medium";
    typeEnumTxt.note = "";
    io_metaRows.typeEnumTxt.push_back(typeEnumTxt);

    typeEnumTxt.enumId = 2;
    typeEnumTxt.descr = "High";
    typeEnumTxt.note = "";
    io_metaRows.typeEnumTxt.push_back(typeEnumTxt);

    // model input parameters
    ParamDicRow paramDic;
    ParamDicTxtLangRow paramTxt;
    ParamDimsRow paramDims;

    //// <SFG_test>
    //// model input parameters: MortalityHazard
    //paramDic.paramId = 0; //SFG Id to be generated by omc after collection sorting, perhaps stored in associated Symbol, perhaps used directly from collection iterator
    //paramDic.paramName = "MortalityHazard";  // must be valid database view name, if we want to use compatibility views
    //paramDic.rank = 0;
    //paramDic.typeId = 12;           // double
    //paramDic.isHidden = false;
    //paramDic.isGenerated = false;
    //paramDic.numCumulated = 0;
    //io_metaRows.paramDic.push_back(paramDic);

    //paramTxt.paramId = 0;
    //paramTxt.langName = "EN";
    //paramTxt.descr = "MortalityHazard short name (EN)";
    //paramTxt.note = "MortalityHazard note (EN)";
    //io_metaRows.paramTxt.push_back(paramTxt);

    //paramTxt.langName = "FR";
    //paramTxt.descr = "MortalityHazard short name (FR)";
    //paramTxt.note = "MortalityHazard note (FR)";
    //io_metaRows.paramTxt.push_back(paramTxt);

    //// Note: rank 0 parameters have no ParamDimsRows entries

    //// </SFG_test>

    //// model input parameters: age by sex
    //paramDic.paramId = 1;
    //paramDic.paramName = "ageSex";  // must be valid database view name, if we want to use compatibility views
    //paramDic.rank = 2;
    //paramDic.typeId = 12;           // double
    //paramDic.isHidden = false;
    //paramDic.isGenerated = false;
    //paramDic.numCumulated = 0;
    //io_metaRows.paramDic.push_back(paramDic);

    //paramTxt.paramId = 1;
    //paramTxt.langName = "EN";
    //paramTxt.descr = "Age by Sex";
    //paramTxt.note = "Age by Sex note";
    //io_metaRows.paramTxt.push_back(paramTxt);

    //paramTxt.langName = "FR";
    //paramTxt.descr = "(FR) Age by Sex";
    //paramTxt.note = "";
    //io_metaRows.paramTxt.push_back(paramTxt);

    //paramDims.paramId = 1;
    //paramDims.name = "dim0";    // must be valid database column name
    //paramDims.pos = 0;
    //paramDims.typeId = 21;      // age type
    //io_metaRows.paramDims.push_back(paramDims);

    //paramDims.name = "dim1";    // must be valid database column name
    //paramDims.pos = 1;
    //paramDims.typeId = 22;      // sex type
    //io_metaRows.paramDims.push_back(paramDims);

    //// model input parameters: salary by age
    //paramDic.paramId = 2;
    //paramDic.paramName = "salaryAge";   // must be valid database view name, if we want to use compatibility views
    //paramDic.rank = 2;
    //paramDic.typeId = 0;                // int
    //paramDic.isHidden = false;
    //paramDic.isGenerated = false;
    //paramDic.numCumulated = 0;
    //io_metaRows.paramDic.push_back(paramDic);

    //paramTxt.paramId = 2;
    //paramTxt.langName = "EN";
    //paramTxt.descr = "Salary by Age";
    //paramTxt.note = "Salary by Age note";
    //io_metaRows.paramTxt.push_back(paramTxt);

    //paramDims.paramId = 2;
    //paramDims.name = "dim0";    // must be valid database column name
    //paramDims.pos = 0;
    //paramDims.typeId = 23;      // salary type
    //io_metaRows.paramDims.push_back(paramDims);

    //paramDims.name = "dim1";    // must be valid database column name
    //paramDims.pos = 1;
    //paramDims.typeId = 21;      // age type
    //io_metaRows.paramDims.push_back(paramDims);



    // model output tables
    //TableDicRow tableDic;
    //tableDic.tableId = 0;
    //tableDic.tableName = "salarySex";   // must be valid database view name, if we want to use compatibility views
    //tableDic.isUser = false;
    //tableDic.rank = 2;
    //tableDic.isSparse = true;   // do not store NULLs
    //tableDic.isHidden = false;
    //io_metaRows.tableDic.push_back(tableDic);

    //TableDicTxtLangRow tableTxt;
    //tableTxt.tableId = 0;
    //tableTxt.langName = "EN";
    //tableTxt.descr = "Salary by Sex";
    //tableTxt.note = "Salary by Sex note";
    //tableTxt.unitDescr = "Sum and count";
    //tableTxt.unitNote = "There are two accumulators in that table: sum and count and four expressions";
    //io_metaRows.tableTxt.push_back(tableTxt);

    //TableDimsRow tableDims;
    //tableDims.tableId = 0;
    //tableDims.name = "dim0";    // must be valid database column name
    //tableDims.pos = 0;
    //tableDims.typeId = 23;      // salary type
    //tableDims.isTotal = false;
    //tableDims.dimSize = 3;      // must include "total" item, if is_total = true
    //io_metaRows.tableDims.push_back(tableDims);

    //tableDims.name = "dim1";    // must be valid database column name
    //tableDims.pos = 1;
    //tableDims.typeId = 22;      // sex type
    //tableDims.isTotal = false;
    //tableDims.dimSize = 2;      // must include "total" item, if is_total = true
    //io_metaRows.tableDims.push_back(tableDims);

    //TableDimsTxtLangRow tableDimsTxt;
    //tableDimsTxt.tableId = 0;
    //tableDimsTxt.name = "dim1";         // text info only for second dimension, nothing for dim0
    //tableDimsTxt.langName = "EN";
    //tableDimsTxt.descr = "Sex Dim";
    //tableDimsTxt.note = "Sex Dim note";
    //io_metaRows.tableDimsTxt.push_back(tableDimsTxt);

    //TableAccRow tableAcc;
    //tableAcc.tableId = 0;
    //tableAcc.accId = 0;
    //tableAcc.name = "acc0";         // must be valid database column name
    //tableAcc.expr = "value_sum()";
    //io_metaRows.tableAcc.push_back(tableAcc);

    //tableAcc.tableId = 0;
    //tableAcc.accId = 1;
    //tableAcc.name = "acc1";         // must be valid database column name
    //tableAcc.expr = "anything here, it is just description";
    //io_metaRows.tableAcc.push_back(tableAcc);

    //TableAccTxtLangRow tableAccTxt;
    //tableAccTxt.tableId = 0;
    //tableAccTxt.accId = 0;
    //tableAccTxt.langName = "EN";
    //tableAccTxt.descr = "Sum of salary by sex";
    //tableAccTxt.note = "notes Sum of salary by sex";
    //io_metaRows.tableAccTxt.push_back(tableAccTxt);

    //tableAccTxt.accId = 1;
    //tableAccTxt.langName = "EN";
    //tableAccTxt.descr = "Count of salary by sex";
    //tableAccTxt.note = "notes Count of salary by sex";
    //io_metaRows.tableAccTxt.push_back(tableAccTxt);

    //TableUnitRow tableUnit;
    //tableUnit.tableId = 0;
    //tableUnit.unitId = 0;
    //tableUnit.name = "Expr0";   // must be valid database view column name
    //tableUnit.decimals = 4;
    //tableUnit.src = "OM_AVG(acc0)";
    //io_metaRows.tableUnit.push_back(tableUnit);

    //TableUnitTxtLangRow tableUnitTxt;
    //tableUnitTxt.tableId = 0;
    //tableUnitTxt.unitId = 0;
    //tableUnitTxt.langName = "EN";
    //tableUnitTxt.descr = "Average acc0";
    //tableUnitTxt.note = "Average acc0 note";
    //io_metaRows.tableUnitTxt.push_back(tableUnitTxt);

    //tableUnitTxt.tableId = 0;
    //tableUnitTxt.unitId = 0;
    //tableUnitTxt.langName = "FR";
    //tableUnitTxt.descr = "(FR) Average acc0";
    //tableUnitTxt.note = "(FR) Average acc0 note";
    //io_metaRows.tableUnitTxt.push_back(tableUnitTxt);

    //tableUnit.unitId = 1;
    //tableUnit.name = "Expr1";       // must be valid database view column name
    //tableUnit.decimals = 4;
    //tableUnit.src = "OM_SUM(acc1)";
    //io_metaRows.tableUnit.push_back(tableUnit);

    //tableUnitTxt.unitId = 1;
    //tableUnitTxt.langName = "EN";
    //tableUnitTxt.descr = "Sum of acc1";
    //tableUnitTxt.note = "";
    //io_metaRows.tableUnitTxt.push_back(tableUnitTxt);

    //tableUnit.unitId = 2;
    //tableUnit.name = "Expr2";       // must be valid database view column name
    //tableUnit.decimals = 2;
    //tableUnit.src = "OM_MIN(acc0)";
    //io_metaRows.tableUnit.push_back(tableUnit);

    //tableUnitTxt.unitId = 2;
    //tableUnitTxt.langName = "EN";
    //tableUnitTxt.descr = "Min of acc0";
    //tableUnitTxt.note = "";
    //io_metaRows.tableUnitTxt.push_back(tableUnitTxt);

    //tableUnit.unitId = 3;
    //tableUnit.name = "Expr3";       // must be valid database view column name
    //tableUnit.decimals = 3;
    //tableUnit.src = "OM_AVG(acc0 * acc1)";
    //io_metaRows.tableUnit.push_back(tableUnit);

    //tableUnitTxt.unitId = 3;
    //tableUnitTxt.langName = "EN";
    //tableUnitTxt.descr = "Average acc0 * acc1";
    //tableUnitTxt.note = "";
    //io_metaRows.tableUnitTxt.push_back(tableUnitTxt);

    // group of parameters and output tables
    GroupLstRow groupLst;
    groupLst.groupId = 1;                   // must be unique across parameter AND otput table groups
    groupLst.isParam = true;                // group of parameters
    groupLst.name = "parameters group 1";
    groupLst.isHidden = false;
    groupLst.isGenerated = false;
    io_metaRows.groupLst.push_back(groupLst);

    groupLst.groupId = 2;                   // must be unique across parameter AND otput table groups
    groupLst.isParam = true;                // group of parameters
    groupLst.name = "parameters group 2";
    groupLst.isHidden = false;
    groupLst.isGenerated = false;
    io_metaRows.groupLst.push_back(groupLst);

    groupLst.groupId = 3;                   // must be unique accross parameter AND otput table groups
    groupLst.isParam = false;               // group of output tables
    groupLst.name = "tables group 1";
    groupLst.isHidden = false;
    groupLst.isGenerated = false;
    io_metaRows.groupLst.push_back(groupLst);

    GroupPcRow groupPc;
    groupPc.groupId = 1;        // group 1 as parent group
    groupPc.childPos = 0;       // first child of group 1
    groupPc.childGroupId = 2;   // group 2 is child of group 1
    groupPc.leafId = -1;        // NULL as parameter id or table id
    io_metaRows.groupPc.push_back(groupPc);

    groupPc.groupId = 2;        // group 2 as parent group
    groupPc.childPos = 0;       // first child of group 2
    groupPc.childGroupId = -1;  // NULL as child of group id
    groupPc.leafId = 0;         // parameter id=0 is a leaf of group 2
    io_metaRows.groupPc.push_back(groupPc);

    groupPc.groupId = 3;        // group 23as parent group
    groupPc.childPos = 0;       // first child of group 3
    groupPc.childGroupId = -1;  // NULL as child of group id
    groupPc.leafId = 0;         // output table id=0 is a leaf of group 3
    io_metaRows.groupPc.push_back(groupPc);

    GroupTxtLangRow groupTxt;
    groupTxt.groupId = 1;
    groupTxt.langName = "EN";
    groupTxt.descr = "Group of parameters";
    groupTxt.note = "Group of parameters note";
    io_metaRows.groupTxt.push_back(groupTxt);
}
