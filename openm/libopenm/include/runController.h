/**
* @file
* OpenM++ modeling library: run controller class to create new model run
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#ifndef RUN_CTRL_H
#define RUN_CTRL_H

#include "metaLoader.h"

using namespace std;

namespace openm
{
    /** run controller: get run options and create new run id and input parameters in database. */
    class RunController : public MetaLoader
    {
    public:
        /** create run controller, initialize run options from command line and ini-file. */
        RunController(int argc, char ** argv) : MetaLoader(argc, argv),
            subFirstNumber(0),
            subPerProcess(1),
            selfSubCount(0),
            setId(0),
            runId(0)
        { }

        /** subsample staring number for current modeling process */
        int subFirstNumber;

        /** number of subsamples for each child modeling process */
        int subPerProcess;

        /** number of subsamples for current process */
        int selfSubCount;

        /** complete model run initialization: create run and input parameters in database. */
        int initRun(bool i_isMpiUsed, IDbExec * i_dbExec, IMsgExec * i_msgExec, MetaRunHolder * io_metaStore);

        /** receive accumulators of output tables subsamples and write into database. */
        bool receiveSubSamples(bool i_isMpiUsed, IDbExec * i_dbExec, IMsgExec * i_msgExec, const MetaRunHolder * i_metaStore);

        /** receive outstanding accumulators and wait until outstanding send completed. */
        void receiveSendLast(bool i_isMpiUsed, IDbExec * i_dbExec, IMsgExec * i_msgExec, const MetaRunHolder * i_metaStore);

        /** return accumulator message tag */
        static int accMsgTag(int i_subNumber, int i_subSampleCount, int i_accIndex)
        {
            return ((int)MsgTag::outSubsampleBase + i_accIndex) * i_subSampleCount + i_subNumber;
        }

    private:
        int setId;      // if > 0 then set id of source input parametes
        int runId;      // model run id

        // helper struct to receive output table values for each accumulator
        struct AccReceiveItem
        {
            int subNumber;          // subsample number to receive
            int tableId;            // output table id
            int accId;              // accumulator id
            long long valueSize;    // size of accumulator data
            bool isReceived;        // if true then data received
            int senderRank;         // sender rank: process where accumulator calculated
            int msgTag;             // accumulator message tag

            AccReceiveItem(
                int i_subNumber,
                int i_subSampleCount,
                int i_subPerProcess,
                int i_tableId, 
                int i_accId, 
                int i_accIndex,
                long long i_valueSize
                ) :
                subNumber(i_subNumber),
                tableId(i_tableId),
                accId(i_accId),
                valueSize(i_valueSize),
                isReceived(false),
                senderRank(accFromRank(i_subNumber, i_subPerProcess)),
                msgTag(accMsgTag(i_subNumber, i_subSampleCount, i_accIndex))
            {
            }

            // return accumulator sender rank
            static int accFromRank(int i_subNumber, int i_subPerProcess)
            {
                return 1 + (i_subNumber / i_subPerProcess);
            }
        };

        vector<AccReceiveItem> accRecvVec;      // list of accumulators to be received

    private:
        // find working set to run the model
        bool nextSetId(IDbExec * i_dbExec, const MetaRunHolder * i_metaStore);

        // create new run or next subsample for existing run
        void createRunSubsample(
            bool i_isMpiUsed, IDbExec * i_dbExec, IMsgExec * i_msgExec, const MetaRunHolder * i_metaStore
            );

        // create run options in run_option table
        void createRunOptions(bool i_isFirstRunCustom, IDbExec * i_dbExec);

        // copy input parameters from "base" run and working set into new run id
        void createRunParameters(
            bool i_isNewRunCreated,
            IDbExec * i_dbExec,
            const ModelDicRow * i_mdRow,
            const MetaRunHolder * i_metaStore
            );

        // create list of accumulators to be received from child modelling processes
        void initAccReceiveList(const MetaRunHolder * i_metaStore);

    private:
        RunController(const RunController & i_runCtrl) = delete;
        RunController & operator=(const RunController & i_runCtrl) = delete;
    };
}

#endif  // RUN_CTRL_H
