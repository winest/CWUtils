#include "CWPlatform.h"
#include <ntstrsafe.h>

#ifndef CW_MEM_TAG_UTILS
#    define CW_MEM_TAG_UTILS 'tUWC'
#endif

namespace KmUtils
{
#ifdef __cplusplus
extern "C" {
#endif

BOOLEAN PlatformCheck( OS_ID aOsId )
{
    RTL_OSVERSIONINFOW osvi;
    RtlZeroMemory( &osvi, sizeof( RTL_OSVERSIONINFOW ) );
    osvi.dwOSVersionInfoSize = sizeof( RTL_OSVERSIONINFOW );
    RtlGetVersion( &osvi );

    //DbgOut( INFO , DBG_UTILS , "dwPlatformId=%d, dwMajorVersion=%d, dwMinorVersion=%d" , osvi.dwPlatformId , osvi.dwMajorVersion , osvi.dwMinorVersion );

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
            break;
    }

    return FALSE;
}

BOOLEAN PlatformCheckRange( OS_ID aMinOsId, OS_ID aMaxOsId )
{
    INT i;
    if ( aMinOsId > aMaxOsId )
    {
        return FALSE;
    }

    for ( i = aMinOsId; i <= aMaxOsId; i++ )
    {
        if ( PlatformCheck( (OS_ID)i ) )
        {
            return TRUE;
        }
    }
    return FALSE;
}

#ifdef __cplusplus
}
#endif

}    //End of namespace KmUtils
