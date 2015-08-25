#include "CWLock.h"

namespace KmUtils
{

BOOLEAN CMyEResource::AcquireReaderLock( ULONG aTimeout )
{
    BOOLEAN bRet = FALSE;

    KeEnterCriticalRegion();
    if ( INFINITE == aTimeout )
    {
        //ExAcquireSharedWaitForExclusive: if already own read access and someone need writer lock, keep wait and let writer get the lock first
        //ExAcquireResourceSharedLite: if already own read access, keep own; if not owned, let writer get the lock first
        //ExAcquireSharedStarveExclusive: read is the most important; writer can get lock only when no reader
        bRet = ExAcquireResourceSharedLite( &m_res , TRUE );
    }
    else
    {
        LARGE_INTEGER liCurrTime , liEndTime;
        KeQuerySystemTime( &liCurrTime );
        liEndTime.QuadPart = ( liCurrTime.QuadPart ) + ( aTimeout * 10000 );

        do
        {
            for ( ULONG i = 0 ; i < MY_RW_LOCK_SPIN_COUNT ; i++ )
            {
                if ( TRUE == ExAcquireResourceSharedLite( &m_res , FALSE ) )
                {
                    bRet = TRUE;
                    break;
                }
            }

            KeQuerySystemTime( &liCurrTime );                    
        } while ( FALSE == bRet && liCurrTime.QuadPart < liEndTime.QuadPart );
    }

    if ( FALSE == bRet )
    {
        KeLeaveCriticalRegion();
    }

    return bRet;
}

VOID CMyEResource::ReleaseReaderLock()
{
    ExReleaseResourceLite( &m_res );
    KeLeaveCriticalRegion();
}

BOOLEAN CMyEResource::AcquireWriterLock( ULONG aTimeout )
{
    BOOLEAN bRet = FALSE;

    KeEnterCriticalRegion();
    if ( INFINITE == aTimeout )
    {
        bRet = ExAcquireResourceExclusiveLite( &m_res , TRUE );
    }
    else
    {
        LARGE_INTEGER liCurrTime , liEndTime;
        KeQuerySystemTime( &liCurrTime );
        liEndTime.QuadPart = ( liCurrTime.QuadPart ) + ( aTimeout * 10000 );

        do
        {
            for ( ULONG i = 0 ; i < MY_RW_LOCK_SPIN_COUNT ; i++ )
            {
                if ( TRUE == ExAcquireResourceExclusiveLite( &m_res , FALSE ) )
                {
                    bRet = TRUE;
                    break;
                }
            }

            KeQuerySystemTime( &liCurrTime );                    
        } while ( FALSE == bRet && liCurrTime.QuadPart < liEndTime.QuadPart );
    }

    if ( FALSE == bRet )
    {
        KeLeaveCriticalRegion();
    }

    return bRet;
}

VOID CMyEResource::ReleaseWriterLock()
{
    ExReleaseResourceLite( &m_res );
    KeLeaveCriticalRegion();
}

VOID CMyEResource::WriterLockToReaderLock()
{
    NT_ASSERT( TRUE == ExIsResourceAcquiredExclusiveLite( &m_res ) );
    ExConvertExclusiveToSharedLite( &m_res );
}



}   //End of namespace KmUtils