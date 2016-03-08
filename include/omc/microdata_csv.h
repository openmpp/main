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
#include <cstdlib>

/**
* Shared common functionality.
*
* Functionality shared by input_csv and output_csv.
*/
class core_csv {
public:

	/**
	* Expand leading $XYZ/the/file.csv using environment variable XYZ
	*
	* @param path Filename to be expanded (in place)
	*/
	static void expand(std::string & path)
	{
		if (path.length() > 0 && '$' == path[0]) {
			auto slash_pos = path.find_first_of("\\/:");
			if (slash_pos == std::string::npos) {
				slash_pos = path.length();
			}
			auto env_name = path.substr(1, slash_pos - 1);
			auto env_value = std::getenv(env_name.c_str());
			if (env_value) {
				path = env_value + path.substr(slash_pos);
			}
		}
	}
};


/**
 * Functionality to read a numeric csv.
 * 
 * A csv file can be opened for forward reading, one record at a time.  Forward seek capability
 * is implemented. A record is split into numeric values which can be accessed by index using the [] operator.
 */
class input_csv : core_csv {

public:
    // ctor
    input_csv()
        : rec_num(0)
    {
    }

    // ctor
    input_csv(const char* fname)
    {
        open(fname);
    }

    // ctor
    input_csv(const std::string& fname)
    {
        open(fname);
    }

#if defined(MODGEN)
    // ctor
    input_csv(const CString& fname)
    {
        open(fname);
    }
#endif

    // dtor
    ~input_csv()
    {
    }

    /**
     * Opens the given numeric csv file.
     *
     * @param fname Filename of the file.
     *
     * @return true if it succeeds, false if it fails.
     */
    void open(const char *fname)
    {
        file_name = fname;
		expand(file_name);
        if (input_stream.is_open()) {
            input_stream.close();
        }
        input_stream.open(file_name.c_str(), std::ios_base::in);
        if (!input_stream.is_open()) {
            std::stringstream ss;
            ss << "Unable to open input csv file '" << fname <<"'";
            ModelExit(ss.str().c_str());
        }
    }

