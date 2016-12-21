/**
 * @file
 * OpenM++ UTF-8 converter: text bytes from source encoding (from Windows code page) to UTF-8.
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/common/omHelper.h"
#include "libopenm/common/omFile.h"
#include "log.h"

using namespace std;
using namespace openm;

#ifdef OM_UCVT_MSSTL
    #include <codecvt>
#endif  // OM_UCVT_MSSTL

#ifdef OM_UCVT_ICONV
    #include <iconv.h>
    #include <langinfo.h>
    #include <xlocale.h>
#endif  // OM_UCVT_ICONV

/** UTF-8 byte order mark bytes */
static const unsigned char BOM_UTF8[] = {0xEF, 0xBB, 0xBF};

/** UTF-16LE byte order mark bytes */
static const unsigned char BOM_UTF16LE[] = {0xFF, 0xFE};

/** UTF-16BE byte order mark bytes */
static const unsigned char BOM_UTF16BE[] = {0xFE, 0xFF};

/** UTF-32LE byte order mark bytes */
static const unsigned char BOM_UTF32LE[] = {0xFF, 0xFE, 0x00, 0x00};

/** UTF-32BE byte order mark bytes */
static const unsigned char BOM_UTF32BE[] = {0x00, 0x00, 0xFE, 0xFF};

static const long long bomMaxLen = 4;   // max size of BOM

#define IN_CVT_SIZE  OM_STR_DB_MAX      /* input conversion buffer size, must be even on Windows */
#define OUT_CVT_SIZE (4 * IN_CVT_SIZE)  /* output conversion buffer size, must be at least 4*input buffer size */

static const long long utf8ProbeLen = OUT_CVT_SIZE;     // max bytes used to probe for valid UTF-8

/** openM++ namespace */
namespace openm
{
    /** character conversion from current encoding to UTF-8 */
    enum class CharCvtFrom
    {
        /** current user code page (encoding) */
        defaultPage,

        /** code page (encoding) specified by name */
        explicitPage,

        /** UTF-8 */
        utf8,

        /**  UTF-16LE */
        utf16Le,

        /**  UTF-16LE */
        utf16Be,

        /**  UTF-16LE */
        utf32Le,

        /**  UTF-16LE */
        utf32Be
    };

    /** UTF-8 converter public interface. */
    class IUtf8Converter
    {
    public:
        /**
         * create new converter instance by specified conversion type.
         *
         * @param[in]   i_from          conversion source encoding name, ie: utf16Le.
         * @param[in]   i_codePageName  if conversion type is explicitPage then code page (encoding name) must be specified, ie: English_US.1252
         *
         * @return  new converter instance or throw exception if conversion not supported.
         * @exception   HelperException throw exception if conversion not supported.
         *          
         * Windows:  conversion from UTF-32 not supported for Windows and throws exception. \n
         * Linux:    conversion throws exception if encoding not supported or iconv return an error.
         */
        static IUtf8Converter * create(CharCvtFrom i_from, const char * i_codePageName = nullptr);

        /** release convertor resources. */
        virtual ~IUtf8Converter(void) throw() = 0;

        /** convert next portion of input text bytes into UTF-8 string. */
        virtual string next(long long i_size, const char * i_text) = 0;
    };
};

// release convertor resources.
IUtf8Converter::~IUtf8Converter(void) throw() { }

// copy convertor: return source bytes as string (no conversion required when source already UTF-8)
class Utf8CopyConverter : public IUtf8Converter
{
public:
    Utf8CopyConverter(void) { }
    ~Utf8CopyConverter(void) throw() { }

    // return copy of input bytes as UTF-8 string
    string next(long long i_size, const char * i_text)
    {
        if (i_size <= 0 || i_text == nullptr) return "";    // return on empty input
        return
            string(i_text, i_text + i_size);
    }
};

#ifdef OM_UCVT_ICONV

