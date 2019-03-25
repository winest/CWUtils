#pragma once

/*
 * Copyright (c) 2009-2015, ChienWei Hung <winestwinest@gmail.com>
 * CWUtils is published under the BSD-3-Clause license.
 *
 * CWUtils is a set of standalone APIs for developers to speed up their 
 * programming. It should be very easy to port them to other projects or 
 * learn how to implement things on different languages and platforms. 
 *
 * The latest version can be found at https://github.com/winest/CWUtils
 */

#include <Windows.h>
#include <time.h>
#include <string>
#include <vector>

#include "CWGeneralUtils.h"

namespace CWUtils
{
#ifdef __cplusplus
extern "C" {
#endif

#ifndef DAY_PER_SEC
#    define DAY_PER_SEC 86400
#endif
#ifndef HOUR_PER_SEC
#    define HOUR_PER_SEC 3600
#endif
#ifndef MINUTE_PER_SEC
#    define MINUTE_PER_SEC 60
#endif
#ifndef MS_PER_SEC
#    define MS_PER_SEC 1000
#endif
#ifndef US_PER_SEC
#    define US_PER_SEC 1000000
#endif
#ifndef NS_PER_SEC
#    define NS_PER_SEC 1000000000
#endif

VOID FormatTimeA( UINT64 aMilli, std::string & aTimeString );
VOID FormatTimeW( UINT64 aMilli, std::wstring & aTimeString );

BOOL GetCurrTimeStringA( std::string & aTimeString, CONST CHAR * aTimeFormat = "%Y%m%d_%H%M%S" );
BOOL GetCurrTimeStringW( std::wstring & aTimeString, CONST WCHAR * aTimeFormat = L"%Y%m%d_%H%M%S" );
#ifdef _WIN32
#    define GetCurrTimeString GetCurrTimeStringW
#else
#    define GetCurrTimeString GetCurrTimeStringA
#endif

bool operator<( const timespec & aSelf, const timespec & aOther );
void AddTime( struct timespec & curr_time, const struct timespec & diff_time );
void DiffTime( const struct timespec & start_time, const struct timespec & stop_time, struct timespec & diff_time );

class CStopWatch
{
    private:
    typedef enum _StopWatchState
    {
        STOP_WATCH_STATE_START,
        STOP_WATCH_STATE_PAUSE,
        STOP_WATCH_STATE_RESUME,
        STOP_WATCH_STATE_STOP
    } StopWatchState;

    public:
    CStopWatch();
    virtual ~CStopWatch() = default;

    public:
    void Start();
    void Pause();
    void Resume();
    void Stop();

    inline size_t GetStopCount() const { return m_vecAllInt.size(); }

    void GetLastInterval( LARGE_INTEGER & aLastInt ) const;
    UINT64 GetLastIntervalInMilli() const;
    UINT64 GetLastIntervalInMicro() const;
    UINT64 GetLastIntervalInNano() const;

    void GetTotalInterval( LARGE_INTEGER & aTotalInt ) const;
    UINT64 GetTotalIntervalInMilli() const;
    UINT64 GetTotalIntervalInMicro() const;
    UINT64 GetTotalIntervalInNano() const;

    double GetAvgIntervalInNano() const;
    UINT64 GetMinIntervalInNano();
    UINT64 Get1QIntervalInNano();
    double GetMedianIntervalInNano();
    UINT64 Get3QIntervalInNano();
    UINT64 GetMaxIntervalInNano();

    private:
    LARGE_INTEGER m_lnFreq;
    LARGE_INTEGER m_lnStart, m_lnStop;
    LARGE_INTEGER m_lnLastInt, m_lnTotalInt;

    std::vector<UINT64> m_vecAllInt;
    StopWatchState m_nState;
};

#ifdef __cplusplus
}
#endif

}    //End of namespace CWUtils