#include "stdafx.h"
#include "CWProcess.h"
#pragma warning( disable : 4127 )

#include <winternl.h>
#include <new>
#include <Psapi.h>
#include <AclAPI.h>

//#include "CWGeneralUtils.h""

#pragma comment( lib , "Psapi.lib" )

namespace CWUtils
{

#ifdef __cplusplus
    extern "C" {
#endif

typedef NTSTATUS (WINAPI * PFN_NTCREATETHREADEX)( OUT PHANDLE hThread , IN ACCESS_MASK DesiredAccess , IN LPVOID ObjectAttributes , IN HANDLE ProcessHandle ,
                                                  IN LPTHREAD_START_ROUTINE lpStartAddress , IN LPVOID lpParameter , IN BOOL CreateSuspended ,
                                                  IN SIZE_T StackZeroBits , IN SIZE_T SizeOfStackCommit , IN SIZE_T SizeOfStackReserve , OUT LPVOID lpBytesBuffer );


BOOL IsWin32Process( HANDLE aProcess )
{
    #ifdef _WIN64
        BOOL bRet = FALSE;
        if ( IsWow64Process( aProcess , &bRet ) )
        {
            return bRet;
        }
        return FALSE;
    #else
        UNREFERENCED_PARAMETER( aProcess );
        return TRUE;
    #endif
}

BOOL ExecuteCommandLine( CONST WCHAR * aCmd , OPTIONAL IN BOOL aDisplay , OPTIONAL OUT DWORD * aExitCode )
{
    BOOL bRet = FALSE;
    DWORD dwCreationFlags = CREATE_UNICODE_ENVIRONMENT;    //CREATE_NEW_CONSOLE, DETACHED_PROCESS, CREATE_NO_WINDOW
    dwCreationFlags |= ( aDisplay ) ? CREATE_NEW_CONSOLE : CREATE_NO_WINDOW;

    STARTUPINFOW stStartupInfo = { 0 };
    ZeroMemory( &stStartupInfo , sizeof(STARTUPINFO) );
    stStartupInfo.cb = sizeof(STARTUPINFO);
    stStartupInfo.dwFlags = STARTF_USESHOWWINDOW;
    stStartupInfo.wShowWindow = ( aDisplay ) ? SW_SHOW : SW_HIDE;

    PROCESS_INFORMATION stProcInfo;
    ZeroMemory( &stProcInfo , sizeof(PROCESS_INFORMATION) );

    DWORD dwRtnState = 0 , dwTimeOut = INFINITE , dwExitCode = 0;
    WCHAR * wzCmd = NULL;

    if ( aCmd )
    {
        //wzCmd is needed because CreateProcessW() may modify the content of wzCmd
        size_t uCmdLen = wcslen( aCmd ) + 1;
        wzCmd = new (std::nothrow) WCHAR[uCmdLen];
        if ( NULL == wzCmd )
        {
            goto exit;
        }

        wcsncpy_s( wzCmd , uCmdLen , aCmd , _TRUNCATE );
        if ( ! CreateProcessW( NULL , wzCmd , NULL , NULL , FALSE , dwCreationFlags , NULL , NULL , &stStartupInfo , &stProcInfo ) )
        {
            goto exit;
        }

        dwRtnState = WaitForSingleObject( stProcInfo.hProcess , dwTimeOut );

        switch ( dwRtnState )
        {
            case WAIT_OBJECT_0 :
                if ( GetExitCodeProcess( stProcInfo.hProcess , &dwExitCode ) )
                {
                    bRet = TRUE;
                }
                break;

            case WAIT_TIMEOUT :
                break;

            default :
                break;
        }

        if ( stProcInfo.hThread )
            CloseHandle( stProcInfo.hThread );

        if ( stProcInfo.hProcess )
            CloseHandle( stProcInfo.hProcess );
    }

exit:
    if ( wzCmd )
    {
        delete [] wzCmd;
    }
    if ( aExitCode )
    {
        *aExitCode = dwExitCode;
    }

    return bRet;
}



BOOL AdjustSelfPrivilege( CONST WCHAR * aPrivilege , BOOL aEnable )
{
    BOOL bRet = FALSE;
    HANDLE hToken = INVALID_HANDLE_VALUE;

    do
    {
        //Get handle to current process's access token
        if ( ! OpenProcessToken( GetCurrentProcess() , TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES , &hToken ) )
        {
            //ShowDebugMsg( L"OpenProcessToken() failed" );
            break;
        }

        CHAR tpOld[sizeof(TOKEN_PRIVILEGES) + sizeof(LUID_AND_ATTRIBUTES) * 32] = {};
        TOKEN_PRIVILEGES tpNew = {};
        DWORD dwTpSize;

        //Find the Luid(Local unique identifier) corresponding to aPrivilege, and set it to tpNew.Privilegesp[0].Luid
        if ( ! LookupPrivilegeValue( NULL , aPrivilege , &tpNew.Privileges[0].Luid ) )
        {
            //ShowDebugMsg( L"LookupPrivilegeValue() failed" );
            break;
        }
        tpNew.PrivilegeCount = 1;

        //Set privilege according to current privilege settings
        if ( GetTokenInformation( hToken , TokenPrivileges , &tpOld , sizeof(tpOld) , &dwTpSize ) )
        {
            TOKEN_PRIVILEGES * pOld = (TOKEN_PRIVILEGES *)tpOld;
            for ( DWORD i = 0 ; i < pOld->PrivilegeCount ; i++ )
            {
                if ( pOld->Privileges[i].Luid.HighPart == tpNew.Privileges[0].Luid.HighPart &&
                     pOld->Privileges[i].Luid.LowPart == tpNew.Privileges[0].Luid.LowPart)
                {
                    tpNew.Privileges[0].Attributes = ( aEnable ) ? ( pOld->Privileges[i].Attributes | SE_PRIVILEGE_ENABLED ) :
                                                                   ( pOld->Privileges[i].Attributes ^ ( SE_PRIVILEGE_ENABLED & pOld->Privileges[i].Attributes ) );
                    break;
                }
            }
        }
        else if ( ERROR_INSUFFICIENT_BUFFER == GetLastError() )
        {
            TOKEN_PRIVILEGES * pOld = (TOKEN_PRIVILEGES *)new (std::nothrow) CHAR[dwTpSize];
            if ( NULL == pOld )
            {
                //ShowDebugMsg( L"Failed to allocate memory" );
                break;
            }
            ZeroMemory( pOld , dwTpSize );
            if ( ! GetTokenInformation( hToken , TokenPrivileges , pOld , dwTpSize , &dwTpSize ) )
            {
                //ShowDebugMsg( L"GetTokenInformation() failed to get current privilege" );
                break;
            }
            for ( DWORD i = 0 ; i < pOld->PrivilegeCount ; i++ )
            {
                if ( pOld->Privileges[i].Luid.HighPart == tpNew.Privileges[0].Luid.HighPart &&
                     pOld->Privileges[i].Luid.LowPart == tpNew.Privileges[0].Luid.LowPart)
                {
                    tpNew.Privileges[0].Attributes = ( aEnable ) ? ( pOld->Privileges[i].Attributes | SE_PRIVILEGE_ENABLED ) :
                                                                   ( pOld->Privileges[i].Attributes ^ ( SE_PRIVILEGE_ENABLED & pOld->Privileges[i].Attributes ) );
                    break;
                }
            }
            delete [] pOld;
        }
        else
        {
            //ShowDebugMsg( L"GetTokenInformation() failed to get current privilege" );
            break;
        }

        //Change the privilege settings now
        if ( ! AdjustTokenPrivileges( hToken , FALSE , &tpNew , 0 , NULL , NULL ) )
        {
            //WriteDebugMsg( "%hs_%lu AdjustTokenPrivileges() failed. GetLastError()=%lu" , __FILE__ , __LINE__ , GetLastError() );
            break;
        }
        else if ( ERROR_NOT_ALL_ASSIGNED == GetLastError() )
        {
            //WriteDebugMsg( "%hs_%lu AdjustTokenPrivileges() only change partial privilege" , __FILE__ , __LINE__ );
            break;
        }
        else {}

        //WriteDebugMsg( "%hs_%lu AdjustTokenPrivileges() succeed" , __FILE__ , __LINE__ );
        bRet = TRUE;
    } while ( 0 );

    if ( INVALID_HANDLE_VALUE != hToken )
    {
        CloseHandle( hToken );
    }

    return bRet;
}

BOOL ChangeDaclPermission( CONST WCHAR * aObjName , SE_OBJECT_TYPE aObjType , 
                           DWORD aPermission , ACCESS_MODE aMode , DWORD aInheritance ,
                           CONST WCHAR * aTrustee , TRUSTEE_FORM aTrusteeForm , TRUSTEE_TYPE aTrusteeType ) 
{
    BOOL bRet = FALSE;
    PACL pOldDacl = NULL , pNewDacl = NULL;

    do
    {
        if ( NULL == aObjName ) 
        {
            SetLastError( ERROR_INVALID_PARAMETER );
            break;
        }

        //Get the existing DACL
        DWORD dwRet = GetNamedSecurityInfoW( aObjName , aObjType , DACL_SECURITY_INFORMATION , NULL , NULL , &pOldDacl , NULL , NULL );
        if ( ERROR_SUCCESS != dwRet )
        {
            SetLastError( dwRet );
            break;
        }  

        //Initialize an EXPLICIT_ACCESS structure for the new ACE
        EXPLICIT_ACCESSW ea = {};
        ea.grfAccessPermissions = aPermission;
        ea.grfAccessMode = aMode;
        ea.grfInheritance= aInheritance;
        ea.Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
        ea.Trustee.TrusteeForm = aTrusteeForm;
        ea.Trustee.TrusteeType = aTrusteeType;
        ea.Trustee.ptstrName = const_cast<WCHAR *>(aTrustee);

        //Create a new ACL that merges the new ACE into the existing DACL
        dwRet = SetEntriesInAclW( 1 , &ea , pOldDacl , &pNewDacl );
        if ( ERROR_SUCCESS != dwRet )
        {
            SetLastError( dwRet );
            break;
        }  

        //Attach the new ACL as the object's DACL
        dwRet = SetNamedSecurityInfoW( const_cast<WCHAR *>(aObjName) , aObjType , DACL_SECURITY_INFORMATION , NULL , NULL , pNewDacl , NULL );
        if ( ERROR_SUCCESS != dwRet )
        {
            SetLastError( dwRet );
            break;
        }

        bRet = TRUE;
    } while ( 0 );
    
    if ( NULL != pNewDacl )
    {
        LocalFree((HLOCAL) pNewDacl); 
    }
    return bRet;
}




HANDLE WINAPI TryCreateRemoteThread( IN HANDLE aProcess , OPTIONAL IN LPSECURITY_ATTRIBUTES aThreadAttributes , 
                                     IN SIZE_T aStackSize , IN LPTHREAD_START_ROUTINE aStartAddress , OPTIONAL IN LPVOID aParameter ,
                                     IN DWORD aCreationFlags , OPTIONAL OUT LPDWORD aThreadId )
{
    //Create remote thread to load our DLL
    if ( FALSE == CWUtils::AdjustSelfPrivilege( SE_DEBUG_NAME , TRUE ) )
    {
        //DbgOut( ERRO , DBG_UTILS , "AdjustSelfPrivilege() failed. GetLastError()=%!WINERROR!" , GetLastError() );
        //Don't break and try to create thread
    }

    HANDLE hRemoteThread = CreateRemoteThread( aProcess , aThreadAttributes , aStackSize , aStartAddress , aParameter , aCreationFlags , aThreadId );
    if ( NULL == hRemoteThread )
    {
        //DbgOut( WARN , DBG_UTILS , "CreateRemoteThread() failed, trying NtCreateThreadEx(). hProcess=0x%p, GetLastError()=%!WINERROR!" , aProcess , GetLastError() );
        
        do 
        {
            PFN_NTCREATETHREADEX NtCreateThreadEx = (PFN_NTCREATETHREADEX)GetProcAddress( GetModuleHandleW(L"ntdll.dll") , "NtCreateThreadEx" );
            if ( NULL == NtCreateThreadEx )
            {  
                //DbgOut( ERRO , DBG_UTILS , "Failed to get address of NtCreateThreadEx. GetLastError()=%!WINERROR!" , GetLastError() );
                break;
            }

            // 3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
            // 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
            //+---------------+---------------+-------------------------------+
            //|G|G|G|G|Res'd|A| StandardRights|         SpecificRights        |
            //|R|W|E|A|     |S|               |                               |
            //+-+-------------+---------------+-------------------------------+
            ACCESS_MASK mask = STANDARD_RIGHTS_ALL | SPECIFIC_RIGHTS_ALL;

            NTSTATUS ntStatus = NtCreateThreadEx( &hRemoteThread , mask , NULL , aProcess , aStartAddress , aParameter , FALSE , 0 , 0 , 0 , NULL );
            if ( ! NT_SUCCESS(ntStatus) )
            {
                //DbgOut( ERRO , DBG_UTILS , "NtCreateThreadEx() failed. ntStatus=%!STATUS!, GetLastError()=%!WINERROR!" , ntStatus , GetLastError() );
                break;
            }
        } while ( 0 );
    }

    return hRemoteThread;
}

#ifdef __cplusplus
    }
#endif

}   //End of namespace CWUtils