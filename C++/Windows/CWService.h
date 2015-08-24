#pragma once

#include <Windows.h>
#include <string>
using std::wstring;

namespace CWUtils
{

#ifdef __cplusplus
extern "C" {
#endif

BOOL IsScmLocked( UINT aRetryTimes , DWORD aRetryWaitTime );
SC_LOCK GetScmLock( UINT aRetryTimes , DWORD aRetryWaitTime );
BOOL ReleaseScmLock( SC_LOCK aScmLock );

BOOL IsServiceAccessible( CONST WCHAR * aServiceName , DWORD aDesiredAccess );
BOOL GetServiceBinaryPath( CONST WCHAR * aServiceName , wstring & aFullPath );

BOOL InstallService( CONST WCHAR * aServicePath , CONST WCHAR * aServiceName , DWORD aServiceType , DWORD aStartType );
BOOL UninstallService( CONST WCHAR * aServiceName );

BOOL GetServiceCurrentState( IN CONST WCHAR * aServiceName , OUT DWORD * aCurrentState );
BOOL StartServiceByName( CONST WCHAR * aServiceName , DWORD aTimeout = 30 * 1000 ); //aTimeout's unit is milli-second
BOOL StopServiceByName( CONST WCHAR * aServiceName , DWORD aTimeout = 30 * 1000 );  //aTimeout's unit is milli-second

BOOL ChangeServiceStartType( CONST WCHAR * aServiceName , DWORD aStartType , DWORD * aLastStartType = NULL );




#ifdef __cplusplus
}
#endif

}   //End of namespace CWUtils
