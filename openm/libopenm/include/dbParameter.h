/**
 * @file
 * OpenM++ data library: input parameter interfaces
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#ifndef DB_PARAMETER_H
#define DB_PARAMETER_H

#include "libopenm/db/dbCommon.h"
#include "libopenm/common/omFile.h"
#include "dbExec.h"
#include "metaRunHolder.h"
#include "libopenm/db/metaSetHolder.h"

namespace openm
{
    /** input parameter reader public interface */
    struct IParameterReader
    {
        virtual ~IParameterReader() throw() = 0;

        /** input parameter reader factory */
        static IParameterReader * create(
            int i_runId,
            const char * i_name, 
            IDbExec * i_dbExec, 
            const MetaRunHolder * i_metaStore
            );
        
        /** return input parameter size: total number of values in the table */
        virtual size_t sizeOf(void) const throw() = 0;

        /**
         * read input parameter values.
         *
         * @param[in]     i_dbExec    database connection
         * @param[in]     i_type      parameter value type, use std::string for string parameters
         * @param[in]     i_size      parameter size (number of parameter values)
         * @param[in,out] io_valueArr array to return parameter values, size must be =i_size, use io_valueArr[i_size] of std::string for string parameters
         */
        virtual void readParameter(
            IDbExec * i_dbExec, const type_info & i_type, size_t i_size, void * io_valueArr
            ) = 0;
    };

    /** public interface of input parameter writer into workset */
    struct IParameterSetWriter
    {
        virtual ~IParameterSetWriter() throw() = 0;

        /** input parameter writer factory */
        static IParameterSetWriter * create(
            int i_setId,
            const char * i_name,
            IDbExec * i_dbExec,
            const MetaRunHolder * i_metaStore,
            const MetaSetHolder * i_metaSet
            );

        /** return input parameter size: total number of values in the table */
        virtual size_t sizeOf(void) const throw() = 0;

        /**
        * write input parameter values.
        *
        * @param[in]     i_dbExec   database connection
        * @param[in]     i_type     parameter type
        * @param[in]     i_size     parameter size (number of parameter values)
        * @param[in,out] i_valueArr array of parameter values, size must be =i_size
        */
        virtual void writeParameter(
            IDbExec * i_dbExec, const type_info & i_type, size_t i_size, void * i_valueArr
            ) = 0;
    };

    /** public interface of input parameter writer for model run */
    struct IParameterRunWriter
    {
        virtual ~IParameterRunWriter() throw() = 0;

        /** input parameter writer factory */
        static IParameterRunWriter * create(
            int i_runId,
            const char * i_name,
            IDbExec * i_dbExec,
            const MetaRunHolder * i_metaStore,
            const char * i_doubleFormat = ""
            );

        /** return input parameter size: total number of values in the table */
        virtual size_t sizeOf(void) const throw() = 0;

        /** load parameter values from csv file into run table. 
        *
        * @param[in] i_dbExec   database connection
        * @param[in] i_filePath path to parameter.csv file
        */
        virtual void loadCsvParameter(IDbExec * i_dbExec, const char * i_filePath) = 0;

        /**
        * calculate run parameter values digest and store only single copy of parameter values.
        *
        * @param[in] i_dbExec   database connection
        * @param[in] i_type     parameter type, use char * for string parameters
        */
        virtual void digestParameter(IDbExec * i_dbExec, const type_info & i_type) = 0;
    };
}

#endif  // DB_PARAMETER_H
