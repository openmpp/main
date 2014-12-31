/**
* @file
* OpenM++ modeling library: run controller class to create new model run
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#ifndef RUN_CTRL_H
#define RUN_CTRL_H

#include "libopenm/common/omFile.h"
#include "libopenm/common/argReader.h"

using namespace std;

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

    /** run controller: get run options and create new run id and input parameters in database. */
    class RunController
    {
    public:
        /** create run controller, initialize run options from command line and ini-file. */
        RunController(int argc, char ** argv);

        /** run id, if defined by run options or zero */
        int runId;

        /** number of subsamples */
        int subSampleCount;

        /** subsample staring number for current modeling process */
        int subBaseNumber;

        /** number of modeling threads */
        int threadCount;

        /** arguments from command line and ini-file. */
        const ArgReader & argOpts(void) const { return argStore; }

        /** complete model run initialization: create run and input parameters in database. */
        MetaRunHolder * init(bool i_isMpiUsed, IDbExec * i_dbExec, IMsgExec * i_msgExec);

    private:
        ArgReader argStore;     // arguments as key-value string pairs with case-neutral search

        // return subsample number for the process rank
        //static int subNumberByRank(int i_rank, int i_subCount)
        //{
        //    return (i_rank > 0 && i_subCount > 0) ? i_rank % i_subCount : 0;
        //}

        // return actual run id for the process rank
        //static int runIdByRank(int i_rank, int i_subCount, int i_baseRunid)
        //{
        //    return i_baseRunid + ((i_rank > 0 && i_subCount > 0) ? i_rank / i_subCount : 0);
        //}

        // create new run or next subsample for existing run
        void createRunSubsample(bool i_isMpiUsed, IDbExec * i_dbExec, IMsgExec * i_msgExec, MetaRunHolder * io_metaStore);

        // create run options and save it in run_option table
        void createRunOptions(IDbExec * i_dbExec, const ModelDicRow * i_mdRow, const MetaRunHolder * i_metaStore);

        // copy input parameters from "base" run and working set into new run id
        void createRunParameters(
            bool i_isNewRunCreated, 
            IDbExec * i_dbExec, 
            const ModelDicRow * i_mdRow, 
            const MetaRunHolder * i_metaStore
            );

        // read metadata tables from db, except of run_option table
        void readMetaTables(IDbExec * i_dbExec, MetaRunHolder * io_metaStore);

        //  broadcast metadata: run id, subsample count, subsample number and meta tables from root to all modelling processes
        void broadcastMetaData(IMsgExec * i_msgExec, MetaRunHolder * io_metaStore);
        
        // broadcast meta table db rows
        template <class MetaTbl>
        void broadcastMetaTable(unique_ptr<MetaTbl> & i_tableUptr, IMsgExec * i_msgExec, MsgTag i_msgTag);
    };
}

#endif  // RUN_CTRL_H
