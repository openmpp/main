/**
* @file
* OpenM++ modeling library: model metadata loader to read and broadcast metadata and run options.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "model.h"

using namespace std;
using namespace openm;

namespace openm
{
    /** short name for ini file name: -ini fileName.ini */
    const char * RunShortKey::iniFile = "ini";

    /** number of sub-values */
    const char * RunOptionsKey::subValueCount = "OpenM.SubValues";

    /** model run name in database */
    const char * RunOptionsKey::runName = "OpenM.RunName";

    /** model run id to restart model run */
    const char * RunOptionsKey::restartRunId = "OpenM.RestartRunId";

    /** working set id to get input parameters */
    const char * RunOptionsKey::setId = "OpenM.SetId";

    /** working set name to get input parameters */
    const char * RunOptionsKey::setName = "OpenM.SetName";

    /** short name for: -s working set name to get input parameters */
    const char * RunShortKey::setName = "s";

    /** base run name to get input parameters */
    const char * RunOptionsKey::baseRunName = "OpenM.BaseRunName";

    /** model run id to get input parameters */
    const char * RunOptionsKey::baseRunId = "OpenM.BaseRunId";

    /** model run digest to get input parameters */
    const char * RunOptionsKey::baseRunDigest = "OpenM.BaseRunDigest";

    /** modeling task id */
    const char * RunOptionsKey::taskId = "OpenM.TaskId";

    /** modeling task name */
    const char * RunOptionsKey::taskName = "OpenM.TaskName";

    /** modeling task run name in database */
    const char * RunOptionsKey::taskRunName = "OpenM.TaskRunName";

    /** modeling task run id */
    const char * RunOptionsKey::taskRunId = "OpenM.TaskRunId";

    /** modeling task under external supervision */
    const char * RunOptionsKey::taskWait = "OpenM.TaskWait";

    /** profile name to get run options */
    const char * RunOptionsKey::profile = "OpenM.Profile";

    /** number of modeling threads */
    const char * RunOptionsKey::threadCount = "OpenM.Threads";

    /** if true then do not run modeling threads at root process */
    const char * RunOptionsKey::notOnRoot = "OpenM.NotOnRoot";

    /** database connection string */
    const char * RunOptionsKey::dbConnStr = "OpenM.Database";

    /** use sparse output tables */
    const char * RunOptionsKey::useSparse = "OpenM.SparseOutput";

    /** sparse NULL value */
    const char * RunOptionsKey::sparseNull = "OpenM.SparseNullValue";

    /** convert to string format for float, double, long double */
    const char * RunOptionsKey::doubleFormat = "OpenM.DoubleFormat";

    /** dir/to/read/input/parameter.csv */
    const char * RunOptionsKey::paramDir = "OpenM.ParamDir";

    /** short version: -p dir/to/read/input/parameter.csv */
    const char * RunShortKey::paramDir = "p";

    /** if true then parameter(s) csv file(s) contain enum id's, default: enum code */
    const char * RunOptionsKey::useIdCsv = "OpenM.IdCsv";

    /** if true then parameter value is enum id, default: enum code */
    const char * RunOptionsKey::useIdParamValue = "OpenM.IdParameterValue";
    
    /** if positive then used for simulation progress reporting, ex: every 10% */
    const char * RunOptionsKey::progressPercent = "OpenM.ProgressPercent";

    /** if positive then used for simulation progress reporting, ex: every 1000 cases or every 0.1 time step */
    const char * RunOptionsKey::progressStep = "OpenM.ProgressStep";

    /** options started with "Parameter." treated as value of model scalar input parameter, ex: -Parameter.Age 42 */
    const char * RunOptionsKey::parameterPrefix = "Parameter";

    /** options started with "SubFrom." used to specify where to get sub-values of input parameter, ex: -SubFrom.Age csv */
    const char * RunOptionsKey::subFromPrefix = "SubFrom";

    /** options started with "SubValues." used specify sub-values of input parameter, ex: -SubValues.Age [1,4] */
    const char * RunOptionsKey::subValuesPrefix = "SubValues";

    /** options started with "SubGroupFrom." used to specify where to get sub-values for a group of input parameter, ex: -SubGroupFrom.Geo csv */
    const char * RunOptionsKey::subGroupFromPrefix = "SubGroupFrom";

    /** options started with "SubGroupValues." used specify sub-values for a group of input parameter, ex: -SubGroupValues.Geo [1,4] */
    const char * RunOptionsKey::subGroupValuesPrefix = "SubGroupValues";

    /** import parameters from all upstream models last runs, ex: -Import.All true */
    const char * RunOptionsKey::importAll = "Import.All";

    /** options started with "Import." used to specify parameters import from upstream model run, ex: -Import.modelOne true */
    const char * RunOptionsKey::importPrefix = "Import";

    /** options started with "ImportRunDigest." used to specify run name to import parameters from, ex: -ImportRunDigest.modelOne abcdef */
    const char * RunOptionsKey::importRunDigestPrefix = "ImportRunDigest";

    /** options started with "ImportRunId." used to specify run id to import parameters from, ex: -ImportRunId.modelOne 101 */
    const char * RunOptionsKey::importRunIdPrefix = "ImportRunId";

    /** options started with "ImportRunName." used to specify run name to import parameters from, ex: -ImportRunName.modelOne GoodRun */
    const char * RunOptionsKey::importRunNamePrefix = "ImportRunName";

    /** options started with "ImportDigest." used to specify model digest to import parameters from last run of that model, ex: -ImportModelDigest.modelOne fedcba */
    const char * RunOptionsKey::importModelDigestPrefix = "ImportModelDigest";

    /** options started with "ImportId." used to specify model id to import parameters from last run of that model, ex: -ImportModelId.modelOne 123 */
    const char * RunOptionsKey::importModelIdPrefix = "ImportModelId";

    /** options started with "ImportExpr." used to specify expression name to import from output table, ex: -ImportExpr.AgeTable expr2 */
    const char * RunOptionsKey::importExprPrefix = "ImportExpr";

    /** options started with "ImportDatabase." used to specify database connection string to import parameters from, ex: -ImportDatabase.modelOne "Database=m1.sqlite;OpenMode=RedaOnly;" */
    const char * RunOptionsKey::importDbPrefix = "ImportDb";

    /** options ended with ".RunDescription" used to specify run decsription, ex: -EN.RunDescription "run model with 50,000 cases" */
    const char * RunOptionsKey::runDescrSuffix = "RunDescription";

    /** options started with "Suppress." used to exclude output table from run resuluts, ex: -Suppress.AgeTable true */
    const char * RunOptionsKey::suppressPrefix = "Suppress";

    /** options started with "SuppressGroup." used to exclude output table group from run resuluts, ex: -SuppressGroup.Income true */
    const char * RunOptionsKey::suppressGroupPrefix = "SuppressGroup";

    /** options started with "NotSuppress." used to include only output table in run results, ex: -NotSuppress.AgeTable true */
    const char * RunOptionsKey::notSuppressPrefix = "NotSuppress";

    /** options started with "NotSuppressGroup." used to include only output table group in run results, ex: -NotSuppressGroup.Income true */
    const char * RunOptionsKey::notSuppressGroupPrefix = "NotSuppressGroup";

    /** trace log to console */
    const char * RunOptionsKey::traceToConsole = "OpenM.TraceToConsole";

    /** trace log to file */
    const char * RunOptionsKey::traceToFile = "OpenM.TraceToFile";

    /** trace log file path */
    const char * RunOptionsKey::traceFilePath = "OpenM.TraceFilePath";

    /** trace log to "stamped" file */
    const char * RunOptionsKey::traceToStamped = "OpenM.TraceToStampedFile";

    /** trace use time-stamp in log "stamped" file name */
    const char * RunOptionsKey::traceUseTs = "OpenM.TraceUseTimeStamp";

    /** trace use pid-stamp in log "stamped" file name */
    const char * RunOptionsKey::traceUsePid = "OpenM.TraceUsePidStamp";

    /** do not prefix trace log messages with date-time */
    const char * RunOptionsKey::traceNoMsgTime = "OpenM.TraceNoMsgTime";

    /** language to display output messages */
    const char * RunOptionsKey::messageLang = "OpenM.MessageLanguage";

    /** log version info */
    const char* RunOptionsKey::version = "OpenM.Version";

    /** sub-value of parameter must be in the input workset */
    const char * RunOptionsKey::dbSubValue = "db";

    /** sub-value of parameter created as integer from 0 to sub-value count */
    const char * RunOptionsKey::iotaSubValue = "iota";

    /** all parameter sub-values must be in parameter.csv file */
    const char * RunOptionsKey::csvSubValue = "csv";

    /** default value of any option */
    const char * RunOptionsKey::defaultValue = "default";

    /** all value for any option */
    const char * RunOptionsKey::allValue = "All";
}

