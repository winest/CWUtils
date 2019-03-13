#include "stdafx.h"
#include "CWLock.h"

namespace CWUtils
{
#ifdef __cplusplus
extern "C" {
#endif

BOOL CRWLockPthread::AcquireReaderLock( DWORD aTimeoutInMilli )
{
    BOOL bRet = FALSE;

    struct timespec timeEnd;
    clock_gettime( CLOCK_REALTIME, &timeEnd );

    timeEnd.tv_sec += aTimeoutInMilli / 1000;
    timeEnd.tv_nsec += ( aTimeoutInMilli % 1000 ) * 1000000;
    timeEnd.tv_sec += timeEnd.tv_nsec / 1000000000;
    timeEnd.tv_nsec = timeEnd.tv_nsec % 1000000000;

    bRet = ( 0 == pthread_rwlock_timedrdlock( &m_rwLock, &timeEnd ) ) ? TRUE : FALSE;
    return bRet;
}

VOID CRWLockPthread::ReleaseReaderLock()
{
    pthread_rwlock_unlock( &m_rwLock );
}

BOOL CRWLockPthread::AcquireWriterLock( DWORD aTimeoutInMilli )
{
    BOOL bRet = FALSE;

    struct timespec timeEnd;
    clock_gettime( CLOCK_REALTIME, &timeEnd );

    timeEnd.tv_sec += aTimeoutInMilli / 1000;
    timeEnd.tv_nsec += ( aTimeoutInMilli % 1000 ) * 1000000;
    timeEnd.tv_sec += timeEnd.tv_nsec / 1000000000;
    timeEnd.tv_nsec = timeEnd.tv_nsec % 1000000000;

    bRet = ( 0 == pthread_rwlock_timedwrlock( &m_rwLock, &timeEnd ) ) ? TRUE : FALSE;
    return bRet;
}

VOID CRWLockPthread::ReleaseWriterLock()
{
    pthread_rwlock_unlock( &m_rwLock );
}

#ifdef __cplusplus
}
#endif

}    //End of namespace CWUtils