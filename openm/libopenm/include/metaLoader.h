/**
* @file
* OpenM++ modeling library: model metadata loader to read and broadcast metadata and run options.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#ifndef META_LOADER_H
#define META_LOADER_H

#include <algorithm>
using namespace std;

#include "libopenm/common/argReader.h"
#include "msg.h"

namespace openm
{
    /** keys for model run options */
    struct RunOptionsKey
    {
        /** number of sub-values */
        static const char * subValueCount;

        /** model run name in database */
        static const char * runName;

        /** model run id to restart model run */
        static const char * restartRunId;

        /** working set id to get input parameters */
        static const char * setId;

        /** working set name to get input parameters */
        static const char * setName;

        /** base run name to get input parameters */
        static const char * baseRunName;

        /** model run id to get input parameters */
        static const char * baseRunId;

        /** model run digest to get input parameters */
        static const char * baseRunDigest;

        /** modeling task id */
        static const char * taskId;

        /** modeling task name */
        static const char * taskName;

        /** modeling task run name in database */
        static const char * taskRunName;

        /** modeling task run id */
        static const char * taskRunId;

        /** modeling task under external supervision */
        static const char * taskWait;

        /** profile name to get run options */
        static const char * profile;

        /** number of modeling threads */
        static const char * threadCount;

        /** if true then do not run modeling threads at root process */
        static const char * notOnRoot;

        /** database connection string */
        static const char * dbConnStr;

        /** use sparse output tables */
        static const char * useSparse;

        /** sparse null value */
        static const char * sparseNull;

        /** convert to string format for float, double, long double */
        static const char * doubleFormat;

        /** dir/to/read/input/parameter.csv */
        static const char * paramDir;

        /** if true then parameter(s) csv file(s) contain enum id's, default: enum code */
        static const char * useIdCsv;

        /** if true then parameter value is enum id, default: enum code */
        static const char * useIdParamValue;

        /** if positive then used for simulation progress reporting, ex: every 10% */
        static const char * progressPercent;

        /** if positive then used for simulation progress reporting, ex: every 1000 cases or every 0.1 time step */
        static const char * progressStep;

        /** options started with "Parameter." treated as value of model scalar input parameter, ex: -Parameter.Age 42 */
        static const char * parameterPrefix;

        /** options started with "SubFrom." used to specify where to get sub-values of input parameter, ex: -SubFrom.Age csv */
        static const char * subFromPrefix;

        /** options started with "SubValues." used specify sub-values of input parameter, ex: -SubValues.Age [1,4] */
        static const char * subValuesPrefix;

        /** options started with "SubGroupFrom." used to specify where to get sub-values for a group of input parameter, ex: -SubGroupFrom.Geo csv */
        static const char * subGroupFromPrefix;

        /** options started with "SubGroupValues." used specify sub-values for a group of input parameter, ex: -SubGroupValues.Geo [1,4] */
        static const char * subGroupValuesPrefix;

        /** import parameters from all upstream models last runs, ex: -Import.All true */
        static const char * importAll;

        /** options started with "Import." used to specify parameters import from upstream model run, ex: -Import.modelOne true */
        static const char * importPrefix;

        /** options started with "ImportRunDigest." used to specify run name to import parameters from, ex: -ImportRunDigest.modelOne abcdef */
        static const char * importRunDigestPrefix;

        /** options started with "ImportRunId." used to specify run id to import parameters from, ex: -ImportRunId.modelOne 101 */
        static const char * importRunIdPrefix;

        /** options started with "ImportRunName." used to specify run name to import parameters from, ex: -ImportRunName.modelOne GoodRun */
        static const char * importRunNamePrefix;

        /** options started with "ImportDigest." used to specify model digest to import parameters from last run of that model, ex: -ImportModelDigest.modelOne fedcba */
        static const char * importModelDigestPrefix;

        /** options started with "ImportId." used to specify model id to import parameters from last run of that model, ex: -ImportModelId.modelOne 123 */
        static const char * importModelIdPrefix;

        /** options started with "ImportExpr." used to specify expression name to import from output table, ex: -ImportExpr.AgeTable expr2 */
        static const char * importExprPrefix;

        /** options started with "ImportDatabase." used to specify database connection string to import parameters from, ex: -ImportDatabase.modelOne "Database=m1.sqlite;OpenMode=RedaOnly;" */
        static const char * importDbPrefix;

        /** options ended with ".RunDescription" used to specify run decsription, ex: -EN.RunDescription "run model with 50,000 cases" */
        static const char * runDescrSuffix;

        /** options started with "Suppress." used to exclude output table or tables group from run results, ex: -Suppress.AgeTable true */
        static const char * suppressPrefix;

        /** trace log to console */
        static const char * traceToConsole;

        /** trace log to file */
        static const char * traceToFile;

        /** trace log file path */
        static const char * traceFilePath;

        /** trace log to "stamped" file */
        static const char * traceToStamped;

        /** trace use time-stamp in log "stamped" file name */
        static const char * traceUseTs;

        /** trace use pid-stamp in log "stamped" file name */
        static const char * traceUsePid;

        /** do not prefix trace log messages with date-time */
        static const char * traceNoMsgTime;
                                       
        /** language to display output messages */
        static const char * messageLang;
        
        /** log version info */
        static const char * version;

        /** sub-value of parameter must be in the input workset */
        static const char * dbSubValue;

        /** sub-value of parameter created as integer from 0 to sub-value count */
        static const char * iotaSubValue;

        /** all parameter sub-values must be in parameter.csv file */
        static const char * csvSubValue;

        /** default value for any option */
        static const char * defaultValue;

        /** all value for any option */
        static const char * allValue;
    };

    /** keys for model run options (short form) */
    struct RunShortKey
    {
        /** short name for ini file name: -ini fileName.ini */
        static const char * iniFile;

        /** short name for: -s working set name to get input parameters */
        static const char * setName;

        /** short name for: -p dir/to/read/input/parameter.csv */
        static const char * paramDir;
    };

    /** model metadata loader: read and broadcast metadata and run options. */
    class MetaLoader
    {
    public:
        /** last cleanup */
        virtual ~MetaLoader(void) noexcept = 0;

        /** total number of sub-values */
        int subValueCount;

        /** max number of modeling threads */
        int threadCount;

        /** arguments from command line and ini-file */
        const ArgReader & argOpts(void) const { return argStore; }

        /** model metadata tables */
        const MetaHolder * meta(void) const { return metaStore.get(); }

        /** return basic model run options */
        const RunOptions modelRunOptions(int i_subCount, int i_subId) const;

        /** initialize run options from command line and ini-file */
        static const ArgReader getRunOptions(int argc, char ** argv);

        /** return sub-values count by parameter id */
        int parameterSubCount(int i_paramId) const
        {
            return binary_search(paramIdSubArr.cbegin(), paramIdSubArr.cend(), i_paramId) ? subValueCount : 1;
        }

        /** return index of parameter by name */
        int parameterIdByName(const char * i_name) const;

        /** return id of output table by name */
        int tableIdByName(const char * i_name) const;

        /** check by name if output table suppressed */
        bool isSuppressed(const char * i_name) const { return isSuppressed(tableIdByName(i_name)); }

        /** check by id if output table suppressed */
        bool isSuppressed(int i_tableId) const
        {
            return binary_search(tableIdSuppressArr.cbegin(), tableIdSuppressArr.cend(), i_tableId);
        }

    protected:
        int modelId;                        // model id in database
        unique_ptr<MetaHolder> metaStore;   // metadata tables
        vector<int> paramIdSubArr;          // ids of parameters where sub-values count same as model run sub-values count
        vector<int> tableIdSuppressArr;     // id's of tables to suppress from calculation and output

        /** create metadata loader. */
        MetaLoader(const ArgReader & i_argStore) :
            subValueCount(0),
            threadCount(1),
            modelId(0),
            argStore(i_argStore)
        { }

        /** return basic model run options */
        const RunOptions & modelRunOptions(void) const { return baseRunOpts; }

        /** set basic model run options */
        void setRunOptions(const RunOptions & i_opts) { baseRunOpts = i_opts; }

        // read metadata tables from db, except of run_option table
        static int readMetaTables(IDbExec * i_dbExec, MetaHolder * io_metaStore, const char * i_name, const char * i_digest);

        /** read model messages from database.
        *
        * User preferd language determined by simple match, for example:
        * if user language is en_CA.UTF-8 then search done for lower case ["en-ca", "en", "model-default-language"].
        */
        void loadMessages(IDbExec * i_dbExec);

        /** merge command line and ini-file arguments with profile_option table values. */
        void mergeOptions(IDbExec * i_dbExec);

        /** insert new or update existing argument option. */
        void setArgOpt(const string & i_key, const string & i_value) { argStore.args[i_key] = i_value; }

        // enum to indicate what kind of sub id's selected: single, default, range or list
        enum class KindSubIds : int
        {
            none = 0,       // SubValues option not defined
            single = 1,     // single sub id
            defaultId = 2,  // "default" single sub id
            range =3,       // range of sub id's [first id, last id]
            list = 4        // list of sub id's
        };

        // parameter sub-values options:
        // SubFrom:   get sub-values from "db", "iota" or "csv"
        // SubValues: can be list of id's: 1,2,3,4 or range: [1,4] or mask: x0F or single id: 7 or default id: "default"
        struct ParamSubOpts
        {
            int subCount = 1;                                   // number of sub-values
            KindSubIds kind = KindSubIds::none;                 // is it a single sub id, range or list of id's
            vector<int> subIds;                                 // list of sub id's to use, if not default id
            const char * from = RunOptionsKey::defaultValue;    // get sub-values from: "db", "iota" or "csv"
        };

        map<int, ParamSubOpts> subOptsMap;  // map parameter id to sub-values options

        // enum to indicate how to find source model run for import
        enum class ImportKind : int
        {
            none = 0,           // no import
            modelName = 1,      // use model name: last run id of model with that name
            modelId = 2,        // use model id: last run id of that model id
            modelDigest = 3,    // use model digest: last run id of that model digest
            runName = 4,        // use run name: last run id with that name
            runId = 5,          // use run id
            runDigest = 6       // use run digest
        };

        // import options for each model run
        struct ImportOpts {
            ImportKind kind = ImportKind::none;
            int modelId = 0;        // source model id
            string modelDigest;     // source model digest
            int runId = 0;          // source run id
            string runName;         // source run name
            string runDigest;       // source run digest
            string connectionStr;   // if not empty then db connection string
        };
        map<string, ImportOpts> importOptsMap;  // import options for each model name

        // import source for each model run: run id and digest, model id and digest
        struct RunImportOpts {
            int runId = 0;                  // source run id
            string runDigest;               // source run digest
            int modelId = 0;                // source model id
            string modelDigest;             // source model digest
            unique_ptr<MetaHolder> meta;    // metadata rows: source model, parameters and output tables to import from
            unique_ptr<IDbExec> dbExec;     // database connection, if null then use "main" database connection
        };

        // import options for parameter
        struct ParamImportOpts {
            string expr;    // output table expression name to import as parameter value
        };
        map<int, ParamImportOpts> paramImportOptsMap;   // parameter id mapped to import options

        // enum to specify what kind of language-specific option it is
        enum class LangOptKind {
            none = 0,       // undefined
            runDescr = 1    // run description
        };

        // language-specific options: map (kind, language id) to option value
        map<pair<LangOptKind, int>, string> langOptsMap;

    private:
        RunOptions baseRunOpts;     // basic model run options
        ArgReader argStore;         // arguments as key-value string pairs with case-neutral search

        // merge parameter name arguments with profile_option table, ie "Parameter.Age" or "SubValues.Age" argument
        void mergeParameterProfile(
            const string & i_profileName, const char * i_prefix, const IProfileOptionTable * i_profileOpt, const vector<ParamDicRow> & i_paramRs
        );

        /** parse sub-value options for input parameters: "SubFrom.Age", "SubValues.Age", "SubGroupFrom.Geo", "SubGroupValues.Geo" */
        void parseParamSubOpts(void);

        // return names of all parameters included in parameter group
        const vector<string> expandParamGroup(int i_modelId, const string & i_groupName) const;

        /** parse parameters import options: get source run to import parameters */
        void parseImportOptions(void);

        /** parse suppression options to build list of tables to exclude from calculation and run output results */
        void parseSuppressOptions(void);

        /** parse language-specific options to get language code and option value */
        void parseLangOptions(void);

    private:
        MetaLoader(const MetaLoader & i_metaLoader) = delete;
        MetaLoader & operator=(const MetaLoader & i_metaLoader) = delete;
    };
}

#endif  // META_LOADER_H
