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
#include <AccCtrl.h>
#pragma warning( pop )

namespace CWUtils
{
#ifdef __cplusplus
extern "C" {
#endif

BOOL IsWin32Process( HANDLE aProcess );

BOOL ExecuteCommandLine( CONST WCHAR * aCmd, OPTIONAL IN BOOL aDisplay = FALSE, OPTIONAL OUT DWORD * aExitCode = NULL );

//Enable the aPrivilege in current process's access token, all privilege constants can be found in MSDN
//Some powerful privileges are SE_BACKUP_NAME, SE_DEBUG_NAME, SE_INCREASE_QUOTA_NAME, and SE_TCB_NAME
BOOL AdjustSelfPrivilege( CONST WCHAR * aPrivilege, BOOL aEnable );

BOOL ChangeDaclPermission( CONST WCHAR * aObjName,
                           SE_OBJECT_TYPE aObjType,
                           DWORD aPermission = GENERIC_ALL,
                           ACCESS_MODE aMode = GRANT_ACCESS,
                           DWORD aInheritance = NO_INHERITANCE,
                           CONST WCHAR * aTrustee = NULL,
                           TRUSTEE_FORM aTrusteeForm = TRUSTEE_IS_NAME,
                           TRUSTEE_TYPE aTrusteeType = TRUSTEE_IS_USER );

HANDLE WINAPI TryCreateRemoteThread( IN HANDLE aProcess,
                                     OPTIONAL IN LPSECURITY_ATTRIBUTES aThreadAttributes,
                                     IN SIZE_T aStackSize,
                                     IN LPTHREAD_START_ROUTINE aStartAddress,
                                     OPTIONAL IN LPVOID aParameter,
                                     IN DWORD aCreationFlags,
                                     OPTIONAL OUT LPDWORD aThreadId );

#ifdef __cplusplus
}
#endif

}    //End of namespace CWUtils
