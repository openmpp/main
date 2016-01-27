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

namespace openm
{
    /** keys for model run options */
    struct RunOptionsKey
    {
        /** parameters started with "Parameter." treated as value of model scalar input parameters */
        static const char * parameterPrefix;

        /** number of sub-samples */
        static const char * subSampleCount;

        /** number of modeling threads */
        static const char * threadCount;

        /** database connection string */
        static const char * dbConnStr;

        /** model run id */
        static const char * runId;

        /** working set id to get input parameters */
        static const char * setId;

        /** working set name to get input parameters */
        static const char * setName;

        /** modeling task id */
        static const char * taskId;

        /** modeling task name */
        static const char * taskName;

        /** modeling task run id */
        static const char * taskRunId;

        /** profile name to get run options, default is model name */
        static const char * profile;

        /** use sparse output tables */
        static const char * useSparse;

        /** sparse null value */
        static const char * sparseNull;

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
    };

    /** keys for model run options (short form) */
    struct RunShortKey
    {
        /** compatibility short name for options file name: -s fileName.ini */
        static const char * optionsFile;

        /** short name for options file name: -ini fileName.ini */
        static const char * optionsFileIni;
    };

    /** model metadata loader: read and broadcast metadata and run options. */
    class MetaLoader
    {
    public:
        /** last cleanup */
        virtual ~MetaLoader(void) throw() = 0;

        /** model id in database */
        int modelId;

        /** total number of subsamples */
        int subSampleCount;

        /** max number of modeling threads */
        int threadCount;

        /** arguments from command line and ini-file */
        const ArgReader & argOpts(void) const { return argStore; }

        /** model metadata tables */
        const MetaRunHolder * meta(void) const { return metaStore.get(); }

        /** initialize run options from command line and ini-file */
        static const ArgReader getRunOptions(int argc, char ** argv);

    protected:
        unique_ptr<MetaRunHolder> metaStore;    // metadata tables

        /** create metadata loader. */
        MetaLoader(const ArgReader & i_argStore) :
            modelId(0),
            subSampleCount(0),
            threadCount(1),
            argStore(i_argStore)
        { }

        // read metadata tables from db, except of run_option table
        static const ModelDicRow * readMetaTables(IDbExec * i_dbExec, MetaRunHolder * io_metaStore);

        // broadcast metadata tables from root to all modeling processes
        void broadcastMetaData(IMsgExec * i_msgExec, MetaRunHolder * io_metaStore);

        // broadcast run options from root to all modeling processes
        void broadcastRunId(IMsgExec * i_msgExec, int * io_runId);

        // broadcast run options from root to all modeling processes
        void broadcastRunOptions(IMsgExec * i_msgExec, unique_ptr<IRunOptionTable> & io_runOptionTbl);

        // merge command line and ini-file arguments with profile_option table values
        void mergeProfile(IDbExec * i_dbExec, const ModelDicRow * i_modelRow);

        // create task run entry in database
        int createTaskRun(int i_taskId, IDbExec * i_dbExec);

        // find modeling task, if specified
        int findTask(IDbExec * i_dbExec, const ModelDicRow * i_modelRow);

        // find source working set for input parameters
        int findWorkset(int i_setId, IDbExec * i_dbExec, const ModelDicRow * i_modelRow);

    private:
        ArgReader argStore;     // arguments as key-value string pairs with case-neutral search

        // broadcast meta table db rows
        template <class MetaTbl>
        static void broadcastMetaTable(IMsgExec * i_msgExec, MsgTag i_msgTag, unique_ptr<MetaTbl> & io_tableUptr);

    private:
        MetaLoader(const MetaLoader & i_metaLoader) = delete;
        MetaLoader & operator=(const MetaLoader & i_metaLoader) = delete;
    };
}

#endif  // META_LOADER_H
