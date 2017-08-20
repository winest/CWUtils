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

VOID FormatTime( UINT64 aMilli , std::string & aTimeString );

BOOL GetCurrTimeStringA( std::string & aTimeString , CONST CHAR * aTimeFormat = "%Y%m%d_%H%M%S" );
BOOL GetCurrTimeStringW( std::wstring & aTimeString , CONST WCHAR * aTimeFormat = L"%Y%m%d_%H%M%S" );
#ifdef _WIN32
    #define GetCurrTimeString GetCurrTimeStringW
#else
    #define GetCurrTimeString GetCurrTimeStringA
#endif

VOID DiffTime( IN const struct timespec & aStart , IN const struct timespec & aEnd , OUT struct timespec & aDiff );

class CStopWatch
{
    public :
        CStopWatch() {}
        virtual ~CStopWatch() {}

    public :
        VOID Start() { clock_gettime( CLOCK_PROCESS_CPUTIME_ID , &m_timeStart ); }
        VOID Stop() { clock_gettime( CLOCK_PROCESS_CPUTIME_ID , &m_timeEnd ); }
        VOID GetInterval( struct timespec & aTime )
        {
            DiffTime( m_timeStart , m_timeEnd , aTime );
        }
        UINT64 GetIntervalInMilli()
        {
            timespec timeDiff;
            DiffTime( m_timeStart , m_timeEnd , timeDiff );
            return timeDiff.tv_sec * 1000 + timeDiff.tv_nsec / 1000000;
        }
        UINT64 GetIntervalInMicro()
        {
            timespec timeDiff;
            DiffTime( m_timeStart , m_timeEnd , timeDiff );
            return timeDiff.tv_sec * 1000000 + timeDiff.tv_nsec / 1000;
        }
        UINT64 GetIntervalInNano()
        {
            timespec timeDiff;
            DiffTime( m_timeStart , m_timeEnd , timeDiff );
            return timeDiff.tv_sec * 1000000000 + timeDiff.tv_nsec;
        }

    private :
        timespec m_timeStart , m_timeEnd;
};



#ifdef __cplusplus
}
#endif

}   //End of namespace CWUtils