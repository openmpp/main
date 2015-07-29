/**
 * @file
 * OpenM++ data library: output table interfaces
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#ifndef DB_OUTPUT_TABLE_H
#define DB_OUTPUT_TABLE_H

#include <cmath>
#include "libopenm/db/dbCommon.h"
#include "dbExec.h"
#include "metaRunHolder.h"

namespace openm
{
    /** output table writer public interface */
    struct IOutputTableWriter
    {
        virtual ~IOutputTableWriter() throw() = 0;

        /** output table writer factory */
        static IOutputTableWriter * create(
            int i_modelId, 
            int i_runId,
            const char * i_name, 
            IDbExec * i_dbExec, 
            const MetaRunHolder * i_metaStore, 
            int i_numSubSamples,
            bool i_isSparse = false, 
            double i_nullValue = DBL_EPSILON
            );

        /** return total number of values for each accumulator */
        virtual long long sizeOf(void) const throw() = 0;

        /** return total number of values for each accumulator */
        static long long sizeOf(int i_modelId, const MetaRunHolder * i_metaStore, int i_tableId);

        /** return number of output aggregated expressions */
        virtual int expressionCount(void) const throw() = 0;

        /**
        * write output table accumulator values
        *
        * @param[in] i_dbExec      database connection
        * @param[in] i_nSubSample  subsample number
        * @param[in] i_size        number of values for each accumulator
        * @param[in] i_accId       accumulator number for the output table (zero based)
        * @param[in] i_valueArr    array of accumulator values
        */
        virtual void writeAccumulator(
            IDbExec * i_dbExec, int i_nSubSample, int i_accId, long long i_size, const double * i_valueArr
            ) = 0;

        /** write all output table values: aggregate subsamples using table expressions */
        virtual void writeAllExpressions(IDbExec * i_dbExec) = 0;

        /**
         * write output table value: aggregated output expression value
         *
         * @param[in] i_dbExec      database connection
         * @param[in] i_nExpression aggregation expression number
         */
        virtual void writeExpression(IDbExec * i_dbExec, int i_nExpression) = 0;
    };
}

#endif  // DB_OUTPUT_TABLE_H
