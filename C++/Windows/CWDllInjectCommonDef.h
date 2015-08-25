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
 * This header define the variables and structures used by both CDllInjectCliens, CDllInjectServer, and CDllInjectMgr
 */


namespace CWUtils
{

#ifdef __cplusplus
extern "C" {
#endif



#pragma pack( push , 8 )

typedef enum _PER_SERVER_EVT_INDEX
{
    PER_SERVER_EVT_INDEX_STOP = 0 , 
    PER_SERVER_EVT_INDEX_REMOTE_REQ ,
    PER_SERVER_EVT_INDEX_REMOTE_RSP_OK ,
    PER_SERVER_EVT_COUNT
} PER_SERVER_EVT_INDEX;

typedef enum _PER_CLIENT_EVT_INDEX
{
    PER_CLIENT_EVT_INDEX_STOP = 0 ,
    PER_CLIENT_EVT_INDEX_REMOTE_REQ_OK ,
    PER_CLIENT_EVT_INDEX_REMOTE_RSP ,
    PER_CLIENT_EVT_COUNT
} PER_CLIENT_EVT_INDEX;

typedef enum _PER_SERVER_MUTEX_INDEX
{
    PER_SERVER_MUTEX_INDEX_REMOTE_INSTANCE = 0 ,
    PER_SERVER_MUTEX_INDEX_SHAREM_MEM_R2L ,
    PER_SERVER_MUTEX_COUNT
} PER_SERVER_MUTEX_INDEX;

typedef enum _PER_SERVER_SM_INDEX
{
    PER_SERVER_SM_INDEX_REMOTE_TO_LOCAL = 0 ,
    PER_SERVER_SM_INDEX_LOCAL_TO_REMOTE ,
    PER_SERVER_SM_COUNT
} PER_SERVER_SM_INDEX;



#define GenerateShareMemoryName( aBuf , aPid ) do                                                                                           \
                                               {                                                                                            \
                                                   WCHAR wzGuid[38] = {};                                                                   \
                                                   if ( FALSE == GetVolumeNameForVolumeMountPointW( L"C:\\" , wzGuid , _countof(wzGuid) ) ) \
                                                   {                                                                                        \
                                                       wcsncpy_s( wzGuid , L"8ABEEEF5-29E8-4EE6-8F56-186D116618F7" , _TRUNCATE );           \
                                                   }                                                                                        \
                                                   _snwprintf_s( aBuf , _TRUNCATE , L"Global\\%ws-0x%04X" , wzGuid , aPid );                \
                                               } while ( 0 )



//This is a named share memory used to exchange all information needed to communicate between
//local(Osprey) and remote(Skype, Line...). All handles are created in Osprey side and duplicated to remote processes
//Be ware to use UINT64 instead of any pointer type since we need to communicate between 32 and 64 bit processes
typedef struct _DLL_INJECT_SERVER_SM_INIT
{
    struct _InitReq
    {
        struct _Local{
            UINT64 pLocalCtx;                               //Per-process context with structure InjectClientInfo
            UINT64 hRemoteProc;                             //Remote process's handle. Job thread will use this to check whether the process exists instead of looking up ClientStateTable to speed up performance
            UINT64 pfnFreeLibrary;                          //Kernel32.dll's FreeLibrary address. This prevent FreeLibrary is hooked by others in the remote process
        } Local;
        
        struct _Remote{
            UINT64 hEvtInitRsp;                             //Per-client event. DuplicateHandle to DLL. Server wait this event and get status from InitRsp
            UINT64 hDllInjectMgrAliveThread;                //Created in DllInjectMgr. DuplicateHandle to DLL. Used for monitoring DllInjectMgr's crash 

            UINT64 hPerServerSm[PER_SERVER_SM_COUNT];           //Per-server share memory. DuplicateHandle to DLL. Used for data exchange (DLL_INJECT_SERVER_SM_DATA_HEADER)            
            UINT64 hPerServerMutex[PER_SERVER_MUTEX_COUNT];     //Per-server mutex. DuplicateHandle to DLL. Used to ensure R2L's share memory integrity

            UINT64 hPerServerEvt[PER_SERVER_EVT_COUNT];
            UINT64 hPerClientEvt[PER_CLIENT_EVT_COUNT];
        } Remote;

        WCHAR wzServerDirPath[MAX_PATH];
        WCHAR wzClientCfgPath[MAX_PATH];
    } InitReq;

    struct _InitRsp
    {
        DWORD dwHookStatus;                                 //Win32 error code, use ERROR_SUCCESS to represent succeed
        UINT64 hModule;                                     //Remote DLL module's handle
    } InitRsp;
} DLL_INJECT_SERVER_SM_INIT , *PDLL_INJECT_SERVER_SM_INIT;






#define DLL_INJECT_SERVER_SM_DATA_HEADER_VER          0
#define DLL_INJECT_SERVER_SM_DATA_HEADER_MAX_SIZE     (64 * 1024)

typedef enum _DLL_INJECT_SERVER_SM_DATA_TYPE
{
    DLL_INJECT_SERVER_SM_DATA_TYPE_SCAN_REQ = 0 ,
    DLL_INJECT_SERVER_SM_DATA_TYPE_SCAN_RSP
} DLL_INJECT_SERVER_SM_DATA_TYPE;

//This is the general structure for per-server's private share memories between local(DllInjectServer) and remote(Skype, Line...)
//Be ware to use UINT64 instead of any pointer type since we need to communicate between 32 and 64 bit processes
typedef struct _DLL_INJECT_SERVER_SM_DATA_HEADER
{
    _DLL_INJECT_SERVER_SM_DATA_HEADER() : dwVersion(DLL_INJECT_SERVER_SM_DATA_HEADER_VER) , dwMaxSize(DLL_INJECT_SERVER_SM_DATA_HEADER_MAX_SIZE) ,
                                          dwRemotePid(0) , hRemoteProc(NULL) , pLocalCtx(NULL) { ZeroMemory(&pReserved,sizeof(pReserved)); }
    DWORD                   dwVersion;
    DWORD                   dwMaxSize;
    DWORD                   dwRemotePid;    //Remote process's PID
    UINT64                  hRemoteProc;    //Remote process's handle
    UINT64                  pLocalCtx;      //Per-process context with structure InjectClientInfo
    UINT64                  pReserved[8];
    DLL_INJECT_SERVER_SM_DATA_TYPE       uDataType;
    DWORD                   dwDataSize;     //Data size in bytes of the pData field
    CHAR                    pData[1];
} DLL_INJECT_SERVER_SM_DATA_HEADER , *PDLL_INJECT_SERVER_SM_DATA_HEADER;

typedef struct _DLL_INJECT_SERVER_SM_DATA_SCAN_REQ
{
    DWORD     dwReqSize;            //Data size in bytes of the pReq field
    CHAR      pReq[1];
} DLL_INJECT_SERVER_SM_DATA_SCAN_REQ;

typedef struct _DLL_INJECT_SERVER_SM_DATA_SCAN_RSP
{
    DWORD     dwRspSize;            //Data size in bytes of the pRsp field
    CHAR      pRsp[1];
} DLL_INJECT_SERVER_SM_DATA_SCAN_RSP;

#pragma pack( pop )


#ifdef __cplusplus
}
#endif

}   //End of namespace CWUtils