// Helper class to get default encoding name
struct DefaultEncodingNameHolder
{
    // obtain default encoding name
    DefaultEncodingNameHolder(void) throw() :
        defaultLocale(NULL)
    {
        try { 
            exit_guard<DefaultEncodingNameHolder> onExit(this, &DefaultEncodingNameHolder::cleanup);
       
            defaultLocale = newlocale(LC_CTYPE_MASK, "", NULL);
            if (defaultLocale != NULL && defaultLocale != LC_GLOBAL_LOCALE) encodingName = nl_langinfo_l(CODESET, defaultLocale);
        } catch (...) { } 
    }
    
    // release locale resources
    ~DefaultEncodingNameHolder(void) throw() { try { cleanup(); } catch (...) { } }

    // return default encoding name or empty "" string if not initialized 
    const char * name(void) const throw() { return encodingName.c_str(); }

private:
    locale_t defaultLocale;     // if not NULL then default locale handler 
    string encodingName;        // encoding name, ie: UTF-8

    // release locale resources
    void cleanup(void) throw() 
    { 
        try { 
            if (defaultLocale != NULL && defaultLocale != LC_GLOBAL_LOCALE) freelocale(defaultLocale);
            defaultLocale = NULL;
        } catch (...) { } 
    }
};

// initilize encoding name at program startup
static DefaultEncodingNameHolder defaultEncodingHolder; 

// Holder class for iconv handler
struct IconvOpenHolder
{
    // wrapper for iconv_open(to, from) and iconv_close()
    IconvOpenHolder(const char * i_toEncoding, const char * i_fromEncoding) :
        iconvHandler((iconv_t)-1)
    {
        if (i_toEncoding == nullptr || i_toEncoding[0] == '\0') throw HelperException("Error: invalid (empty) destination encoding name");
        if (i_fromEncoding == nullptr || i_fromEncoding[0] == '\0') throw HelperException("Error: invalid (empty) source encoding name");
            
        iconvHandler = iconv_open(i_toEncoding, i_fromEncoding);
        if (iconvHandler == (iconv_t)-1) throw HelperException("Error: iconv_open failed");
    }
    
    // release iconv resources
    ~IconvOpenHolder(void) throw() { try { cleanup(); } catch (...) { } }

    // return iconv handler or (iconv_t)-1 on errors  
    iconv_t handler(void) const throw() { return iconvHandler; }

    // release iconv resources
    void cleanup(void) throw() 
    { 
        try { 
            if (iconvHandler != (iconv_t)-1) iconv_close(iconvHandler);
            iconvHandler = (iconv_t)-1;
        } catch (...) { } 
    }

private:
    iconv_t iconvHandler;   // if not -1 then handler returned by iconv_open 
};

// convert from specified encoding to UTF-8
class ExpicitPageConverter : public IUtf8Converter
{
public:
    ExpicitPageConverter(const char * i_encodingName) :
    	theIconv("UTF-8", i_encodingName)
    {
    }
    ~ExpicitPageConverter(void) throw() { }

    // convert next portion of input text bytes into UTF-8 string.
    string next(long long i_size, const char * i_text)
    {
        if (i_size <= 0 || i_text == nullptr) return "";   // return on empty input

        // input and output byte buffres
        char outData[OUT_CVT_SIZE + 1];
        char * inBytes = (char *)i_text;
        char * outBytes = (char *)outData;
        size_t outSize = OUT_CVT_SIZE;
        size_t inSize = i_size;

        // convert portion of input bytes
        size_t convRet = iconv(theIconv.handler(), &inBytes, &inSize, &outBytes, &outSize);

        if (convRet == (size_t)-1) {
            if (errno != EINVAL) throw HelperException("Error at conversion into UTF-8");
            // EINVAL: incomplete input byte sequence - this not an error
        }

        // append converted bytes to output
        outData[OUT_CVT_SIZE] = '\0';
        if (outSize > sizeof(char)) *outBytes = '\0';

        string sOut;
        if (outSize < OUT_CVT_SIZE) sOut.append(outData, OUT_CVT_SIZE - outSize);

        return sOut;    // return result string
    }

private:
    IconvOpenHolder theIconv;   // iconv converter
};

