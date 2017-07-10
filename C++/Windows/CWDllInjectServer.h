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

/*
 * Each CDllInjectServer is responsible for a rule in the configuration file.
 * It will implement the DLL injection and communication between local and remote process.
 * It also call user-registered callback at preset timing.
 * Programmers should use CDllInjectMgr to handle DLL injection instead of using CDllInjectServer directly.
 */

#include <Windows.h>
#include <winternl.h>
#include <Strsafe.h>
#include <Psapi.h>
#include <process.h>
#include <string>
#include <map>
#include <list>
#include <vector>

#include "CWGeneralUtils.h"
#include "CWString.h"
#include "CWFile.h"
#include "CWProcess.h"
#include "CWIni.h"

#include "CWDllInjectCommonDef.h"




namespace CWUtils
{

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack( push , 8 )

typedef unsigned long DLL_INJECT_SERVER_CFG_TYPE;
enum
{
    DLL_INJECT_SERVER_CFG_ENABLE_DISABLE        = 0 ,
    DLL_INJECT_SERVER_CFG_IS_ENABLED            = 1
};

//Called when the DLL is injected to remote process
typedef DWORD (*DllInjectServerInjectedCbk)( IN DWORD aPid , IN VOID * aUserCtx );
//Called when the DLL in the remote process send some data for scanning
typedef DWORD (*DllInjectServerScanCbk)( IN DWORD aPid , IN VOID * aUserCtx , CHAR * aBuf , DWORD aBufSize );
//Called when the DLL is removed in remote process
typedef DWORD (*DllInjectServerUnInjectedCbk)( IN DWORD aPid , IN VOID * aUserCtx );

typedef struct _DllInjectServerUserCfg
{
    std::wstring wstrDllPath32;
    std::wstring wstrDllPath64;
    VOID * pUserCtx;
    DllInjectServerInjectedCbk InjectedCbk;
    DllInjectServerScanCbk ScanCbk;
    DllInjectServerUnInjectedCbk UnInjectedCbk;
} DllInjectServerUserCfg;

#pragma pack( pop )



#define    DLL_INJECT_SERVER_ENABLE_DEFAULT             FALSE
#define    DLL_INJECT_SERVER_WORKER_COUNT_DEFAULT       2
#define    DLL_INJECT_SERVER_WORKER_COUNT_MIN           1
#define    DLL_INJECT_SERVER_WORKER_COUNT_MAX           10

class CDllInjectServer
{
    private :
        #pragma pack( push , 8 )

        typedef enum _INJECT_CLIENT_STATE
        {
            INJECT_CLIENT_STATE_PREPARING = 0 ,
            INJECT_CLIENT_STATE_CAN_INJECT ,
            INJECT_CLIENT_STATE_ALREADY_INJECTED ,
            INJECT_CLIENT_STATE_NEED_UPDATE_PATTERN ,
            INJECT_CLIENT_STATE_INJECT_FAIL
        } INJECT_CLIENT_STATE;

        class InjectClientInfo
        {
            public :
                InjectClientInfo() : lRefCnt(0) , uInjectClientState(INJECT_CLIENT_STATE_PREPARING) , hRemoteProc(NULL) , hRemoteDll(NULL)  
                {
                    ZeroMemory( &hPerClientEvt , sizeof(hPerClientEvt) ); 
                }
                ~InjectClientInfo() 
                {
                    this->ClearNonReusablePart();
                }
                VOID ClearNonReusablePart()
                {
                    for ( size_t i = 0 ; i < _countof(hPerClientEvt) ; i++ )
                    {
                        CloseHandle( hPerClientEvt[i] );
                        hPerClientEvt[i] = NULL;
                    }
                    CloseHandle( hRemoteProc );
                    hRemoteProc = NULL;
                    hRemoteDll = NULL;
                }
                volatile LONG lRefCnt;
                std::wstring wstrProcPath;
                INJECT_CLIENT_STATE uInjectClientState;

