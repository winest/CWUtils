#include "stdafx.h"
#include "CWPlatform.h"


namespace CWUtils
{
#ifdef __cplusplus
extern "C" {
#endif

BOOL PlatformCheck( OS_ID aOsId )
{
    static OSVERSIONINFOEXW osvi = { 0 };
    ZeroMemory( &osvi, sizeof( OSVERSIONINFOEXW ) );

    if ( osvi.dwOSVersionInfoSize == 0 )
    {
        osvi.dwOSVersionInfoSize = sizeof( osvi );
        if ( !::GetVersionExW( (OSVERSIONINFOW *)&osvi ) )
        {
            osvi.dwOSVersionInfoSize = sizeof( OSVERSIONINFOW );
            if ( !GetVersionExW( (OSVERSIONINFOW *)&osvi ) )
                return FALSE;
        }
    }

    //DbgOut(INFO,DBG_SENTRY,"OS Ver : [dwPlatformId=%d],[dwMajorVersion=%d],[dwMinorVersion=%d]",osvi.dwPlatformId,osvi.dwMajorVersion,osvi.dwMinorVersion);

    switch ( aOsId )
    {
        case OS_WIN_WINDOWS:
            return osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS;

        case OS_WIN_NT:
            return osvi.dwPlatformId == VER_PLATFORM_WIN32_NT;

        case OS_WIN_NT35:
            return ( osvi.dwPlatformId == VER_PLATFORM_WIN32_NT && osvi.dwMajorVersion == 3 );

        case OS_WIN_95:
            return ( osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS &&
                     ( osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0 ) );

        case OS_WIN_NT4:
            return ( osvi.dwPlatformId == VER_PLATFORM_WIN32_NT && osvi.dwMajorVersion == 4 );

        case OS_WIN_98:
            return ( osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS &&
                     ( osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10 ) );

        case OS_WIN_2000:
            return ( osvi.dwPlatformId == VER_PLATFORM_WIN32_NT &&
                     ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 ) );

        case OS_WIN_ME:
            return ( osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS &&
                     ( osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 90 ) );

        case OS_WIN_XP:
            return ( osvi.dwPlatformId == VER_PLATFORM_WIN32_NT &&
                     ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1 ) );

        case OS_WIN_XP_SP2PLUS:
            return ( osvi.dwPlatformId == VER_PLATFORM_WIN32_NT &&
                     ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1 && osvi.wServicePackMajor >= 2 ) );

        case OS_WIN_2003:
            return ( osvi.dwPlatformId == VER_PLATFORM_WIN32_NT &&
                     ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2 ) );

        case OS_WIN_BEFORE_VISTA:
            return ( osvi.dwPlatformId == VER_PLATFORM_WIN32_NT && osvi.dwMajorVersion < 6 );

        case OS_WIN_VISTA:
            return ( osvi.dwPlatformId == VER_PLATFORM_WIN32_NT &&
                     ( osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0 ) );

        case OS_WIN_7:
            return ( osvi.dwPlatformId == VER_PLATFORM_WIN32_NT &&
                     ( osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 1 ) );

        case OS_WIN_8:
            return ( osvi.dwPlatformId == VER_PLATFORM_WIN32_NT &&
                     ( osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 2 ) );

        case OS_WIN_UNKNOWN_NEW:
            return ( osvi.dwPlatformId == VER_PLATFORM_WIN32_NT &&
                     ( osvi.dwMajorVersion >= 6 && osvi.dwMinorVersion > 2 ) );
        default:
            //DbgOut(ERRO,DBG_SENTRY,"Wrong aOsId=%d",aOsId);
            break;
    }

    return false;
}

BOOL PlatformCheckRange( OS_ID aMinOsId, OS_ID aMaxOsId )
{
    INT i;
    if ( aMinOsId > aMaxOsId )
    {
        //DbgOut(ERRO,DBG_SENTRY,"Wrong input range [%d,%d]",aMinOsId,aMaxOsId);
        return FALSE;
    }

    for ( i = aMinOsId; i <= aMaxOsId; i++ )
    {
        if ( PlatformCheck( (OS_ID)i ) )
        {
            //DbgOut(INFO,DBG_SENTRY,"Win version matched [%d]",i);
            return TRUE;
        }
    }
    return FALSE;
}

#ifdef __cplusplus
}
#endif

}    //End of namespace CWUtils