/**
* @file    microdata_csv.h
* Classes for run-time functionality to read and write csv files in Modgen or OpenM++ models.
*
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

/**
 * Functionality associated with reading a numeric csv.
 * 
 * A csv file can be opened for forward reading, one record at a time.  Forward seek cpability
 * is implemented. A requested record can be split into numeric values into the member named
 * 'fields', a vector of double's.
 */
class input_csv {

public:
    // ctor
    input_csv()
        : record_number(0)
    {
    }

    // ctor
    input_csv(const char* file_name)
    {
        open(file_name);
    }

    // ctor
    input_csv(const std::string& file_name)
    {
        open(file_name);
    }

#if !defined(OPENM)
    // ctor
    input_csv(const CString& file_name)
    {
        open(file_name);
    }
#endif

    // dtor
    ~input_csv()
    {
    }

    /**
     * Opens the given numeric csv file.
     *
     * @param file_name Filename of the file.
     *
     * @return true if it succeeds, false if it fails.
     */
    bool open(const char *file_name)
    {
        if (input_stream.is_open()) {
            input_stream.close();
        }
        input_stream.open(file_name, std::ios_base::in);
        if (input_stream.is_open()) {
            record_number = 0;
            fields.clear();
            return true;
        }
        else {
            fields.clear();
            return false;
        }
    }

    /**
     * Opens the specified numeric csv file.
     * 
     * @param file_name Filename of the file.
     *
     * @return true if it succeeds, false if it fails.
     */
    bool open(const std::string& file_name)
    {
        return open(file_name.c_str());
    }

#if !defined(OPENM)

    /**
     * Opens the specified numeric csv file.
     * 
     * This overloaded version allows the use of a file parameter as argument in Modgen, where a
     * 'string' is an MFC CString.
     *
     * @param file_name Filename of the file.
     *
     * @return true if it succeeds, false if it fails.
     */
    bool open(const CString &file_name)
    {
        const char *fn = (LPCSTR)file_name;
        return open(fn);
    }
#endif

    /**
     * Closes the numeric csv file.
     */
    void close()
    {
        input_stream.close();
        record_number = 0;
        fields.clear();
    }

    /**
     * Gets the given record and places values into fields.
     * 
     * The record number of the first record is 0.  The target record cannot be earlier than the
     * current record.  Reading beyond end-of-file is treated as an error.
     *
     * @param target_record Number of the record to be read.
     *
     * @return true if it succeeds, false if it fails.
     */
    bool get_record(long long target_record)
    {
        if (!input_stream.is_open() || target_record < record_number) {
            fields.clear();
            return false;
        }
        std::string line;
        while (target_record > record_number) {
            // skip records to target record
            std::getline(input_stream, line);
            if (input_stream.eof()) {
                fields.clear();
                return false;
            }
            ++record_number;
        }
        std::getline(input_stream, line);
        if (input_stream.eof()) {
            fields.clear();
            return false;
        }
        // parse line into fields
        std::string fld;
        std::stringstream ss(line);
        fields.clear();
        while(std::getline(ss, fld, ',')) {
            fields.push_back(std::strtod(fld.c_str(), nullptr));
        }
        ++record_number;

        return true;
    }

    /**
     * Number of the current record.
     * 
     * This is the number of the record which will be read on the next call to get_record.
     *
     * @return A long.
     */
    long current_record() const
    {
        return record_number;
    }

    std::vector<double> fields;

private:
    std::ifstream input_stream;
    long record_number;
};
