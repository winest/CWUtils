#include "stdafx.h"
#include "CWIni.h"
using std::wstring;
using std::list;
using std::map;
using std::pair;

namespace CWUtils
{
BOOL GetIniSectionNames( CONST WCHAR * aIniPath, list<wstring> & aSectionNames )
{
    BOOL bRet = FALSE;
    WCHAR wzBuf[8];
    WCHAR * wzNames = NULL;
    BOOL bUseHeap = FALSE;

    DWORD dwRet = GetPrivateProfileSectionNamesW( wzBuf, _countof( wzBuf ), aIniPath );
    if ( 0 == dwRet )
    {
        goto exit;
    }
    else if ( dwRet == ( _countof( wzBuf ) - 2 ) )
    {
        bUseHeap = TRUE;
        ULONG ulBufSize = _countof( wzBuf );
        do
        {
            if ( NULL != wzNames )
            {
                delete[] wzNames;
            }
            ulBufSize = ulBufSize * 2;
            wzNames = new ( std::nothrow ) WCHAR[ulBufSize];
            if ( NULL == wzNames )
            {
                goto exit;
            }

            dwRet = GetPrivateProfileSectionNamesW( wzNames, ulBufSize, aIniPath );
        } while ( dwRet == ( ulBufSize - 2 ) );
    }
    else
    {
        wzNames = wzBuf;
    }

    CONST WCHAR * wzCurrName = wzNames;
    while ( L'\0' != wzCurrName[0] )
    {
        std::wstring wstrName( wzCurrName );
        aSectionNames.push_back( wstrName );
        wzCurrName += wstrName.length() + 1;
    }
    bRet = TRUE;

exit:
    if ( bUseHeap && wzNames )
    {
        delete[] wzNames;
    }

    return bRet;
}

BOOL GetIniSectionValues( const WCHAR * aIniPath, const WCHAR * aSectionName, map<wstring, wstring> & aKeyVal )
{
    BOOL bRet = FALSE;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    WCHAR * wzBuf = NULL;
    aKeyVal.clear();

    if ( NULL == aIniPath )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        goto exit;
    }
    else
    {
        hFile =
            CreateFileW( aIniPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
        if ( INVALID_HANDLE_VALUE == hFile )
            goto exit;
    }

    DWORD dwFileSize = GetFileSize( hFile, NULL );
    if ( INVALID_FILE_SIZE == dwFileSize )
    {
        goto exit;
    }
    else
    {
        wzBuf = new ( std::nothrow ) WCHAR[dwFileSize];
        if ( NULL == wzBuf )
            goto exit;
    }

    if ( GetPrivateProfileSectionW( aSectionName, wzBuf, dwFileSize, aIniPath ) )
    {
        for ( WCHAR * wch = wzBuf; *wch; wch += ( wcslen( wch ) + 1 ) )
        {
            wstring line = wch;
            size_t splitter = line.find_first_of( L'=' );
            if ( splitter != wstring::npos && splitter > 0 )
            {
                aKeyVal.insert( pair<wstring, wstring>( line.substr( 0, splitter ), line.substr( splitter + 1 ) ) );
            }
        }
        bRet = TRUE;
    }

exit:
    if ( INVALID_HANDLE_VALUE != hFile )
        CloseHandle( hFile );
    if ( NULL != wzBuf )
        delete[] wzBuf;
    return bRet;
}

}    //End of namespace CWUtils