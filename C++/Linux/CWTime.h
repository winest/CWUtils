#pragma once

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