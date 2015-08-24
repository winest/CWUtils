#pragma once

#include <Windows.h>
#include <string>
using std::wstring;

namespace CWUtils
{

#ifdef __cplusplus
extern "C" {
#endif

VOID FormatTime( UINT64 aMilli , wstring & aTimeString );

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