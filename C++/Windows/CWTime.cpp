#include "stdafx.h"
#include "CWTime.h"
using namespace std;

namespace CWUtils
{
#ifdef __cplusplus
extern "C" {
#endif

BOOL CDECL _FormatStringW( OUT wstring & aOutString, IN CONST WCHAR * aFormat, ... )
{
    BOOL bRet = FALSE;
    aOutString.clear();

    if ( NULL != aFormat )
    {
        va_list args = NULL;
        va_start( args, aFormat );
        SIZE_T len =
            _vscwprintf( aFormat, args ) + 1;    //Get formatted string length and adding one for null-terminator

        WCHAR * wzBuf = new ( std::nothrow ) WCHAR[len];
        if ( NULL != wzBuf )
        {
            if ( 0 < _vsnwprintf_s( wzBuf, len, _TRUNCATE, aFormat, args ) )
                aOutString = wzBuf;

            delete[] wzBuf;
            bRet = TRUE;
        }

        va_end( args );
    }
    else
    {
        SetLastError( ERROR_INVALID_PARAMETER );
    }

    return bRet;
}

VOID FormatTime( UINT64 aMilli, wstring & aTimeString )
{
    UINT uMilli = ( UINT )( aMilli % 1000 );
    aMilli /= 1000;

    UINT uHour = ( UINT )( aMilli / 60 / 60 );
    aMilli -= uHour * ( 60 * 60 );

    UINT uMin = ( UINT )( aMilli / 60 );
    aMilli -= uMin * 60;

    UINT uSec = ( UINT )( aMilli % 60 );
    _FormatStringW( aTimeString, L"%02u:%02u:%02u.%03u", uHour, uMin, uSec, uMilli );
}

BOOL GetCurrTimeStringA( std::string & aTimeString, CONST CHAR * aTimeFormat )
{
    time_t t;
    struct tm info;
    CHAR szBuf[4096];

    time( &t );
    localtime_s( &info, &t );
    size_t uRet = strftime( szBuf, _countof( szBuf ), aTimeFormat, &info );
    if ( 0 < uRet )
    {
        aTimeString.assign( szBuf, uRet );
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

BOOL GetCurrTimeStringW( std::wstring & aTimeString, CONST WCHAR * aTimeFormat )
{
    time_t t;
    struct tm info;
    WCHAR wzBuf[4096];

    time( &t );
    localtime_s( &info, &t );
    size_t uRet = wcsftime( wzBuf, _countof( wzBuf ), aTimeFormat, &info );
    if ( 0 < uRet )
    {
        aTimeString.assign( wzBuf, uRet );
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


#ifdef __cplusplus
}
#endif

}    //End of namespace CWUtils