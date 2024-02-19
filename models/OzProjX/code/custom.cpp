/**
 * @file   custom.cpp
 * Custom C++ file for the model
 * 
 * This file is compiled and linked into the model.
 * Place global definitions (if any) in this file.
 * All global definitions must be thread-safe.
 */

// globals

#include <string>
#include <iostream>
#include <sstream>
#include <filesystem>
namespace fs = std::filesystem;

/**
 * @fn  std::string piece_it(std::string path, int subs, int sub)
 *
 * @brief   Append suffix .subs.sub to stem portion of path if subs > 1
 *
 * @param   path    Full pathname of the file.
 * @param   subs    The subs.
 * @param   sub     The sub.
 *
 * @returns A std::string.
 */
std::string piece_it(std::string path, int subs, int sub)
{
    if (subs <= 1) {
        // no suffix if the run has only one sub.
        return path;
    }

    // build extension suffix
    // example:  .003.032
    std::ostringstream ss;
    ss
        << "."
        << std::setw(3)
        << std::setfill('0')
        << subs
        << "."
        << std::setw(3)
        << std::setfill('0')
        << sub
        ;

    fs::path thePath(path);
    std::string stem = thePath.stem().string();
    std::string ext = thePath.extension().string();
    stem += ss.str();
    thePath.replace_filename(stem + ext);
    return thePath.string();
}