/** array of model run option keys. */
static const char * runOptKeyArr[] = {
    RunOptionsKey::subValueCount,
    RunOptionsKey::runName,
    RunOptionsKey::restartRunId,
    RunOptionsKey::setId,
    RunOptionsKey::setName,
    RunOptionsKey::baseRunName,
    RunOptionsKey::baseRunId,
    RunOptionsKey::baseRunDigest,
    RunOptionsKey::taskId,
    RunOptionsKey::taskName,
    RunOptionsKey::taskRunName,
    RunOptionsKey::taskWait,
    RunOptionsKey::profile,
    RunOptionsKey::importAll,
    RunOptionsKey::threadCount,
    RunOptionsKey::notOnRoot,
    RunOptionsKey::dbConnStr,
    RunOptionsKey::useSparse,
    RunOptionsKey::sparseNull,
    RunOptionsKey::doubleFormat,
    RunOptionsKey::progressPercent,
    RunOptionsKey::progressStep,
    RunOptionsKey::paramDir,
    RunOptionsKey::useIdCsv,
    RunOptionsKey::useIdParamValue,
    RunOptionsKey::traceToConsole,
    RunOptionsKey::traceToFile,
    RunOptionsKey::traceFilePath,
    RunOptionsKey::traceToStamped,
    RunOptionsKey::traceUseTs,
    RunOptionsKey::traceUsePid,
    RunOptionsKey::traceNoMsgTime,
    RunOptionsKey::messageLang,
    RunOptionsKey::version,
    ArgKey::iniFile,
    ArgKey::runStamp,
    ArgKey::logToConsole,
    ArgKey::logToFile,
    ArgKey::logFilePath,
    ArgKey::logToStamped,
    ArgKey::logUseTs,
    ArgKey::logUsePid,
    ArgKey::logNoMsgTime,
    ArgKey::logSql
};
static const size_t runOptKeySize = sizeof(runOptKeyArr) / sizeof(const char *);

/** array of short and full option names, used to find full option name by short. */
static const pair<const char *, const char *> shortPairArr[] = 
{
    make_pair(RunShortKey::iniFile, ArgKey::iniFile),
    make_pair(RunShortKey::setName, RunOptionsKey::setName),
    make_pair(RunShortKey::paramDir, RunOptionsKey::paramDir)
};
static const size_t shortPairSize = sizeof(shortPairArr) / sizeof(const pair<const char *, const char *>);

/** option prefixes to treat in a special way, ie: "Parameter" or "SubFrom" */
static const char * prefixOptArr[] = {
    RunOptionsKey::parameterPrefix, 
    RunOptionsKey::subFromPrefix,
    RunOptionsKey::subValuesPrefix,
    RunOptionsKey::subGroupFromPrefix,
    RunOptionsKey::subGroupValuesPrefix,
    RunOptionsKey::importPrefix,
    RunOptionsKey::importRunDigestPrefix,
    RunOptionsKey::importRunIdPrefix,
    RunOptionsKey::importRunNamePrefix,
    RunOptionsKey::importModelDigestPrefix,
    RunOptionsKey::importModelIdPrefix,
    RunOptionsKey::importDbPrefix,
    RunOptionsKey::importExprPrefix,
    RunOptionsKey::suppressPrefix,
    RunOptionsKey::suppressGroupPrefix,
    RunOptionsKey::notSuppressPrefix,
    RunOptionsKey::notSuppressGroupPrefix
};
static const size_t prefixOptSize = sizeof(prefixOptArr) / sizeof(const char *);

/** option suffixes to provide language-specific options, ex: "EN.RunDescription" */
static const char * langSuffixOptArr[] = {
    RunOptionsKey::runDescrSuffix
};
static const size_t langSuffixOptSize = sizeof(langSuffixOptArr) / sizeof(const char *);

/** last cleanup */
MetaLoader::~MetaLoader(void) noexcept { }

/** return id of parameter by name */
int MetaLoader::parameterIdByName(const char * i_name) const
{
    if (i_name == nullptr || i_name[0] == '\0') throw ModelException("invalid (empty) input parameter name");

    const ParamDicRow * paramRow = metaStore->paramDic->byModelIdName(modelId, i_name);
    if (paramRow == nullptr) throw DbException("parameter not found in parameters dictionary: %s", i_name);

    return paramRow->paramId;
}

/** return id of output table by name */
int MetaLoader::tableIdByName(const char * i_name) const
{
    if (i_name == nullptr || i_name[0] == '\0') throw ModelException("invalid (empty) output table name");

    const TableDicRow * tblRow = metaStore->tableDic->byModelIdName(modelId, i_name);
    if (tblRow == nullptr) throw DbException("output table not found in tables dictionary: %s", i_name);

    return tblRow->tableId;
}

