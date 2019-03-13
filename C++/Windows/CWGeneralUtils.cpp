#include "stdafx.h"
#include "CWGeneralUtils.h"
#include <string>
using std::wstring;

namespace CWUtils
{
BOOL _GetLastErrorStringW( IN OUT std::wstring & aString, WORD aLang = MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US ) )
{
    BOOL bRet = FALSE;
    WCHAR * wzMsgBuf;
    size_t sizeLen = (size_t)FormatMessageW( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL,
                                             GetLastError(), aLang, (WCHAR *)&wzMsgBuf, 0, NULL );
    if ( 0 < sizeLen )
    {
        //Remove \r\n at the end of the message
        CONST size_t sizeCRLF = _countof( L"\r\n" ) - 1;
        if ( sizeCRLF <= sizeLen && 0 == wcsncmp( &wzMsgBuf[sizeLen - sizeCRLF], L"\r\n", sizeCRLF ) )
        {
            sizeLen -= sizeCRLF;
        }

        aString.assign( wzMsgBuf, sizeLen );
        LocalFree( wzMsgBuf );    //Free the buffer
        bRet = TRUE;
    }
    return bRet;
}

BOOL _SaveToFile( IN CONST WCHAR * aSavePath, BOOL aAppend, IN CONST BYTE * aData, IN DWORD aDataSize )
{
    wstring wstrSavePath = aSavePath;
    DWORD dwOption = ( aAppend ) ? OPEN_ALWAYS : CREATE_ALWAYS;
    DWORD dwWrite = 0, dwWritten = 0;
    HANDLE hFile = CreateFileW( aSavePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, dwOption,
                                FILE_ATTRIBUTE_NORMAL, NULL );
    if ( INVALID_HANDLE_VALUE != hFile )
    {
        if ( aAppend && INVALID_SET_FILE_POINTER == SetFilePointer( hFile, 0, NULL, FILE_END ) )
        {
            CloseHandle( hFile );
            return FALSE;
        }
        for ( dwWritten = 0; dwWritten < aDataSize; dwWritten += dwWrite )
        {
            if ( FALSE == WriteFile( hFile, aData, aDataSize - dwWritten, &dwWrite, NULL ) || 0 == dwWrite )
                break;
        }
        CloseHandle( hFile );
    }
    return ( dwWritten == aDataSize );
}

BOOL _SaveToFileEx( IN CONST WCHAR * aSavePath, BOOL aAppend, IN CONST CHAR * aFormat, ... )
{
    BOOL bRet = FALSE;

    if ( NULL != aFormat )
    {
        va_list args = NULL;
        va_start( args, aFormat );
        size_t len =
            _vscprintf( aFormat, args ) + 1;    //Get formatted string length and adding one for null-terminator

        CHAR * szBuf = new ( std::nothrow ) CHAR[len];
        if ( NULL != szBuf )
        {
            if ( 0 < _vsnprintf_s( szBuf, len, _TRUNCATE, aFormat, args ) )
            {
                bRet = _SaveToFile( aSavePath, aAppend, (CONST BYTE *)szBuf, ( DWORD )( len - 1 ) );
            }
            delete[] szBuf;
        }
        va_end( args );
    }
    else
    {
        SetLastError( ERROR_INVALID_PARAMETER );
    }

    return bRet;
}

VOID ShowDebugMsg( CONST WCHAR * aReason, BOOL aShowInRelease )
{
    WORD wLang = 0;
#ifdef _DEBUG
    aShowInRelease = TRUE;
    wLang = MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US );    //English
#else
    wLang = MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT );    //Local language
#endif
    if ( TRUE == aShowInRelease )
    {
        wstring wstrLastErr;
        _GetLastErrorStringW( wstrLastErr, wLang );
        MessageBoxW( NULL, wstrLastErr.c_str(), aReason, MB_OK | MB_ICONINFORMATION );    //Display the string
    }
}


VOID WriteDebugMsg( CONST IN CHAR * aFormat, ... )
{
    DWORD dwGetLastError = GetLastError();

    if ( NULL != aFormat )
    {
        va_list args = NULL;
        va_start( args, aFormat );
        size_t len =
            _vscprintf( aFormat, args ) + 1;    //Get formatted string length and adding one for null-terminator

        CHAR * szBuf = new ( std::nothrow ) CHAR[len];
        if ( NULL != szBuf )
        {
            if ( 0 < _vsnprintf_s( szBuf, len, _TRUNCATE, aFormat, args ) )
            {
                _SaveToFileEx( DBG_LOG_PATH, TRUE, "[%04X][%04X] ", GetCurrentProcessId(), GetCurrentThreadId() );
                _SaveToFile( DBG_LOG_PATH, TRUE, (CONST BYTE *)szBuf, ( DWORD )( len - 1 ) );
                if ( ERROR_SUCCESS == dwGetLastError )
                {
                    _SaveToFileEx( DBG_LOG_PATH, TRUE, "\r\n" );
                }
                else
                {
                    wstring wstrLastErr;
                    _GetLastErrorStringW( wstrLastErr );
                    _SaveToFileEx( DBG_LOG_PATH, TRUE, ". GetLastError()=%lu (%ws)\r\n", dwGetLastError,
                                   wstrLastErr.c_str() );
                }
            }
            delete[] szBuf;
        }
        va_end( args );
    }
    else
    {
        SetLastError( ERROR_INVALID_PARAMETER );
    }
}

UINT BitRange( UINT aNum, INT aIndexStart, INT aIndexEnd )
{
    UINT uMask = ( 1 << ( aIndexEnd - aIndexStart + 1 ) ) - 1;
    return ( aNum >> aIndexStart ) & uMask;
}




}    //End of namespace CWUtils