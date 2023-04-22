/**
 * @file
 * OpenM++ platform-specific utility functions.
 */
 // Copyright (c) 2013-2023 OpenM++
 // This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/common/omOS.h"

#if !defined(_WIN32) && !defined(__linux__) && !defined(__APPLE__)
    std::tuple<uint64_t, uint64_t> openm::getProcessMemorySize(void) { return { 0, 0 }; } // not implemented
#endif

#ifdef _WIN32

#include <windows.h>
#include <psapi.h>

#endif

using namespace std;

#ifdef _WIN32

/** return number of milliseconds since epoch to measure intervals */
int64_t openm::getMilliseconds(void)
{
    return chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now().time_since_epoch()).count();
}

#else // _WIN32

/** return number of milliseconds since epoch to measure intervals */
int64_t openm::getMilliseconds(void)
{
    const int64_t NANO_PER_SECOND = 1000000000;
    const int64_t MILLI_PER_NANO = 1000000;

    struct timespec ts;
    if (!timespec_get(&ts, TIME_UTC)) return 0;

    return ((int64_t)ts.tv_sec * NANO_PER_SECOND + (int64_t)ts.tv_nsec) / MILLI_PER_NANO;
}

#endif // _WIN32

#ifndef _WIN32

// get user prefered locale name: en-CA en-CA or en_CA.UTF-8 or empty "" string on error
const string openm::getDefaultLocaleName(void)
{
    return locale("").name();
}

#else   // _WIN32

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// get user prefered locale name: en-CA en-CA or en_CA.UTF-8 or empty "" string on error
const string openm::getDefaultLocaleName(void)
{
    // try user locale and on error system default locale
    string name;
    wchar_t wlcn[LOCALE_NAME_MAX_LENGTH + 1];

    if (GetUserDefaultLocaleName(wlcn, LOCALE_NAME_MAX_LENGTH) <= 0) {
        if (GetSystemDefaultLocaleName(wlcn, LOCALE_NAME_MAX_LENGTH) <= 0) return name; // empty value on error
    }
    wlcn[LOCALE_NAME_MAX_LENGTH] = '\0';

    // convert from Windows wchar to normal string
    char lcn[LOCALE_NAME_MAX_LENGTH + 1];

    size_t nLcn = std::wcstombs(lcn, wlcn, LOCALE_NAME_MAX_LENGTH);
    if (nLcn >= LOCALE_NAME_MAX_LENGTH || nLcn == static_cast<size_t>(-1)) return name; // empty value on error

    lcn[LOCALE_NAME_MAX_LENGTH] = '\0';

    name = lcn;
    return name;
}
#endif  // _WIN32

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

#include <fstream>

// parse /proc/self/status line and return memory value in bytes, for example: VmRSS: 568 kB
static tuple<bool, uint64_t> getProcStatusMemoryValue(const string_view i_line, const char * i_key, size_t i_keyLen)
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
    uint64_t nVal = strtoull((i_line.data() + (nPos + i_keyLen)), &pE, 10);

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

    try {
        const size_t nM = 4095;
        char line[nM + 1];

        while (inpSt.get(line, nM))
        {
            if (!isMem) tie(isMem, nMem) = getProcStatusMemoryValue(line, "VmRSS:", rssLen);
            if (!isMax) tie(isMax, nMax) = getProcStatusMemoryValue(line, "VmHWM:", hwmLen);
            if (isMem && isMax) break;  // both memory values found

            if (!inpSt.ignore()) break; // skip end of line
        }
    }
    catch (...) {
        return { 0, 0 };    // read error: memory size unknown
    }
    return { nMem, nMax };
}

#endif // __linux__

#ifdef __APPLE__

#include <sys/resource.h>
#include <mach/mach.h>

tuple<uint64_t, uint64_t> openm::getProcessMemorySize(void)
{
    uint64_t nMem = 0;
    uint64_t nMax = 0;

    struct rusage ru;
    if (!getrusage(RUSAGE_SELF, &ru)) {
        // on MacOS this value is in bytes, not in Kbytes
        // and it is the same as task_basic_info.resident_size
        // reported max memory value on MacOS is higher than it is in reality, it is inflated
        nMax = ru.ru_maxrss;
    }

    task_vm_info_data_t tvi;
    mach_msg_type_number_t cnt = TASK_VM_INFO_COUNT;
    if (task_info(mach_task_self(), TASK_VM_INFO, (task_info_t)&tvi, &cnt) == KERN_SUCCESS) {
        nMem = tvi.phys_footprint;
    }

    return { nMem, nMax };
}

#endif // __APPLE__
