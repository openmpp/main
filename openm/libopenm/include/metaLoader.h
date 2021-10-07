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
        static constexpr const char * subValueCount = "OpenM.SubValues";

        /** model run name in database */
        static constexpr const char * runName = "OpenM.RunName";

        /** model run id to restart model run */
        static constexpr const char * restartRunId = "OpenM.RestartRunId";

        /** working set id to get input parameters */
        static constexpr const char * setId = "OpenM.SetId";

        /** working set name to get input parameters */
        static constexpr const char * setName = "OpenM.SetName";

        /** base run name to get input parameters */
        static constexpr const char * baseRunName = "OpenM.BaseRunName";

        /** model run id to get input parameters */
        static constexpr const char * baseRunId = "OpenM.BaseRunId";

        /** model run digest to get input parameters */
        static constexpr const char * baseRunDigest = "OpenM.BaseRunDigest";

        /** modeling task id */
        static constexpr const char * taskId = "OpenM.TaskId";

        /** modeling task name */
        static constexpr const char * taskName = "OpenM.TaskName";

        /** modeling task run name in database */
        static constexpr const char * taskRunName = "OpenM.TaskRunName";

        /** modeling task run id */
        static constexpr const char * taskRunId = "OpenM.TaskRunId";

        /** modeling task under external supervision */
        static constexpr const char * taskWait = "OpenM.TaskWait";

        /** profile name to get run options */
        static constexpr const char * profile = "OpenM.Profile";

        /** number of modeling threads */
        static constexpr const char * threadCount = "OpenM.Threads";

        /** if true then do not run modeling threads at root process */
        static constexpr const char * notOnRoot = "OpenM.NotOnRoot";

        /** database connection string */
        static constexpr const char * dbConnStr = "OpenM.Database";

        /** use sparse output tables */
        static constexpr const char * useSparse = "OpenM.SparseOutput";

        /** sparse null value */
        static constexpr const char * sparseNull = "OpenM.SparseNullValue";

        /** convert to string format for float, double, long double */
        static constexpr const char * doubleFormat = "OpenM.DoubleFormat";

        /** dir/to/read/input/parameter.csv */
        static constexpr const char * paramDir = "OpenM.ParamDir";

        /** if true then parameter(s) csv file(s) contain enum id's, default: enum code */
        static constexpr const char * useIdCsv = "OpenM.IdCsv";

        /** if true then parameter value is enum id, default: enum code */
        static constexpr const char * useIdParamValue = "OpenM.IdParameterValue";

        /** if positive then used for simulation progress reporting, ex: every 10% */
        static constexpr const char * progressPercent = "OpenM.ProgressPercent";

        /** if positive then used for simulation progress reporting, ex: every 1000 cases or every 0.1 time step */
        static constexpr const char * progressStep = "OpenM.ProgressStep";

        /** options started with "Parameter." treated as value of model scalar input parameter, ex: -Parameter.Age 42 */
        static constexpr const char * parameterPrefix = "Parameter";

        /** options started with "SubFrom." used to specify where to get sub-values of input parameter or group of parameters, ex: -SubFrom.Age csv */
        static constexpr const char * subFromPrefix = "SubFrom";

        /** options started with "SubValues." used specify sub-values of input parameter or group of parameters, ex: -SubValues.Age [1,4] */
        static constexpr const char * subValuesPrefix = "SubValues";

        /** exclude output tables or tables group from run results, ex: -Tables.Suppress AgeTable,IncomeGroup */
        static constexpr const char * tableSuppress = "Tables.Suppress";

        /** include only specified output tables or tables group into run results, ex: -Tables.Retain AgeTable,IncomeGroup */
        static constexpr const char * tableRetain = "Tables.Retain";

        /** import parameters from all upstream models last runs, ex: -Import.All true */
        static constexpr const char * importAll = "Import.All";

        /** options started with "Import." used to specify parameters import from upstream model run, ex: -Import.modelOne true */
        static constexpr const char * importPrefix = "Import";

        /** options started with "ImportRunDigest." used to specify run name to import parameters from, ex: -ImportRunDigest.modelOne abcdef */
        static constexpr const char * importRunDigestPrefix = "ImportRunDigest";

        /** options started with "ImportRunId." used to specify run id to import parameters from, ex: -ImportRunId.modelOne 101 */
        static constexpr const char * importRunIdPrefix = "ImportRunId";

        /** options started with "ImportRunName." used to specify run name to import parameters from, ex: -ImportRunName.modelOne GoodRun */
        static constexpr const char * importRunNamePrefix = "ImportRunName";

        /** options started with "ImportDigest." used to specify model digest to import parameters from last run of that model, ex: -ImportModelDigest.modelOne fedcba */
        static constexpr const char * importModelDigestPrefix = "ImportModelDigest";

        /** options started with "ImportId." used to specify model id to import parameters from last run of that model, ex: -ImportModelId.modelOne 123 */
        static constexpr const char * importModelIdPrefix = "ImportModelId";

        /** options started with "ImportExpr." used to specify expression name to import from output table, ex: -ImportExpr.AgeTable expr2 */
        static constexpr const char * importExprPrefix = "ImportExpr";

        /** options started with "ImportDb." used to specify database connection string to import parameters from, ex: -ImportDb.modelOne "Database=m1.sqlite;OpenMode=RedaOnly;" */
        static constexpr const char * importDbPrefix = "ImportDb";

        /** options ended with ".RunDescription" used to specify run decsription, ex: -EN.RunDescription "run model with 50,000 cases" */
        static constexpr const char * runDescrSuffix = "RunDescription";

        /** options ended with ".RunNotesPath" used to specify path to run notes file, ex: -EN.RunNotesPath EN_notes.md */
        static constexpr const char * runNotePathSuffix = "RunNotesPath";

        /** trace log to console */
        static constexpr const char * traceToConsole = "OpenM.TraceToConsole";

        /** trace log to file */
        static constexpr const char * traceToFile = "OpenM.TraceToFile";

        /** trace log file path */
        static constexpr const char * traceFilePath = "OpenM.TraceFilePath";

        /** trace log to "stamped" file */
        static constexpr const char * traceToStamped = "OpenM.TraceToStampedFile";

        /** trace use time-stamp in log "stamped" file name */
        static constexpr const char * traceUseTs = "OpenM.TraceUseTimeStamp";

        /** trace use pid-stamp in log "stamped" file name */
        static constexpr const char * traceUsePid = "OpenM.TraceUsePidStamp";

        /** do not prefix trace log messages with date-time */
        static constexpr const char * traceNoMsgTime = "OpenM.TraceNoMsgTime";
                                       
        /** language to display output messages */
        static constexpr const char * messageLang = "OpenM.MessageLanguage";
        
        /** sub-value of parameter must be in the input workset */
        static constexpr const char * dbSubValue = "db";

        /** sub-value of parameter created as integer from 0 to sub-value count */
        static constexpr const char * iotaSubValue = "iota";

        /** all parameter sub-values must be in parameter.csv file */
        static constexpr const char * csvSubValue = "csv";

        /** default value for any option */
        static constexpr const char * defaultValue = "default";

        /** all value for any option */
        static constexpr const char * allValue = "All";
    };

    /** keys for model run options (short form) */
    struct RunShortKey
    {
        /** short name for ini file name: -ini fileName.ini */
        static constexpr const char * iniFile = "ini";

        /** short name for: -s working set name to get input parameters */
        static constexpr const char * setName = "s";

        /** short name for: -p dir/to/read/input/parameter.csv */
        static constexpr const char * paramDir = "p";
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

        // language-specific options: map language id to run description and notes
        map<int, pair<string, string> > langOptsMap;

    private:
        RunOptions baseRunOpts;     // basic model run options
        ArgReader argStore;         // arguments as key-value string pairs with case-neutral search

        // merge parameter name arguments with profile_option table, ie "Parameter.Age" or "SubValues.Age" argument
        void mergeParameterProfile(
            const string & i_profileName, const char * i_prefix, const IProfileOptionTable * i_profileOpt, const vector<ParamDicRow> & i_paramRs
        );

        /** parse sub-value options for input parameters: "SubFrom.Age", "SubValues.Age" */
        void parseParamSubOpts(void);

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
