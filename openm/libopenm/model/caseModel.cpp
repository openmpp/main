// OpenM++ modeling library: case-based model
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "model.h"
using namespace openm;

ICaseModel::~ICaseModel() throw() { }

CaseModel::CaseModel(
    bool i_isMpiUsed,
    int i_modelId,
    int i_subCount, 
    int i_subNumber,
    IDbExec * i_dbExec, 
    IMsgExec * i_msgExec, 
    const MetaRunHolder * i_metaStore
    ) :
    ModelBase(i_isMpiUsed, i_modelId, i_dbExec, i_msgExec, i_metaStore)
{
    if (i_msgExec == NULL) throw ModelException("invalid (NULL) message passing interface");
    if (i_metaStore == NULL) throw ModelException("invalid (NULL) metadata tables storage");

    setOptions(i_subCount, i_subNumber);
}

CaseModel::~CaseModel(void) throw()
{
    try {
        // theLog->logMsg("CaseModel dtor");
    }
    catch (...) { }
}

/** set default run options for case-based models. */
void CaseModel::defaultOptions(CaseRunOptions & io_options)
{
    defaultBaseOptions(io_options);
    io_options.cases = 1;
}

/** get case-based model default run options as list of (key, value) strings. */
NoCaseMap CaseModel::defaultOptions(void)
{
    CaseRunOptions opts;
    defaultOptions(opts);
    NoCaseMap optMap = defaultBaseOptions(std::cref(opts));
    optMap[RunOptionsKey::cases] = to_string(opts.cases);
    return optMap;
}

/** set run id and run options for case-based model. */
void CaseModel::setOptions(int i_subCount, int i_subNumber)
{
    defaultOptions(runOptions_);
    setBaseOptions(i_subCount, i_subNumber, runOptions_);
    runOptions_.cases = metaStore->runOption->longValue(metaStore->runId, RunOptionsKey::cases, runOptions_.cases);
}
