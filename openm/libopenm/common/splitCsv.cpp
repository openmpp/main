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
    // trim source and return empty result if source line empty or space only
    list<string> resultLst;
    string srcValues = trim(i_values);
    string::size_type nSrcLen = srcValues.length();

    if (nSrcLen <= 0) return resultLst;     // source string is empty: return empty list

    // no delimiters: return entire source string as first element of result list
    size_t nDelimLen = (i_delimiters != nullptr) ? strnlen(i_delimiters, OM_STRLEN_MAX) : 0;
    if (0 == nDelimLen) {
        resultLst.push_back(srcValues);
        return resultLst;
    }

    // find delimiter(s) and append values into output list
    string::size_type nStart = 0;
    bool isInside = false;
    bool isDelim = false;

    for (string::size_type nPos = 0; nPos < nSrcLen; nPos++) {

        char chNow = srcValues[nPos];

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

            // trim spaces and unquote value if required
            size_t nLen = (nPos == nSrcLen - 1 && !isDelim) ? nSrcLen - nStart : nPos - nStart;

            string sVal = i_isUnquote ?
                toUnQuoted(srcValues.substr(nStart, nLen), i_quote) :
                trim(srcValues.substr(nStart, nLen));

            resultLst.push_back(sVal);

            // if this is like: a,b, where delimiter at the end of line then append empty "" value
            if (nPos == nSrcLen - 1 && isDelim) resultLst.push_back("");

            nStart = nPos + 1;  // skip delimiter and continue
            isDelim = false;
        }
    }

    return resultLst;
}
