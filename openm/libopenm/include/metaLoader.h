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
#include "modelRunState.h"

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

        /** modelling task id */
        static const char * taskId;

        /** modelling task name */
        static const char * taskName;

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
        /** create metadata loader, initialize run options from command line and ini-file. */
        MetaLoader(int argc, char ** argv);

        /** model id in database */
        int modelId;

        /** total number of subsamples */
        int subSampleCount;

        /** number of modeling processes */
        int processCount;

        /** max number of modeling threads */
        int threadCount;

        /** arguments from command line and ini-file. */
        const ArgReader & argOpts(void) const { return argStore; }

        /** read metadata from db, deterine number of subsamples, merge command line and ini-file options with db profile table. */
        MetaRunHolder * init(bool i_isMpiUsed, IDbExec * i_dbExec, IMsgExec * i_msgExec);

    protected:
        int taskId;             // if > 0 then modelling task id
        int taskRunId;          // if > 0 then modelling task run id
        ArgReader argStore;     // arguments as key-value string pairs with case-neutral search

        // task item: workset, result run and status
        struct TaskItem
        {
            int setId;              // set id of input parameters
            int runId;              // if >0 then run id
            ModelRunState state;    // run status

            TaskItem(int i_setId) : setId(i_setId), runId(0) { }

        private:
            TaskItem(void) = delete;
        };

        list<TaskItem> taskRunLst;  // pairs of (set, run) for modelling task

    protected:
        // broadcast run options from root to all modelling processes
        void broadcastRunId(IMsgExec * i_msgExec, int * io_runId);

        // broadcast run options from root to all modelling processes
        void broadcastRunOptions(IMsgExec * i_msgExec, unique_ptr<IRunOptionTable> & i_runOptionTbl);

        // find source working set for input parameters
        int findWorkset(IDbExec * i_dbExec, const ModelDicRow * i_mdRow);

    private:
        // initialization for main (root) process
        void initRoot(IDbExec * i_dbExec, MetaRunHolder * io_metaStore);

        // read metadata tables from db, except of run_option table
        const ModelDicRow * readMetaTables(IDbExec * i_dbExec, MetaRunHolder * io_metaStore);

        //  broadcast metadata: run id, subsample count and meta tables from root to all modelling processes
        void broadcastMetaData(IMsgExec * i_msgExec, MetaRunHolder * io_metaStore);

        // broadcast meta table db rows
        template <class MetaTbl>
        static void broadcastMetaTable(unique_ptr<MetaTbl> & i_tableUptr, IMsgExec * i_msgExec, MsgTag i_msgTag);

        // merge command line and ini-file arguments with profile_option table values
        void mergeProfile(IDbExec * i_dbExec, const ModelDicRow * i_mdRow, const MetaRunHolder * i_metaStore);

        // read modelling task definition
        void readTask(IDbExec * i_dbExec, const ModelDicRow * i_mdRow);

        // create task run entry in database
        void createTaskRun(IDbExec * i_dbExec);

        // find modelling task, if specified
        void findTask(IDbExec * i_dbExec, const ModelDicRow * i_mdRow);

    private:
        MetaLoader(const MetaLoader & i_metaLoader) = delete;
        MetaLoader & operator=(const MetaLoader & i_metaLoader) = delete;
    };
}

#endif  // META_LOADER_H
