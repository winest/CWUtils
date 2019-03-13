#include "stdafx.h"
#include "CWDllInjectMgr.h"
#pragma warning( disable : 4127 )

#include "_GenerateTmh.h"
#include "CWDllInjectMgr.tmh"

namespace CWUtils
{
#ifdef __cplusplus
extern "C" {
#endif



#define DLL_INJECT_MGR_HELPER32 L"DllInjectMgrHelper32.exe"

#define KERNEL32_MODULE_NAME L"kernel32.dll"
#define LOADLIBRARY_FUNCTION_NAMEA "LoadLibraryW"
#define FREELIBRARY_FUNCTION_NAMEA "FreeLibrary"
#define LOADLIBRARY_FUNCTION_NAMEW L"LoadLibraryW"
#define FREELIBRARY_FUNCTION_NAMEW L"FreeLibrary"



UINT CALLBACK ActiveThread( VOID * aArg )
{
    HANDLE hActiveThreadQuitEvent = (HANDLE)aArg;
    WaitForSingleObject( hActiveThreadQuitEvent, INFINITE );
    return 0;
}

BOOL CDllInjectMgr::Init( CONST WCHAR * aCfgPath )
{
    _ASSERT( NULL == m_hActiveThreadQuitEvent && NULL == m_hActiveThread );
    DbgOut( VERB, DBG_DLL_INJECT_MGR, "Enter" );

    BOOL bRet = FALSE;
    do
    {
        wstring wstrCfgPath;
        if ( FALSE == CWUtils::RelativeToFullPath( aCfgPath, wstrCfgPath ) )
        {
            DbgOut( ERRO, DBG_DLL_INJECT_MGR, "RelativeToFullPath() failed. GetLastError()=%!WINERROR!",
                    GetLastError() );
            break;
        }

        if ( FALSE == this->GetFunctionAddresses() )
        {
            DbgOut( ERRO, DBG_DLL_INJECT_MGR, "GetFunctionAddresses() failed. GetLastError()=%!WINERROR!",
                    GetLastError() );
            break;
        }

        if ( FALSE == this->CreateActiveThread() )
        {
            DbgOut( ERRO, DBG_DLL_INJECT_MGR, "CreateActiveThread() failed" );
            break;
        }

        if ( FALSE == this->CreateServers( wstrCfgPath.c_str() ) )
        {
            DbgOut( ERRO, DBG_DLL_INJECT_MGR, "CreateServers() failed" );
            break;
        }

        bRet = TRUE;
    } while ( 0 );

    if ( FALSE == bRet )
    {
        this->DestroyServers();
        this->DestroyActiveThread();
    }

    DbgOut( VERB, DBG_DLL_INJECT_MGR, "Leave" );
    return bRet;
}

BOOL CDllInjectMgr::UnInit()
{
    DbgOut( VERB, DBG_DLL_INJECT_MGR, "Enter" );

    this->DestroyServers();
    this->DestroyActiveThread();

    DbgOut( VERB, DBG_DLL_INJECT_MGR, "Leave" );
    return TRUE;
}


BOOL CDllInjectMgr::RegisterDllInject( CONST WCHAR * aRuleName, DllInjectServerUserCfg * aUserCfg )
{
    BOOL bRet = FALSE;
    do
    {
        if ( NULL == aRuleName || NULL == aUserCfg )
        {
            SetLastError( ERROR_INVALID_PARAMETER );
            break;
        }
        size_t uIndex = this->GetRuleIndexByName( aRuleName );
        if ( (size_t)-1 == uIndex )
        {
            SetLastError( ERROR_NOT_FOUND );
            break;
        }

        bRet = m_vecDllInjectServer[uIndex]->RegisterDllInject( aUserCfg );
    } while ( 0 );
    return bRet;
}

BOOL CDllInjectMgr::UnregisterDllInject( CONST WCHAR * aRuleName )
{
    BOOL bRet = FALSE;
    do
    {
        if ( NULL == aRuleName )
        {
            SetLastError( ERROR_INVALID_PARAMETER );
            break;
        }
        size_t uIndex = this->GetRuleIndexByName( aRuleName );
        if ( (size_t)-1 == uIndex )
        {
            SetLastError( ERROR_NOT_FOUND );
            break;
        }

        bRet = m_vecDllInjectServer[uIndex]->UnregisterDllInject();
    } while ( 0 );
    return bRet;
}

BOOL CDllInjectMgr::StartMonitor( CONST WCHAR * aRuleName, BOOL aCheckExistProcs )
{
    BOOL bRet = FALSE;
    do
    {
        if ( NULL == aRuleName )
        {
            SetLastError( ERROR_INVALID_PARAMETER );
            break;
        }
        size_t uIndex = this->GetRuleIndexByName( aRuleName );
        if ( (size_t)-1 == uIndex )
        {
            SetLastError( ERROR_NOT_FOUND );
            break;
        }

        bRet = m_vecDllInjectServer[uIndex]->StartMonitor( aCheckExistProcs );
    } while ( 0 );
    return bRet;
}

BOOL CDllInjectMgr::StopMonitor( CONST WCHAR * aRuleName )
{
    BOOL bRet = FALSE;
    do
    {
        if ( NULL == aRuleName )
        {
            SetLastError( ERROR_INVALID_PARAMETER );
            break;
        }
        size_t uIndex = this->GetRuleIndexByName( aRuleName );
        if ( (size_t)-1 == uIndex )
        {
            SetLastError( ERROR_NOT_FOUND );
            break;
        }

        bRet = m_vecDllInjectServer[uIndex]->StopMonitor();
    } while ( 0 );
    return bRet;
}

BOOL CDllInjectMgr::OnProcessCreateTerminate( BOOL aCreate, DWORD aPid, CONST WCHAR * aProcPath )
{
    DbgOut( VERB, DBG_DLL_INJECT_MGR, "Enter. aCreate=%d, aPid=0x%X, aProcPath=%ws", aCreate, aPid, aProcPath );

    BOOL bRet = FALSE;
    CONST WCHAR * wzBaseName = CWUtils::GetPathBaseNameW( aProcPath );
    for ( size_t i = 0; i < m_vecDllInjectServer.size(); i++ )
    {
        if ( NULL != m_vecDllInjectServer[i] &&
             TRUE == m_vecDllInjectServer[i]->OnProcessCreateTerminate( aCreate, aPid, aProcPath, wzBaseName ) )
        {
            bRet = TRUE;
        }
    }
    DbgOut( VERB, DBG_DLL_INJECT_MGR, "Leave" );
    return bRet;
}

BOOL CDllInjectMgr::OnThreadCreateTerminate( BOOL aCreate, DWORD aPid, DWORD aTid )
{
    DbgOut( VERB, DBG_DLL_INJECT_MGR, "Enter. aCreate=%d, aPid=0x%X, aTid=0x%X", aCreate, aPid, aTid );

    BOOL bRet = FALSE;
    for ( size_t i = 0; i < m_vecDllInjectServer.size(); i++ )
    {
        if ( NULL != m_vecDllInjectServer[i] &&
             TRUE == m_vecDllInjectServer[i]->OnThreadCreateTerminate( aCreate, aPid, aTid ) )
        {
            bRet = TRUE;
        }
    }
    DbgOut( VERB, DBG_DLL_INJECT_MGR, "Leave" );
    return bRet;
}

BOOL CDllInjectMgr::OnImageLoaded( DWORD aPid, CONST WCHAR * aImagePath )
{
    DbgOut( VERB, DBG_DLL_INJECT_MGR, "Enter. aPid=0x%X, aImagePath=%ws", aPid, aImagePath );

    BOOL bRet = FALSE;
    CONST WCHAR * wzBaseName = CWUtils::GetPathBaseNameW( aImagePath );
    for ( size_t i = 0; i < m_vecDllInjectServer.size(); i++ )
    {
        if ( NULL != m_vecDllInjectServer[i] &&
             TRUE == m_vecDllInjectServer[i]->OnImageLoaded( aPid, aImagePath, wzBaseName ) )
        {
            bRet = TRUE;
        }
    }
    DbgOut( VERB, DBG_DLL_INJECT_MGR, "Leave" );
    return bRet;
}



BOOL CDllInjectMgr::GetConfig( CONST WCHAR * aRuleName,
                               DLL_INJECT_SERVER_CFG_TYPE aCfgType,
                               IN OUT VOID * pData,
                               IN OUT UINT * uDataSize )
{
    DbgOut( VERB, DBG_DLL_INJECT_MGR, "Enter. aRuleName=%ws , aCfgType=%d", aRuleName, aCfgType );

    BOOL bRet = FALSE;
    size_t uIndex = this->GetRuleIndexByName( aRuleName );
    if ( (size_t)-1 != uIndex )
    {
        bRet = m_vecDllInjectServer[uIndex]->GetConfig( aCfgType, pData, uDataSize );
    }

    DbgOut( VERB, DBG_DLL_INJECT_MGR, "Leave" );
    return bRet;
}

BOOL CDllInjectMgr::ChangeConfig( CONST WCHAR * aRuleName,
                                  DLL_INJECT_SERVER_CFG_TYPE aCfgType,
                                  IN VOID * pData,
                                  IN UINT uDataSize )
{
    DbgOut( VERB, DBG_DLL_INJECT_MGR, "Enter. aRuleName=%ws , aCfgType=%d", aRuleName, aCfgType );

    BOOL bRet = FALSE;
    size_t uIndex = this->GetRuleIndexByName( aRuleName );
    if ( (size_t)-1 != uIndex )
    {
        bRet = m_vecDllInjectServer[uIndex]->ChangeConfig( aCfgType, pData, uDataSize );
    }

    DbgOut( VERB, DBG_DLL_INJECT_MGR, "Leave" );
    return bRet;
}



BOOL CDllInjectMgr::GetFunctionAddresses()
{
    //Find the addresses of LoadLibraryW() and FreeLibrary() of x86 and x64 processes
    BOOL bRet = FALSE;
    do
    {
#ifdef _WIN64
        DbgOut( INFO, DBG_DLL_INJECT_MGR, "Get x64 address by GetProcAddress()" );
        m_uLoadLibraryW64 =
            (DWORD)GetProcAddress( GetModuleHandleW( KERNEL32_MODULE_NAME ), LOADLIBRARY_FUNCTION_NAMEA );
        m_uFreeLibrary64 =
            (DWORD)GetProcAddress( GetModuleHandleW( KERNEL32_MODULE_NAME ), FREELIBRARY_FUNCTION_NAMEA );
        DbgOut( INFO, DBG_DLL_INJECT_MGR, "m_uLoadLibraryW64=0x%0I64X, m_uFreeLibrary32=0x%0I64X", m_uLoadLibraryW64,
                m_uFreeLibrary64 );
        if ( NULL == m_uLoadLibraryW64 || NULL == m_uFreeLibrary64 )
        {
            DbgOut( ERRO, DBG_DLL_INJECT_MGR, "Failed to get x64 address" );
            break;
        }

        //Get x86's address of LoadLibraryW() and FreeLibrary() from DllInjectMgrHelper32
        wstring wstrModDir, wstrDllInjectMgrHelper;
        CWUtils::GetModuleDir( GetModuleHandleW( NULL ), wstrModDir );
        wstrDllInjectMgrHelper = wstrModDir + DLL_INJECT_MGR_HELPER32;
        DbgOut( INFO, DBG_DLL_INJECT_MGR, "Get x86 address by %ws", wstrModDir.c_str() );
        if ( FALSE == CWUtils::IsFileExist( wstrDllInjectMgrHelper.c_str() ) )
        {
            DbgOut( ERRO, DBG_DLL_INJECT_MGR, "Failed to find %ws", wstrDllInjectMgrHelper.c_str() );
            break;
        }

        WCHAR wzDllInjectMgrHelperLoadLibrary[MAX_PATH], wzDllInjectMgrHelperFreeLibrary[MAX_PATH];
        _snwprintf_s( wzDllInjectMgrHelperLoadLibrary, _TRUNCATE, L"\"%ws\" %ws", wstrDllInjectMgrHelper.c_str(),
                      LOADLIBRARY_FUNCTION_NAMEW );
        _snwprintf_s( wzDllInjectMgrHelperFreeLibrary, _TRUNCATE, L"\"%ws\" %ws", wstrDllInjectMgrHelper.c_str(),
                      FREELIBRARY_FUNCTION_NAMEW );
        if ( FALSE ==
             CWUtils::ExecuteCommandLine( wzDllInjectMgrHelperLoadLibrary, FALSE, (DWORD *)&m_uLoadLibraryW32 ) )
        {
            DbgOut( ERRO, DBG_DLL_INJECT_MGR, "Failed to get response for command \"%ws\"",
                    wzDllInjectMgrHelperLoadLibrary );
            break;
        }
        if ( FALSE ==
             CWUtils::ExecuteCommandLine( wzDllInjectMgrHelperFreeLibrary, FALSE, (DWORD *)&m_uFreeLibrary32 ) )
        {
            DbgOut( ERRO, DBG_DLL_INJECT_MGR, "Failed to get response for command \"%ws\"",
                    wzDllInjectMgrHelperFreeLibrary );
            break;
        }
#else
        DbgOut( INFO, DBG_DLL_INJECT_MGR, "Get x86 address by GetProcAddress()" );
        m_uLoadLibraryW32 =
            (DWORD)GetProcAddress( GetModuleHandleW( KERNEL32_MODULE_NAME ), LOADLIBRARY_FUNCTION_NAMEA );
        m_uFreeLibrary32 =
            (DWORD)GetProcAddress( GetModuleHandleW( KERNEL32_MODULE_NAME ), FREELIBRARY_FUNCTION_NAMEA );
#endif

        DbgOut( INFO, DBG_DLL_INJECT_MGR, "m_uLoadLibraryW32=0x%08X, m_uFreeLibrary32=0x%08X", m_uLoadLibraryW32,
                m_uFreeLibrary32 );
        if ( NULL == m_uLoadLibraryW32 || NULL == m_uFreeLibrary32 )
        {
            DbgOut( ERRO, DBG_DLL_INJECT_MGR, "Failed to get x86 address" );
            break;
        }

        bRet = TRUE;
    } while ( 0 );

    return bRet;
}

BOOL CDllInjectMgr::CreateActiveThread()
{
    //Create thread at the beginning since Init() may call StartHook() to inject DLL to remote process
    //Remote injected DLL will monitor m_hActiveThread to determine whether DllInjectMgr is crashed or not
    BOOL bRet = FALSE;

    do
    {
        UINT uActiveTid;
        if ( NULL == m_hActiveThreadQuitEvent )
        {
            m_hActiveThreadQuitEvent = CreateEventW( NULL, FALSE, FALSE, NULL );
            if ( NULL == m_hActiveThreadQuitEvent )
            {
                DbgOut( ERRO, DBG_DLL_INJECT_MGR,
                        "Failed to create m_hActiveThreadQuitEvent. GetLastError()=%!WINERROR!", GetLastError() );
                break;
            }
        }

        if ( NULL == m_hActiveThread )
        {
            m_hActiveThread = (HANDLE)_beginthreadex( NULL, 0, ActiveThread, m_hActiveThreadQuitEvent, 0, &uActiveTid );
            if ( NULL == m_hActiveThread )
            {
                DbgOut( ERRO, DBG_DLL_INJECT_MGR, "Failed to create m_hActiveThread. GetLastError()=%!WINERROR!",
                        GetLastError() );
                break;
            }
            DbgOut( INFO, DBG_DLL_INJECT_MGR,
                    "Active thread is created. m_hActiveThreadQuitEvent=0x%p, m_hActiveThread=0x%p, uActiveTid=0x%04X",
                    m_hActiveThreadQuitEvent, m_hActiveThread, uActiveTid );
        }

        bRet = TRUE;
    } while ( 0 );

    return bRet;
}

BOOL CDllInjectMgr::DestroyActiveThread()
{
    if ( m_hActiveThreadQuitEvent )
    {
        SetEvent( m_hActiveThreadQuitEvent );
        CloseHandle( m_hActiveThreadQuitEvent );
        m_hActiveThreadQuitEvent = NULL;
    }

    if ( m_hActiveThread )
    {
        WaitForSingleObject( m_hActiveThread, INFINITE );
        CloseHandle( m_hActiveThread );
        m_hActiveThread = NULL;
    }
    return TRUE;
}


BOOL CDllInjectMgr::CreateServers( CONST WCHAR * aCfgPath )
{
    BOOL bRet = FALSE;

    do
    {
        //Read configuration from aCfgPath
        list<wstring> lsSections;
        if ( FALSE == CWUtils::GetIniSectionNames( aCfgPath, lsSections ) || 0 == lsSections.size() )
        {
            DbgOut( ERRO, DBG_DLL_INJECT_MGR, "Config not found or empty. aCfgPath=%ws", aCfgPath );
            break;
        }

        //Each rule has one corresponding server
        size_t uRuleIndex = 0;
        for ( list<wstring>::iterator it = lsSections.begin(); it != lsSections.end(); it++ )
        {
            CDllInjectServer * pCtrl = new ( std::nothrow )
                CDllInjectServer( m_hActiveThread, aCfgPath, uRuleIndex, it->c_str(), m_uLoadLibraryW32,
                                  m_uFreeLibrary32, m_uLoadLibraryW64, m_uFreeLibrary64 );
            if ( NULL != pCtrl )
            {
                m_vecDllInjectServer.push_back( pCtrl );
                uRuleIndex++;
            }
            else
            {
                DbgOut( ERRO, DBG_DLL_INJECT_MGR, "pCtrl is NULL" );
            }
        }

        bRet = TRUE;
    } while ( 0 );

    return bRet;
}

BOOL CDllInjectMgr::DestroyServers()
{
    for ( size_t i = 0; i < m_vecDllInjectServer.size(); i++ )
    {
        if ( NULL != m_vecDllInjectServer[i] )
        {
            m_vecDllInjectServer[i]->StopMonitor();
            delete m_vecDllInjectServer[i];
            m_vecDllInjectServer[i] = NULL;
        }
    }
    return TRUE;
}

size_t CDllInjectMgr::GetRuleIndexByName( CONST WCHAR * aRuleName )
{
    if ( NULL != aRuleName )
    {
        for ( size_t i = 0; i < m_vecDllInjectServer.size(); i++ )
        {
            CONST WCHAR * wzRuleName = m_vecDllInjectServer[i]->GetRuleName();
            if ( 0 == _wcsicmp( wzRuleName, aRuleName ) )
            {
                return i;
            }
        }
    }
    return (size_t)-1;
}


#ifdef __cplusplus
}
#endif

}    //End of namespace CWUtils