// create new converter instance by specified conversion type.
IUtf8Converter * IUtf8Converter::create(CharCvtFrom i_from, const char * i_codePageName)
{
    switch (i_from)
    {
    case CharCvtFrom::explicitPage:
        return new ExpicitPageConverter(i_codePageName);
    case CharCvtFrom::utf8:
        return new Utf8CopyConverter();
    case CharCvtFrom::utf16Le:
        return new ExpicitPageConverter("UTF-16LE");
    case CharCvtFrom::utf16Be:
        return new ExpicitPageConverter("UTF-16BE");
    case CharCvtFrom::utf32Le:
        return new ExpicitPageConverter("UTF-32LE");
    case CharCvtFrom::utf32Be:
        return new ExpicitPageConverter("UTF-32BE");

    default:    // CharCvtFrom::defaultPage
        if (defaultEncodingHolder.name() == nullptr || defaultEncodingHolder.name()[0] == '\0') throw HelperException("Error: unknown default encoding name");

        // if default encoding already UTF-8 then return copy of input text else do character conversion
        if (equalNoCase(defaultEncodingHolder.name(), "UTF-8") || equalNoCase(defaultEncodingHolder.name(), "UTF8"))
            return new Utf8CopyConverter();

        return new ExpicitPageConverter(defaultEncodingHolder.name());
    }
}

#endif  // OM_UCVT_ICONV

#ifdef OM_UCVT_MSSTL
//
// classes below does not intended to be public
// it's created as MS-specic and tested on VC++ only
//

// convert from default Windows code page to UTF-8
class DefaultPageConverter : public IUtf8Converter
{
public:
    DefaultPageConverter(void) { }
    ~DefaultPageConverter(void) throw() { }

    // convert next portion of input text bytes into UTF-8 string.
    string next(long long i_size, const char * i_text)
    {
        if (i_size <= 0 || i_text == nullptr) return "";   // return on empty input

        wstring ws = wcvt.from_bytes(i_text, i_text + i_size);
        return 
            u8cvt.to_bytes(ws.c_str());
    }

private:
    wstring_convert<codecvt<wchar_t, char, mbstate_t>, wchar_t> wcvt;   // to convert source bytes into wide char
    wstring_convert<codecvt_utf8<wchar_t>, wchar_t> u8cvt;              // to convert wide char into UTF-8
};

// convert from specified Windows code page to UTF-8
class ExpicitPageConverter : public IUtf8Converter
{
public:
    ExpicitPageConverter(const char * i_codePageName) 
    { 
        wcvt.reset(
            new wstring_convert<codecvt<wchar_t, char, mbstate_t>, wchar_t>(new codecvt_byname<wchar_t, char, mbstate_t>(i_codePageName))
            );
    }
    ~ExpicitPageConverter(void) throw() { }

    // convert next portion of input text bytes into UTF-8 string.
    string next(long long i_size, const char * i_text)
    {
        if (i_size <= 0 || i_text == nullptr) return "";   // return on empty input

        wstring ws = wcvt->from_bytes(i_text, i_text + i_size);
		return
            u8cvt.to_bytes(ws.c_str());
    }

private:
    unique_ptr<wstring_convert<codecvt<wchar_t, char, mbstate_t>, wchar_t>> wcvt;   // to convert source bytes into wide char
    wstring_convert<codecvt_utf8<wchar_t>, wchar_t> u8cvt;                          // to convert wide char into UTF-8
};

// convert from UTF-16LE to UTF-8
class Utf16LePageConverter : public IUtf8Converter
{
public:
    Utf16LePageConverter(void) { }
    ~Utf16LePageConverter(void) throw() { }

    // convert next portion of input text bytes into UTF-8 string.
    string next(long long i_size, const char * i_text)
    {
        if (i_size <= 0 || i_text == nullptr) return "";   // return on empty input

        wstring ustr = wcvt.from_bytes(i_text, i_text + i_size);
        return 
            u8cvt.to_bytes(ustr.c_str());
    }

private:
    wstring_convert<codecvt_utf16<wchar_t, 0x10ffff, codecvt_mode::little_endian>, wchar_t> wcvt;   // to convert UTF-16LE bytes into wide char
    wstring_convert<codecvt_utf8<wchar_t>, wchar_t> u8cvt;                                          // to convert wide char into UTF-8
};