                HANDLE hRemoteProc;                         //Local handle of the remote process
                HMODULE hRemoteDll;                         //Remote handle of the remote DLL
                HANDLE hPerClientEvt[PER_CLIENT_EVT_COUNT];     //Will be filled when injecting to a new process
        };

        typedef struct _DllInjectServerCommonCfg
        {
            _DllInjectServerCommonCfg() : bEnabled(DLL_INJECT_SERVER_ENABLE_DEFAULT) , ulWorkerCnt(DLL_INJECT_SERVER_WORKER_COUNT_DEFAULT) {}
            BOOL bEnabled;
            ULONG ulWorkerCnt;                  //How many job worker will be created for this kind of process
            std::wstring wstrProcPath;          //The destination process path we want to inject the DLL
            std::wstring wstrClientCfgPath;       //Will be sent to shared memory so clients can get config file from this path
        } DllInjectServerCommonCfg;

        typedef enum _INJECT_JOB_TYPE
        {
            INJECT_JOB_TYPE_SCAN_REQ = 0
        } INJECT_JOB_TYPE;

        typedef struct _InjectJob
        {
            DWORD                   dwOwnerPid;
            HANDLE                  hOwnerProc;
            VOID *                  pOwnerCtx;
            INJECT_JOB_TYPE         uJobType;
            DWORD                   dwJobDataSize;
            CHAR                    pJobData[1];
        } InjectJob;

        #pragma pack( pop )

    public :
        CDllInjectServer( HANDLE aActiveThread , CONST WCHAR * aCfgPath , size_t aRuleIndex , CONST WCHAR * aRuleName , 
                          UINT32 aLoadLibraryW32 , UINT32 aFreeLibrary32 , UINT64 aLoadLibraryW64 , UINT64 aFreeLibrary64 ) :
            m_hActiveThread(aActiveThread) , m_wstrCfgPath(aCfgPath) , m_uRuleIndex(aRuleIndex) , m_wstrRuleName(aRuleName) ,
            m_uLoadLibraryW32(aLoadLibraryW32) , m_uFreeLibrary32(aFreeLibrary32) , m_uLoadLibraryW64(aLoadLibraryW64) , m_uFreeLibrary64(aFreeLibrary64) ,
            m_bStarted(FALSE) , m_lJobThreadCnt(0) , m_hEvtNewJob(NULL) , m_hJobCreatorThread(NULL) , m_hJobWorkerThreads(NULL) 
        {
            InitializeCriticalSection( &m_csClientStateTable );
            InitializeCriticalSection( &m_csSmL2R );
            InitializeCriticalSection( &m_csJobs );
            ZeroMemory( &m_hPerServerEvt , sizeof(m_hPerServerEvt) );
            ZeroMemory( &m_hPerServerMutex , sizeof(m_hPerServerMutex) );
            ZeroMemory( &m_hPerServerSm , sizeof(m_hPerServerSm) );
            ZeroMemory( &m_pPerServerSm , sizeof(m_pPerServerSm) );

            CWUtils::GetModuleDir( GetModuleHandleW(NULL) , m_wstrModDir );
        }
        virtual ~CDllInjectServer()
        {
            DeleteCriticalSection( &m_csJobs );
            DeleteCriticalSection( &m_csSmL2R );
            DeleteCriticalSection( &m_csClientStateTable );
        }

    public :
        size_t GetRuleIndex() { return m_uRuleIndex; }
        CONST WCHAR * GetRuleName() { return m_wstrRuleName.c_str(); }

        BOOL RegisterDllInject( DllInjectServerUserCfg * aUserCfg );
        BOOL UnregisterDllInject();

        BOOL StartMonitor( BOOL aCheckExistProcs );
        BOOL StopMonitor();
        BOOL IsStarted() { return InterlockedCompareExchange( reinterpret_cast<volatile LONG*>( &m_bStarted ) , 0 , 0 ); }        

        DWORD SendData( DWORD aPid , CHAR * aReqBuf , DWORD aReqBufSize , CHAR * aRspBuf , DWORD * aRspBufSize );
        
