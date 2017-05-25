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
 * Use CDllInjectClient at client side to communicate with the CDllInjectServer which locates in your program.
 */


#include "CWDllInjectCommonDef.h"
#include <Windows.h>
#include <string>

namespace CWUtils
{

#ifdef __cplusplus
extern "C" {
#endif



//Return Win32 error code. Use ERROR_SUCCESS to tell DllInjectServer that we successfully do the initialization work
typedef DWORD (CALLBACK *PFN_DLL_INJECT_CLIENT_INIT_CBK)( CONST WCHAR * aServerDirPath , CONST WCHAR * aClientCfgPath );

class CDllInjectClient
{
    public :
        CDllInjectClient() {}
        virtual ~CDllInjectClient() { this->Disconnect(); }

    public :
        BOOL Connect( CONST WCHAR * aKey , PFN_DLL_INJECT_CLIENT_INIT_CBK aInitCbk = NULL );
        BOOL Disconnect();
        BOOL IsConnected() { return InterlockedCompareExchange( reinterpret_cast<volatile LONG*>( &m_bConnected ) , 0 , 0 ); }

        BOOL SendData( CHAR * aReqBuf , DWORD aReqBufSize , CHAR * aRspBuf , DWORD * aRspBufSize );

        HANDLE GetServerQuitEvt()   { return ( this->IsConnected() ) ? (HANDLE)m_SmInit.InitReq.Remote.hPerServerEvt[PER_SERVER_EVT_INDEX_STOP] : NULL; }
        HANDLE GetClientQuitEvt() { return ( this->IsConnected() ) ? (HANDLE)m_SmInit.InitReq.Remote.hPerClientEvt[PER_CLIENT_EVT_INDEX_STOP] : NULL; }
        HANDLE GetServerAliveEvt()  { return ( this->IsConnected() ) ? (HANDLE)m_SmInit.InitReq.Remote.hDllInjectMgrAliveThread : NULL; }

        LPTHREAD_START_ROUTINE GetFreeLibraryAddr() { return ( this->IsConnected() ) ? (LPTHREAD_START_ROUTINE)m_SmInit.InitReq.Local.pfnFreeLibrary : NULL; }

        //Return the full directory path of the process who run DllInjectMgr/DllInjectServer
        CONST WCHAR * GetServerDirPath()  { return ( this->IsConnected() ) ? m_SmInit.InitReq.wzServerDirPath : NULL; }
        //Return the client configuration path written in the ClientCfgPath field
        CONST WCHAR * GetClientCfgPath()  { return ( this->IsConnected() ) ? m_SmInit.InitReq.wzClientCfgPath : NULL; }

    private :
        BOOL m_bConnected;
        std::wstring m_wstrKey;

        DLL_INJECT_SERVER_SM_INIT m_SmInit;
        DLL_INJECT_SERVER_SM_DATA_HEADER * m_SmData[PER_SERVER_SM_COUNT];
};

#ifdef __cplusplus
}
#endif

}   //End of namespace CWUtils