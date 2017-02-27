/** modelOneDb: OpenM++ db library usage example */
// Copyright (c) 2016 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#ifndef MODEL_ONE_DB_H
#define MODEL_ONE_DB_H

#include "libopenm/omLog.h"
#include "libopenm/omError.h"
#include "libopenm/common/argReader.h"
#include "libopenm/db/dbMetaRow.h"

// includes below normally not exposed to the model
// it is a public interfaces inside of runtime libarary
#include "dbExec.h"
#include "metaHolder.h"
#include "dbParameter.h"
#include "dbOutputTable.h"

using namespace openm;

// read model metadata from db (part of metadata which required to run the model)
extern MetaHolder * readMetaTables(IDbExec * i_dbExec);

// select first model run where number of subsamples == 1 and completed successfuly
extern RunLstRow selectFirstRun(IDbExec * i_dbExec, const MetaHolder * i_metaStore);

// read all model parameters
extern void readAllParameters(IDbExec * i_dbExec, const MetaHolder * i_metaStore, int i_runId);

// read input parameter values used for specific model run */
extern void readModelParameter(
    IDbExec * i_dbExec,
    const MetaHolder * i_metaStore,
    int i_runId,
    const char * i_name,
    int i_subId, 
    const type_info & i_type,
    size_t i_size,
    void * io_valueArr
);

// do the simulation
extern void doSimulation(void);

// write all model output tables
extern void writeAllOutputTables(IDbExec * i_dbExec, const MetaHolder * i_metaStore, const ArgReader & i_argOpts, int i_runId);

// write model output table
extern void writeDemoOutputTable(
    IDbExec * i_dbExec,
    const MetaHolder * i_metaStore,
    int i_runId,
    const ArgReader & i_argOpts,
    const char * i_name,
    size_t i_accCount,
    size_t i_accSize,
    const double * i_accValues
);

namespace openm
{
    /** default error message: "demo model unknown error" */
    extern const char demoUnknownErrorMessage[];   

    /** demo model exception */
    typedef OpenmException<4000, demoUnknownErrorMessage> DemoException;

    /** model run status codes */
    // copy from omModel.h
    struct RunStatus
    {
        /** p = run in progress */
        static const char * progress;

        /** s = completed successfully */
        static const char * done;
    };
}

#endif  // MODEL_ONE_DB_H
