#include "stdafx.h"
#include "CWTime.h"
using std::string;

namespace CWUtils
{

#ifdef __cplusplus
extern "C" {
#endif

BOOL CDECL _FormatStringA( OUT string & aOutString , IN CONST CHAR * aFormat , ... )
{
    BOOL bRet = FALSE;
    aOutString.clear();
    CHAR szBuf[4096];
    CHAR * pBuf = szBuf;

    if ( NULL != aFormat )
    {
        va_list args;
        va_start( args , aFormat );
        SIZE_T len = vsnprintf( NULL , 0 , aFormat , args ) + 1;    //Get formatted string length and adding one for null-terminator

        if ( _countof(szBuf) < len )
        {
            pBuf = new (std::nothrow) CHAR[len];
        }
        if ( NULL != pBuf )
        {
            if ( 0 < vsnprintf( pBuf , len , aFormat , args ) )
            {
                aOutString = pBuf;
            }

            if ( pBuf != szBuf )
            {
                delete [] pBuf;
            }
            bRet = TRUE;
        }

        va_end( args );
    }
    else
    {
        errno = EINVAL;
    }

    return bRet; 
}

VOID FormatTime( UINT64 aMilli , string & aTimeString )
{
    UINT uMilli = (UINT)( aMilli % 1000 );
    aMilli /= 1000;

    UINT uHour = (UINT)( aMilli / 60 / 60 );
    aMilli -= uHour * ( 60 * 60 );

    UINT uMin = (UINT)( aMilli / 60 );
    aMilli -= uMin * 60;

    UINT uSec = (UINT)( aMilli % 60 );
    _FormatStringA( aTimeString , "%02u:%02u:%02u.%03u" , uHour , uMin , uSec , uMilli );
}

BOOL GetCurrTimeStringA( std::string & aTimeString , CONST CHAR * aTimeFormat )
{
    time_t t;
    struct tm * pTimeInfo;
    CHAR szBuf[4096];

    time( &t );
    pTimeInfo = localtime( &t );
    size_t uRet = strftime( szBuf , _countof(szBuf) , aTimeFormat , pTimeInfo );
    if ( 0 < uRet )
    {
        aTimeString.assign( szBuf , uRet );
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

BOOL GetCurrTimeStringW( std::wstring & aTimeString , CONST WCHAR * aTimeFormat )
{
    time_t t;
    struct tm * pTimeInfo;
    WCHAR wzBuf[4096];

    time( &t );
    pTimeInfo = localtime( &t );
    size_t uRet = wcsftime( wzBuf , _countof(wzBuf) , aTimeFormat , pTimeInfo );
    if ( 0 < uRet )
    {
        aTimeString.assign( wzBuf , uRet );
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

VOID DiffTime( IN const struct timespec & aStart , IN const struct timespec & aEnd , OUT struct timespec & aDiff )
{
    if ( (aEnd.tv_nsec - aStart.tv_nsec) < 0 )
    {
        aDiff.tv_sec = aEnd.tv_sec - aStart.tv_sec - 1;
        aDiff.tv_nsec = aEnd.tv_nsec - aStart.tv_nsec + 1000000000;
    } else
    {
        aDiff.tv_sec = aEnd.tv_sec - aStart.tv_sec;
        aDiff.tv_nsec = aEnd.tv_nsec - aStart.tv_nsec;
    }
}

#ifdef __cplusplus
}
#endif

}   //End of namespace CWUtils