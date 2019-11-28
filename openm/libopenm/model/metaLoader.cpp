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

    /** options started with "Parameter." treated as value of model scalar input parameter, ex: "-Parameter.Age 42" */
    const char * RunOptionsKey::parameterPrefix = "Parameter";

    /** options started with "SubFrom." used to specify where to get sub-values of input parameter, ex: "-SubFrom.Age csv" */
    const char * RunOptionsKey::subFromPrefix = "SubFrom";

    /** options started with "SubValues." used specify sub-values of input parameter, ex: "-SubValues.Age [1,4]" */
    const char * RunOptionsKey::subValuesPrefix = "SubValues";

    /** options started with "SubGroupFrom." used to specify where to get sub-values for a group of input parameter, ex: "-SubGroupFrom.Geo csv" */
    const char * RunOptionsKey::subGroupFromPrefix = "SubGroupFrom";

    /** options started with "SubGroupValues." used specify sub-values for a group of input parameter, ex: "-SubGroupValues.Geo [1,4]" */
    const char * RunOptionsKey::subGroupValuesPrefix = "SubGroupValues";

    /** number of modeling threads */
    const char * RunOptionsKey::threadCount = "OpenM.Threads";

    /** if true then do not run modeling threads at root process */
    const char * RunOptionsKey::notOnRoot = "OpenM.NotOnRoot";

    /** database connection string */
    const char * RunOptionsKey::dbConnStr = "OpenM.Database";

    /** model run id to restart model run */
    const char * RunOptionsKey::restartRunId = "OpenM.RestartRunId";

    /** model run name in database */
    const char * RunOptionsKey::runName = "OpenM.RunName";

    /** working set id to get input parameters */
    const char * RunOptionsKey::setId = "OpenM.SetId";

    /** working set name to get input parameters */
    const char * RunOptionsKey::setName = "OpenM.SetName";

    /** short name for: -s working set name to get input parameters */
    const char * RunShortKey::setName = "s";
    
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
}

/** array of model run option keys. */
static const char * runOptKeyArr[] = {
    RunOptionsKey::subValueCount,
    RunOptionsKey::threadCount,
    RunOptionsKey::notOnRoot,
    RunOptionsKey::dbConnStr,
    RunOptionsKey::restartRunId,
    RunOptionsKey::runName,
    RunOptionsKey::setId,
    RunOptionsKey::setName,
    RunOptionsKey::taskId,
    RunOptionsKey::taskName,
    RunOptionsKey::taskRunName,
    RunOptionsKey::taskWait,
    RunOptionsKey::profile,
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
    RunOptionsKey::subGroupValuesPrefix
};
static const size_t prefixOptSize = sizeof(prefixOptArr) / sizeof(const char *);

/** last cleanup */
MetaLoader::~MetaLoader(void) noexcept { }