        //The following three callbacks may call StartInject() and StopInject(). Return FALSE if any error happen inside
        BOOL OnProcessCreateTerminate( BOOL aCreate , DWORD aPid , CONST WCHAR * aProcPath , CONST WCHAR * aBaseName );
        BOOL OnThreadCreateTerminate( BOOL aCreate , DWORD aPid , DWORD aTid );
        BOOL OnImageLoaded( DWORD aPid , CONST WCHAR * aImagePath , CONST WCHAR * aBaseName );
        
        BOOL GetConfig( DLL_INJECT_SERVER_CFG_TYPE aCfgType , IN OUT VOID * pData , IN OUT UINT * pDataSize  );
        BOOL ChangeConfig( DLL_INJECT_SERVER_CFG_TYPE aCfgType , IN VOID * pData , IN UINT uDataSize );

    protected :
        BOOL ReloadCommonConfig();
        BOOL ReloadClientStateTable();

        BOOL CreateCommonHandles();
        BOOL DestroyCommonHandles();

        BOOL CreateJobThreads();
        BOOL DestroyJobThreads();
        static UINT CALLBACK JobCreatorThread( VOID * pThis );  //Return Win32 error code
        DWORD DoJobCreator();                                   //Return Win32 error code
        static UINT CALLBACK JobWorkerThread( VOID * pThis );   //Return Win32 error code
        DWORD DoJobWorker();                                    //Return Win32 error code
        BOOL PushToJobQueue( IN InjectJob * pJob );
        BOOL PopFromJobQueue( IN OUT InjectJob ** pJob );     //Return FALSE if JobQueue is empty, TRUE otherwise      

        BOOL StartInject( DWORD aPid );
        BOOL StopInject( DWORD aPid );
        
    private :
        HANDLE m_hActiveThread;
        std::wstring m_wstrModDir;                  //Current directory path
        CONST std::wstring m_wstrCfgPath;           //Ini path
        CONST size_t m_uRuleIndex;
        CONST std::wstring m_wstrRuleName;
        UINT32 m_uLoadLibraryW32 , m_uFreeLibrary32;
        UINT64 m_uLoadLibraryW64 , m_uFreeLibrary64;

        volatile BOOL m_bStarted;
        DllInjectServerCommonCfg m_CommonCfg;
        DllInjectServerUserCfg m_UserCfg;

        CRITICAL_SECTION m_csClientStateTable;
        std::map<DWORD , InjectClientInfo> m_mapClientStateTable;   //<key , value> = <PID , Interesting processes' information>

        HANDLE m_hPerServerEvt[PER_SERVER_EVT_COUNT];        //Per-server event. Used to communicate between remote client and local server
        HANDLE m_hPerServerMutex[PER_SERVER_MUTEX_COUNT];    //Per-server mutex. Used to ensure R2L(Remote to Local)'s share memory integrity
        HANDLE m_hPerServerSm[PER_SERVER_SM_COUNT];          //Per-server share memory handle. Used for data exchange (DLL_INJECT_SERVER_SM_DATA_HEADER_HEADER_HEADER). R2L is protected by mutex and L2R is protected by critical section
        PDLL_INJECT_SERVER_SM_DATA_HEADER m_pPerServerSm[PER_SERVER_SM_COUNT];    //Per-server share memory address. Used for data exchange (DLL_INJECT_SERVER_SM_DATA_HEADER_HEADER_HEADER). R2L is protected by mutex and L2R is protected by critical section
        CRITICAL_SECTION m_csSmL2R;                      //Per-server critical section. Used to ensure L2R(Local to Remote)'s share memory integrity

        volatile LONG m_lJobThreadCnt;
        HANDLE m_hEvtNewJob;
        CRITICAL_SECTION m_csJobs;
        std::list<InjectJob *> m_JobQueue;
        HANDLE m_hJobCreatorThread;
        HANDLE * m_hJobWorkerThreads;
};

#ifdef __cplusplus
}
#endif

}   //End of namespace CWUtils