/** initialize run options from command line and ini-file. */
const ArgReader MetaLoader::getRunOptions(int argc, char ** argv)
{
    // get command line options
    ArgReader ar;
    ar.parseCommandLine(
        argc, argv, true, false, runOptKeySize, runOptKeyArr, shortPairSize, shortPairArr, prefixOptSize, prefixOptArr, langSuffixOptSize, langSuffixOptArr
    );

    // load options from ini-file and append parameters section
    ar.loadIniFile(
        ar.strOption(ArgKey::iniFile).c_str(), runOptKeySize, runOptKeyArr, prefixOptSize, prefixOptArr, langSuffixOptSize, langSuffixOptArr
    );

    // dependency in log options: if LogToFile is true then file name must be non-empty else must be empty
    ar.adjustLogSettings(argc, argv);

    return ar;
}

/** return basic model run options */
const RunOptions MetaLoader::modelRunOptions(int i_subCount, int i_subId) const 
{ 
    RunOptions opts(baseRunOpts);
    opts.subValueCount = i_subCount;
    opts.subValueId = i_subId;
    return opts;
}

// read metadata tables from db, except of run_option table
int MetaLoader::readMetaTables(IDbExec * i_dbExec, MetaHolder * io_metaStore, const char * i_name, const char * i_digest)
{
    // find model by name digest
    io_metaStore->modelTable.reset(IModelDicTable::create(i_dbExec, i_name, i_digest));

    io_metaStore->modelRow = io_metaStore->modelTable->byNameDigest(i_name, i_digest);
    if (io_metaStore->modelRow == nullptr) throw DbException("model %s not found in the database", i_name);

    int mId = io_metaStore->modelRow->modelId;

    // read metadata tables
    io_metaStore->typeDic.reset(ITypeDicTable::create(i_dbExec, mId));
    io_metaStore->typeEnumLst.reset(ITypeEnumLstTable::create(i_dbExec, mId));
    io_metaStore->paramDic.reset(IParamDicTable::create(i_dbExec, mId));
    io_metaStore->paramDims.reset(IParamDimsTable::create(i_dbExec, mId));
    io_metaStore->paramImport.reset(IParamImportTable::create(i_dbExec, mId));
    io_metaStore->tableDic.reset(ITableDicTable::create(i_dbExec, mId));
    io_metaStore->tableDims.reset(ITableDimsTable::create(i_dbExec, mId));
    io_metaStore->tableAcc.reset(ITableAccTable::create(i_dbExec, mId));
    io_metaStore->tableExpr.reset(ITableExprTable::create(i_dbExec, mId));
    io_metaStore->langLst.reset(ILangLstTable::create(i_dbExec));
    io_metaStore->groupLst.reset(IGroupLstTable::create(i_dbExec, mId));
    io_metaStore->groupPc.reset(IGroupPcTable::create(i_dbExec, mId));

    return mId;
}

/** read model messages from database.
*
* User preferd language determined by simple match, for example:
* if user language is en_CA.UTF-8 then search done for lower case ["en-ca", "en", "model-default-language"].
*/
void MetaLoader::loadMessages(IDbExec * i_dbExec)
{
    if (i_dbExec == nullptr) throw ModelException("invalid (NULL) database connection");

    // get model language-specific messages in all languages
    // get common runtime language-specific messages in all languages
    unique_ptr<IModelWordTable> mwTbl(IModelWordTable::create(i_dbExec, metaStore->modelRow->modelId));
    unique_ptr<ILangWordTable> wordTbl(ILangWordTable::create(i_dbExec));

    // if language list already set (stored in theLog) then use it
    // else get user prefered locale and "normalize" language part of it: en_CA.UTF-8 => en-ca
    list<string> langLst = theLog->getLanguages();
    if (langLst.empty()) langLst = splitLanguageName(getDefaultLocaleName());

    // make list of language id's by srorten user language: en-ca, en
    vector<int> langIdArr;
    bool isDef = false;     // if true then it is model default language

    for (const string & lang : langLst) {

        // if language code found then add id into the list
        const LangLstRow * langRow = metaStore->langLst->findFirst(
            [&lang](const LangLstRow & i_row) -> bool { return normalizeLanguageName(i_row.code) == lang; }
        );
        if (langRow != nullptr) {
            langIdArr.push_back(langRow->langId);
            if (!isDef) isDef = langRow->langId == metaStore->modelRow->defaultLangId;
        }
    }

    // append model default language at the bottom of the list, if not already there
    if (!isDef) {
        const LangLstRow * langRow = metaStore->langLst->byKey(metaStore->modelRow->defaultLangId);
        if (langRow != nullptr) {
            langIdArr.push_back(langRow->langId);
            langLst.push_back(normalizeLanguageName(langRow->code));
        }
    }

    // get existing version of model messages list from theLog storage
    // append to message map by selecting most specific language from db model messages
    unordered_map<string, string> msgMap = theLog->getLanguageMessages();

    for (int langId : langIdArr) {
        for (ptrdiff_t nRow = 0; nRow < mwTbl->rowCount(); nRow++) {

            const ModelWordRow * mwRow = mwTbl->byIndex(nRow);
            if (mwRow->langId != langId) continue;      // skip other languages

            // if not already in message map then use it
            if (msgMap.find(mwRow->code) == msgMap.end()) msgMap[mwRow->code] = mwRow->value;
        }
    }

    // append common runtime messages
    for (int langId : langIdArr) {
        for (ptrdiff_t nRow = 0; nRow < wordTbl->rowCount(); nRow++) {

            const LangWordRow * wordRow = wordTbl->byIndex(nRow);
            if (wordRow->langId != langId) continue;      // skip other languages

            // if not already in message map then use it
            if (msgMap.find(wordRow->code) == msgMap.end()) msgMap[wordRow->code] = wordRow->value;
        }
    }

    // reduce message map: remove empty and not translated messages (where translated value same as non translated code)
    for (auto it = msgMap.begin(); it != msgMap.end();) {
        if (it->first.empty() || it->second.empty() || it->first == it->second) {
            it = msgMap.erase(it);
        }
        else {
            it++;
        }
    }

    // set language specific message for the log
    if (!msgMap.empty()) theLog->swapLanguageMessages(langLst, msgMap);
}