/** initialize run options from command line and ini-file. */
const ArgReader MetaLoader::getRunOptions(int argc, char ** argv)
{
    // get command line options
    ArgReader ar;
    ar.parseCommandLine(
        argc, argv, true, false, runOptKeySize, runOptKeyArr, shortPairSize, shortPairArr, prefixOptSize, prefixOptArr
    );

    // load options from ini-file and append parameters section
    ar.loadIniFile(
        ar.strOption(ArgKey::iniFile).c_str(), runOptKeySize, runOptKeyArr, prefixOptSize, prefixOptArr
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
int MetaLoader::readMetaTables(IDbExec * i_dbExec, MetaHolder * io_metaStore)
{
    // find model by name digest
    io_metaStore->modelTable.reset(IModelDicTable::create(i_dbExec, OM_MODEL_NAME, OM_MODEL_DIGEST));

    io_metaStore->modelRow = io_metaStore->modelTable->byNameDigest(OM_MODEL_NAME, OM_MODEL_DIGEST);
    if (io_metaStore->modelRow == nullptr) throw DbException("model %s not found in the database", OM_MODEL_NAME);

    int mId = io_metaStore->modelRow->modelId;

    // read metadata tables
    io_metaStore->typeDic.reset(ITypeDicTable::create(i_dbExec, mId));
    io_metaStore->typeEnumLst.reset(ITypeEnumLstTable::create(i_dbExec, mId));
    io_metaStore->paramDic.reset(IParamDicTable::create(i_dbExec, mId));
    io_metaStore->paramDims.reset(IParamDimsTable::create(i_dbExec, mId));
    io_metaStore->tableDic.reset(ITableDicTable::create(i_dbExec, mId));
    io_metaStore->tableDims.reset(ITableDimsTable::create(i_dbExec, mId));
    io_metaStore->tableAcc.reset(ITableAccTable::create(i_dbExec, mId));
    io_metaStore->tableExpr.reset(ITableExprTable::create(i_dbExec, mId));
    io_metaStore->groupLst.reset(IGroupLstTable::create(i_dbExec, mId));
    io_metaStore->groupPc.reset(IGroupPcTable::create(i_dbExec, mId));

    return mId;
}

/** read model messages from database.
*
* User preferd language determined by simple match, for example:
* if user language is en_CA.UTF-8 then search done for lower case ["en-ca", "en", "model-default-langauge"].
*/
void MetaLoader::loadMessages(IDbExec * i_dbExec)
{
    if (i_dbExec == nullptr) throw ModelException("invalid (NULL) database connection");

    // find model by name digest
    unique_ptr<IModelDicTable> mdTbl(IModelDicTable::create(i_dbExec, OM_MODEL_NAME, OM_MODEL_DIGEST));

    const ModelDicRow *mdRow = mdTbl->byNameDigest(OM_MODEL_NAME, OM_MODEL_DIGEST);
    if (mdRow == nullptr) throw DbException("model %s not found in the database", OM_MODEL_NAME);

    // get list of languages
    // get model language-specific messages in all languages
    // get common runtime language-specific messages in all languages
    unique_ptr<ILangLstTable> langTbl(ILangLstTable::create(i_dbExec));
    unique_ptr<IModelWordTable> mwTbl(IModelWordTable::create(i_dbExec, mdRow->modelId));
    unique_ptr<ILangWordTable> wordTbl(ILangWordTable::create(i_dbExec));

    // if language list already set (stored in theLog) then use it
    // else get user prefered locale and "normalize" language part of it: en_CA.UTF-8 => en-ca
    list<string> langLst = theLog->getLanguages();
    if (langLst.empty()) langLst = splitLanguageName(getDefaultLocaleName());

    // make list of language id's by shroten user language: en-ca, en
    vector<int> langIdArr;
    bool isDef = false;     // if true then it is model default language

    for (const string & lang : langLst) {

        // if language code found then add id into the list
        const LangLstRow * langRow = langTbl->findFirst(
            [&lang](const LangLstRow & i_row) -> bool { return normalizeLanguageName(i_row.code) == lang; }
        );
        if (langRow != nullptr) {
            langIdArr.push_back(langRow->langId);
            if (!isDef) isDef = langRow->langId == mdRow->defaultLangId;
        }
    }

    // append model default language at the bottom of the list, if not already there
    if (!isDef) {
        const LangLstRow * langRow = langTbl->byKey(mdRow->defaultLangId);
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
* use default values for basic run options, i.e. SparseOutput = false
* validate scalar parameter value option, eg: Parameter.Age 42
* merge parameter sub-values options, eg: SubFrom.Age csv  SubValues.Age 8 SubValues.Sex default
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
    for (const ParamSubOpts & ps : subOptsArr) {

        const ParamDicRow * paramRow = metaStore->paramDic->byKey(modelId, ps.paramId);
        if (argStore.isOptionExist((paramPrefix + paramRow->paramName).c_str()))
            throw DbException("%s.%s cannot be combined with: \"%s\" or \"%s\"",
                RunOptionsKey::parameterPrefix, paramRow->paramName.c_str(), RunOptionsKey::subFromPrefix, RunOptionsKey::subValuesPrefix);
    }

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

    // return parameter name or group name from sub-values option
    const string parseParamOrGroupName(const string & i_optKey, size_t i_prefixLen)
    {
        if (i_optKey.length() <= i_prefixLen) throw ModelException("invalid (empty) parameter name or group specified: %s", i_optKey.c_str());

        return i_optKey.substr(i_prefixLen);
    }

}

/** parse sub-value options for input parameters: "SubFrom.Age", "SubValues.Age", "SubGroupFrom.Geo", "SubGroupValues.Geo"
*
* parse and validate parameter sub-values options, eg: 
*   SubFrom.Age        csv
*   SubValues.Age      [0,15]
*   SubValues.Sex      default
*   SubGroupFrom.Geo   csv
*   SubGroupValues.Geo [0,15]
* validate "SubFrom." and "SubGroupFrom." options value, it must one of "db", "csv" or "iota"
* "SubValues." and "SubGroupValues." option can be:
*   list of id's: SubValues.Age 2,1,4,3
*   range:        SubValues.Age [1,4]
*   mask:         SubValues.Age x0F
*   single id:    SubValues.Age 7
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

        // check option key: parameter name or group name maust not be empty
        string argName;
        if (isFromOpt) argName = parseParamOrGroupName(optIt->first, fromPrefix.length());
        if (isValuesOpt) argName = parseParamOrGroupName(optIt->first, valPrefix.length());
        if (isFromGroupOpt) argName = parseParamOrGroupName(optIt->first, fromGroupPrefix.length());
        if (isValuesGroupOpt) argName = parseParamOrGroupName(optIt->first, valGroupPrefix.length());

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
                ParamSubOpts & ps = addParamSubOpts(pName);

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
                ParamSubOpts & ps = addParamSubOpts(pName);

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
                    ps.subCount = ps.subIds.size();
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
                        ps.subCount = ps.subIds.size();
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

    // sort by parameter id
    std::sort(subOptsArr.begin(), subOptsArr.end(), ParamSubOpts::keyLess);

    // set defult values and validate sub-values options, collect parameter id's with multiple sub-values
    for (ParamSubOpts & ps : subOptsArr) {

        // if from option specified and number of sub-values not explicitly specified by "SubValues."
        // then assume number of sub-values for the model run
        // if from option not specified then use default rules
        if (ps.from != RunOptionsKey::defaultValue) {
            if (ps.kind == KindSubIds::none) ps.subCount = subValueCount;
        }

        // if SubFrom.Parameter = iota then parameter cannot have SubId.Parameter or SubValues.Parameter option
        if (ps.from == RunOptionsKey::iotaSubValue && ps.kind != KindSubIds::none)
            throw ModelException(
                "invalid options for parameter %s: option %s cannot be combined with %s",
                metaStore->paramDic->byKey(modelId, ps.paramId)->paramName.c_str(), RunOptionsKey::iotaSubValue, RunOptionsKey::subValuesPrefix
            );

        // sub-values count must 1 or equal to number of sub-values for that model run
        if (ps.subCount != 1 && ps.subCount != subValueCount)
            throw ModelException("invalid number of sub-values: %d specified for parameter: %s, expected 1 or %d",
                ps.subCount, metaStore->paramDic->byKey(modelId, ps.paramId)->paramName.c_str(), subValueCount);

        // collect parameter id's with multiple sub-values
        if (ps.subCount > 1) paramIdSubArr.push_back(ps.paramId);
    }
}

// find existing or add new parameter sub-values options
MetaLoader::ParamSubOpts & MetaLoader::addParamSubOpts(const string & i_paramName)
{
    // find parameter by name: it must be a model parameter
    const ParamDicRow * paramRow = metaStore->paramDic->byModelIdName(modelId, i_paramName);
    if (paramRow == nullptr)
        throw DbException("parameter %s is not an input parameter of model %s, id: %d", i_paramName.c_str(), metaStore->modelRow->name.c_str(), modelId);

    // find parameter in the list of sub-value parameters
    for (ParamSubOpts & ps : subOptsArr) {
        if (ps.paramId == paramRow->paramId) return ps; // found existing parameter sub-value options
    }

    // append parameter new options to the list of sub-value parameters
    subOptsArr.push_back(ParamSubOpts(paramRow->paramId));

    return subOptsArr.back();
}

// return name of parameters by model group name
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
