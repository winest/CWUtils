#include "stdafx.h"
#include "EventMgr.h"

namespace CWUtils
{

CEventMgr CEventMgr::m_self;

BOOL CEventMgr::RegisterCallback( const ULONG aEventId , EventCallback aCallback )
{
    if ( m_self.m_hChkStop == NULL || aCallback == NULL )
        return FALSE;

    EnterCriticalSection( &m_self.m_cs );
    map<ULONG , MapValue>::iterator itEvent = m_self.m_mapEvent.find( aEventId );
    if ( itEvent != m_self.m_mapEvent.end() )    //If the event handling thread already exist
    {
        //If the callback is already registered, just return TRUE
        list<EventCallback>::iterator itCallback;
        for ( itCallback = itEvent->second.lsCallback.begin() ; itCallback != itEvent->second.lsCallback.end() ; ++itCallback )
        {
            if ( *itCallback == aCallback )
            {
                LeaveCriticalSection( &m_self.m_cs );
                return TRUE;
            }
        }
        itEvent->second.lsCallback.push_back( aCallback );
        LeaveCriticalSection( &m_self.m_cs );
        return TRUE;
    }
    else
    {
        LeaveCriticalSection( &m_self.m_cs );
        return m_self.AllocEventThread( aEventId , aCallback );        
    }    
}


BOOL CEventMgr::UnRegisterCallback( const ULONG aEventId , EventCallback aCallback )
{
    if ( m_self.m_hChkStop == NULL || aCallback == NULL )
        return FALSE;

    EnterCriticalSection( &m_self.m_cs );
    map<ULONG , MapValue>::iterator itEvent = m_self.m_mapEvent.find( aEventId );
    if ( itEvent != m_self.m_mapEvent.end() )    //If the event handling thread exists
    {
        list<EventCallback>::iterator itCallback;
        for ( itCallback = itEvent->second.lsCallback.begin() ; itCallback != itEvent->second.lsCallback.end() ; )
        {
            if ( *itCallback == aCallback )
            {
                itEvent->second.lsCallback.erase( itCallback++ );
                break;
            }
            else
            {
                ++itCallback;
            }
        }
        if ( itEvent->second.lsCallback.size() == 0 )    //No callback anyone, close the thread
        {
            HANDLE hThread = itEvent->second.hThread;
            
            itEvent->second.bStopThread = TRUE;
            SetEvent( m_self.m_hChkStop );
            ResetEvent( m_self.m_hChkStop );
            LeaveCriticalSection( &m_self.m_cs );

            DWORD ret = WaitForSingleObject( hThread , 1 * 1000 );
            return ( WAIT_OBJECT_0 == ret ) ? TRUE : FALSE;
        }
    }
    LeaveCriticalSection( &m_self.m_cs );
    return TRUE;
}


BOOL CEventMgr::ForgeEvent( ULONG aEventId )
{
    BOOL bRet = FALSE;

    WCHAR wzEventName[MAX_PATH];
    swprintf_s( wzEventName , L"%ws-%08X" , EVENT_MGR_PREFIX , aEventId );

    HANDLE hEvent = CreateEventW( NULL , TRUE , FALSE , wzEventName );
    if ( hEvent != NULL )
    {
        if ( SetEvent( hEvent ) == TRUE )
            bRet = TRUE;

        ResetEvent( hEvent );
        CloseHandle( hEvent );
    }    
    return bRet;
}


//Handle each event
UINT CALLBACK CEventMgr::EventThread( VOID * aArgs )
{
    DWORD dwRet = 0;
    BOOL bRun = TRUE;
    EventThreadArgs * args = (EventThreadArgs *)aArgs;
    ULONG ulEventId = args->ulEventId;
    HANDLE hEvents[2] = { m_self.m_hChkStop , args->hEvent };

    while ( bRun )
    {
        dwRet = WaitForMultipleObjects( _countof(hEvents) , hEvents , FALSE , INFINITE );
        switch ( dwRet )
        {
            case WAIT_OBJECT_0 :        //Check whether we have to stop
            {
                map<ULONG , MapValue>::iterator itEvent = m_self.m_mapEvent.find( ulEventId );
                if ( itEvent == m_self.m_mapEvent.end() || itEvent->second.bStopThread )
                    bRun = FALSE;
                break;
            }

            case WAIT_OBJECT_0 + 1 :    //Get event
            {
                EnterCriticalSection( &m_self.m_cs );
                map<ULONG , MapValue>::iterator itEvent;
                list<EventCallback>::iterator itCallback;
                itEvent = m_self.m_mapEvent.find( ulEventId );
                if ( itEvent == m_self.m_mapEvent.end() || itEvent->second.bStopThread )
                {
                    bRun = FALSE;
                }
                else
                {
                    for ( itCallback = itEvent->second.lsCallback.begin() ; itCallback != itEvent->second.lsCallback.end() ; itCallback++ )
                    {
                        (*itCallback)( itEvent->second.ulEventId );
                    }
                }
                LeaveCriticalSection( &m_self.m_cs );
                break;
            }

            default :
            {
                bRun = FALSE;
                break;
            }            
        }
    }
    m_self.FreeEventThread( ulEventId , args );
    InterlockedDecrement( &m_self.m_nTotalThreads );
    return dwRet;
}

BOOL CEventMgr::AllocEventThread( ULONG aEventId , EventCallback aCallback )
{
    //EventThread is allocated only if there is no thread handling such event type
    WCHAR wzEventName[MAX_PATH];
    swprintf_s( wzEventName , L"%ws-%08X" , EVENT_MGR_PREFIX , aEventId );

    HANDLE hEvent = CreateEventW( NULL , TRUE , FALSE , wzEventName );
    if ( hEvent == NULL )
    {
        return FALSE;
    }
    else
    {
        EventThreadArgs * args = new EventThreadArgs();
        args->ulEventId = aEventId;
        args->hEvent = hEvent;

        //Create a suspend thread, fill m_mapEvent, and then resume the thread
        InterlockedIncrement( &this->m_nTotalThreads );
        HANDLE hThread = (HANDLE)_beginthreadex( NULL , NULL , CEventMgr::EventThread , (VOID *)args , CREATE_SUSPENDED , NULL );
        if ( hThread != NULL )
        {
            EnterCriticalSection( &this->m_cs );
            this->m_mapEvent[aEventId].bStopThread = FALSE;
            this->m_mapEvent[aEventId].hEvent = hEvent;
            this->m_mapEvent[aEventId].hThread = hThread;
            this->m_mapEvent[aEventId].ulEventId = aEventId;
            this->m_mapEvent[aEventId].lsCallback.push_back( aCallback );
            LeaveCriticalSection( &this->m_cs );

            if ( ResumeThread( hThread ) == (DWORD)-1 )
            {
                this->FreeEventThread( aEventId , args );
                InterlockedDecrement( &this->m_nTotalThreads );
                return FALSE;
            }
            else
            {
                return TRUE;
            }
        }
        else
        {
            this->FreeEventThread( aEventId , args );
            InterlockedDecrement( &(this->m_nTotalThreads) );
            return FALSE;
        }
    }
}

BOOL CEventMgr::FreeEventThread( ULONG aEventId , EventThreadArgs * aArgs )
{
    map<ULONG , MapValue>::iterator itEvent = this->m_mapEvent.find( aEventId );    
    if ( itEvent != this->m_mapEvent.end() )
        this->m_mapEvent.erase( itEvent );

    if ( aArgs != NULL )
        delete aArgs;

    return TRUE;
}


}    //End of namespace CWUtils