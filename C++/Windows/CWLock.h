#pragma once
#include <Windows.h>

namespace CWUtils
{

#ifdef __cplusplus
    extern "C" {
#endif



//On multiprocessor systems, this value define number of times that a thread tries to spin before actually performing a wait
//operation (see InitializeCriticalSectionAndSpinCount API)
#if ( _WIN32_WINNT_WINXP <= _WIN32_WINNT )
    #ifndef RW_LOCK_SPIN_COUNT
        #define RW_LOCK_SPIN_COUNT 400
    #endif RW_LOCK_SPIN_COUNT
#endif

class CLock
{
    public :
        virtual BOOL Lock() = NULL;
        virtual BOOL Unlock() = NULL;
};

class CCriticalSection : public CLock
{
    public :
        inline CCriticalSection() 
        {
            #if ( _WIN32_WINNT_WINXP <= _WIN32_WINNT )
                InitializeCriticalSectionAndSpinCount( &m_cs , RW_LOCK_SPIN_COUNT );
            #else
                InitializeCriticalSection( &m_cs );
            #endif
        }
        inline virtual ~CCriticalSection()
        {
            DeleteCriticalSection( &m_cs );
        }

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
    public :
        inline CAutoLock( CLock * cLock ) : m_Lock( cLock )
        {
            m_Lock->Lock();
        }
        inline ~CAutoLock()
        {
            m_Lock->Unlock();
        }
    private:
        CAutoLock();
        CLock * m_Lock;
};


class CRWLock
{
    protected :
        CRWLock() { QueryPerformanceFrequency(&m_lnFreq); }
        virtual ~CRWLock() {}
    public :
        virtual BOOL AcquireReaderLock( DWORD dwTimeout = INFINITE ) = NULL;
        virtual VOID ReleaseReaderLock() = NULL;
        virtual BOOL AcquireWriterLock( DWORD dwTimeout = INFINITE ) = NULL;
        virtual VOID ReleaseWriterLock() = NULL;

    protected :
        LARGE_INTEGER m_lnFreq;
};



//Forward reference
class CRWLockSlim : public CRWLock
{
    public :
        CRWLockSlim() : m_bWin7AndLater(FALSE)
        {
            InitializeSRWLock( &m_srwLock );

            OSVERSIONINFOEXW osvi = { 0 };
            osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);
            if ( GetVersionExW( (OSVERSIONINFOW *)&osvi ) )
            {
                m_bWin7AndLater = ( 6 <= osvi.dwMajorVersion && 1 <= osvi.dwMinorVersion ) ? TRUE : FALSE;
            }
        }
        virtual ~CRWLockSlim()
        {
        }

        virtual BOOL AcquireReaderLock( DWORD dwTimeout = INFINITE );
        virtual VOID ReleaseReaderLock();
        virtual BOOL AcquireWriterLock( DWORD dwTimeout = INFINITE );
        virtual VOID ReleaseWriterLock();

    private :
        BOOL m_bWin7AndLater;
        SRWLOCK m_srwLock;
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