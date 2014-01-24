// OpenM++ modeling library: time-based model
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "model.h"
using namespace openm;

ITimeModel::~ITimeModel() throw() { }

TimeModel::TimeModel(
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

TimeModel::~TimeModel(void) throw()
{
    try {
        // theLog->logMsg("TimeModel dtor");
    }
    catch (...) { }
}

/** set default run options for time-based models. */
void TimeModel::defaultOptions(TimeRunOptions & io_options)
{
    defaultBaseOptions(io_options);
    io_options.simulationEnd = 1;
}

/** get time-based model default run options as list of (key, value) strings. */
NoCaseMap TimeModel::defaultOptions(void)
{
    TimeRunOptions opts;
    defaultOptions(opts);
    NoCaseMap optMap = defaultBaseOptions(std::cref(opts));
    optMap[RunOptionsKey::endTime] = to_string(opts.simulationEnd);
    return optMap;
}

/** set run id and run options for time-based model. */
void TimeModel::setOptions(int i_subCount, int i_subNumber)
{
    defaultOptions(runOptions_);
    setBaseOptions(i_subCount, i_subNumber, runOptions_);
    runOptions_.simulationEnd = metaStore->runOption->longValue(metaStore->runId, RunOptionsKey::endTime, runOptions_.simulationEnd);
}