/** merge command line and ini-file arguments with profile_option table values.
*
* - use default values for basic run options, i.e. SparseOutput = false
* - validate scalar parameter value option, eg: Parameter.Age 42
* - parse parameter sub-values options, eg: SubFrom.Age csv  SubValues.Age 8 SubValues.Sex default
* - parse parameter import options
* - parse output tables suppress options
*/
void MetaLoader::mergeOptions(IDbExec * i_dbExec)
{
    // initial values are hard-coded default run options
    NoCaseMap defaultOpt;

    defaultOpt[RunOptionsKey::useSparse] = baseRunOpts.useSparse ? "true" : "false";
    defaultOpt[RunOptionsKey::sparseNull] = toString(baseRunOpts.nullValue);
    defaultOpt[RunOptionsKey::threadCount] = "1";

    // load default run options from profile options, default profile name = model name
    string profileName = argStore.strOption(RunOptionsKey::profile);

    if (!profileName.empty()) {

        // update run options: merge command line and ini-file with profile and hard-coded default values
        unique_ptr<IProfileOptionTable> profileTbl(IProfileOptionTable::create(i_dbExec, profileName));

        for (size_t nOpt = 0; nOpt < runOptKeySize; nOpt++) {

            if (argStore.isOptionExist(runOptKeyArr[nOpt])) continue;   // option specified at command line or ini-file

            // find option in profile_option table
            const ProfileOptionRow * optRow = profileTbl->byKey(profileName, runOptKeyArr[nOpt]);
            if (optRow != nullptr) {
                argStore.args[runOptKeyArr[nOpt]] = optRow->value;      // add option from database
            }
            else {  // no database value for that option key, use hard-coded default
                NoCaseMap::const_iterator defIt = defaultOpt.find(runOptKeyArr[nOpt]);
                if (defIt != defaultOpt.cend()) argStore.args[runOptKeyArr[nOpt]] = defIt->second;
            }
        }

        // update prefixed options, e.g. "Parameter." or "SubValues.": merge command line and ini-file with profile table
        vector<ParamDicRow> paramVec = metaStore->paramDic->rows();
        for (size_t nPref = 0; nPref < prefixOptSize; nPref++) {
            mergeParameterProfile(profileName, prefixOptArr[nPref], profileTbl.get(), paramVec);
        }
    }

    // if run stamp option not specified then use log time stamp by default
    string rStamp = argStore.strOption(ArgKey::runStamp);
    if (rStamp.empty()) {
        argStore.args[ArgKey::runStamp] = theLog->timeStamp();
    }
    else {
        if (rStamp.length() > OM_CODE_DB_MAX) argStore.args[ArgKey::runStamp] = rStamp.substr(0, OM_CODE_DB_MAX);
    }

    // validate "Parameter." options: it must be name of scalar input parameter
    string paramPrefix = string(RunOptionsKey::parameterPrefix) + ".";
    size_t nPrefix = paramPrefix.length();

    for (NoCaseMap::const_iterator optIt = argStore.args.cbegin(); optIt != argStore.args.cend(); optIt++) {

        if (!equalNoCase(optIt->first.c_str(), paramPrefix.c_str(), nPrefix)) continue;   // it is not a "Parameter."

        if (optIt->first.length() <= nPrefix) throw ModelException("invalid (empty) parameter name argument specified for %s option", paramPrefix.c_str());

        // find parameter by name: it must be a model parameter
        string sName = optIt->first.substr(nPrefix);

        const ParamDicRow * paramRow = metaStore->paramDic->byModelIdName(modelId, sName);
        if (paramRow == nullptr)
            throw DbException("parameter %s is not an input parameter of model %s, id: %d", sName.c_str(), metaStore->modelRow->name.c_str(), modelId);

        // check the rank: it must scalar
        if (paramRow->rank != 0) throw DbException("parameter %s is not a scalar", sName.c_str());

        // parameter value can not be empty
        if (optIt->second.empty()) throw ModelException("invalid (empty) value specified for parameter %s", sName.c_str());
    }

    // merge sub-values options for input  parameters: "SubFrom.Age", "SubValues.Age"
    subValueCount = argOpts().intOption(RunOptionsKey::subValueCount, 1);
    if (subValueCount <= 0) throw ModelException("Invalid number of sub-values: %d", subValueCount);

    parseParamSubOpts();

    // validate sub-values options: cannot be combined with "Parameter." option
    for (const auto & ps : subOptsMap) {

        const ParamDicRow * paramRow = metaStore->paramDic->byKey(modelId, ps.first);
        if (argStore.isOptionExist((paramPrefix + paramRow->paramName).c_str()))
            throw DbException("%s.%s cannot be combined with: \"%s\" or \"%s\"",
                RunOptionsKey::parameterPrefix, paramRow->paramName.c_str(), RunOptionsKey::subFromPrefix, RunOptionsKey::subValuesPrefix);
    }

    // parse parameters import options
    parseImportOptions();

    // parse suppression options to build list of tables to exclude from calculation and run output results
    parseSuppressOptions();

    // parse language-specific options
    parseLangOptions();

    // merge model run options
    baseRunOpts.useSparse = argStore.boolOption(RunOptionsKey::useSparse);
    baseRunOpts.nullValue = argStore.doubleOption(RunOptionsKey::sparseNull, FLT_MIN);
    baseRunOpts.progressPercent = argStore.intOption(RunOptionsKey::progressPercent, 0);
    baseRunOpts.progressStep = argStore.doubleOption(RunOptionsKey::progressStep, 0.0);
}

namespace
{
    /** convert string to integer number and return true if all chars converted, expected string to be trimmed from white spaces. */
    bool toInt(const string & i_src, int & io_number, int base = 10)
    {
        io_number = 0;
        try {
            io_number = std::stoi(i_src, nullptr, base);
        }
        catch (...) {
            return false;
        }
        return true;
    }

    // remove option prefix and the rest, for example from Parameter.Age remove "Parameter." and Age
    const string removeOptPrefix(const string & i_optKey, const string & i_prefix)
    {
        size_t nLen = i_prefix.length();
        if (i_optKey.length() <= nLen) throw ModelException("invalid (empty) name or group name specified: %s", i_optKey.c_str());

        return i_optKey.substr(nLen);
    }

}

