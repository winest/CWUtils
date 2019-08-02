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

#pragma warning( push, 0 )
#include <Windows.h>
#include <queue>
#pragma warning( pop )

namespace CWUtils
{
#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif


template<class Type>
class CQueueMRMW
{
    public:
    CQueueMRMW()
    {
        InitializeCriticalSection( &m_csData );
        m_hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );    //This event will remain signaled until the queue is empty
    }
    ~CQueueMRMW()
    {
        DeleteCriticalSection( &m_csData );
        CloseHandle( m_hEvent );
    }

    VOID Push( CONST Type & oNewData )
    {
        EnterCriticalSection( &m_csData );
        m_oQueue.push( oNewData );
        SetEvent( m_hEvent );
        LeaveCriticalSection( &m_csData );
    }

    Type Pop()
    {
        EnterCriticalSection( &m_csData );

        Type popData = m_oQueue.front();
        m_oQueue.pop();

        if ( !m_oQueue.size() )
            ResetEvent( m_hEvent );

        LeaveCriticalSection( &m_csData );
        return popData;
    }

    //Helper method to get the event handle. User can wait on this event to handle incoming data
    HANDLE GetEvent() { return m_hEvent; }

    private:
    std::queue<Type> m_oQueue;    //Contains the actual data
    CRITICAL_SECTION m_csData;    //To synchronize access to m_csData among multiple threads
    HANDLE m_hEvent;              //For signal presence of absence of data
};



}    //End of namespace CWUtils