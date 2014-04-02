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
        /** number of sub-samples */
        static const char * subSampleCount;

        /** database connection string */
        static const char * dbConnStr;

        /** model run id */
        static const char * runId;

        /** working set id to get input parameters */
        static const char * setId;

        /** working set name to get input parameters */
        static const char * setName;

        /** profile name to get run options, default is model name */
        static const char * profile;

        /** use sparse output tables */
        static const char * useSparse;

        /** sparse null value */
        static const char * sparseNull;

        /** parameters started with "Parameter." treated as value of model scalar input parameters */
        static const char * parameterPrefix;
    };

    /** keys for model run options (short form) */
    struct RunShortKey
    {
        /** short name for options file name: -s fileName.ini */
        static const char * optionsFile;

        /** compatibility short name for options file name: -sc fileName.ini */
        static const char * optionsFileSc;
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

        /** subsample number for current modeling process */
        int subSampleNumber;

        /** arguments from command line and ini-file. */
        const ArgReader & argOpts(void) const { return argStore; }

        /** complete model run initialization: create run and input parameters in database. */
        MetaRunHolder * init(bool i_isMpiUsed, IDbExec * i_dbExec, IMsgExec * i_msgExec);

    private:
        bool isSubCountCmd;     // if true then sub-sample count specified on command line
        ArgReader argStore;     // arguments as key-value string pairs with case-neutral search

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

        // broadcast metadata tables from root to all modeling processes
        void broadcastMetaTables(IMsgExec * i_msgExec, MetaRunHolder * io_metaStore);
        
        // broadcast meta table db rows
        template <class MetaTbl>
        void broadcastTable(unique_ptr<MetaTbl> & i_tableUptr, IMsgExec * i_msgExec, MsgTag i_msgTag);
    };
}

#endif  // RUN_CTRL_H