/** parse sub-value options for input parameters: "SubFrom.Age", "SubValues.Age", "SubGroupFrom.Geo", "SubGroupValues.Geo"
*
* parse and validate parameter sub-values options, for example:     \n
*   SubFrom.Age        csv          \n
*   SubValues.Age      [0,15]       \n
*   SubValues.Sex      default      \n
*   SubGroupFrom.Geo   csv          \n
*   SubGroupValues.Geo [0,15]       \n
*
* validate "SubFrom." and "SubGroupFrom." options value, it must one of "db", "csv" or "iota".  \n
* "SubValues." and "SubGroupValues." option can be:     \n
*   list of id's: SubValues.Age 2,1,4,3                 \n
*   range:        SubValues.Age [1,4]                   \n
*   mask:         SubValues.Age x0F                     \n
*   single id:    SubValues.Age 7                       \n
*   default id:   SubValues.Age default
*/
void MetaLoader::parseParamSubOpts(void)
{
    string fromPrefix = string(RunOptionsKey::subFromPrefix) + ".";
    string valPrefix = string(RunOptionsKey::subValuesPrefix) + ".";
    string fromGroupPrefix = string(RunOptionsKey::subGroupFromPrefix) + ".";
    string valGroupPrefix = string(RunOptionsKey::subGroupValuesPrefix) + ".";

    for (NoCaseMap::const_iterator optIt = argStore.args.cbegin(); optIt != argStore.args.cend(); optIt++) {

        // check is it sub-values option
        bool isFromOpt = equalNoCase(optIt->first.c_str(), fromPrefix.c_str(), fromPrefix.length());
        bool isValuesOpt = equalNoCase(optIt->first.c_str(), valPrefix.c_str(), valPrefix.length());
        bool isFromGroupOpt = equalNoCase(optIt->first.c_str(), fromGroupPrefix.c_str(), fromGroupPrefix.length());
        bool isValuesGroupOpt = equalNoCase(optIt->first.c_str(), valGroupPrefix.c_str(), valGroupPrefix.length());

        if (!isFromOpt && !isValuesOpt && !isFromGroupOpt && !isValuesGroupOpt) continue; // it is not a parameter sub-value option

        // check option key: parameter name or group name must not be empty
        string argName;
        if (isFromOpt) argName = removeOptPrefix(optIt->first, fromPrefix);
        if (isValuesOpt) argName = removeOptPrefix(optIt->first, valPrefix);
        if (isFromGroupOpt) argName = removeOptPrefix(optIt->first, fromGroupPrefix);
        if (isValuesGroupOpt) argName = removeOptPrefix(optIt->first, valGroupPrefix);

        if (argName.empty()) throw ModelException("invalid (empty) parameter name or group name: %s", optIt->first.c_str());

        // make list of parameters name for that sub-value option
        // if this is a group of parameters then expand it as to include names of all parameter members
        vector<string> pnArr;
        if (isFromOpt || isValuesOpt) {
            pnArr.push_back(argName);
        }
        else {
            pnArr = expandParamGroup(modelId, argName);
        }

        // validate "SubFrom." or "SubGroupFrom.", it must have value as one of: "db", "iota", "csv"
        if (isFromOpt || isFromGroupOpt) {
            for (const string & pName : pnArr) {

                // find parameter by name: it must be a model parameter
                const ParamDicRow * paramRow = metaStore->paramDic->byModelIdName(modelId, pName);
                if (paramRow == nullptr)
                    throw DbException("parameter %s is not an input parameter of model %s, id: %d", pName.c_str(), metaStore->modelRow->name.c_str(), modelId);

                ParamSubOpts & ps = subOptsMap[paramRow->paramId];  // insert new or get existing options

                if (equalNoCase(optIt->second.c_str(), RunOptionsKey::dbSubValue)) ps.from = RunOptionsKey::dbSubValue;
                if (equalNoCase(optIt->second.c_str(), RunOptionsKey::iotaSubValue)) ps.from = RunOptionsKey::iotaSubValue;
                if (equalNoCase(optIt->second.c_str(), RunOptionsKey::csvSubValue)) ps.from = RunOptionsKey::csvSubValue;

                if (ps.from != RunOptionsKey::dbSubValue && ps.from != RunOptionsKey::iotaSubValue && ps.from != RunOptionsKey::csvSubValue)
                    throw ModelException("invalid value specified for %s, expected one of: %s %s %s",
                        optIt->first.c_str(), RunOptionsKey::dbSubValue, RunOptionsKey::iotaSubValue, RunOptionsKey::csvSubValue);
            }
        }

        // validate "SubValues." or "SubGroupValues." options, it must have parameter name and value as range or comma-separated list or hex mask or "default":
        //  range:      SubValues.Age [4,7]
        //  list:       SubValues.Age 4,5,6,7
        //  mask:       SubValues.Age xF0
        //  single id:  SubValues.Age 7
        //  default id: SubValues.Age default
        // number of sub-values must be 1 or exactly equal to number of sub-values in that model run
        if (isValuesOpt || isValuesGroupOpt) {

            for (const string & pName : pnArr) {

                // find parameter by name: it must be a model parameter
                const ParamDicRow * paramRow = metaStore->paramDic->byModelIdName(modelId, pName);
                if (paramRow == nullptr)
                    throw DbException("parameter %s is not an input parameter of model %s, id: %d", pName.c_str(), metaStore->modelRow->name.c_str(), modelId);

                ParamSubOpts & ps = subOptsMap[paramRow->paramId];  // insert new or get existing options

                // convert option value to get sub-value id's: range or mask or comma-separated list
                string sVal = trim(optIt->second);
                if (sVal.empty()) throw ModelException("invalid (empty) value specified for %s", optIt->first.c_str());

                size_t nLen = sVal.length();
                bool isRange = nLen >= 5 && sVal[0] == '[' && sVal[nLen - 1] == ']';
                bool isMask = !isRange && nLen >= 2 && (sVal[0] == 'x' || sVal[0] == 'X');

                // convert first and last sub-value id from range: SubValues.Age [4,7]
                if (isRange) {
                    list<string> sLst = splitCsv(sVal.substr(1, nLen - 2));
                    if (sLst.size() != 2) throw ModelException("invalid range values specified: %s %s", optIt->first.c_str(), optIt->second.c_str());

                    int nFirst = 0, nLast = 0;
                    if (!toInt(sLst.front(), nFirst)) throw ModelException("invalid range value specified: %s %s", optIt->first.c_str(), optIt->second.c_str());
                    if (!toInt(sLst.back(), nLast)) throw ModelException("invalid range value specified: %s %s", optIt->first.c_str(), optIt->second.c_str());
                    if (nFirst > nLast) {
                        swap(nFirst, nLast);
                    }
                    ps.subCount = (nLast + 1) - nFirst;

                    ps.subIds.clear();
                    for (int n = nFirst; n <= nLast; n++) {
                        ps.subIds.push_back(n);
                    }
                    ps.kind = ps.subCount > 1 ? KindSubIds::range : KindSubIds::single;
                }

                // convert mask into array of sub-value id's: SubValues.Age xF0 => 4,5,6,7
                if (isMask) {

                    size_t k = sVal.length();
                    ps.subIds.clear();
                    int nId = 0;
                    while (--k > 0) {

                        int dgt = 0;
                        if (!toInt(string(sVal, k, 1), dgt, 16)) throw ModelException("invalid (not hexadecimal) value specified: %s %s", optIt->first.c_str(), optIt->second.c_str());

                        for (int j = 0; j < 4; j++) {
                            if (dgt % 2 != 0) {
                                ps.subIds.push_back(nId);
                                if (ps.subIds.size() > (size_t)subValueCount) throw ModelException("invalid (to many values) specified: %s %s", optIt->first.c_str(), optIt->second.c_str());
                            }
                            dgt >>= 1;
                            if (++nId >= INT32_MAX)
                                throw ModelException("invalid (out of range) value specified: %s %s", optIt->first.c_str(), optIt->second.c_str());
                        }
                    }
                    ps.subCount = (int)ps.subIds.size();
                    ps.kind = ps.subCount > 1 ? KindSubIds::list : KindSubIds::single;
                }

                // convert list of sub-value id's: SubValues.Age 4,5,6,7
                // it can be single id or default id: SubValues.Age default
                if (!isRange && !isMask) {

                    if (equalNoCase(sVal.c_str(), RunOptionsKey::defaultValue)) {
                        ps.kind = KindSubIds::defaultId;
                        ps.subCount = 1;
                        ps.subIds = { 0 };
                    }
                    else {
                        list<string> sLst = splitCsv(sVal);

                        ps.subIds.clear();
                        for (const string & sv : sLst) {
                            int nId = 0;
                            if (!toInt(sv, nId)) throw ModelException("invalid value specified: %s %s", optIt->first.c_str(), optIt->second.c_str());

                            ps.subIds.push_back(nId);
                            if (ps.subIds.size() > (size_t)subValueCount) throw ModelException("invalid (to many values) specified: %s %s", optIt->first.c_str(), optIt->second.c_str());
                        }
                        ps.subCount = (int)ps.subIds.size();
                        ps.kind = ps.subCount > 1 ? KindSubIds::list : KindSubIds::single;
                    }
                }

                // check is it single id or list of sequential id's (range)
                if (ps.kind == KindSubIds::list) {
                    bool isSeq = true;
                    for (size_t k = 0; k < ps.subIds.size(); k++) {
                        isSeq = k == 0 || (isSeq && ps.subIds[k] == ps.subIds[k - 1] + 1);
                    }
                    if (isSeq) ps.kind = KindSubIds::range;
                }

                if (ps.kind != KindSubIds::single && ps.kind != KindSubIds::defaultId && ps.kind != KindSubIds::range && ps.kind != KindSubIds::list)
                    throw ModelException("invalid value specified: %s %s", optIt->first.c_str(), optIt->second.c_str());
            }
        }
    }

    // set defult values and validate sub-values options, collect parameter id's with multiple sub-values
    for (auto & ps : subOptsMap) {

        // if from option specified and number of sub-values not explicitly specified by "SubValues."
        // then assume number of sub-values for the model run
        // if from option not specified then use default rules
        if (ps.second.from != RunOptionsKey::defaultValue) {
            if (ps.second.kind == KindSubIds::none) ps.second.subCount = subValueCount;
        }

        // if SubFrom.Parameter = iota then parameter cannot have SubId.Parameter or SubValues.Parameter option
        if (ps.second.from == RunOptionsKey::iotaSubValue && ps.second.kind != KindSubIds::none)
            throw ModelException(
                "invalid options for parameter %s: option %s cannot be combined with %s",
                metaStore->paramDic->byKey(modelId, ps.first)->paramName.c_str(), RunOptionsKey::iotaSubValue, RunOptionsKey::subValuesPrefix
            );

        // sub-values count must 1 or equal to number of sub-values for that model run
        if (ps.second.subCount != 1 && ps.second.subCount != subValueCount)
            throw ModelException("invalid number of sub-values: %d specified for parameter: %s, expected 1 or %d",
                ps.second.subCount, metaStore->paramDic->byKey(modelId, ps.first)->paramName.c_str(), subValueCount);

        // collect parameter id's with multiple sub-values
        if (ps.second.subCount > 1) paramIdSubArr.push_back(ps.first);
    }
}

