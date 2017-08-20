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

namespace CWUtils
{

#ifdef __cplusplus
extern "C" {
#endif

VOID FormatTime( UINT64 aMilli , std::wstring & aTimeString );

BOOL GetCurrTimeStringA( std::string & aTimeString , CONST CHAR * aTimeFormat = "%Y%m%d_%H%M%S" );
BOOL GetCurrTimeStringW( std::wstring & aTimeString , CONST WCHAR * aTimeFormat = L"%Y%m%d_%H%M%S" );
#ifdef _WIN32
    #define GetCurrTimeString GetCurrTimeStringW
#else
    #define GetCurrTimeString GetCurrTimeStringA
#endif

class CStopWatch
{
    public :
        CStopWatch() 
        {
            QueryPerformanceFrequency(&m_lnFreq);
            m_lnStart.QuadPart = 0;
            m_lnStop.QuadPart = 0;
        }
        virtual ~CStopWatch() {}

    public :
        VOID Start() { QueryPerformanceCounter(&m_lnStart); }
        VOID Stop() { QueryPerformanceCounter(&m_lnStop); }
        UINT64 GetIntervalInMilli() { return (m_lnStop.QuadPart-m_lnStart.QuadPart) * 1000 / m_lnFreq.QuadPart; }
        UINT64 GetIntervalInMicro() { return (m_lnStop.QuadPart-m_lnStart.QuadPart) * 1000000 / m_lnFreq.QuadPart; }
        UINT64 GetIntervalIn100Nano() { return (m_lnStop.QuadPart-m_lnStart.QuadPart) * 10000000 / m_lnFreq.QuadPart; }

    private :
        LARGE_INTEGER m_lnFreq , m_lnStart , m_lnStop;
};



#ifdef __cplusplus
}
#endif

}   //End of namespace CWUtils