    /**
     * Opens the specified numeric csv file.
     * 
     * @param fname Filename of the file.
     *
     * @return true if it succeeds, false if it fails.
     */
    void open(const std::string& fname)
    {
        open(fname.c_str());
    }

#if defined(MODGEN)
    /**
     * Opens the specified numeric csv file.
     * 
     * This overloaded version allows the use of a file parameter as argument in Modgen, where a
     * 'string' is an MFC CString.
     *
     * @param fname Filename of the file.
     *
     * @return true if it succeeds, false if it fails.
     */
    void open(const CString &fname)
    {
        CStringA fname2(fname);
        const char *fn = fname2;
        open(fn);
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
    * Reads an optional header at the beginning of the csv input file
    *
    * If a header is present, it must be read after opening the file
    * and before reading any data records.  A header does not count
    * as a record for record-numbering purposes.
    */
    std::string read_header()
    {
        if (!input_stream.is_open() || rec_num > 0) {
            std::stringstream ss;
            ss << "bad read_header at record " << rec_num << " in input csv file '" << file_name << "'";
            ModelExit(ss.str().c_str());
            // not reached, but keep the C++ compiler happy by returning something.
            return "";
        }
        std::string line;
        std::getline(input_stream, line);
        return line;
    }

    /**
     * Array indexer operator.
     *
     * @param index Zero-based index for the fields
     *
     * @return The field value
     */
    double operator[](std::size_t index) const
    {
        if (index >= 0 && index < fields.size()) {
            return fields[index];
        }
        else {
            std::stringstream ss;
            ss << "Field " << index << " not present in record " << rec_num << " in input csv file '" << file_name << "'";
            ModelExit(ss.str().c_str());
            // not reached, but keep the C++ compiler happy by returning something.
            return std::numeric_limits<double>::quiet_NaN();
        }
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
     * The record number of the first record (not including a possible header line) is 0.
     * The target record cannot be earlier than the current record.
     * Reading beyond end-of-file is treated as an error.  Fields which are empty
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
     * This is the number of the record which will be read on the next call to read_record.
     * The number of the first record is 0.
     * A header, if present, is not considered to be a record.
     *
     * @return A long.
     */
    long record_number() const
    {
        return rec_num;
    }

private:
    std::string file_name;
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
class output_csv : core_csv {

public:
    // ctor
    output_csv()
        : rec_num(0)
    {
    }

    // ctor
    output_csv(const char* fname)
    {
        open(fname);
    }

    // ctor
    output_csv(const std::string& fname)
    {
        open(fname);
    }

#if defined(MODGEN)
    // ctor
    output_csv(const CString& fname)
    {
        open(fname);
    }
#endif

    // dtor
    ~output_csv()
    {
    }

    /**
     * Opens the given numeric csv file for output.
     *
     * @param fname Filename of the file.
     */
    void open(const char *fname)
    {
        file_name = fname;
		expand(file_name);
		rec_num = 0;
        fields.clear();
        if (output_stream.is_open()) {
            output_stream.close();
        }
        output_stream.open(file_name.c_str(), ios_base::out | ios_base::trunc);
        if (!output_stream.is_open()) {
            std::stringstream ss;
            ss << "Unable to open csv file '" << fname <<"' for output.";
            ModelExit(ss.str().c_str());
        }
    }

    /**
     * Opens the specified numeric csv file for writing
     * 
     * @param fname Filename of the file.
     */
    void open(const std::string& fname)
    {
        open(fname.c_str());
    }

#if defined(MODGEN)
    /**
     * Opens the specified numeric csv file.
     * 
     * This overloaded version allows the use of a file parameter as argument in Modgen, where a
     * 'string' is an MFC CString.
     *
     * @param fname Filename of the file.
     */
    void open(const CString &fname)
    {
        CStringA fname2(fname);
        const char *fn = fname2;
        open(fn);
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

    /**
    * Writes an optional header at the beginning of the csv output file
    *
    * A header must be written after opening the file
    * and before writing any data records.  A header does not count
    * as a record for record-numbering purposes.
    */
    void write_header(std::string hdr)
    {
        if (!output_stream.is_open() || rec_num > 0) {
            std::stringstream ss;
            ss << "bad write_header at record " << rec_num << " in output csv file '" << file_name << "'";
            ModelExit(ss.str().c_str());
            // not reached.
            return;
        }
        output_stream << hdr << std::endl;
    }

    /**
    * Specifies precision of output for all fields
    *
    * The default C++ stream precision is 6 digits.
    * Call before writing any records.
    */
    void precision(int prec)
    {
        if (!output_stream.is_open() || rec_num > 0) {
            std::stringstream ss;
            ss << "bad precision at record " << rec_num << " in output csv file '" << file_name << "'";
            ModelExit(ss.str().c_str());
            // not reached.
            return;
        }
        output_stream.precision(prec);
    }

    /**
     * Push a field into the record.
     * 
     * The record will be written by a sunsequent call to write_record
     *
     * @param val The value.
     */
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
        size_t fields_left = fields.size();

#if ( !defined(_MSC_VER) || _MSC_VER >= 1800 )
        for (auto &val : fields) {
            if (std::isfinite(val)) {
                output_stream << val;
            }
#else
        // _MSC_VER == 1800 means Visual Studio 2013
        // This fall-back code is for Modgen 11 which uses VS 2010.
        // VS 2010 does not support range-based for or std::isfinite().
        for (size_t j = 0; fields_left > 0; ++j ) {
            double &val = fields[j];
            if (_finite(val)) {
                output_stream << val;
            }
#endif

            --fields_left;
            if (fields_left) {
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
     * Number of the record to be written.
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
    std::string file_name;
    std::vector<double> fields;
    std::ofstream output_stream;
    long rec_num;
};
