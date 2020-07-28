/**
 * @file
 * OpenM++ common utility functions: split csv line.
 */
// OpenM++ common helper utilities
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "helper.h"

/**   
* split and trim comma-separated list of values (other delimiters can be used too, ie: semicolon).
*
* RFC 4180 difference: 
* skip space-only lines, trim values unless it is " quoted ",
* line can end with CRLF or LF, multi-line strings not supported.
*
* @param[in] i_values       source string of comma separated values.
* @param[in] i_delimiters   list of delimiters, default: comma.
* @param[in] i_isUnquote    if true then do "unquote ""csv"" ", default: false.
* @param[in] i_quote        quote character, default: sql single ' quote.
*/
list<string> openm::splitCsv(const string & i_values, const char * i_delimiters, bool i_isUnquote, char i_quote)
{
    list<string> resultLst;
    splitCsv(i_values, resultLst, i_delimiters, i_isUnquote, i_quote);
    return resultLst;
}

/**
* split and trim comma-separated list of values (other delimiters can be used too, ie: semicolon).
*
* RFC 4180 difference: it does skip space-only lines and trim values unless it is " quoted ".
*
* @param[in] i_values         source string of comma separated values.
* @param[in,out] io_resultLst source string of comma separated values.
* @param[in] i_delimiters     list of delimiters, default: comma.
* @param[in] i_isUnquote      if true then do "unquote ""csv"" ", default: false.
* @param[in] i_quote          quote character, default: sql single ' quote.
* @param[in] i_locale         locale to trim space characters, defaut: user default locale.
*/
void openm::splitCsv(
    const string & i_values, 
    list<string> & io_resultLst, 
    const char * i_delimiters, 
    bool i_isUnquote, 
    char i_quote, 
    const locale & i_locale)
{ 
    // if source is empty or contains only CR LF then return empty result
    string::size_type nSrcLen = i_values.length();
    if (string::size_type n = i_values.find_last_not_of("\r\n"); nSrcLen <= 0 || n == string::npos) {
        io_resultLst.clear();
        return;             // return empty result: source line is empty or contains only CR LF
    }
    else {
        nSrcLen = n + 1;    // ignore CR LF at the end of the source line
    }

    // no delimiters: return entire source string as first element of result list
    size_t nDelimLen = (i_delimiters != nullptr) ? strnlen(i_delimiters, OM_STRLEN_MAX) : 0;
    if (0 == nDelimLen) {
        io_resultLst.clear();
        io_resultLst.push_back(trim(i_values));
        return;
    }

    // position and size in already existing list to set new values
    size_t lstSize = io_resultLst.size();
    size_t lstPos = 0;
    list<string>::iterator lstIt = io_resultLst.begin();

    // find delimiter(s) and append values into output list
    string::size_type nStart = 0;
    bool isInside = false;
    bool isDelim = false;

    for (string::size_type nPos = 0; nPos < nSrcLen; nPos++) {

        char chNow = i_values[nPos];

        // if unquote required and this is quote then toogle 'inside '' quote' status
        if (i_isUnquote && chNow == i_quote) {
            isInside = !isInside;
        }

        // if not 'inside' of quoted value then check for any of delimiters
        if (!isInside) {
            isDelim = any_of(
                i_delimiters,
                i_delimiters + nDelimLen,
                [chNow](const char i_delim) -> bool { return chNow == i_delim; }
            );
        }

        // if this is delimiter or end of string then unquote value and append to the list
        if (isDelim || nPos == nSrcLen - 1) {

            // column size: until end of line or until next delimiter
            size_t nLen = (nPos == nSrcLen - 1 && !isDelim) ? nSrcLen - nStart : nPos - nStart;

            // if previous list had enough columns then use it else append new column
            // trim spaces and unquote value if required
            if (lstPos < lstSize) {
                lstIt->clear();     // clear previous value
                lstIt->append((
                    i_isUnquote ?
                    toUnQuoted(i_values.substr(nStart, nLen), i_quote, i_locale) :
                    trim(i_values.substr(nStart, nLen), i_locale)
                    ));
                lstIt++;    // next column in existing list buffer
            }
            else {  // append new column
                io_resultLst.emplace_back((
                    i_isUnquote ?
                    toUnQuoted(i_values.substr(nStart, nLen), i_quote, i_locale) :
                    trim(i_values.substr(nStart, nLen), i_locale)
                    ));
            }
            lstPos++;   // column count in the current list

            // if this is like: a,b, where delimiter at the end of line then append empty "" value
            if (nPos == nSrcLen - 1 && isDelim) {
                if (lstPos < lstSize) {
                    lstIt->clear();     // clear previous column value
                    lstIt++;
                }
                else {
                    io_resultLst.emplace_back("");  // append new empty column
                }
                lstPos++;
            }

            nStart = nPos + 1;  // skip delimiter and continue
            isDelim = false;
        }
    }

    // if current line have less columns than previous buffer then trim extra columns from result
    if (lstPos < lstSize) {
        io_resultLst.resize(lstPos);
    }
}