// return names of all parameters included in parameter group
const vector<string> MetaLoader::expandParamGroup(int i_modelId, const string & i_groupName) const
{
    // find parameters group
    const GroupLstRow * grpRow = metaStore->groupLst->findFirst(
        [i_modelId, &i_groupName](const GroupLstRow & i_row) -> bool {
            return i_row.isParam && i_row.modelId == i_modelId && i_row.name == i_groupName;
        }
    );
    if (grpRow == nullptr)
        throw DbException("parameters group not found: %s in the model %s, id: %d", i_groupName.c_str(), metaStore->modelRow->name.c_str(), modelId);

    vector<int> idArr = metaStore->groupPc->groupLeafs(modelId, grpRow->groupId);   // get all members parameter id of that group

    // get array of parameters name by id's
    vector<string> pnArr;
    for (int nId : idArr) {

        const ParamDicRow * paramRow = metaStore->paramDic->byKey(modelId, nId);
        if (paramRow == nullptr)
            throw DbException("parameter id: %d not found in the group: %s of model %s, id: %d", nId, i_groupName.c_str(), metaStore->modelRow->name.c_str(), modelId);

        pnArr.push_back(paramRow->paramName);
    }
    return pnArr;
}

/** parse parameter import options.
*
* Get source run and source model to import parameters.
* 
* If for example:       \n
*   -Import.All true    \n
* then import all parameters specified in model_import_table.
*
* If for example:       \n
*   -ImportModelDb.MyModel "Database=my_model.sqlite;OpenMode=RedaOnly;"     \n
* then use above connection string to open MyModel database
* else use default rule to find MyModel model in database:
*   if model name MyModel exist in current database then use it to import parameters
*   else open default database: MyModel.sqlite.
*
* Following options can be specified to find model run to import parameters from model MyModel,
* in priority oredr from lowest to highest:     \n
*   -Import.MyModel true                : use last run where model name = MyModel                           \n
*   -ImportModelId.MyModel 123          : use last run where model id = 123 and name MyModel                \n
*   -ImportModelDigest.MyModel fedcba   : use last run where digest = fedcba and name MyModel               \n
*   -ImportRunName.MyModel GoodRun      : use last run where run name = GoodRun and model name = MyModel    \n
*   -ImportRunId.MyModel 101            : use run where id = 101 and model name = MyModel                   \n
*   -ImportRunDigest.MyModel abcdef     : use run where digest = abcdef and model name = MyModel            \n
*
* It is possible to specify multiple options to find model run to import parameters.
* If for example:       \n
*   -ImportModelId.MyModel 123 -ImportRunName.MyModel GoodRun   \n
* then result is a run where model name = MyModel and model id = 123 and run name = GoodRun
*
* If output table has multiple expressions then by default first expression (zero index) imported as parameter value.
* To import other expression use:   \n
*   ImportExpr.AgeTable expr2       \n
*
* If output table has multiple accumulators then by default first accumulator (zero index) imported as parameter sub values.
* To import other accumulator use:  \n
*   ImportAcc.AgeTable acc4         \n
*/
void MetaLoader::parseImportOptions(void)
{
    string modelNamePrefix = string(RunOptionsKey::importPrefix) + ".";
    string modelIdPrefix = string(RunOptionsKey::importModelIdPrefix) + ".";
    string modelDigestPrefix = string(RunOptionsKey::importModelDigestPrefix) + ".";
    string runIdPrefix = string(RunOptionsKey::importRunIdPrefix) + ".";
    string runNamePrefix = string(RunOptionsKey::importRunNamePrefix) + ".";
    string runDigestPrefix = string(RunOptionsKey::importRunDigestPrefix) + ".";
    string dbPrefix = string(RunOptionsKey::importDbPrefix) + ".";
    string exprPrefix = string(RunOptionsKey::importExprPrefix) + ".";

    // add all models from model_parameter_import
    // if Import.All is true then for each model set import source as last run of the model with that name
    vector<ParamImportRow> piArr = metaStore->paramImport->byModelId(modelId);

    if (argStore.boolOption(RunOptionsKey::importAll)) {
        for (const ParamImportRow & pi : piArr) {
            importOptsMap[pi.fromModel].kind = ImportKind::modelName;
        }
    }

    // get name from option key: Import.modelOne => modelOne
    auto nameKey = [](const string & i_key, size_t i_prefixLen) -> const string {
        if (i_key.length() <= i_prefixLen) throw ModelException("invalid (empty) name specified for %s option", i_key.c_str());
        return i_key.substr(i_prefixLen);
    };

    // for each model run option:
    // if option is started from import prefix then add model name into import options map with coressponding option value
    for (NoCaseMap::const_iterator optIt = argStore.args.cbegin(); optIt != argStore.args.cend(); optIt++) {

        if (equalNoCase(optIt->first.c_str(), RunOptionsKey::importAll)) continue;  // skip "Import.All" option

        // if it is import from run digest: "ImportRunDigest.modelOne abcdef"
        if (equalNoCase(optIt->first.c_str(), runDigestPrefix.c_str(), runDigestPrefix.length())) {
            string mName = nameKey(optIt->first, runDigestPrefix.length());

            importOptsMap[mName].runDigest = optIt->second;
            if(importOptsMap[mName].kind < ImportKind::runDigest) importOptsMap[mName].kind = ImportKind::runDigest;
            continue;
        }

        // if it is import from run id: "ImportRunId.modelOne 101"
        if (equalNoCase(optIt->first.c_str(), runIdPrefix.c_str(), runIdPrefix.length())) {
            string mName = nameKey(optIt->first, runIdPrefix.length());

            int nId = 0;
            bool isOk = toInt(optIt->second, nId);
            if (!isOk || nId <= 0) throw ModelException("invalid run id specified: %s %s", optIt->first.c_str(), optIt->second.c_str());

            importOptsMap[mName].runId = nId;
            if (importOptsMap[mName].kind < ImportKind::runId) importOptsMap[mName].kind = ImportKind::runId;
            continue;
        }

        // if it is import from run name: "ImportRunName.modelOne GoodRun"
        if (equalNoCase(optIt->first.c_str(), runNamePrefix.c_str(), runNamePrefix.length())) {
            string mName = nameKey(optIt->first, runNamePrefix.length());

            importOptsMap[mName].runName = optIt->second;
            if (importOptsMap[mName].kind < ImportKind::runName) importOptsMap[mName].kind = ImportKind::runName;
            continue;
        }

        // if it is import from model digest: "ImportModelDigest.modelOne fedcba"
        if (equalNoCase(optIt->first.c_str(), modelDigestPrefix.c_str(), modelDigestPrefix.length())) {
            string mName = nameKey(optIt->first, modelDigestPrefix.length());

            importOptsMap[mName].modelDigest = optIt->second;
            if (importOptsMap[mName].kind < ImportKind::modelDigest) importOptsMap[mName].kind = ImportKind::modelDigest;
            continue;
        }

        // if it is import from model id: "ImportModelId.modelOne 123"
        if (equalNoCase(optIt->first.c_str(), modelIdPrefix.c_str(), modelIdPrefix.length())) {
            string mName = nameKey(optIt->first, modelIdPrefix.length());

            int nId = 0;
            bool isOk = toInt(optIt->second, nId);
            if (!isOk || nId <= 0) throw ModelException("invalid model id specified: %s %s", optIt->first.c_str(), optIt->second.c_str());

            importOptsMap[mName].modelId = nId;
            if (importOptsMap[mName].kind < ImportKind::modelId) importOptsMap[mName].kind = ImportKind::modelId;
            continue;
        }

        // if it is import from model name: "Import.modelOne true"
        if (equalNoCase(optIt->first.c_str(), modelNamePrefix.c_str(), modelNamePrefix.length())) {
            string mName = nameKey(optIt->first, modelNamePrefix.length());

            if (argStore.boolOption(optIt->first.c_str())) {
                if (auto it = importOptsMap.find(mName); it != importOptsMap.end()) {
                    if (it->second.kind == ImportKind::none) importOptsMap[mName].kind = ImportKind::modelName;
                }
                else {
                    importOptsMap[mName].kind = ImportKind::modelName;
                }
            }
            continue;
        }

        // if it is import database connection string: ImportDb.modelOne "Database=m1.sqlite;"
        if (equalNoCase(optIt->first.c_str(), dbPrefix.c_str(), dbPrefix.length())) {
            string mName = nameKey(optIt->first, dbPrefix.length());

            importOptsMap[mName].connectionStr = optIt->second;
            if (importOptsMap[mName].kind == ImportKind::none) importOptsMap[mName].kind = ImportKind::modelName;
            continue;
        }

        // if it is import expression name: ImportExpr.AgeTable expr2
        if (equalNoCase(optIt->first.c_str(), exprPrefix.c_str(), exprPrefix.length())) {
            string tName = nameKey(optIt->first, exprPrefix.length());

            const auto piRows = metaStore->paramImport->findAll(
                [&tName](const ParamImportRow & i_row) -> bool { return i_row.fromName == tName; }
            );

            bool isFound = false;
            for (const auto & pi : piRows) {
                paramImportOptsMap[pi.paramId].expr = optIt->second;
                isFound = true;
            }
            if (!isFound) throw ModelException("invalid output table specified: %s %s", optIt->first.c_str(), optIt->second.c_str());
            continue;
        }
    }

    // check: all model names must exist in model_parameter_import
    for (auto impOptsIt = importOptsMap.begin(); impOptsIt != importOptsMap.end(); ) {

        if (impOptsIt->second.kind == ImportKind::none) {   // remove import if it is disabled for that model name
            importOptsMap.erase(impOptsIt->first);
            continue;
        }
        
        // model name must exist in model_parameter_import
        auto it = find_if(
            piArr.cbegin(),
            piArr.cend(),
            [impOptsIt](const ParamImportRow & i_pi) -> bool { return i_pi.fromModel == impOptsIt->first; }
        );
        if (it == piArr.cend()) throw ModelException("invalid model name specified: %s", impOptsIt->first.c_str());

        ++impOptsIt;
    }
}

