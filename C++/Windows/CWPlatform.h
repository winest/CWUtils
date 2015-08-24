#pragma once

#include <Windows.h>

namespace CWUtils
{

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _OS_ID
{
    OS_WIN_WINDOWS = 1,
    OS_WIN_95 ,
    OS_WIN_98 ,
    OS_WIN_ME ,

    OS_WIN_NT ,
    OS_WIN_NT35 ,
    OS_WIN_NT4 ,
    OS_WIN_2000 ,
    OS_WIN_XP ,
    OS_WIN_XP_SP2PLUS ,
    OS_WIN_2003 ,
    OS_WIN_BEFORE_VISTA ,
    OS_WIN_VISTA ,
    OS_WIN_7 ,
    OS_WIN_8 ,
    OS_WIN_UNKNOWN_NEW
} OS_ID;

BOOL PlatformCheck( OS_ID aOsId );

BOOL PlatformCheckRange( OS_ID aMinOsId , OS_ID aMaxOsId );

#ifdef __cplusplus
}
#endif

}   //End of namespace CWUtils
