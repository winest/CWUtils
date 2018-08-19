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

#include <ctime>
#include <cstdarg>
#include <cerrno>
#include <string>

namespace CWUtils
{

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DAY_PER_SEC
    #define DAY_PER_SEC 86400
#endif
#ifndef HOUR_PER_SEC
    #define HOUR_PER_SEC 3600
#endif
#ifndef MINUTE_PER_SEC
    #define MINUTE_PER_SEC 60
#endif
#ifndef MS_PER_SEC
    #define MS_PER_SEC 1000
#endif
#ifndef US_PER_SEC
    #define US_PER_SEC 1000000
#endif
#ifndef NS_PER_SEC
    #define NS_PER_SEC 1000000000
#endif

VOID FormatTime( UINT64 aMilli , std::string & aTimeString );

BOOL GetCurrTimeStringA( std::string & aTimeString , CONST CHAR * aTimeFormat = "%Y%m%d_%H%M%S" );
BOOL GetCurrTimeStringW( std::wstring & aTimeString , CONST WCHAR * aTimeFormat = L"%Y%m%d_%H%M%S" );
#ifdef _WIN32
    #define GetCurrTimeString GetCurrTimeStringW
#else
    #define GetCurrTimeString GetCurrTimeStringA
#endif

bool operator<( const timespec & aSelf , const timespec & aOther );
struct timespec & operator+=( const timespec & aOther );
struct timespec operator+( const timespec & aSelf , const timespec & aOther );
struct timespec & operator-=( const timespec & aOther );
struct timespec operator-( const timespec & aSelf , const timespec & aOther );

class StopWatch
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
    StopWatch();
    virtual ~StopWatch() = default;

public:
    void Start();
    void Pause();
    void Resume();
    void Stop();

    inline size_t GetStopCount() const
    {
        return m_vecAllInt.size();
    }

    void GetLastInterval( struct timespec & aLastInt ) const;
    uint64_t GetLastIntervalInMilli() const;
    uint64_t GetLastIntervalInMicro() const;
    uint64_t GetLastIntervalInNano() const;

    void GetTotalInterval( struct timespec & aTotalInt ) const;
    uint64_t GetTotalIntervalInMilli() const;
    uint64_t GetTotalIntervalInMicro() const;
    uint64_t GetTotalIntervalInNano() const;

    double GetAvgIntervalInNano() const;
    uint64_t GetMinIntervalInNano();
    uint64_t Get1QIntervalInNano();
    double GetMedianIntervalInNano();
    uint64_t Get3QIntervalInNano();
    uint64_t GetMaxIntervalInNano();

private:
    timespec m_timeStart, m_timeStop;
    timespec m_timeLastInt, m_timeTotalInt;

    std::vector<uint64_t> m_vecAllInt;
    StopWatchState m_nState;
};



#ifdef __cplusplus
}
#endif

}   //End of namespace CWUtils
