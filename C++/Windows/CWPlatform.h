#pragma once

/*
 * Copyright (c) 2009-2020, ChienWei Hung <winestwinest@gmail.com>
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
#pragma warning( pop )

namespace CWUtils
{
#ifdef __cplusplus
extern "C" {
#endif

#if defined( _WIN32 ) || defined( _WIN64 )
    #define PLATFORM_WINDOWS
#elif defined( __APPLE__ ) || defined( __MACH__ )
    #define PLATFORM_MAC_OS
#elif define ( __ANDROID__ )
    #define PLATFORM_ANDROID
#else
    #define PLATFORM_LINUX
#endif

typedef enum _OS_ID
{
    OS_WIN_WINDOWS = 1,
    OS_WIN_95,
    OS_WIN_98,
    OS_WIN_ME,

    OS_WIN_NT,
    OS_WIN_NT35,
    OS_WIN_NT4,
    OS_WIN_2000,
    OS_WIN_XP,
    OS_WIN_XP_SP2PLUS,
    OS_WIN_2003,
    OS_WIN_BEFORE_VISTA,
    OS_WIN_VISTA,
    OS_WIN_7,
    OS_WIN_8,
    OS_WIN_UNKNOWN_NEW
} OS_ID;

BOOL PlatformCheck( OS_ID aOsId );

BOOL PlatformCheckRange( OS_ID aMinOsId, OS_ID aMaxOsId );

#ifdef __cplusplus
}
#endif

}    //End of namespace CWUtils