// convert from UTF-16BE to UTF-8
class Utf16BePageConverter : public IUtf8Converter
{
public:
    Utf16BePageConverter(void) { }
    ~Utf16BePageConverter(void) throw() { }

    // convert next portion of input text bytes into UTF-8 string.
    string next(long long i_size, const char * i_text)
    {
        if (i_size <= 0 || i_text == nullptr) return "";   // return on empty input

        wstring ustr = wcvt.from_bytes(i_text, i_text + i_size);
        return 
            u8cvt.to_bytes(ustr.c_str());
    }

private:
    wstring_convert<codecvt_utf16<wchar_t, 0x10ffff, (codecvt_mode)0>, wchar_t> wcvt;   // to convert UTF-16BE bytes into wide char
    wstring_convert<codecvt_utf8<wchar_t>, wchar_t> u8cvt;                              // to convert wide char into UTF-8
};

// create new converter instance by specified conversion type.
IUtf8Converter * IUtf8Converter::create(CharCvtFrom i_from, const char * i_codePageName)
{
    switch (i_from)
    {
    case CharCvtFrom::explicitPage:
        return new ExpicitPageConverter(i_codePageName);
    case CharCvtFrom::utf8:
        return new Utf8CopyConverter();
    case CharCvtFrom::utf16Le:
        return new Utf16LePageConverter();
    case CharCvtFrom::utf16Be:
        return new Utf16BePageConverter();
    case CharCvtFrom::utf32Le:
    case CharCvtFrom::utf32Be:
        throw HelperException("Error: conversion from UTF-32 not impelemented on Windows");
    default:    // CharCvtFrom::defaultPage
        return new DefaultPageConverter();
    }
}

#endif  // OM_UCVT_MSSTL

/**
* convert null-terminated bytes from current user (or specified) code page to UTF-8 string.
*     
* @param[in] i_byteArr      source bytes
* @param[in] i_codePageName (optional) name of encoding or Windows code page, ie: English_US.1252
*
* @return UTF-8 string.
*/
const string openm::toUtf8(const char * i_byteArr, const char * i_codePageName)
{
    return (i_byteArr != nullptr) ? toUtf8(strnlen(i_byteArr, OM_STRLEN_MAX), i_byteArr, i_codePageName) : "";
}

/**
* convert bytes from current user (or specified) code page to UTF-8 string.
*     
* @param[in] i_size         number of bytes to convert
* @param[in] i_byteArr      source bytes
* @param[in] i_codePageName (optional) name of encoding or Windows code page, ie: English_US.1252
*
* @return UTF-8 string.
*/
const string openm::toUtf8(size_t i_size, const char * i_byteArr, const char * i_codePageName)
{
    if (i_size <= 0 || i_byteArr == nullptr) return "";   // return empty "" string if no input data
    
    unique_ptr<IUtf8Converter> cvt(IUtf8Converter::create(
        (i_codePageName != nullptr && i_codePageName[0] != '\0') ? CharCvtFrom::explicitPage : CharCvtFrom::defaultPage, 
        i_codePageName
        ));
    return cvt->next(i_size, i_byteArr);
}

