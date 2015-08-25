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

#include <Windows.h>
#include <string>
#include <list>
#include <vector>
#include "CWQueue.h"
using std::wstring;
using std::list;
using std::vector;

namespace CWUtils
{

#ifdef __cplusplus
extern "C" {
#endif



#define MONITOR_THREAD_STOP_MAX_RETRY_COUNT 10

typedef ULONG (CALLBACK * MyMonitorFileProc)( DWORD aAction , CONST wstring & aSrcPath , CONST wstring & aDstPath );

typedef enum _CWFileMonitorEvent
{
    MY_FILE_MONITOR_EVT_NONE = 0 ,
    MY_FILE_MONITOR_EVT_LOCAL_ADD ,
    MY_FILE_MONITOR_EVT_LOCAL_REMOVE ,
    MY_FILE_MONITOR_EVT_LOCAL_RENAME , 
    MY_FILE_MONITOR_EVT_LOCAL_MODIFY
} CWFileMonitorEvent;

typedef struct _CWFileMonitorNotification
{
    CWFileMonitorEvent dwEvent;
    std::wstring wstrSrcPath;
    std::wstring wstrDstPath;
} CWFileMonitorNotification;

class CFileMonitor;

class CFileMonitorRequest
{
    public :
        CFileMonitorRequest( CFileMonitor * aParent ) : m_parent(aParent) , m_hMonitorPath(INVALID_HANDLE_VALUE) {}        
        ~CFileMonitorRequest() { StopCheck( (ULONG_PTR)this ); }

        BOOL Init( CONST WCHAR * aMonitorPath , BOOL aRecursive , DWORD aFilter , DWORD aShareMemSize );

        static VOID NTAPI StartCheck( ULONG_PTR aRequest );
        static VOID NTAPI StopCheck( ULONG_PTR aRequest );

    public :    //Friend methods of CFileMonitor        
        VOID PushToQueue( CFileMonitor * aMonitor , CWFileMonitorNotification & aNotification );  //Push notification data to queue for later processing
        VOID DecrementWorkCount( CFileMonitor * aMonitor );           //Decrement work count in CFileMonitor

    private :
        VOID CheckFileChange();
        static VOID CALLBACK OnChangeComplete( DWORD aErrorCode , DWORD aBytesTransfered , LPOVERLAPPED aOverlapped );
        VOID ProcessNotification();
        CWFileMonitorEvent GetNotifyEvent( CONST FILE_NOTIFY_INFORMATION * aNotifyInfo );
        wstring GetNotifyPath( CONST FILE_NOTIFY_INFORMATION * aNotifyInfo );   //Convert the FileName in FILE_NOTIFY_INFORMATION to the full path

    private :
        CFileMonitor * m_parent;

        HANDLE m_hMonitorPath;
        wstring m_wstrMonitorPath;
        BOOL m_bRecursive;
        DWORD m_dwFilter;
        OVERLAPPED m_overlapped;
        
        vector<BYTE> m_vecShareMem;     //Use vector to ensure it's a DWORD-aligned formatted buffer
        vector<BYTE> m_vecShareMemBak;  //Used to backup share memory once completion routine is called so we can receive next notification as soon as possible
};


class CFileMonitor
{
    public :
        CFileMonitor() : m_hEventStop(NULL) , m_hThreadMonitor(NULL) , m_hThreadDispatcher(NULL) , m_uTotalThread(0) , m_uTotalWork(0) , m_fnOnFileChanged(NULL) {}
        ~CFileMonitor() { Stop(); }

        VOID SetFileChangedProc( MyMonitorFileProc aFileChangedProc );

        //User should call SetFileChangedProc() before Start()
        BOOL Start( CONST WCHAR * aMonitorPath , BOOL aRecursive , 
                    DWORD aFilter = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION ,
                    DWORD aShareMemSize = 1 * 1024 * 1024 );
        BOOL Stop();

        
    private :
        BOOL NormalizeParam( CONST WCHAR * aMonitorPath , BOOL & aRecursive , DWORD & aFilter , DWORD & aShareMemSize );
        BOOL CreateMonitorDispatcherThread();

        static UINT CALLBACK MonitorThread( VOID * aArgs );
        static UINT CALLBACK DispatcherThread( VOID * aArgs );

    private :   //Friend methods for CFileMonitorRequest
        //Push data into m_queue
        friend VOID CFileMonitorRequest::PushToQueue( CFileMonitor * aMonitor , CWFileMonitorNotification & aNotification );

        //Decrement m_uTotalWork when exit
        friend VOID CFileMonitorRequest::DecrementWorkCount( CFileMonitor * aMonitor );


    private :
        list<CFileMonitorRequest *> m_lsRequest;
        volatile UINT m_uTotalWork;
        volatile UINT m_uTotalThread;

        HANDLE m_hEventStop;
        HANDLE m_hThreadMonitor , m_hThreadDispatcher;

        CQueueMRMW<CWFileMonitorNotification> m_queue;
        volatile MyMonitorFileProc m_fnOnFileChanged;
};

#ifdef __cplusplus
}
#endif

}   //End of namespace CWUtils