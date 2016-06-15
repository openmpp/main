// OpenM++ data library: class to produce new model sql from metadata
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "modelSqlWriter.h"

using namespace openm;

/** create new sql script file */
ModelSqlWriter::ModelSqlWriter(const string & i_filePath)
{
    // save file path and obtain default locale
    outFilePath = i_filePath;
    defaultLoc = locale("");

    // open file
    exit_guard<ofstream> onExit(&outFs, &ofstream::close);   // close on exit

    outFs.open(outFilePath, ios::out | ios::trunc);
    if (outFs.fail()) throw HelperException("Failed to create file: %s", outFilePath.c_str());

    outFs.imbue(locale::classic());     // for double and float output

    onExit.hold();  // keep file open until dtor
}

/** close sql script file */
ModelSqlWriter::~ModelSqlWriter() throw() 
{ 
    try { outFs.close(); } catch (...) { }
}

/** throw exception on output stream fail */
void ModelSqlWriter::throwOnFail(void) const
{
    if (outFs.fail()) throw HelperException("Failed to write into file: %s", outFilePath.c_str());
}

/** write string into sql script file */
void ModelSqlWriter::write(const char * i_str)
{
    if (i_str == nullptr) return;   // nothing to write

    outFs << i_str;
    if (outFs.fail()) throw HelperException("Failed to write into file: %s", outFilePath.c_str());
}

/** write line into sql script file */
void ModelSqlWriter::writeLine(const string & i_line)
{
    outFs << i_line << '\n';
    if (outFs.fail()) throw HelperException("Failed to write into file: %s", outFilePath.c_str());
}

/** write string value sql-quoted: O'Brien -> 'O''Brien' */
void ModelSqlWriter::writeQuoted(const string & i_str, bool i_isAppendComma)
{
    writeQuoted(i_str.cbegin(), i_str.cend());
    if (i_isAppendComma) {
        outFs << ", ";
        if (outFs.fail()) throw HelperException("Failed to write into file: %s", outFilePath.c_str());
    }
}

/** write substring from i_begin until i_end as sql-quoted */
void ModelSqlWriter::writeQuoted(string::const_iterator i_begin, string::const_iterator i_end)
{
    outFs << '\'';
    if (outFs.fail()) throw HelperException("Failed to write into file: %s", outFilePath.c_str());

    for (string::const_iterator it = i_begin; it != i_end; ++it) {
        outFs << *it;
        if (outFs.fail()) throw HelperException("Failed to write into file: %s", outFilePath.c_str());

        if (*it == '\'') outFs << '\'';
        if (outFs.fail()) throw HelperException("Failed to write into file: %s", outFilePath.c_str());
    }
    outFs << '\'';
    if (outFs.fail()) throw HelperException("Failed to write into file: %s", outFilePath.c_str());
}

/** write string value space trimed and sql-quoted */
void ModelSqlWriter::writeTrimQuoted(const string & i_str, bool i_isAppendComma)
{
    const ctype<char> & chType = use_facet<ctype<char> >(defaultLoc);

    string::const_iterator begIt;
    for (begIt = i_str.cbegin(); begIt != i_str.cend(); ++begIt) {
        if (!chType.is(ctype<char>::space, *begIt)) break;
    }

    string::const_iterator endIt = begIt;

    if (begIt != i_str.cend()) {
        for (endIt = i_str.cend() - 1; endIt != begIt; --endIt) {
            if (!chType.is(ctype<char>::space, *endIt)) {
                ++endIt;
                break;
            }
        }
    }

    writeQuoted(begIt, endIt);
    if (i_isAppendComma) {
        outFs << ", ";
        if (outFs.fail()) throw HelperException("Failed to write into file: %s", outFilePath.c_str());
    }
}