/**
* return true if bytes are valid UTF-8.
*     
* See table in Description section of article https://en.wikipedia.org/wiki/UTF-8
*
* @param[in] i_size         number of bytes to convert
* @param[in] i_byteArr      source bytes
*
* @return true if valid UTF-8.
*/
bool openm::isUtf8(size_t i_size, const char * i_byteArr)
{
    bool result = true;

    // beginning of possible UTF-8 sequence in buffer
    unsigned char * curr = (unsigned char *) i_byteArr;

    size_t bytes_left = i_size;
    size_t continuation_bytes = 0;

    while (bytes_left > 0) {
        // Binary literals require C++14,
        // but are already supported by ompp targeted compilers.

        unsigned char the_byte = *curr;

        if (continuation_bytes == 0) {
            // Check for valid lead byte of a UTF-8 sequence
            if ((the_byte & 0b10000000) == 0b00000000) {
                // Lead byte is 0xxxxxxx
                // Lead byte of a 1-byte UTF-8 sequence (7 bits)
                // 0 continuation bytes, accept
                curr++;
                bytes_left--;
                continue;
            }
            else if ((the_byte & 0b11100000) == 0b11000000) {
                // Lead byte is 110xxxxx
                // Lead byte of a 2-byte UTF-8 sequence (11 bits)
                // Check for 1 valid continuation bytes
                curr++;
                bytes_left--;
                continuation_bytes = 1;
                continue;
            }
            else if ((the_byte & 0b11110000) == 0b11100000) {
                // Byte #1 is 1110xxxx
                // Lead byte of a 3-byte UTF-8 sequence (16 bits)
                // Check for 2 valid continuation bytes
                curr++;
                bytes_left--;
                continuation_bytes = 2;
                continue;
            }
            else if ((the_byte & 0b11111000) == 0b11110000) {
                // Byte #1 is 11110xxx
                // Lead byte of a 4-byte UTF-8 sequence (21 bits)
                // Check for 3 valid continuation bytes
                curr++;
                bytes_left--;
                continuation_bytes = 3;
                continue;
            }
            else {
                // Invalid lead byte for UTF-8
                result = false;
                break;
            }
        }
        else { // continuation_bytes > 0
            // Check that current byte is a valid continuation byte of a UTF-8 sequence
            if (bytes_left == 0) {
                // UTF-8 sequence truncated at end of buffer, accept
                break;
            }
            if ((the_byte & 0b11000000) == 0b10000000) {
                // Continuation byte is 10xxxxxx
                // Valid continuation byte of a UTF-8 sequence
                curr++;
                bytes_left--;
                continuation_bytes--;
                continue;
            }
            else {
                // Invalid continuation byte for UTF-8
                result = false;
                break;
            }
        }
    }

    return result;
}

/**
* read file and return content as UTF-8 as string.
*     
* @param[in] i_filePath     path to the file
* @param[in] i_codePageName (optional) name of encoding or Windows code page, ie: English_US.1252
*
* @return file content as UTF-8 string.
*/
const string openm::fileToUtf8(const char * i_filePath, const char * i_codePageName)
{
    // read and convert file into list of utf8 strings splitted by \n new line
    list<string> contentLst = fileToUtf8Lines(i_filePath, i_codePageName);

    // calculate total length of output string
    string::size_type totalSize = 0;
    for (const string & s : contentLst) {
        totalSize += s.length();
    }

    // append result to output string
    string fileStr;
    fileStr.reserve(totalSize + 1);

    for (const string & s : contentLst) {
        fileStr += s;
    }
    return fileStr;
}

