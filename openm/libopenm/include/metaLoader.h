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
        /** options started with "Parameter." treated as value of model scalar input parameters, ex: "-Parameter.Age 42" */
        static const char * parameterPrefix;

        /** options started with "SubValue." used to describe sub-values of model input parameters, ex: "-SubValue.Age csv" */
        static const char * subValuePrefix;

        /** number of sub-values */
        static const char * subValueCount;

        /** number of modeling threads */
        static const char * threadCount;

        /** if true then do not run modeling threads at root process */
        static const char * notOnRoot;

        /** database connection string */
        static const char * dbConnStr;

        /** model run id to restart model run */
        static const char * restartRunId;

        /** model run name to in database */
        static const char * runName;

        /** working set id to get input parameters */
        static const char * setId;

        /** working set name to get input parameters */
        static const char * setName;

        /** modeling task id */
        static const char * taskId;

        /** modeling task name */
        static const char * taskName;

        /** modeling task under external supervision */
        static const char * taskWait;

        /** modeling task run id */
        static const char * taskRunId;

        /** profile name to get run options, default is model name */
        static const char * profile;

        /** use sparse output tables */
        static const char * useSparse;

        /** sparse null value */
        static const char * sparseNull;

        /** if positive then used for simulation progress reporting, ex: every 10% */
        static const char * progressPercent;

        /** if positive then used for simulation progress reporting, ex: every 1000 cases or every 0.1 time step */
        static const char * progressStep;

        /** convert to string format for float, double, long double */
        static const char * doubleFormat;

        /** dir/to/read/input/parameter.csv */
        static const char *paramDir;

        /** if true then parameter(s) csv file(s) contain enum id's, default: enum code */
        static const char *useIdCsv;

        /** if true then parameter value is enum id, default: enum code */
        static const char * useIdParamValue;

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
        
        /** sub-value of parameter must be in the input workset */
        static const char * dbSubValue;

        /** sub-value of parameter created as integer from 0 to sub-value count */
        static const char * iotaSubValue;

        /** all parameter sub-values must be in parameter.csv file */
        static const char * csvSubValue;
    };

    /** keys for model run options (short form) */
    struct RunShortKey
    {
        /** short name for options file name: -ini fileName.ini */
        static const char * optionsFile;

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
        virtual ~MetaLoader(void) throw() = 0;

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

    protected:
        int modelId;                        // model id in database
        unique_ptr<MetaHolder> metaStore;   // metadata tables
        vector<int> paramIdSubArr;          // ids of parameters with sub-values

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
        static int readMetaTables(IDbExec * i_dbExec, MetaHolder * io_metaStore);

        /** read model messages from database.
        *
        * User preferd language determined by simple match, for example:
        * if user language is en_CA.UTF-8 then search done for lower case ["en-ca", "en", "model-default-langauge"].
        */
        void loadMessages(IDbExec * i_dbExec);

        // merge command line and ini-file arguments with profile_option table values
        void mergeOptions(IDbExec * i_dbExec);

        // create task run entry in database
        int createTaskRun(int i_taskId, IDbExec * i_dbExec);

        // find modeling task, if specified
        int findTask(IDbExec * i_dbExec);

        // find source working set for input parameters
        int findWorkset(int i_setId, IDbExec * i_dbExec) const;

    private:
        RunOptions baseRunOpts;     // basic model run options
        ArgReader argStore;         // arguments as key-value string pairs with case-neutral search

        // merge parameter name arguments with profile_option table, ie "Parameter.Age" or "SubValue.Age" argument
        void mergeParameterProfile(
            const string & i_profileName, const char * i_prefix, const IProfileOptionTable * i_profileOpt, const vector<ParamDicRow> & i_paramRs
        );

    private:
        MetaLoader(const MetaLoader & i_metaLoader) = delete;
        MetaLoader & operator=(const MetaLoader & i_metaLoader) = delete;
    };
}

#endif  // META_LOADER_H
