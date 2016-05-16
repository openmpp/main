/**
 * @file
 * OpenM++ data library: wrapper around file stream to write into sql script file
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#ifndef MODEL_SQL_WRITER_H
#define MODEL_SQL_WRITER_H

#include <fstream>
#include <streambuf>

using namespace std;

namespace openm
{
    /** wrapper around output stream to write into sql file or digest string. */
    template<class Tostream> struct ModelStreamWriter
    {
    public:
        /** sql script file path */
        string outFilePath;

        /** output stream */
        Tostream outFs;

        /** throw exception on output stream fail */
        void throwOnFail(void) const;

        /** write string into output stream */
        void write(const char * i_str);

        /** write line into output stream */
        void writeLine(const string & i_line);

        /** write string value sql-quoted: O'Brien -> 'O''Brien' */
        void writeQuoted(const string & i_str, bool i_isAppendComma = false);

        /** write string value space trimed and sql-quoted */
        void writeTrimQuoted(const string & i_str, bool i_isAppendComma = false);

    protected:
        locale defaultLoc;  // default locale for char classification

        /** write substring from i_begin until i_end as sql-quoted */
        void writeQuoted(string::const_iterator i_begin, string::const_iterator i_end);
    };

    /** wrapper around string stream to write into digest string. */
    struct ModelDigestWriter : ModelStreamWriter<ostringstream>
    {
    public:
        /** create new digest string writer */
        ModelDigestWriter(const string & i_name);
    };

    /** wrapper around file stream to write into sql script file. */
    struct ModelSqlWriter : ModelStreamWriter<ofstream>
    {
    public:
        /** create new sql script file */
        ModelSqlWriter(const string & i_filePath);

        /** close sql script file */
        ~ModelSqlWriter() throw();
    };
}

#endif  // MODEL_SQL_WRITER_H
