/**
 * @file
 * OpenM++ data library: output table interfaces
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#ifndef DB_OUTPUT_TABLE_H
#define DB_OUTPUT_TABLE_H

#include <cmath>
#include <cfloat>
#include "libopenm/db/dbCommon.h"
#include "dbExec.h"
#include "metaHolder.h"

namespace openm
{
    /** output table writer public interface */
    struct IOutputTableWriter
    {
        virtual ~IOutputTableWriter() noexcept = 0;

        /** output table writer factory to write accumulators */
        static IOutputTableWriter * create(
            int i_runId,
            const char * i_name,
            IDbExec * i_dbExec,
            const MetaHolder * i_metaStore,
            int i_subCount,
            bool i_isSparse = false,
            double i_nullValue = FLT_MIN
        );

        /** output table writer factory to write expressions*/
        static IOutputTableWriter * create(
            int i_runId,
            const char * i_name,
            IDbExec * i_dbExec,
            const MetaHolder * i_metaStore,
            int i_subCount,
            const char * i_doubleFormat = ""
        );

        /** return total number of values for each accumulator */
        virtual size_t sizeOf(void) const noexcept = 0;

        /** return total number of values for each accumulator */
        static size_t sizeOf(const MetaHolder * i_metaStore, int i_tableId);

        /** return number of output aggregated expressions */
        virtual int expressionCount(void) const noexcept = 0;

        /**
        * write output table accumulator values
        *
        * @param[in] i_dbExec      database connection
        * @param[in] i_subId       sub-value index
        * @param[in] i_size        number of values for each accumulator
        * @param[in] i_accId       accumulator number for the output table (zero based)
        * @param[in] i_valueArr    array of accumulator values
        */
        virtual void writeAccumulator(
            IDbExec * i_dbExec, int i_subId, int i_accId, size_t i_size, const double * i_valueArr
            ) = 0;

        /** write all output table values: aggregate sub-values using table expressions */
        virtual void writeAllExpressions(IDbExec * i_dbExec) = 0;

        /** calculate output table values digest and store only single copy of output values */
        virtual void digestOutput(IDbExec * i_dbExec) = 0;
    };

    /** output table reader public interface */
    struct IOutputTableReader
    {
        virtual ~IOutputTableReader() noexcept = 0;

        /** return number of values to select from the table */
        virtual size_t sizeOf(void) const noexcept = 0;

        /** return output table size: total number of values in the table */
        virtual size_t totalSizeOf(void) const noexcept = 0;

        /**
         * read output table values.
         *
         * @param[in]     i_dbExec    database connection
         * @param[in]     i_isNanFill if true then initially fill io_valueArr with quiet_NaN else zero fill
         * @param[in]     i_size      number of values to return
         * @param[in,out] io_valueArr array to return output values, size must be =i_size
         */
        virtual void readTable(IDbExec * i_dbExec, bool i_isNanFill, size_t i_size, double * io_valueArr) = 0;
    };

    /** output table expressions reader public interface */
    struct IOutputTableExprReader : public IOutputTableReader
    {
        virtual ~IOutputTableExprReader() noexcept = 0;

        /** output table reader factory: create new reader table expressions reader */
        static IOutputTableExprReader * create(
            int i_runId,
            const char * i_name,
            IDbExec * i_dbExec,
            const MetaHolder * i_metaStore,
            const char * i_expr = ""
        );
    };

    /** output table accumulators reader public interface */
    struct IOutputTableAccReader : public IOutputTableReader
    {
        virtual ~IOutputTableAccReader() noexcept = 0;

        /** output table reader factory: create new reader table accumulators reader */
        static IOutputTableAccReader * create(
            int i_runId,
            const char * i_name,
            IDbExec * i_dbExec,
            const MetaHolder * i_metaStore,
            const char * i_acc = "",
            const vector<int> & i_subIdArr = {}
        );
    };
}

#endif  // DB_OUTPUT_TABLE_H
