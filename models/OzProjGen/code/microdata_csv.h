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
#include <limits>

/**
 * Functionality to read a numeric csv.
 * 
 * A csv file can be opened for forward reading, one record at a time.  Forward seek capability
 * is implemented. A record is split into numeric values into the member named 'fields', which
 * is a vector of double's.
 */
class input_csv {

public:
    // ctor
    input_csv()
        : rec_num(0)
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
            rec_num = 0;
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
        rec_num = 0;
        fields.clear();
    }

    /**
     * Array indexer operator.
     *
     * @param index Zero-based index for the fields
     *
     * @return The field value.
     */
    double operator[](std::size_t index) const
    {
        return fields[index];
    }

    /**
     * Gets the number of fields.
     *
     * @return A std::size_t.
     */
    std::size_t size() const
    {
        return fields.size();
    }

    /**
     * Reads the given record and places values into fields.
     * 
     * The record number of the first record is 0.  The target record cannot be earlier than the
     * current record.  Reading beyond end-of-file is treated as an error.  Fields which are empty
     * in the csv are read as a quiet NaN.
     *
     * @param target_record Number of the record to be read.
     *
     * @return true if it succeeds, false if it fails.
     */
    bool read_record(long long target_record)
    {
        if (!input_stream.is_open() || target_record < rec_num) {
            fields.clear();
            return false;
        }
        std::string line;
        while (target_record > rec_num) {
            // skip records to target record
            std::getline(input_stream, line);
            if (input_stream.eof()) {
                fields.clear();
                return false;
            }
            ++rec_num;
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
            if (fld.size()) {
                fields.push_back(std::strtod(fld.c_str(), nullptr));
            }
            else {
                fields.push_back(std::numeric_limits<double>::quiet_NaN());
            }
        }
        ++rec_num;

        return true;
    }

    /**
     * Number of the current record.
     * 
     * This is the number of the record which will be read on the next call to get_record.
     *
     * @return A long.
     */
    long record_number() const
    {
        return rec_num;
    }

private:
    std::vector<double> fields;
    std::ifstream input_stream;
    long rec_num;
};

/**
 * Functionality to write a numeric csv.
 * 
 * A csv file can be opened for writing one record at a time.  Fields are pushed into the class
 * using the &lt;&lt; operator and written by calling write_record().  The record number is the
 * number of the record which would be written next, or equivalently the number of records
 * written so far. Fields with non-finite values are output as empty values in the csv.
 */
class output_csv {

public:
    // ctor
    output_csv()
        : rec_num(0)
    {
    }

    // ctor
    output_csv(const char* file_name)
    {
        open(file_name);
    }

    // ctor
    output_csv(const std::string& file_name)
    {
        open(file_name);
    }

#if !defined(OPENM)
    // ctor
    output_csv(const CString& file_name)
    {
        open(file_name);
    }
#endif

    // dtor
    ~output_csv()
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
        rec_num = 0;
        fields.clear();
        if (output_stream.is_open()) {
            output_stream.close();
        }
        output_stream.open(file_name, ios_base::out | ios_base::trunc);
        if (output_stream.is_open()) {
            return true;
        }
        else {
            return false;
        }
    }

    /**
     * Opens the specified numeric csv file for writing
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
        output_stream.close();
        rec_num = 0;
        fields.clear();
    }

    void operator <<(double val)
    {
        fields.push_back(val);
    }

    /**
     * Writes the current record using fields.
     * 
     * @return true if it succeeds, false if it fails.
     */
    bool write_record()
    {
        if (!output_stream.is_open()) {
            rec_num = 0;
            fields.clear();
            return false;
        }
        size_t field_count = fields.size();
#if defined(OPENM)
        for (auto &val : fields) {
            if (std::isfinite(val)) {
                output_stream << val;
            }
#else
        // Modgen 11 uses VS2010 which does not suport range-based for
        // or std::isfinite().  Use equivalents until Modgen moves to VS 2013.
        for (size_t j = 0; j < field_count; ++j ) {
            double &val = fields[j];
            if (_finite(val)) {
                output_stream << val;
            }
#endif
            --field_count;
            if (field_count) {
                output_stream << ',';
            }
            else {
                output_stream << std::endl;
            }
        }
        fields.clear();
        ++rec_num;

        return true;
    }

    /**
     * Number of the current record.
     * 
     * This is the number of the record which will be written on the next call to get_record. or
     * equivalently, the number of records written.
     *
     * @return A long.
     */
    long record_number() const
    {
        return rec_num;
    }

private:
    std::vector<double> fields;
    std::ofstream output_stream;
    long rec_num;
};
