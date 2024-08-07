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
#include "metaHolder.h"
#include "libopenm/db/metaSetHolder.h"

namespace openm
{
    /** input parameter reader public interface */
    struct IParameterReader
    {
        virtual ~IParameterReader() noexcept = 0;

        /** input parameter reader factory */
        static IParameterReader * create(
            int i_runId,
            const char * i_name, 
            IDbExec * i_dbExec, 
            const MetaHolder * i_metaStore
            );
        
        /** return input parameter id */
        virtual int parameterId(void) const noexcept = 0;

        /** return input parameter size for single sub value */
        virtual size_t sizeOf(void) const noexcept = 0;

        /**
         * read input parameter single sub value.
         *
         * @param[in]     i_dbExec    database connection
         * @param[in]     i_subId     parameter sub value id
         * @param[in]     i_type      parameter value type, use std::string for string parameters
         * @param[in]     i_size      parameter size (number of parameter values in single sub value)
         * @param[in,out] io_valueArr array to return parameter values, size must be =i_size, use io_valueArr[] of std::string for string parameters
         */
        virtual void readParameter(
            IDbExec * i_dbExec, int i_subId, const type_info & i_type, size_t i_size, void * io_valueArr
            ) = 0;

        /**
         * read all sub values of input parameter.
         *
         * @param[in]     i_dbExec    database connection
         * @param[in]     i_type      parameter value type, use std::string for string parameters
         * @param[in]     i_subCount number of parameter sub-values
         * @param[in]     i_size      parameter size (number of parameter values in single sub value)
         * @param[in,out] io_valueArr array to return parameter values, size must be =i_size * i_subCount, use io_valueArr[] of std::string for string parameters
         */
        virtual void readParameter(
            IDbExec * i_dbExec, const type_info & i_type, int i_subCount, size_t i_size, void * io_valueArr
        ) = 0;

        /**
         * read input parameter single selected sub values.
         *
         * @param[in]     i_dbExec    database connection
         * @param[in]     i_subIdArr  vector of sub value ids to select
         * @param[in]     i_type      parameter value type, use std::string for string parameters
         * @param[in]     i_size      parameter size (number of parameter values in single sub value)
         * @param[in,out] io_valueArr array to return parameter values, size must be =i_size * i_subIdArr.size(), use io_valueArr[] of std::string for string parameters
         */
        virtual void readParameter(
            IDbExec * i_dbExec, const vector<int> & i_subIdArr, const type_info & i_type, size_t i_size, void * io_valueArr
        ) = 0;
    };

    /** public interface of input parameter writer into workset */
    struct IParameterSetWriter
    {
        virtual ~IParameterSetWriter() noexcept = 0;

        /** input parameter writer factory */
        static IParameterSetWriter * create(
            int i_setId,
            const char * i_name,
            IDbExec * i_dbExec,
            const MetaHolder * i_metaStore,
            const MetaSetHolder * i_metaSet
            );

        /** return input parameter size for single sub value */
        virtual size_t sizeOf(void) const noexcept = 0;

        /** write parameter: write all sub values of the parameter into db set table
        *
        * @param[in]     i_dbExec      database connection
        * @param[in]     i_subCount  nmuber of  sub values
        * @param[in]     i_type           parameter value type, use std::string for string parameters
        * @param[in]     i_size           parameter size for single sub value
        * @param[in,out] i_valueArr array of parameter values, size must be == i_size * i_subCount
        */
        virtual void writeParameter(
            IDbExec * i_dbExec, const type_info & i_type, int i_subCount, size_t i_size, void * i_valueArr
        ) = 0;
    };

    /** public interface of input parameter writer for model run */
    struct IParameterRunWriter
    {
        virtual ~IParameterRunWriter() noexcept = 0;

        /** input parameter writer factory */
        static IParameterRunWriter * create(
            int i_runId,
            const char * i_name,
            IDbExec * i_dbExec,
            const MetaHolder * i_metaStore,
            const char * i_doubleFormat = ""
            );

        /** return input parameter size for single sub value */
        virtual size_t sizeOf(void) const noexcept = 0;

        /** write parameter: write all sub values of the parameter into db run table
        *
        * @param[in]     i_dbExec   database connection
        * @param[in]     i_type     parameter value type, use std::string for string parameters
        * @param[in]     i_subCount number of parameter sub-values
        * @param[in]     i_size     parameter size for single sub value
        * @param[in,out] i_valueArr array of parameter values, size must be == i_size * i_subCount
        */
        virtual void writeParameter(
            IDbExec * i_dbExec, const type_info & i_type, int i_subCount, size_t i_size, void * i_valueArr
        ) = 0;

        /** load parameter values from csv file into run table, it can be any of: .csv .tsv .id.csv .id.tsv file.
        *
        * @param[in] i_dbExec   database connection
        * @param[in] i_subIdArr sub-value id's to select from csv
        * @param[in] i_filePath path to parameter.csv file, it can be any of: .csv .tsv .id.csv .id.tsv file.
        * @param[in] i_isIdCsv  if true then create csv file contains enum id's, default: enum code
        * 
        * if file extension .tsv then it is a tab separated file, by default comma separated file expected.
        * if i_isIdCsv is true or file extension is .id.csv or .id.tsv then file contains enum id's, by default enum codes expected
        */
        virtual void loadCsvParameter(IDbExec * i_dbExec, const vector<int> & i_subIdArr, const char * i_filePath, bool i_isIdCsv = false) = 0;

        /**
        * calculate run parameter values digest and store only single copy of parameter values.
        *
        * @param[in] i_dbExec   database connection
        * @param[in] i_subCount number of parameter sub-values
        * @param[in] i_type     parameter type, use char * for string parameters
        */
        virtual void digestParameter(IDbExec * i_dbExec, int i_subCount, const type_info & i_type) = 0;
    };
}

#endif  // DB_PARAMETER_H
