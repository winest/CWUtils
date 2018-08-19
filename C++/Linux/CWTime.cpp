#include "stdafx.h"
#include "CWTime.h"
using std::string;

namespace CWUtils
{

#ifdef __cplusplus
extern "C" {
#endif

BOOL CDECL _FormatStringA( OUT string & aOutString , IN CONST CHAR * aFormat , ... )
{
    BOOL bRet = FALSE;
    aOutString.clear();
    CHAR szBuf[4096];
    CHAR * pBuf = szBuf;

    if ( NULL != aFormat )
    {
        va_list args;
        va_start( args , aFormat );
        SIZE_T len = vsnprintf( NULL , 0 , aFormat , args ) + 1;    //Get formatted string length and adding one for null-terminator

        if ( _countof(szBuf) < len )
        {
            pBuf = new (std::nothrow) CHAR[len];
        }
        if ( NULL != pBuf )
        {
            if ( 0 < vsnprintf( pBuf , len , aFormat , args ) )
            {
                aOutString = pBuf;
            }

            if ( pBuf != szBuf )
            {
                delete [] pBuf;
            }
            bRet = TRUE;
        }

        va_end( args );
    }
    else
    {
        errno = EINVAL;
    }

    return bRet; 
}

VOID FormatTime( UINT64 aMilli , string & aTimeString )
{
    UINT uMilli = (UINT)( aMilli % MS_PER_SEC );
    aMilli /= MS_PER_SEC;

    UINT uHour = (UINT)( aMilli / HOUR_PER_SEC );
    aMilli -= uHour * ( HOUR_PER_SEC );

    UINT uMin = (UINT)( aMilli / MINUTE_PER_SEC );
    aMilli -= uMin * MINUTE_PER_SEC;

    UINT uSec = (UINT)( aMilli % 60 );
    _FormatStringA( aTimeString , "%02u:%02u:%02u.%03u" , uHour , uMin , uSec , uMilli );
}

BOOL GetCurrTimeStringA( std::string & aTimeString , CONST CHAR * aTimeFormat )
{
    time_t t;
    struct tm * pTimeInfo;
    CHAR szBuf[4096];

    time( &t );
    pTimeInfo = localtime( &t );
    size_t uRet = strftime( szBuf , _countof(szBuf) , aTimeFormat , pTimeInfo );
    if ( 0 < uRet )
    {
        aTimeString.assign( szBuf , uRet );
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

BOOL GetCurrTimeStringW( std::wstring & aTimeString , CONST WCHAR * aTimeFormat )
{
    time_t t;
    struct tm * pTimeInfo;
    WCHAR wzBuf[4096];

    time( &t );
    pTimeInfo = localtime( &t );
    size_t uRet = wcsftime( wzBuf , _countof(wzBuf) , aTimeFormat , pTimeInfo );
    if ( 0 < uRet )
    {
        aTimeString.assign( wzBuf , uRet );
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

bool operator<( const timespec & aSelf , const timespec & aOther )
{
    if (LIKELY(aSelf.tv_sec != aOther.tv_sec))
    {
        return aSelf.tv_sec < aOther.tv_sec;
    }
    else
    {
        return aSelf.tv_nsec < aOther.tv_nsec;
    }
}

struct timespec & operator+=( const timespec & aOther )
{
    this->tv_sec += aDiff.tv_sec;
    this->tv_nsec += aDiff.tv_nsec;
    if ( this->tv_nsec >= NS_PER_SEC )
    {
        this->tv_sec++;
        this->tv_nsec -= NS_PER_SEC;
    }
    return *this;
}

struct timespec operator+( const timespec & aSelf , const timespec & aOther )
{
    return struct timespec(aSelf) += aOther;
}


struct timespec & operator-=( const timespec & aOther )
{
    if ( (this->tv_nsec - aOther.tv_nsec) < 0 )
    {
        this->tv_sec  = this->tv_sec - aOther.tv_sec - 1;
        this->tv_nsec = this->tv_nsec - aOther.tv_nsec + NS_PER_SEC;
    }
    else
    {
        this->tv_sec  -= aOther.tv_sec;
        this->tv_nsec -= aOther.tv_nsec;
    }
    return *this;
}

struct timespec operator-( const timespec & aSelf , const timespec & aOther )
{
    return struct timespec(aSelf) -= aOther;
}






StopWatch::StopWatch()
    : m_nState( StopWatchState::STOP_WATCH_STATE_STOP )
{
    memset( &m_timeStart , 0 , sizeof(m_timeStart) );
    memset( &m_timeStop , 0 , sizeof(m_timeStop) );

    memset( &m_timeLastInt, 0, sizeof(m_timeLastInt) );
    memset( &m_timeTotalInt, 0, sizeof(m_timeTotalInt) );

    m_vecAllInt.reserve( 100000 );
}

void StopWatch::Start()
{
    assert( m_nState == StopWatchState::STOP_WATCH_STATE_STOP );
    memset( &m_timeLastInt , 0 , sizeof(m_timeLastInt) );
    clock_gettime( CLOCK_PROCESS_CPUTIME_ID , &m_timeStart );
    m_nState = StopWatchState::STOP_WATCH_STATE_START;
}

void StopWatch::Pause()
{
    if ( m_nState == StopWatchState::STOP_WATCH_STATE_START || m_nState == StopWatchState::STOP_WATCH_STATE_RESUME )
    {
        clock_gettime( CLOCK_PROCESS_CPUTIME_ID , &m_timeStop );
        struct timespec timeDiff = m_timeStop - m_timeStart;
        m_timeLastInt += timeDiff;

        m_nState = StopWatchState::STOP_WATCH_STATE_PAUSE;
    }
}

void StopWatch::Resume()
{
    if ( m_nState == StopWatchState::STOP_WATCH_STATE_PAUSE )
    {
        clock_gettime( CLOCK_PROCESS_CPUTIME_ID , &m_timeStart );
        m_nState = StopWatchState::STOP_WATCH_STATE_RESUME;
    }
}

void StopWatch::Stop()
{
    if ( m_nState != StopWatchState::STOP_WATCH_STATE_STOP )
    {
        clock_gettime( CLOCK_PROCESS_CPUTIME_ID , &m_timeStop );
        struct timespec timeDiff = m_timeStop - m_timeStart;
        m_timeLastInt += timeDiff;

        // Update statistics
        if ( UNLIKELY(m_vecAllInt.size() == m_vecAllInt.capacity()) )
        {
            m_vecAllInt.reserve( m_vecAllInt.capacity() * 2 );
        }
        m_vecAllInt.push_back( m_timeLastInt.tv_sec * NS_PER_SEC + m_timeLastInt.tv_nsec );
        m_timeTotalInt += m_timeLastInt;

        m_nState = StopWatchState::STOP_WATCH_STATE_STOP;
    }
}

void StopWatch::GetLastInterval( struct timespec & aLastInt ) const
{
    aLastInt = m_timeLastInt;
}
uint64_t StopWatch::GetLastIntervalInMilli() const
{
    return m_timeLastInt.tv_sec * MS_PER_SEC + m_timeLastInt.tv_nsec / 1000000;
}
uint64_t StopWatch::GetLastIntervalInMicro() const
{
    return m_timeLastInt.tv_sec * US_PER_SEC + m_timeLastInt.tv_nsec / 1000;
}
uint64_t StopWatch::GetLastIntervalInNano() const
{
    return m_timeLastInt.tv_sec * NS_PER_SEC + m_timeLastInt.tv_nsec;
}

void StopWatch::GetTotalInterval( struct timespec & aTotalInt ) const
{
    aTotalInt = m_timeTotalInt;
}
uint64_t StopWatch::GetTotalIntervalInMilli() const
{
    return m_timeTotalInt.tv_sec * MS_PER_SEC + m_timeTotalInt.tv_nsec / 1000000;
}
uint64_t StopWatch::GetTotalIntervalInMicro() const
{
    return m_timeTotalInt.tv_sec * US_PER_SEC + m_timeTotalInt.tv_nsec / 1000;
}
uint64_t StopWatch::GetTotalIntervalInNano() const
{
    return m_timeTotalInt.tv_sec * NS_PER_SEC + m_timeTotalInt.tv_nsec;
}

double StopWatch::GetAvgIntervalInNano() const
{
    return (double)(m_timeTotalInt.tv_sec * NS_PER_SEC + m_timeTotalInt.tv_nsec) / m_vecAllInt.size();
}

uint64_t StopWatch::GetMinIntervalInNano()
{
    std::nth_element( m_vecAllInt.begin(), m_vecAllInt.begin() , m_vecAllInt.end() );
    return m_vecAllInt.front();
}

uint64_t StopWatch::Get1QIntervalInNano()
{
    if ( UNLIKELY(m_vecAllInt.size() == 0) )
    {
        return 0;
    }

    INT nFirstQuartilePos = m_vecAllInt.size() / 4;
    std::nth_element(m_vecAllInt.begin(),
                     m_vecAllInt.begin() + nFirstQuartilePos,
                     m_vecAllInt.end());
    return m_vecAllInt[nFirstQuartilePos];
}

double StopWatch::GetMedianIntervalInNano()
{
    if ( UNLIKELY(m_vecAllInt.size() == 0) )
    {
        return 0;
    }

    std::sort( m_vecAllInt.begin() , m_vecAllInt.end() );
    if ( m_vecAllInt.size() % 2 == 0 )
    {
        return static_cast<double>( m_vecAllInt[m_vecAllInt.size() / 2 - 1] + m_vecAllInt[m_vecAllInt.size() / 2] ) /  2;
    }
    else
    {
        return static_cast<double>( m_vecAllInt[m_vecAllInt.size() / 2] );
    }
}

uint64_t StopWatch::Get3QIntervalInNano()
{
    if ( UNLIKELY(m_vecAllInt.size() == 0) )
    {
        return 0;
    }

    INT nThirdQuartilePos = m_vecAllInt.size() * 3 / 4;
    std::nth_element(m_vecAllInt.begin(),
                     m_vecAllInt.begin() + nThirdQuartilePos,
                     m_vecAllInt.end());
    return m_vecAllInt[nThirdQuartilePos];
}

uint64_t StopWatch::GetMaxIntervalInNano()
{
    if ( UNLIKELY(m_vecAllInt.size() == 0) )
    {
        return 0;
    }

    std::nth_element(m_vecAllInt.begin(),
                     m_vecAllInt.begin() + m_vecAllInt.size() - 1,
                     m_vecAllInt.end());
    return m_vecAllInt.back();
}



#ifdef __cplusplus
}
#endif

}   //End of namespace CWUtils