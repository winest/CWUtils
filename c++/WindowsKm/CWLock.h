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

#include <wdm.h>

#ifndef INFINITE
    #define INFINITE        0xFFFFFFFF
#endif

namespace KmUtils
{

#define MY_RW_LOCK_SPIN_COUNT 400

class CMyRWLock
{
    protected :
        CMyRWLock() {}
        virtual ~CMyRWLock() {}
    public :
        virtual BOOLEAN AcquireReaderLock( ULONG dwTimeout = INFINITE ) = NULL;
        virtual VOID ReleaseReaderLock() = NULL;
        virtual BOOLEAN AcquireWriterLock( ULONG dwTimeout = INFINITE ) = NULL;
        virtual VOID ReleaseWriterLock() = NULL;
};




class CMyEResource : public CMyRWLock
{
    public :
        CMyEResource()
        {    
            ExInitializeResourceLite( &m_res );
        }
        virtual ~CMyEResource()
        {
            ExDeleteResourceLite( &m_res );
        }

        virtual BOOLEAN AcquireReaderLock( ULONG aTimeout = INFINITE );
        virtual VOID ReleaseReaderLock();
        virtual BOOLEAN AcquireWriterLock( ULONG aTimeout = INFINITE );
        virtual VOID ReleaseWriterLock();

        VOID WriterLockToReaderLock();  //Assume user has already get the writer lock

    protected :
        ERESOURCE m_res;
};



class CMyAutoReaderLock
{
    public :
        inline CMyAutoReaderLock( CMyRWLock * aRWLock ) : m_lock( aRWLock )
        {
            m_lock->AcquireReaderLock();
        }
        inline ~CMyAutoReaderLock()
        {
            m_lock->ReleaseReaderLock();
        }
    protected :
        CMyAutoReaderLock() {}
        CMyRWLock * m_lock;
};

class CMyAutoWriterLock
{
    public :
        inline CMyAutoWriterLock( CMyRWLock * aRWLock ) : m_lock( aRWLock )
        {
            m_lock->AcquireWriterLock();
        }
        inline ~CMyAutoWriterLock()
        {
            m_lock->ReleaseWriterLock();
        }
    protected :
        CMyAutoWriterLock() {}
        CMyRWLock * m_lock;
};



}   //End of namespace KmUtils