/** parse suppression options to build list of tables to exclude from calculation and run output results.
*
* There are two ways to specify tables suppression: \n
*   Suppress.AgeTable    true       \n
*   SuppressGroup.Income true       \n
* this means suppress only AgeTable and Income group of tables. \n
* Or:                               \n
*   NotSuppress.AgeTable    true    \n
*   NotSuppressGroup.Income true    \n
* result in suppression of all output tables except of AgeTable and Income group of tables. \n
*
* Suppress and not suppress options are mutually excluse and cannot be mixed.
* For example, if model run options are: -Suppress.A true -NotSuppress.B true
* then result is an error (model run exception).
*/
void MetaLoader::parseSuppressOptions(void)
{
    string suppPrefix = string(RunOptionsKey::suppressPrefix) + ".";
    string suppGroupPrefix = string(RunOptionsKey::suppressGroupPrefix) + ".";
    string notSuppPrefix = string(RunOptionsKey::notSuppressPrefix) + ".";
    string notSuppGroupPrefix = string(RunOptionsKey::notSuppressGroupPrefix) + ".";

    bool isAnySupp = false;
    bool isAnyNotSupp = false;
    set<int> tblIds;

    for (NoCaseMap::const_iterator optIt = argStore.args.cbegin(); optIt != argStore.args.cend(); optIt++) {

        // check is it one of suppress option
        bool isSupp = equalNoCase(optIt->first.c_str(), suppPrefix.c_str(), suppPrefix.length());
        bool isSuppGroup = equalNoCase(optIt->first.c_str(), suppGroupPrefix.c_str(), suppGroupPrefix.length());
        bool isNotSupp = equalNoCase(optIt->first.c_str(), notSuppPrefix.c_str(), notSuppPrefix.length());
        bool isNotSuppGroup = equalNoCase(optIt->first.c_str(), notSuppGroupPrefix.c_str(), notSuppGroupPrefix.length());

        if (!isSupp && !isSuppGroup && !isNotSupp && !isNotSuppGroup) continue; // it is not a suppress option

        isAnySupp = isAnySupp || isSupp || isSuppGroup;
        isAnyNotSupp = isAnyNotSupp || isNotSupp || isNotSuppGroup;

        if (isAnySupp && isAnyNotSupp)
            throw ModelException("run optins %s and %s are mutually exclusive and cannot be mixed: %s",
                RunOptionsKey::suppressPrefix, RunOptionsKey::notSuppressPrefix, optIt->first.c_str());

        // check option key: table name or group name must not be empty
        string argName;
        if (isSupp) argName = removeOptPrefix(optIt->first, suppPrefix);
        if (isSuppGroup) argName = removeOptPrefix(optIt->first, suppGroupPrefix);
        if (isNotSupp) argName = removeOptPrefix(optIt->first, notSuppPrefix);
        if (isNotSuppGroup) argName = removeOptPrefix(optIt->first, notSuppGroupPrefix);

        if (argName.empty()) throw ModelException("invalid (empty) output table name or group name: %s", optIt->first.c_str());

        // find table id by name
        // or expand table group to get id's of all table members
        if (isSupp || isNotSupp) {

            const TableDicRow * tblRow = metaStore->tableDic->byModelIdName(modelId, argName);
            if (tblRow == nullptr) throw new DbException("output table not found in tables dictionary: %s", argName.c_str());

            tblIds.insert(tblRow->tableId);
        }
        else {
            // find ouput table group
            const GroupLstRow * grpRow = metaStore->groupLst->findFirst(
                [this, &argName](const GroupLstRow & i_row) -> bool {
                    return !i_row.isParam && i_row.modelId == modelId && i_row.name == argName;
                }
            );
            if (grpRow == nullptr)
                throw DbException("output tables group not found: %s in the model %s, id: %d", argName.c_str(), metaStore->modelRow->name.c_str(), modelId);

            vector<int> idArr = metaStore->groupPc->groupLeafs(modelId, grpRow->groupId);   // get all members id of that group

            tblIds.insert(idArr.cbegin(), idArr.cend());
        }
    }

    // build sorted vector of table id's to suppress
    // if not suppress options are used then
    // build list of table id's to suppress by excluding "not suppress" id from the model table list
    if (isAnySupp) {
        tableIdSuppressArr.assign(tblIds.cbegin(), tblIds.cend());
    }
    else if (isAnyNotSupp) {
        tableIdSuppressArr.clear();
        for (ptrdiff_t nRow = 0; nRow < metaStore->tableDic->rowCount(); nRow++) {

            const TableDicRow * tRow = metaStore->tableDic->byIndex(nRow);

            const auto it = tblIds.find(tRow->tableId);
            if (tblIds.find(tRow->tableId) == tblIds.cend()) tableIdSuppressArr.push_back(tRow->tableId);
        }
    }
    else {
        tableIdSuppressArr.clear();
    }
}

