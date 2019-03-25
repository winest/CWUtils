#pragma once
#include <VersionHelpers.h>

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

namespace CWUtils
{
#ifdef __cplusplus
extern "C" {
#endif



//On multiprocessor systems, this value define number of times that a thread tries to spin before actually performing a wait
//operation (see InitializeCriticalSectionAndSpinCount API)
#if ( _WIN32_WINNT_WINXP <= _WIN32_WINNT )
#    ifndef RW_LOCK_SPIN_COUNT
#        define RW_LOCK_SPIN_COUNT 400
#    endif RW_LOCK_SPIN_COUNT
#endif

class CLock
{
    public:
    virtual BOOL Lock() = 0;
    virtual BOOL Unlock() = 0;
};

class CCriticalSection : public CLock
{
    public:
    inline CCriticalSection()
    {
#if ( _WIN32_WINNT_WINXP <= _WIN32_WINNT )
        InitializeCriticalSectionAndSpinCount( &m_cs, RW_LOCK_SPIN_COUNT );
#else
        InitializeCriticalSection( &m_cs );
#endif
    }
    inline virtual ~CCriticalSection() { DeleteCriticalSection( &m_cs ); }

    inline BOOL Lock()
    {
        EnterCriticalSection( &m_cs );
        return TRUE;
    }
    inline BOOL Unlock()
    {
        LeaveCriticalSection( &m_cs );
        return TRUE;
    }

    private:
    CRITICAL_SECTION m_cs;
};

class CAutoLock
{
    public:
    inline CAutoLock( CLock * cLock ) : m_lock( cLock ) { m_lock->Lock(); }
    inline ~CAutoLock() { m_lock->Unlock(); }

    private:
    CAutoLock();
    CLock * m_lock;
};


class CRWLock
{
    protected:
    CRWLock() { QueryPerformanceFrequency( &m_lnFreq ); }
    virtual ~CRWLock() {}

    public:
    virtual BOOL AcquireReaderLock( DWORD aTimeoutInMilli = INFINITE ) = 0;
    virtual VOID ReleaseReaderLock() = 0;
    virtual BOOL AcquireWriterLock( DWORD aTimeoutInMilli = INFINITE ) = 0;
    virtual VOID ReleaseWriterLock() = 0;

    protected:
    LARGE_INTEGER m_lnFreq;
};



//Forward reference
class CRWLockSlim : public CRWLock
{
    public:
    CRWLockSlim() : m_bWin7AndLater( FALSE )
    {
        InitializeSRWLock( &m_srwLock );
        m_bWin7AndLater = IsWindows7OrGreater();
    }
    virtual ~CRWLockSlim() {}

    virtual BOOL AcquireReaderLock( DWORD dwTimeout = INFINITE );
    virtual VOID ReleaseReaderLock();
    virtual BOOL AcquireWriterLock( DWORD dwTimeout = INFINITE );
    virtual VOID ReleaseWriterLock();

    private:
    BOOL m_bWin7AndLater;
    SRWLOCK m_srwLock;
};


class CAutoReadLock
{
    public:
    inline CAutoReadLock( CRWLock * aRWLock ) : m_lock( aRWLock ) { m_lock->AcquireReaderLock(); }
    inline ~CAutoReadLock() { m_lock->ReleaseReaderLock(); }

    protected:
    CAutoReadLock() {}
    CRWLock * m_lock;
};

class CAutoWriteLock
{
    public:
    inline CAutoWriteLock( CRWLock * aRWLock ) : m_lock( aRWLock ) { m_lock->AcquireWriterLock(); }
    inline ~CAutoWriteLock() { m_lock->ReleaseWriterLock(); }

    protected:
    CAutoWriteLock() {}
    CRWLock * m_lock;
};


#ifdef __cplusplus
}
#endif

}    //End of namespace CWUtils