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

#include "WinDef.h"
#include <pthread.h>
#include <time.h>

namespace CWUtils
{

#ifdef __cplusplus
    extern "C" {
#endif



class CLock
{
    public :
        virtual BOOL Lock() = 0;
        virtual BOOL Unlock() = 0;
};

class CCriticalSection : public CLock
{
    public :
        inline CCriticalSection()
        {
            pthread_mutexattr_t lockAttr;
            pthread_mutexattr_settype( &lockAttr , PTHREAD_MUTEX_RECURSIVE_NP );
            pthread_mutex_init( &m_mutex , &lockAttr );
            pthread_mutexattr_destroy( &lockAttr );
        }
        inline virtual ~CCriticalSection()
        {
            pthread_mutex_destroy( &m_mutex );
        }

        inline BOOL Lock()
        {
            return ( 0 == pthread_mutex_lock( &m_mutex ) ) ? TRUE : FALSE;
        }
        inline BOOL Unlock()
        {
            return ( 0 == pthread_mutex_unlock( &m_mutex ) ) ? TRUE : FALSE;
        }

    private:
        pthread_mutex_t m_mutex;
};

class CAutoLock
{
    public :
        inline CAutoLock( CLock * cLock ) : m_lock( cLock )
        {
            m_lock->Lock();
        }
        inline ~CAutoLock()
        {
            m_lock->Unlock();
        }
    private:
        CAutoLock();
        CLock * m_lock;
};


class CRWLock
{
    protected :
        CRWLock() {}
        virtual ~CRWLock() {}
    public :
        virtual BOOL AcquireReaderLock( DWORD aTimeoutInMilli = INFINITE ) = 0;
        virtual VOID ReleaseReaderLock() = 0;
        virtual BOOL AcquireWriterLock( DWORD aTimeoutInMilli = INFINITE ) = 0;
        virtual VOID ReleaseWriterLock() = 0;
};



//Forward reference
class CRWLockPthread : public CRWLock
{
    public :
        CRWLockPthread()
        {
            pthread_rwlock_init( &m_rwLock , NULL );
        }
        virtual ~CRWLockPthread()
        {
            pthread_rwlock_destroy( &m_rwLock );
        }

        virtual BOOL AcquireReaderLock( DWORD dwTimeout = INFINITE );
        virtual VOID ReleaseReaderLock();
        virtual BOOL AcquireWriterLock( DWORD dwTimeout = INFINITE );
        virtual VOID ReleaseWriterLock();

    private :
        pthread_rwlock_t m_rwLock;
};


class CAutoReadLock
{
    public :
        inline CAutoReadLock( CRWLock * aRWLock ) : m_lock( aRWLock )
        {
            m_lock->AcquireReaderLock();
        }
        inline ~CAutoReadLock()
        {
            m_lock->ReleaseReaderLock();
        }
    protected :
        CAutoReadLock() {}
        CRWLock * m_lock;
};

class CAutoWriteLock
{
    public :
        inline CAutoWriteLock( CRWLock * aRWLock ) : m_lock( aRWLock )
        {
            m_lock->AcquireWriterLock();
        }
        inline ~CAutoWriteLock()
        {
            m_lock->ReleaseWriterLock();
        }
    protected :
        CAutoWriteLock() {}
        CRWLock * m_lock;
};


#ifdef __cplusplus
}
#endif

}   //End of namespace CWUtils
