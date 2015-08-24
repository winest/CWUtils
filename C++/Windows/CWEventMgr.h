#pragma once

#include <Windows.h>
#include <process.h>
#include <map>
#include <list>
using std::map;
using std::list;

namespace CWUtils
{

#define EVENT_MGR_PREFIX    L"Global\\CEventMgr-3936645F-289B-4E59-AFF4-B7F59D9D5A4F"

typedef BOOL (* EventCallback) ( ULONG aEventId );
class CEventMgr
{
    //Data structures
    private :
        typedef struct _MapValue
        {
            _MapValue() { bStopThread = FALSE; hThread = hEvent = NULL; lsCallback.clear(); }
            ~_MapValue() { bStopThread = FALSE; CloseHandle( hThread ); CloseHandle( hEvent ); hThread = hEvent = NULL; lsCallback.clear(); }
            BOOL bStopThread;
            HANDLE hThread;
            HANDLE hEvent;
            ULONG ulEventId;
            list<EventCallback> lsCallback;
        } MapValue;
        typedef struct _EventThreadArgs
        {
            ULONG ulEventId;
            HANDLE hEvent;
        } EventThreadArgs;

    //Constructor and destructor, set constructor as protected to guarantee a single instance
    protected :
        CEventMgr()
        {
            InitializeCriticalSection( &m_cs );
            m_mapEvent.clear();
            m_nTotalThreads = 0;

            m_hChkStop = CreateEventW( NULL , TRUE , FALSE , NULL );
        }

    public :
        static CEventMgr * GetInstasnce() { return &m_self; }
        virtual ~CEventMgr()
        {
            if ( m_mapEvent.size() > 0 )
            {
                EnterCriticalSection( &m_cs );
                map<ULONG , MapValue>::iterator itEvent;
                for ( itEvent = m_mapEvent.begin() ; itEvent != m_mapEvent.end() ; itEvent++ )
                {
                    itEvent->second.bStopThread = TRUE;
                }
                LeaveCriticalSection( &m_cs );

                UINT uRetryCount = 10;
                SetEvent( m_hChkStop );
                while ( m_nTotalThreads > 0 && uRetryCount-- )
                {
                    Sleep( 100 );
                }
                ResetEvent( m_hChkStop );
                CloseHandle( m_hChkStop );
                m_mapEvent.clear();
            }
            else{}
            
            DeleteCriticalSection( &m_cs );
        }

    public :
        BOOL RegisterCallback(const ULONG aEventId , EventCallback aCallback);
        BOOL UnRegisterCallback(const ULONG aEventId , EventCallback aCallback);

        BOOL ForgeEvent(ULONG aEventId);

    private :
        static UINT CALLBACK EventThread(VOID * aArgs);

        BOOL AllocEventThread( ULONG aEventId , EventCallback aCallback );
        BOOL FreeEventThread( ULONG aEventId , EventThreadArgs * aArgs );
        
    private :        
        map<ULONG , MapValue> m_mapEvent;
        CRITICAL_SECTION m_cs;        
        HANDLE m_hChkStop;
        UINT m_nTotalThreads;    //Count of EventThread

        static CEventMgr m_self;
};

}    //End of namespace CWUtils