/**
* read file, split by linefeed \n and return content as list of UTF-8 as strings.
*     
* @param[in] i_filePath     path to the file
* @param[in] i_codePageName (optional) name of encoding or Windows code page, ie: English_US.1252
*
* @return file content as UTF-8 string.
*/
const list<string> openm::fileToUtf8Lines(const char * i_filePath, const char * i_codePageName)
{
    if (i_filePath == nullptr || i_filePath[0] == '\0') throw HelperException("Invalid (empty) file name");

    // open file
    ifstream inpSt;
    exit_guard<ifstream> onExit(&inpSt, &ifstream::close);  // close on exit

    inpSt.open(i_filePath, ios_base::in | ios_base::binary);
    if (inpSt.fail()) throw HelperException("Error at file open: %s", i_filePath);

    // detect encoding by checking BOM
    unsigned char bomBuf[bomMaxLen];

    inpSt.read((char *)bomBuf, bomMaxLen);
    if (inpSt.bad()) throw HelperException("Error at file read");

    long long bomReadCount = inpSt.gcount();
    long long bomSize = 0;

    CharCvtFrom fromChar = CharCvtFrom::defaultPage;

    if (bomReadCount >= (long long)sizeof(BOM_UTF8) &&
        bomBuf[0] == BOM_UTF8[0] && bomBuf[1] == BOM_UTF8[1] && bomBuf[2] == BOM_UTF8[2]) {
        fromChar = CharCvtFrom::utf8;
        bomSize = sizeof(BOM_UTF8);
    }

    if (bomReadCount >= (long long)sizeof(BOM_UTF16LE) &&
        bomBuf[0] == BOM_UTF16LE[0] && bomBuf[1] == BOM_UTF16LE[1]) {
        fromChar = CharCvtFrom::utf16Le;
        bomSize = sizeof(BOM_UTF16LE);
    }

    if (bomReadCount >= (long long)sizeof(BOM_UTF16BE) &&
        bomBuf[0] == BOM_UTF16BE[0] && bomBuf[1] == BOM_UTF16BE[1]) {
        fromChar = CharCvtFrom::utf16Be;
        bomSize = sizeof(BOM_UTF16BE);
    }

    if (bomReadCount >= (long long)sizeof(BOM_UTF32LE) &&
        bomBuf[0] == BOM_UTF32LE[0] && bomBuf[1] == BOM_UTF32LE[1] &&
        bomBuf[2] == BOM_UTF32LE[2] && bomBuf[3] == BOM_UTF32LE[3]) {
        fromChar = CharCvtFrom::utf32Le;
        bomSize = sizeof(BOM_UTF32LE);
    }

    if (bomReadCount >= (long long)sizeof(BOM_UTF32BE) &&
        bomBuf[0] == BOM_UTF32BE[0] && bomBuf[1] == BOM_UTF32BE[1] &&
        bomBuf[2] == BOM_UTF32BE[2] && bomBuf[3] == BOM_UTF32BE[3]) {
        fromChar = CharCvtFrom::utf32Be;
        bomSize = sizeof(BOM_UTF32BE);
    }

    // if no BOM and code page name specified then use the page for conversion
    if (fromChar == CharCvtFrom::defaultPage && i_codePageName != nullptr && i_codePageName[0] != '\0') fromChar = CharCvtFrom::explicitPage;

    // if no BOM and no explicit code page name, probe content to see if it appears to be UTF-8
    if (fromChar == CharCvtFrom::defaultPage) {
        unsigned char utf8ProbeBuf[utf8ProbeLen];

        // No BOM, rewind file to beginning
        inpSt.clear();
        inpSt.seekg(0);

        inpSt.read((char *)utf8ProbeBuf, utf8ProbeLen);
        if (inpSt.bad()) throw HelperException("Error at file read");

        long long probeSize = inpSt.gcount();

        if (isUtf8((size_t)probeSize, (const char *)utf8ProbeBuf)) {
            // looks like UTF-8, treat as if so
            fromChar = CharCvtFrom::utf8;
            bomSize = 0;
        }
    }

    // create converter
    unique_ptr<IUtf8Converter> cvt(IUtf8Converter::create(fromChar, i_codePageName));

    // move file position to the first byte after BOM found
    inpSt.clear();
    inpSt.seekg(bomSize);

    //
    // read until eof, convert bytes to UTF-8 and append to list of lines splitted by \n new line
    //
    list<string> contentLst;
    string lastLine;
    string nextCvt;
    char rdBuf[IN_CVT_SIZE];

    while (inpSt) {

        // read and convert next portion
        inpSt.read(rdBuf, IN_CVT_SIZE);
        long long rdSize = inpSt.gcount();

        if (rdSize > 0) {

            nextCvt = lastLine + cvt->next(rdSize, rdBuf);  // next portion is rest of previous buffer and new utf8 bytes

            // split by \n and append to output list
            size_t nLen = nextCvt.length();
            size_t nStart = 0;
            do {
                size_t nextPos = nextCvt.find('\n', nStart);    // find position of next new line

                // if new line found then append line to output list
                if (nextPos != string::npos) {
                    contentLst.push_back(nextCvt.substr(nStart, (nextPos + 1) - nStart));
                    nStart = nextPos + 1;
                    lastLine.clear();
                }
                else {  // new line not found: store the rest of current buffer as begin of next line
                    lastLine = nextCvt.substr(nStart);
                    break;
                }
            }
            while (nStart < nLen);
        }
        //        if (totalSize >= OM_STRLEN_MAX) throw HelperException("File is too large for in-memory read");
    }
    if (inpSt.bad()) throw HelperException("Error at file read");

    if (!lastLine.empty())  contentLst.push_back(lastLine);

    return contentLst;
}

#undef OUT_CVT_SIZE
#undef IN_CVT_SIZE
