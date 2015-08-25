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
 * Use CDllInjectMgr at server side to communicate with the CDllInjectclient which locates in a remote process. 
 * The calling flow must be Init()->RegisterDllInject()->[StartMonitor()->StopMonitor()]*->UnregisterDllInject()->UnInit()
 */

#include <Windows.h>
#include <vector>
#include "DllInjectCommonDef.h"
#include "DllInjectServer.h"



namespace CWUtils
{

#ifdef __cplusplus
extern "C" {
#endif



class CDllInjectMgr
{
    public :
        CDllInjectMgr()
        {
            ZeroMemory( &m_vecDllInjectServer , sizeof(m_vecDllInjectServer) );
        }
        virtual ~CDllInjectMgr()
        {
            this->UnInit();
        }

    public :
        BOOL Init( CONST WCHAR * aCfgPath );
        BOOL UnInit();
        BOOL RegisterDllInject( CONST WCHAR * aRuleName , DllInjectServerUserCfg * aUserCfg );
        BOOL UnregisterDllInject( CONST WCHAR * aRuleName , DllInjectServerUserCfg * aUserCfg );
        BOOL StartMonitor( CONST WCHAR * aRuleName , BOOL aCheckExistProcs = FALSE );
        BOOL StopMonitor( CONST WCHAR * aRuleName );

        //Return TRUE if any rule is hit. FALSE otherwise
        BOOL OnProcessCreateTerminate( BOOL aCreate , DWORD aPid , CONST WCHAR * aProcPath );
        BOOL OnThreadCreateTerminate( BOOL aCreate , DWORD aPid , DWORD aTid );
        BOOL OnImageLoaded( DWORD aPid , CONST WCHAR * aImagePath );

        BOOL GetConfig( CONST WCHAR * aRuleName , DLL_INJECT_SERVER_CFG_TYPE aCfgType , IN OUT VOID * pData , IN OUT UINT * uDataSize  );
        BOOL ChangeConfig( CONST WCHAR * aRuleName , DLL_INJECT_SERVER_CFG_TYPE aCfgType , IN VOID * pData , IN UINT uDataSize );

    protected :
        BOOL GetFunctionAddresses();

        BOOL CreateActiveThread();
        BOOL DestroyActiveThread();

        BOOL CreateServers( CONST WCHAR * aCfgPath );
        BOOL DestroyServers();

        size_t GetRuleIndexByName( CONST WCHAR * aRuleName );

    private :
        std::vector<CDllInjectServer *> m_vecDllInjectServer;

        UINT32 m_uLoadLibraryW32 , m_uFreeLibrary32;
        UINT64 m_uLoadLibraryW64 , m_uFreeLibrary64;
};


#ifdef __cplusplus
}
#endif

}   //End of namespace CWUtils