/** parse language-specific options to get language code and option value.
*
* For example: -EN.RunDescription "run the model with 50,000 cases"
*/
void MetaLoader::parseLangOptions(void)
{
    for (NoCaseMap::const_iterator optIt = argStore.args.cbegin(); optIt != argStore.args.cend(); optIt++) {

        LangOptKind kind = LangOptKind::none;
        int lid = 0;
        size_t nMax = 0;
        for (size_t n = 0; kind == LangOptKind::none && n < langSuffixOptSize; n++) {

            string sfx = string(".") + langSuffixOptArr[n];
            if (!endWithNoCase(optIt->first, sfx.c_str())) continue;    // option does not end with this language-specific suffix

            // validate language prefix of options: it must be non-empty and exist in lang_lst table
            if (optIt->first.length() <= sfx.length()) throw ModelException("invalid (empty) language specified for %s option", optIt->first.c_str());

            const LangLstRow * langRow = metaStore->langLst->byCode(optIt->first.substr(0, optIt->first.length() - sfx.length()));
            if (langRow == nullptr)
                throw ModelException("invalid language specified for %s option", optIt->first.c_str());

            // this is language-specific option, only run description option supported
            kind = LangOptKind::runDescr;
            lid = langRow->langId;
            nMax = OM_DESCR_DB_MAX;
        }

        // if this is this language-specific option then store language code and value
        if (kind != LangOptKind::none) {
            langOptsMap[pair<LangOptKind, int>(kind, lid)] = optIt->second.substr(0, nMax);
        }
    }
}

// merge parameter name arguments with profile_option table, ie "Parameter.Age" or "SubValues.Age" argument
void MetaLoader::mergeParameterProfile(
    const string & i_profileName, const char * i_prefix, const IProfileOptionTable * i_profileOpt, const vector<ParamDicRow> & i_paramRs
)
{
    // merge command-line or ini-file arguments with profile_option table
    string prefixDot = string(i_prefix) + ".";

    for (vector<ParamDicRow>::const_iterator paramIt = i_paramRs.cbegin(); paramIt != i_paramRs.cend(); ++paramIt) {

        string argName = prefixDot + paramIt->paramName;

        if (argStore.isOptionExist(argName.c_str())) continue;  // parameter specified at command line or ini-file

        // find option in profile_option table
        const ProfileOptionRow * optRow = i_profileOpt->byKey(i_profileName, argName.c_str());
        if (optRow != nullptr) {
            argStore.args[argName] = optRow->value;             // add option from database
        }
    }
}
