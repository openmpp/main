/**
 * @file
 * OpenM++ data library: input parameter interfaces
 */
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#ifndef DB_PARAMETER_H
#define DB_PARAMETER_H

#include "libopenm/db/dbCommon.h"
#include "dbExec.h"
#include "metaRunHolder.h"

namespace openm
{
    /** input parameter reader public interface */
    struct IParameterReader
    {
        virtual ~IParameterReader() throw() = 0;

        /** input parameter reader factory */
        static IParameterReader * create(
            int i_modelId, 
            int i_runId,
            const char * i_name, 
            IDbExec * i_dbExec, 
            const MetaRunHolder * i_metaStore
            );

        /** return input parameter size: total number of values in the table */
        virtual long long sizeOf(void) const throw() = 0;

        /**
         * read input parameter values.
         *
         * @param[in]     i_dbExec    database connection
         * @param[in]     i_type      parameter type
         * @param[in]     i_size      parameter size (number of parameter values)
         * @param[in,out] io_valueArr array to return parameter values, size must be =i_size
         */
        virtual void readParameter(
            IDbExec * i_dbExec, const type_info & i_type, long long i_size, void * io_valueArr
            ) = 0;
    };
}

#endif  // DB_PARAMETER_H
