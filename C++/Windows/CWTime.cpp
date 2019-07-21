#include "stdafx.h"
#include "CWTime.h"
#include "CWString.h"

#include <cassert>
#include <algorithm>

namespace CWUtils
{
#ifdef __cplusplus
extern "C" {
#endif

VOID FormatTimeA( UINT64 aMilli, std::string & aTimeString )
{
    UINT uMilli = ( UINT )( aMilli % MS_PER_SEC );
    aMilli /= MS_PER_SEC;

    UINT uHour = ( UINT )( aMilli / HOUR_PER_SEC );
    aMilli -= uHour * ( HOUR_PER_SEC );

    UINT uMin = ( UINT )( aMilli / MINUTE_PER_SEC );
    aMilli -= uMin * MINUTE_PER_SEC;

    UINT uSec = ( UINT )( aMilli % 60 );
    CWUtils::FormatStringA( aTimeString, "%02u:%02u:%02u.%03u", uHour, uMin, uSec, uMilli );
}

VOID FormatTimeW( UINT64 aMilli, std::wstring & aTimeString )
{
    UINT uMilli = ( UINT )( aMilli % MS_PER_SEC );
    aMilli /= MS_PER_SEC;

    UINT uHour = ( UINT )( aMilli / HOUR_PER_SEC );
    aMilli -= uHour * ( HOUR_PER_SEC );

    UINT uMin = ( UINT )( aMilli / MINUTE_PER_SEC );
    aMilli -= uMin * MINUTE_PER_SEC;

    UINT uSec = ( UINT )( aMilli % 60 );
    CWUtils::FormatStringW( aTimeString, L"%02u:%02u:%02u.%03u", uHour, uMin, uSec, uMilli );
}

BOOL GetCurrTimeStringA( std::string & aTimeString, CONST CHAR * aTimeFormat )
{
    time_t t;
    struct tm info;
    CHAR szBuf[4096];

    time( &t );
    localtime_s( &info, &t );
    size_t uRet = strftime( szBuf, _countof( szBuf ), aTimeFormat, &info );
    if ( 0 < uRet )
    {
        aTimeString.assign( szBuf, uRet );
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

BOOL GetCurrTimeStringW( std::wstring & aTimeString, CONST WCHAR * aTimeFormat )
{
    time_t t;
    struct tm info;
    WCHAR wzBuf[4096];

    time( &t );
    localtime_s( &info, &t );
    size_t uRet = wcsftime( wzBuf, _countof( wzBuf ), aTimeFormat, &info );
    if ( 0 < uRet )
    {
        aTimeString.assign( wzBuf, uRet );
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

bool operator<( const timespec & aSelf, const timespec & aOther )
{
    if ( LIKELY( aSelf.tv_sec != aOther.tv_sec ) )
    {
        return aSelf.tv_sec < aOther.tv_sec;
    }
    else
    {
        return aSelf.tv_nsec < aOther.tv_nsec;
    }
}

void AddTime( struct timespec & curr_time, const struct timespec & diff_time )
{
    curr_time.tv_sec += diff_time.tv_sec;
    curr_time.tv_nsec += diff_time.tv_nsec;
    if ( curr_time.tv_nsec >= NS_PER_SEC )
    {
        curr_time.tv_sec++;
        curr_time.tv_nsec -= NS_PER_SEC;
    }
}

void DiffTime( const struct timespec & start_time, const struct timespec & stop_time, struct timespec & diff_time )
{
    if ( ( stop_time.tv_nsec - start_time.tv_nsec ) < 0 )
    {
        diff_time.tv_sec = stop_time.tv_sec - start_time.tv_sec - 1;
        diff_time.tv_nsec = stop_time.tv_nsec - start_time.tv_nsec + NS_PER_SEC;
    }
    else
    {
        diff_time.tv_sec = stop_time.tv_sec - start_time.tv_sec;
        diff_time.tv_nsec = stop_time.tv_nsec - start_time.tv_nsec;
    }
}

CStopWatch::CStopWatch() : m_nState( StopWatchState::STOP_WATCH_STATE_STOP )
{
    QueryPerformanceFrequency( &m_lnFreq );
    m_lnStart.QuadPart = 0;
    m_lnStop.QuadPart = 0;

    m_lnLastInt.QuadPart = 0;
    m_lnTotalInt.QuadPart = 0;

    m_vecAllInt.reserve( 100000 );
}

void CStopWatch::Start()
{
    assert( m_nState == StopWatchState::STOP_WATCH_STATE_STOP );
    m_lnLastInt.QuadPart = 0;
    QueryPerformanceCounter( &m_lnStart );
    m_nState = StopWatchState::STOP_WATCH_STATE_START;
}

void CStopWatch::Pause()
{
    if ( m_nState == StopWatchState::STOP_WATCH_STATE_START || m_nState == StopWatchState::STOP_WATCH_STATE_RESUME )
    {
        QueryPerformanceCounter( &m_lnStop );
        m_lnLastInt.QuadPart += m_lnStop.QuadPart - m_lnStart.QuadPart;

        m_nState = StopWatchState::STOP_WATCH_STATE_PAUSE;
    }
}

void CStopWatch::Resume()
{
    if ( m_nState == StopWatchState::STOP_WATCH_STATE_PAUSE )
    {
        QueryPerformanceCounter( &m_lnStart );
        m_nState = StopWatchState::STOP_WATCH_STATE_RESUME;
    }
}

void CStopWatch::Stop()
{
    if ( m_nState != StopWatchState::STOP_WATCH_STATE_STOP )
    {
        QueryPerformanceCounter( &m_lnStop );
        m_lnLastInt.QuadPart += m_lnStop.QuadPart - m_lnStart.QuadPart;

        // Update statistics
        if ( UNLIKELY( m_vecAllInt.size() == m_vecAllInt.capacity() ) )
        {
            m_vecAllInt.reserve( m_vecAllInt.capacity() * 2 );
        }
        m_vecAllInt.push_back( m_lnLastInt.QuadPart * NS_PER_SEC / m_lnFreq.QuadPart );
        m_lnTotalInt.QuadPart += m_vecAllInt.back();

        m_nState = StopWatchState::STOP_WATCH_STATE_STOP;
    }
}

void CStopWatch::GetLastInterval( LARGE_INTEGER & aLastInt ) const
{
    aLastInt = m_lnLastInt;
}
UINT64 CStopWatch::GetLastIntervalInMilli() const
{
    return m_lnLastInt.QuadPart / US_PER_SEC;
}
UINT64 CStopWatch::GetLastIntervalInMicro() const
{
    return m_lnLastInt.QuadPart / MS_PER_SEC;
}
UINT64 CStopWatch::GetLastIntervalInNano() const
{
    return m_lnLastInt.QuadPart;
}

void CStopWatch::GetTotalInterval( LARGE_INTEGER & aTotalInt ) const
{
    aTotalInt = m_lnTotalInt;
}
UINT64 CStopWatch::GetTotalIntervalInMilli() const
{
    return m_lnTotalInt.QuadPart / US_PER_SEC;
}
UINT64 CStopWatch::GetTotalIntervalInMicro() const
{
    return m_lnTotalInt.QuadPart / MS_PER_SEC;
}
UINT64 CStopWatch::GetTotalIntervalInNano() const
{
    return m_lnTotalInt.QuadPart;
}

double CStopWatch::GetAvgIntervalInNano() const
{
    return static_cast<double>( m_lnTotalInt.QuadPart ) / m_vecAllInt.size();
}

UINT64 CStopWatch::GetMinIntervalInNano()
{
    std::nth_element( m_vecAllInt.begin(), m_vecAllInt.begin(), m_vecAllInt.end() );
    return m_vecAllInt.front();
}

UINT64 CStopWatch::Get1QIntervalInNano()
{
    if ( UNLIKELY( m_vecAllInt.size() == 0 ) )
    {
        return 0;
    }

    INT nFirstQuartilePos = m_vecAllInt.size() / 4;
    std::nth_element( m_vecAllInt.begin(), m_vecAllInt.begin() + nFirstQuartilePos, m_vecAllInt.end() );
    return m_vecAllInt[nFirstQuartilePos];
}

double CStopWatch::GetMedianIntervalInNano()
{
    if ( UNLIKELY( m_vecAllInt.size() == 0 ) )
    {
        return 0;
    }

    std::sort( m_vecAllInt.begin(), m_vecAllInt.end() );
    if ( m_vecAllInt.size() % 2 == 0 )
    {
        return static_cast<double>( m_vecAllInt[m_vecAllInt.size() / 2 - 1] + m_vecAllInt[m_vecAllInt.size() / 2] ) / 2;
    }
    else
    {
        return static_cast<double>( m_vecAllInt[m_vecAllInt.size() / 2] );
    }
}

UINT64 CStopWatch::Get3QIntervalInNano()
{
    if ( UNLIKELY( m_vecAllInt.size() == 0 ) )
    {
        return 0;
    }

    INT nThirdQuartilePos = m_vecAllInt.size() * 3 / 4;
    std::nth_element( m_vecAllInt.begin(), m_vecAllInt.begin() + nThirdQuartilePos, m_vecAllInt.end() );
    return m_vecAllInt[nThirdQuartilePos];
}

UINT64 CStopWatch::GetMaxIntervalInNano()
{
    if ( UNLIKELY( m_vecAllInt.size() == 0 ) )
    {
        return 0;
    }

    std::nth_element( m_vecAllInt.begin(), m_vecAllInt.begin() + m_vecAllInt.size() - 1, m_vecAllInt.end() );
    return m_vecAllInt.back();
}

#ifdef __cplusplus
}
#endif

}    //End of namespace CWUtils