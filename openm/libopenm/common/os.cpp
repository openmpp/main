/**
 * @file
 * OpenM++ platform-specific utility functions.
 */
 // Copyright (c) 2013-2023 OpenM++
 // This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/common/omOS.h"

#ifdef _WIN32

#include <windows.h>
#include <psapi.h>

#endif // _WIN32

#ifdef __linux__

#include <fstream>

#endif // __linux__

#ifdef __APPLE__
#endif // __APPLE__

using namespace std;

#ifdef _WIN32

/**
 * return current process memory size and max peak memory size, in bytes.
 *
 * Windows-specific: return WorkingSetSize and PeakWorkingSetSize using GetProcessMemoryInfo.
 */
tuple<uint64_t, uint64_t> openm::getProcessMemorySize(void)
{
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
    {
        return { pmc.WorkingSetSize, pmc.PeakWorkingSetSize };
    }
    return { 0, 0 };   // return zero on error
}

#endif // _WIN32

#ifdef __linux__

// parse /proc/self/status line and return memory value in bytes, for example: VmRSS: 568 kB
tuple <bool, uint64_t> getProcStatusMemoryValue(const string & i_line, const char * i_key, size_t i_keyLen)
{
    // VmRSS: 1 kB
    if (i_line.length() < i_keyLen + 3 + strnlen("kB", OM_STRLEN_MAX)) return { false, 0 }; // line too short

    size_t nPos = i_line.find(i_key);
    if (nPos == string::npos) return { false, 0 };  // key not found

    size_t nKb = i_line.find("kB", nPos + 1);
    if (nKb == string::npos) nKb = i_line.find("kb", nPos + 1);
    if (nKb == string::npos) nKb = i_line.find("Kb", nPos + 1);
    if (nKb == string::npos) nKb = i_line.find("KB", nPos + 1);
    if (nKb == string::npos) return { true, 0 };                // line must end with kB

    if (nPos + i_keyLen + 1 >= nKb) return { true, 0 };         // value is empty

    // convert value: memory in KBytes
    char * pE = nullptr;
    uint64_t nVal = strtoull((i_line.c_str() + (nPos + i_keyLen)), &pE, 10);

    if (nVal >= INT64_MAX) return { true, 0 };      // invalid memory size

    nVal *= 1024ULL;
    if (nVal >= UINT64_MAX) return { true, 0 };     // invalid memory size

    return { true, nVal };
}

/**
 * return current process memory size and max peak memory size, in bytes.
 *
 * Linux-specific: return byte values of VmRSS and VmHWM from /proc/self/status.
 */
tuple<uint64_t, uint64_t> openm::getProcessMemorySize(void)
{
    // open file
    ifstream inpSt;
    exit_guard<ifstream> onExit(&inpSt, &ifstream::close);  // close on exit

    inpSt.open("/proc/self/status", ios_base::in | ios_base::binary);
    if (inpSt.fail()) return { 0, 0 };

    bool isMem = false;
    bool isMax = false;
    uint64_t nMem = 0;
    uint64_t nMax = 0;
    size_t rssLen = strnlen("VmRSS:", OM_STRLEN_MAX);
    size_t hwmLen = strnlen("VmHWM:", OM_STRLEN_MAX);

    string line;
    while (getline(inpSt, line))
    {
        if (!isMem) tie(isMem, nMem) = getProcStatusMemoryValue(line, "VmRSS:", rssLen);
        if (!isMax) tie(isMax, nMax) = getProcStatusMemoryValue(line, "VmHWM:", hwmLen);
        if (isMem && isMax) break;  // both memory values found
    }
    return { nMem, nMax };
}
#endif // __linux__
