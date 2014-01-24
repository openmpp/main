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

        /** starting seed for random number generator */
        static const char * seed;

        /** number of cases */
        static const char * cases;

        /** simulation end time */
        static const char * endTime;

        /** database connection string */
        static const char * dbConnStr;

        /** model run id */
        static const char * runId;

        /** working set id to get input parameters */
        static const char * setId;

        /** profile name to get run options, default is model name */
        static const char * profile;

        /** use sparse output tables */
        static const char * useSparse;

        /** sparse null value */
        static const char * sparseNull;
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

        /** run options as key-value pairs */
        const ArgReader & args(void) const { return argStore; }

        /** complete model run initialization: create run and input parameters in database. */
        void init(bool i_isMpiUsed, IDbExec * i_dbExec, IMsgExec * i_msgExec);

        /** load metadata tables from db and broadcast to all modeling processes. */
        MetaRunHolder * loadMetaTables(bool i_isMpiUsed, IDbExec * i_dbExec, IMsgExec * i_msgExec);

    private:
        ArgReader argStore;     // run options as key-value string pairs with case-neutral search
        bool isSubCountCmd;     // if true then sub-sample count specified on command line

        // save run options in run_option table
        void saveRunOptions(IDbExec * i_dbExec, const ModelDicRow * i_mdRow);

        // copy input parameters from "base" run and working set into new run id
        void createRunParameters(bool i_isNewRunCreated, IDbExec * i_dbExec, const ModelDicRow * i_mdRow);

        // broadcast meta table db rows
        template <class MetaTbl>
        void broadcastTable(unique_ptr<MetaTbl> & i_tableUptr, IMsgExec * i_msgExec, MsgTag i_msgTag);
    };
}

#endif  // RUN_CTRL_H
