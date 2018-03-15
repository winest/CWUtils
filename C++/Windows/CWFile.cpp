#include "stdafx.h"
#pragma warning( disable : 4127 )
#include "CWFile.h"

using std::vector;
using std::string;
using std::wstring;

namespace CWUtils
{

#ifdef __cplusplus
extern "C" {
#endif

VOID _SplitStringW( CONST wstring & aSrcString , vector<wstring> & aOutput , CONST wstring & aDelimiter )
{    
    wstring::size_type lastPos = aSrcString.find_first_not_of( aDelimiter , 0 );   //Skip delimiters at beginning    
    wstring::size_type pos     = aSrcString.find_first_of( aDelimiter , lastPos ); //Find first "non-delimiter"

    while ( wstring::npos != pos || wstring::npos != lastPos )
    {        
        aOutput.push_back( aSrcString.substr(lastPos , pos - lastPos) );   //Found a token, add it to the vector        
        lastPos = aSrcString.find_first_not_of( aDelimiter , pos );        //Skip delimiters. Note the "not_of"        
        pos = aSrcString.find_first_of( aDelimiter , lastPos );            //Find next "non-delimiter"
    }
}

BOOL _StringToWString( IN CONST std::string & aString , OUT std::wstring & aWString , DWORD aCodePage )
{
    BOOL bRet = FALSE;
    aWString.clear();

    DWORD dwFlag = ( CP_UTF8 == aCodePage ) ? MB_ERR_INVALID_CHARS : 0;
    WCHAR wzBuf[4096];
    INT nBuf = _countof( wzBuf );
    INT nBufCopied = MultiByteToWideChar( aCodePage , dwFlag , aString.c_str() , (INT)aString.size() , wzBuf , nBuf );
    if ( 0 != nBufCopied )
    {
        aWString.assign( wzBuf , nBufCopied );
        bRet = TRUE;
    }
    else if ( ERROR_INSUFFICIENT_BUFFER == GetLastError() )
    {
        nBuf = MultiByteToWideChar( aCodePage , dwFlag , aString.c_str() , (INT)aString.size() , NULL , 0 );
        WCHAR * wzNewBuf = new (std::nothrow) WCHAR[nBuf];
        if ( NULL != wzNewBuf )
        {
            nBufCopied = MultiByteToWideChar( aCodePage , dwFlag , aString.c_str() , (INT)aString.size() , wzNewBuf , nBuf );
            if ( 0 != nBufCopied )
            {
                aWString.assign( wzNewBuf , nBufCopied );
                bRet = TRUE;
            }
            delete [] wzNewBuf;
        }
    }
    else{}
    return bRet;
}



BOOL RelativeToFullPath( CONST WCHAR * aRelativePath , std::wstring & aFullPath )
{
    BOOL bRet = FALSE;
    WCHAR * wzNewBuf = NULL;
    do 
    {
        if ( NULL == aRelativePath )
        {
            break;
        }
     
        WCHAR wzBuf[4096] = {};
        DWORD dwSize = GetFullPathNameW( aRelativePath , _countof(wzBuf) , wzBuf , NULL );
        if ( _countof(wzBuf) <= dwSize )
        {
            wzNewBuf = new (std::nothrow) WCHAR[dwSize];
            if ( NULL == wzNewBuf )
            {
                break;
            }
            ZeroMemory( wzNewBuf , sizeof(WCHAR) * dwSize );
            if ( dwSize <= GetFullPathNameW( aRelativePath , _countof(wzBuf) , wzBuf , NULL ) )
            {
                break;
            }
            aFullPath = wzNewBuf;
        }
        else
        {
            aFullPath = wzBuf;
        }
        
        bRet = TRUE;
    } while ( 0 );
    
    if ( NULL != wzNewBuf )
    {
        delete [] wzNewBuf;
    }
    return bRet;
}

BOOL IsPathExist( CONST WCHAR * aFullPath )
{
    WIN32_FIND_DATAW FindFileData;
    HANDLE hFind = FindFirstFileW( aFullPath , &FindFileData );
    if ( INVALID_HANDLE_VALUE == hFind )
    {
        return FALSE;
    }
    else
    {
        FindClose( hFind );
        return TRUE;
    }
}

BOOL IsFileExist( CONST WCHAR * aFullPath )
{
    DWORD dwAttr = GetFileAttributesW( aFullPath );
    return ( (dwAttr != INVALID_FILE_ATTRIBUTES) && !(dwAttr & FILE_ATTRIBUTE_DIRECTORY) );
}

UINT64 GetFileSizeByPath( IN CONST WCHAR * aFullPath )
{
    UINT64 uRet = 0;
    HANDLE hFile = CreateFileW( aFullPath , GENERIC_READ , FILE_SHARE_READ , NULL , OPEN_EXISTING , FILE_ATTRIBUTE_NORMAL , NULL );
    if ( INVALID_HANDLE_VALUE != hFile )
    {
        LARGE_INTEGER size;
        if ( 0 != GetFileSizeEx( hFile , &size) )
        {
            uRet = size.QuadPart;
        }
        CloseHandle( hFile );
    }    
    return uRet;
}

BOOL GetFileContent( IN CONST WCHAR * aFullPath , IN OUT std::string & aContent )
{
    HANDLE hFile = CreateFileW( aFullPath , GENERIC_READ , FILE_SHARE_READ , NULL , OPEN_EXISTING , FILE_ATTRIBUTE_NORMAL , NULL );
    if ( INVALID_HANDLE_VALUE != hFile )
    {
        LARGE_INTEGER size;
        if ( 0 != GetFileSizeEx( hFile , &size) )
        {
            aContent.reserve( (SIZE_T)size.QuadPart );
        }

        BYTE byBuf[8192];
        DWORD dwRead = 0;
        while ( FALSE != ReadFile( hFile , byBuf , sizeof(byBuf) , &dwRead , NULL ) && 0 < dwRead )
        {
            aContent.append( (CONST CHAR *)byBuf , dwRead );
        }

        CloseHandle( hFile );
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

BOOL SaveToFile( IN CONST WCHAR * aSavePath , BOOL aAppend , IN CONST BYTE * aData , IN DWORD aDataSize )
{
    wstring wstrSavePath = aSavePath;
    if ( FALSE == CreateFileDir( wstrSavePath ) )
    {
        return FALSE;
    }

    DWORD dwOption = ( aAppend ) ? OPEN_ALWAYS : CREATE_ALWAYS;
    DWORD dwWrite = 0 , dwWritten = 0;
    HANDLE hFile = CreateFileW( aSavePath , GENERIC_READ | GENERIC_WRITE , FILE_SHARE_READ , 
                                NULL , dwOption , FILE_ATTRIBUTE_NORMAL , NULL );
    if ( INVALID_HANDLE_VALUE != hFile )
    {
        if ( aAppend && INVALID_SET_FILE_POINTER == SetFilePointer( hFile , 0 , NULL , FILE_END ) )
        {
            CloseHandle( hFile );
            return FALSE;
        }
        for ( dwWritten = 0 ; dwWritten < aDataSize ; dwWritten += dwWrite )
        {
            if ( FALSE == WriteFile( hFile , aData , aDataSize-dwWritten , &dwWrite , NULL ) || 0 == dwWrite )
                break;
        }
        CloseHandle( hFile );
    }
    return ( dwWritten == aDataSize );
}

BOOL SaveToFileEx( IN CONST WCHAR * aSavePath , BOOL aAppend , IN CONST CHAR * aFormat , ... )
{
    BOOL bRet = FALSE;

    if ( NULL != aFormat )
    {
        va_list args = NULL;
        va_start( args , aFormat );
        SIZE_T len = _vscprintf( aFormat , args ) + 1;    //Get formatted string length and adding one for null-terminator

        CHAR * szBuf = new (std::nothrow) CHAR[len];
        if ( NULL != szBuf )
        {
            if ( 0 < _vsnprintf_s( szBuf , len , _TRUNCATE , aFormat , args ) )
            {
                bRet = SaveToFile( aSavePath , aAppend , (CONST BYTE *)szBuf , (DWORD)(len - 1) );
            }
            delete [] szBuf;
        }
        va_end( args );
    }
    else
    {
        SetLastError( ERROR_INVALID_PARAMETER );
    }

    return bRet; 
}

BOOL ForceCopyFile( CONST WCHAR * aSrcPath , CONST WCHAR * aDstPath , BOOL aRemoveSrc )
{
    BOOL bRet = FALSE;

    for ( ;; ) 
    {
        if ( 0 == _wcsicmp( aSrcPath , aDstPath ) )
        {
            bRet = TRUE;
            break;
        }

        DWORD dwAttr = GetFileAttributesW( aDstPath );
        if ( INVALID_FILE_ATTRIBUTES != dwAttr )
        {
            if ( FILE_ATTRIBUTE_DIRECTORY & dwAttr )
            {
                SetLastError( ERROR_DIRECTORY );
                break;
            }

            if ( FILE_ATTRIBUTE_READONLY & dwAttr )
            {
                dwAttr &= ~FILE_ATTRIBUTE_READONLY;
                if ( FALSE == SetFileAttributesW( aDstPath , dwAttr ) )
                {
                    break;
                }
            }
        }
        
        if ( FALSE == ::CopyFileW( aSrcPath , aDstPath , FALSE ) )
        {
            break;
        }
        
        if ( aRemoveSrc )
        {
            DeleteFileW( aSrcPath );
        }
        bRet = TRUE;
    }
    
    return bRet;
}

BOOL ForceMoveFile( CONST WCHAR * aSrcPath , CONST WCHAR * aDstPath )
{
    return ForceCopyFile( aSrcPath , aDstPath , TRUE );
}

VOID CreateTempFile( OUT std::wstring & aFilePath , IN CONST WCHAR * aPreferFolderPath )
{
    aFilePath.clear();
    WCHAR wzFileName[MAX_PATH];
    for ( ;; ) 
    {
        if ( NULL != aPreferFolderPath && 0 != GetTempFileNameW( aPreferFolderPath , NULL , 0 , wzFileName ) )
        {
            aFilePath = aPreferFolderPath;
            break;
        }            
                
        WCHAR wzFolderPath[MAX_PATH+1];
        DWORD dwFolderLen = GetTempPathW( _countof(wzFolderPath) , wzFolderPath );
        if ( 0 != dwFolderLen && MAX_PATH >= dwFolderLen && 0 < GetTempFileNameW( wzFolderPath , NULL , 0 , wzFileName ) )
        {
            aFilePath = wzFolderPath;
            break;
        }

        GetTempFileNameW( L"." , NULL , 0 , wzFileName );
    }

    if ( aFilePath.length() && L'\\' != aFilePath[aFilePath.length()-1] )
    {
        aFilePath.push_back( L'\\' );
    }
    aFilePath.append( wzFileName );
    if ( MAX_PATH <= aFilePath.length() )
    {
        aFilePath.insert( 0 , L"\\\\?\\" );
    }
}

BOOL GetFileVersion( CONST WCHAR * aFullPath , WORD * aMajor , WORD * aMinor , WORD * aRevision , WORD * aBuildNumber )
{
    BOOL bSuccess = FALSE;
    DWORD dwVerHandle = 0;

    DWORD dwVerInfoSize = ::GetFileVersionInfoSizeW( (LPCWSTR)aFullPath , &dwVerHandle );

    if ( dwVerInfoSize != 0 )
    {
        BYTE *byVerInfoBuf = new (std::nothrow) BYTE[dwVerInfoSize];
        if ( byVerInfoBuf != NULL )
        {
            if( ::GetFileVersionInfoW( aFullPath , dwVerHandle , dwVerInfoSize , byVerInfoBuf ) )
            {
                VS_FIXEDFILEINFO * tempVerBuf;
                UINT uTempBufLen = 0;
                if( ::VerQueryValueW( byVerInfoBuf , L"\\" , (LPVOID *)&tempVerBuf , &uTempBufLen ) )
                {
                    if ( aMajor ) { *aMajor = HIWORD( tempVerBuf->dwFileVersionMS ); }
                    if ( aMinor ) { *aMinor = LOWORD( tempVerBuf->dwFileVersionMS ); }
                    if ( aRevision ) { *aRevision = HIWORD( tempVerBuf->dwFileVersionLS ); }
                    if ( aBuildNumber ) { *aBuildNumber = LOWORD( tempVerBuf->dwFileVersionLS ); }
                    bSuccess = TRUE;
                }
            }
            delete [] byVerInfoBuf;
            byVerInfoBuf = NULL;
        }
    }
    return bSuccess;
}

VERSION_COMPARE CompareFileVersion( CONST WCHAR * aNewFilePath , CONST WCHAR * aExistFilePath )
{
    VERSION_COMPARE vcRet = VERSION_UNAVAILABLE;
    WORD wNewMajor = 0 , wNewMinor = 0 , wNewRev = 0 , wNewBuild = 0;
    WORD wExistMajor = 0 , wExistMinor = 0 , wExistRev = 0 , wExistBuild = 0;

    if ( GetFileVersion( aNewFilePath , &wNewMajor , &wNewMinor , &wNewRev , &wNewBuild ) && 
         GetFileVersion( aExistFilePath , &wExistMajor , &wExistMinor , &wExistRev , &wExistBuild ) )
    {
        WORD wNew[4] , wExist[4];
        wNew[0] = wNewMajor; wNew[1] = wNewMinor; wNew[2] = wNewRev; wNew[3] = wNewBuild;
        wExist[0] = wExistMajor; wExist[1] = wExistMinor; wExist[2] = wExistRev; wExist[3] = wExistBuild;
        
        vcRet = VERSION_EQUAL;
        for ( INT i = 0 ; i < 4 ; i++ )
        {
            if ( wNew[i] > wExist[i] )
            {
                vcRet = VERSION_NEWER;
                break;
            }
            else if ( wNew[i] < wExist[i] )
            {
                vcRet = VERSION_OLDER;
                break;
            }
            else{}
        }
    }
    return vcRet;
}


BOOL DevicePathToDrivePath( IN CONST WCHAR * aDevicePath , OUT wstring & aDrivePath )
{
    BOOL bRet = FALSE;

    for ( ;; ) 
    {
        if ( NULL == aDevicePath )
        {
            break;
        }

        //Obtain the list of drives available in the system. Formatted as ""C:\(NULL)D:\(NULL)E:\(NULL)(NULL)"
        WCHAR wzDrives[(4 * 26) + 1] = { 0 };
        DWORD dwWritten = GetLogicalDriveStringsW( _countof(wzDrives) - 1 , wzDrives );
        if ( 0 == dwWritten || _countof(wzDrives) <= dwWritten )
        {
            break;
        }

        WCHAR wzDeviceName[MAX_PATH + 1] = { 0 };
        for ( DWORD i = 0 ; i < dwWritten ; i += 4 )
        {
            //QueryDosDeviceW() needs the format like "C:(NULL)" instead of "C:\(NULL)"
            wzDrives[i + 2] = L'\0';
            DWORD dwQueryRet = QueryDosDeviceW( &wzDrives[i] , wzDeviceName , _countof(wzDeviceName) - 1 );
            if ( 0 == dwQueryRet || _countof(wzDeviceName) <= dwQueryRet )
            {
                continue;
            }

            SIZE_T sizeDeviceNameLen = wcslen( wzDeviceName );
            if ( 0 == _wcsnicmp( aDevicePath , wzDeviceName , sizeDeviceNameLen ) )
            {
                aDrivePath.clear();
                aDrivePath.append( &wzDrives[i] );
                aDrivePath.append( aDevicePath + sizeDeviceNameLen );

                bRet = TRUE;
                break;
            }
        }
    }
    
    return bRet;
}

BOOL GetFilePathFromHandle( IN HANDLE aFile , OUT wstring & aFilePath )
{
    BOOL bRet = FALSE;
    HANDLE hMapFile = NULL;
    VOID * pViewOfFile = NULL;

    for ( ;; ) 
    {
#if ( _WIN32_WINNT_VISTA <= _WIN32_WINNT )
        //Use different method since Vista
        if ( IsWindowsVistaOrGreater() )
        {        
            WCHAR wzPath[MAX_PATH + 1] = { 0 };
            DWORD dwNewPathLen = GetFinalPathNameByHandleW( aFile , wzPath , _countof(wzPath) - 1 , FILE_NAME_NORMALIZED );
            if ( 0 == dwNewPathLen )
            {
                break;
            }
            else if ( _countof(wzPath) <= dwNewPathLen )
            {
                WCHAR * wzNewPath = new (std::nothrow) WCHAR[dwNewPathLen];
                if ( NULL != wzNewPath )
                {
                    if ( dwNewPathLen >= GetFinalPathNameByHandleW( aFile , wzNewPath , dwNewPathLen - 1 , FILE_NAME_NORMALIZED ) )
                    {
                        aFilePath = wzNewPath;
                        bRet = TRUE;
                    }
                    delete [] wzNewPath;
                }
            }
            else
            {
                aFilePath = wzPath;
                bRet = TRUE;
            }
        }
        else    //Use GetMappedFileNameW() for old platforms
        {   
#endif    //End of #if ( _WIN32_WINNT_VISTA <= _WIN32_WINNT )

            //Check file size since files with zero size cannot be mapped
            DWORD dwFileSizeHigh = 0 , dwFileSizeLow = 0;
            dwFileSizeLow = ::GetFileSize( aFile , &dwFileSizeHigh );

            if ( ( INVALID_FILE_SIZE == dwFileSizeLow ) && ( ERROR_SUCCESS != GetLastError() ) ) 
            {
                break;
            }
            else if( ( dwFileSizeLow == 0 ) && ( dwFileSizeHigh == 0 ) ) 
            {
                SetLastError( ERROR_EMPTY );
                break;
            }
            else{}

            //Map the file into memory
            hMapFile = CreateFileMappingW( aFile , NULL , PAGE_READONLY , 0 , 1 , NULL );
            if ( NULL == hMapFile )
            {
                break;
            }
            pViewOfFile = MapViewOfFile( hMapFile , FILE_MAP_READ , 0 , 0 , 1 );
            if ( NULL == pViewOfFile )
            {
                break;
            }

            //Get the file name
            WCHAR wzPath[MAX_PATH + 1] = { 0 };
            if ( ! GetMappedFileNameW( GetCurrentProcess() , pViewOfFile , wzPath , _countof(wzPath) - 1 ) )
            {
                break;
            }

            bRet = DevicePathToDrivePath( wzPath , aFilePath );
#if ( _WIN32_WINNT_VISTA <= _WIN32_WINNT )
        }
#endif    //End of #if ( _WIN32_WINNT_VISTA <= _WIN32_WINNT )
    }
    
    //Cleanup
    if ( NULL != pViewOfFile )
    {
        UnmapViewOfFile( pViewOfFile );
    }
    if ( NULL != hMapFile ) 
    {
        CloseHandle( hMapFile );
    }

    return bRet;
}

BOOL GetCurrentProcessPath( OUT wstring & aProcPath )
{
    BOOL bRet = FALSE;
    UINT uReturnSize = 0;
    WCHAR wzPath[MAX_PATH] = { 0 };

    uReturnSize = ::GetModuleFileNameW( NULL , wzPath , _countof(wzPath) );
    if ( _countof(wzPath) <= uReturnSize || ERROR_INSUFFICIENT_BUFFER == GetLastError() )
    {
        WCHAR * wzPathEx = new (std::nothrow) WCHAR[uReturnSize + 1];
        if ( NULL != wzPathEx )
        {
            wzPathEx[uReturnSize] = L'\0';
            uReturnSize = ::GetModuleFileNameW( NULL , wzPathEx , uReturnSize + 1 );
            if ( 0 != uReturnSize )
            {
                aProcPath = wzPathEx;
                bRet = TRUE;
            }
            delete [] wzPathEx;
            wzPathEx = NULL;
        }
    }
    else
    {
        aProcPath = wzPath;
        bRet = TRUE;
    }
    return bRet;
}

BOOL IsDirExist( CONST WCHAR * aDirPath )
{
    DWORD dwAttr = GetFileAttributesW( aDirPath );
    return ( (dwAttr != INVALID_FILE_ATTRIBUTES) && (dwAttr & FILE_ATTRIBUTE_DIRECTORY) );
}

BOOL IsEmptyDir( CONST WCHAR * aDirPath )
{
    wstring wstrDir = aDirPath;
    HANDLE hFind;
    WIN32_FIND_DATAW find;
    if ( wstrDir.length() > 0 && wstrDir[wstrDir.length()-1] != L'\\' )
        wstrDir.push_back( L'\\' );
    wstrDir.push_back( L'*' );

    INT count = 0;

    hFind = FindFirstFileW( wstrDir.c_str() , &find );
    if ( hFind == INVALID_HANDLE_VALUE )
    {
        return TRUE;   //Wrong path
    }
    else
    {
        if ( find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY &&
             ( wcsncmp( find.cFileName , L"." , 1 ) == 0 ) || ( wcsncmp( find.cFileName , L".." , 2 ) == 0 ) )
        {
            count++;
            while ( FindNextFileW( hFind , &find ) != 0 )
            {
                if ( ! ( find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )  ||
                     ( wcsncmp( find.cFileName , L"." , 1 ) != 0 ) || ( wcsncmp( find.cFileName , L".." , 2 ) != 0 ) )
                {
                    FindClose( hFind );
                    return FALSE;
                }

                count++;                
            }
        }
    }
    
    FindClose( hFind );
    return ( count <= 2 ) ? TRUE : FALSE;
}

BOOL GetWindowsDir( wstring & aWinDir )
{
    BOOL bRet = FALSE;
    UINT uReturnSize = 0;
    WCHAR wzWinDir[MAX_PATH] = { 0 };

    uReturnSize = ::GetSystemWindowsDirectoryW( (LPWSTR)&wzWinDir , _countof(wzWinDir) );
    if ( _countof(wzWinDir) <= uReturnSize )
    {
        WCHAR * wzWinDirEx = new (std::nothrow) WCHAR[uReturnSize + 1];
        if ( NULL != wzWinDirEx )
        {
            wzWinDirEx[uReturnSize] = L'\0';
            uReturnSize = ::GetSystemWindowsDirectoryW( wzWinDirEx , uReturnSize );
            if ( 0 != uReturnSize )
            {
                aWinDir = wzWinDirEx;
                bRet = TRUE;
            }
            delete [] wzWinDirEx;
            wzWinDirEx = NULL;
        }
    }
    else
    {
        aWinDir = wzWinDir;
        bRet = TRUE;
    }

    if ( TRUE == bRet && L'\\' != aWinDir[aWinDir.length()-1] )
    {
        aWinDir.push_back( L'\\' );
    }
    return bRet;
}

BOOL GetModuleDir( HMODULE aModule , wstring & aFolder )
{
    aFolder = L"";
    WCHAR wzModuleName[MAX_PATH] = { 0 };
    DWORD dwRet = ::GetModuleFileNameW( aModule, wzModuleName, _countof(wzModuleName));
    if ( 0 == dwRet )
    {
        return FALSE;
    }
    else
    {
        wstring wstrFullpath = wzModuleName;
        aFolder = wstrFullpath.substr( 0 , wstrFullpath.find_last_of(L'\\') + 1 );
        return TRUE;
    }
}

BOOL GetFileDir( IN CONST WCHAR * aSearchPath , IN CONST WCHAR * aFindingFile , OUT wstring & aResult )
{
    WIN32_FIND_DATAW wfd;
    ::ZeroMemory( &wfd , sizeof(wfd) );
    wstring wstrSource = aSearchPath;
    if ( wstrSource.length() > 0 && wstrSource[wstrSource.length()-1] != L'\\' )
        wstrSource += L"\\";

    wstring wstrFind = wstrSource + L"*.*";
    HANDLE hFile = ::FindFirstFileW( wstrFind.c_str() , &wfd );
    if ( hFile == INVALID_HANDLE_VALUE )
    {
        return FALSE;
    }

    do
    {
        if ( wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
        {
            if ( wcsncmp( wfd.cFileName , L"." , 2 ) != 0 && wcsncmp( wfd.cFileName , L".." , 3 ) != 0 )
            {
                if ( TRUE == GetFileDir( (wstrSource+wfd.cFileName).c_str() , aFindingFile , aResult ) )
                {
                    ::FindClose( hFile );
                    return TRUE;
                }
            }

            continue;
        }

        if ( _wcsicmp( wfd.cFileName, aFindingFile ) == 0 )
        {
            ::FindClose( hFile );
            aResult = wstrSource;
            return TRUE;
        }
    } while ( ::FindNextFileW(hFile , &wfd) );

    ::FindClose( hFile );
    return FALSE;
}

BOOL CreateDir( CONST wstring & aDir )
{
    //Check whether the directory exists or not
    if ( TRUE == IsDirExist( aDir.c_str() ) )
    {
        return TRUE;
    }

    //Create from the root directory
    vector<wstring> tokens;
    _SplitStringW( aDir , tokens , L"\\" );
    wstring path;
    UINT i;
    for ( i = 0 ; i < tokens.size() ; i++ )
    {
        path.append( tokens[i] );
        HANDLE hDir = CreateFileW( path.c_str() , GENERIC_READ , FILE_SHARE_READ | FILE_SHARE_WRITE , NULL , 
                                   OPEN_EXISTING , FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS , NULL );
        if ( INVALID_HANDLE_VALUE == hDir )
        {
            if ( ERROR_FILE_NOT_FOUND == GetLastError() && TRUE == CreateDirectoryW(path.c_str(),NULL) )
                path.push_back( L'\\' );
            else
                break;
        }
        else
        {
            CloseHandle( hDir );
            path.push_back( L'\\' );
        }
    }
    if ( i != tokens.size() )
        return FALSE;
    else
        return TRUE;
}

//Delete the folder recursively
BOOL DeleteDir( wstring aDir )
{
    HANDLE hFind;
    WIN32_FIND_DATAW find;
    if ( aDir.length() > 0 && aDir[aDir.length()-1] != L'\\' )
        aDir.push_back( L'\\' );

    hFind = FindFirstFileW( (aDir + L"*").c_str() , &find );

    if ( hFind == INVALID_HANDLE_VALUE )
    {
        return FALSE;   //Wrong path
    }
    else
    {
        if ( find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )    //Directory
        {
            if ( ( wcsncmp( find.cFileName , L"." , 1 ) == 0 ) || ( wcsncmp( find.cFileName , L".." , 2 ) == 0 ) )
            {}
            else
            {
                DeleteDir( aDir + find.cFileName );
            }
        }
        else    //File
        {
            wstring wstrFilePath = aDir + find.cFileName ;
            DWORD dwAttr = GetFileAttributesW( wstrFilePath.c_str() );
            dwAttr &= ~FILE_ATTRIBUTE_READONLY;
            SetFileAttributesW( wstrFilePath.c_str() , dwAttr );
            DeleteFileW( wstrFilePath.c_str() );
        }
    }

    while ( FindNextFileW( hFind , &find ) != 0 )
    {
        if ( find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )    //Directory
        {
            if ( ( wcsncmp( find.cFileName , L"." , 1 ) == 0 ) || ( wcsncmp( find.cFileName , L".." , 2 ) == 0 ) )
            {}
            else
            {
                DeleteDir( aDir + find.cFileName );
            }
        }
        else    //File
        {
            wstring wstrFilePath = aDir + find.cFileName ;
            DWORD dwAttr = GetFileAttributesW( wstrFilePath.c_str() );
            dwAttr &= ~FILE_ATTRIBUTE_READONLY;
            SetFileAttributesW( wstrFilePath.c_str() , dwAttr );
            DeleteFileW( wstrFilePath.c_str() );
        }
    }

    FindClose( hFind );
    RemoveDirectoryW( aDir.c_str() );    
    return TRUE;
}

//Create directory for the file if the directory doesn't exist
//If aFilePath="C:\123\456", if will create "C:\123"
//If aFilePath="C:\123\456\", if will create "C:\123\456"
//If aFilePath="456", if will not create anything
BOOL CreateFileDir( CONST wstring & aFileFullPath )
{
    CONST WCHAR * wzLast = wcsrchr( aFileFullPath.c_str() , L'\\' );
    BOOL bFullPath = ( NULL != wzLast );
    if ( 0 == aFileFullPath.length() )
    {
        return FALSE;
    }
    else if ( FALSE == bFullPath )
    {
        return TRUE;
    }
    else if ( L'\\' == aFileFullPath[aFileFullPath.length() - 1] )
    {
        return CreateDir( aFileFullPath );
    }
    else
    {
        wstring wstrDir( aFileFullPath , 0 , wzLast - aFileFullPath.c_str() + 1 );
        return CreateDir( wstrDir );
    }    
}






BOOL CFile::Open( CONST CHAR * aPath , UINT32 aOpenAttr , CONST std::string & aLineSep )
{
    string strPath = aPath;
    wstring wstrPath;
    _StringToWString( strPath , wstrPath , CP_ACP );
    return this->Open( wstrPath.c_str() , aOpenAttr , aLineSep );
}

BOOL CFile::Open( CONST WCHAR * aPath , UINT32 aOpenAttr , CONST std::string & aLineSep )
{
    BOOL bRet = FALSE;

    do 
    {
        if ( NULL != m_hFile )
        {
            break;
        }

        DWORD dwCreateDisposition = 0;
        if ( aOpenAttr & FILE_OPEN_ATTR_CREATE_IF_NOT_EXIST )
        {
            dwCreateDisposition = OPEN_ALWAYS;
        }
        else if ( aOpenAttr & FILE_OPEN_ATTR_CREATE_ALWAYS )
        {
            dwCreateDisposition = CREATE_ALWAYS;
        }
        else if ( aOpenAttr & FILE_OPEN_ATTR_OPEN_EXISTING )
        {
            dwCreateDisposition = OPEN_EXISTING;
        }
        else
        {
            break;
        }

        DWORD dwAccess = 0;
        if ( aOpenAttr & FILE_OPEN_ATTR_READ )
        {
            dwAccess |= GENERIC_READ;
        }
        if ( aOpenAttr & FILE_OPEN_ATTR_WRITE )
        {
            dwAccess |= GENERIC_WRITE;
        }
        
        HANDLE hFile = CreateFileW( aPath , dwAccess , FILE_SHARE_READ , NULL , dwCreateDisposition , FILE_ATTRIBUTE_NORMAL , NULL );
        if ( INVALID_HANDLE_VALUE == hFile )
        {
            break;
        }

        if ( aOpenAttr & FILE_OPEN_ATTR_MOVE_TO_END )
        {
            SetFilePointer( hFile , 0 , 0 , FILE_END );
        }
        m_hFile = hFile;
        m_strLineSep = aLineSep;
        bRet = TRUE;
    } while ( 0 );
    
    return bRet;
}

BOOL CFile::Write( CONST UCHAR * aData , SIZE_T aDataSize )
{
    BOOL bRet = FALSE;
    DWORD dwWritten;

    do 
    {
        if ( NULL == m_hFile )
        {
            break;
        }
        if ( FALSE == WriteFile( m_hFile , aData , (DWORD)aDataSize , &dwWritten , NULL ) )
        {
            break;
        }
        if ( dwWritten != (DWORD)aDataSize )
        {
            break;
        }
        bRet = TRUE;
    } while ( 0 );
    
    return bRet;
}

BOOL CFile::WriteLine()
{
    return this->Write( (CONST UCHAR *)m_strLineSep.c_str() , m_strLineSep.size() );
}

BOOL CFile::WriteLine( CONST UCHAR * aData , SIZE_T aDataSize )
{
    BOOL bRet = FALSE;
    bRet = this->Write( aData , aDataSize );
    if ( FALSE != bRet )
    {
        bRet = this->Write( (CONST UCHAR *)m_strLineSep.c_str() , m_strLineSep.size() );
    }
    return bRet;
}

BOOL CFile::Read( std::string & aData , SIZE_T aDataSize , BOOL aAppend )
{
    if ( FALSE == aAppend )
    {
        aData.clear();
    }
    DWORD dwLeft = aDataSize;
    
    if ( m_strReadBuf.size() > 0 )
    {
        size_t uCopied = min( dwLeft , m_strReadBuf.size() );
        aData.append( m_strReadBuf , 0 , uCopied );
        dwLeft -= uCopied;
        m_strReadBuf.erase( 0 , uCopied );
    }

    if ( dwLeft > 0 )
    {
        BYTE byBuf[8192];
        DWORD dwRead = 0;
        while ( FALSE != ReadFile( m_hFile , byBuf , dwLeft , &dwRead , NULL ) && 0 < dwRead )
        {
            aData.append( (CONST CHAR *)byBuf , dwRead );
            dwLeft -= dwRead;
        }
    }

    return ( dwLeft == 0 ) ? TRUE : FALSE;
}

BOOL CFile::ReadLine( std::string & aData , BOOL aAppend )
{
    BOOL bRet = FALSE;

    if ( FALSE == aAppend )
    {
        aData.clear();
    }

    do 
    {
        if ( m_strReadBuf.size() )
        {
            size_t uPos = m_strReadBuf.find( m_strLineSep );
            if ( uPos != string::npos )
            {
                aData = m_strReadBuf.substr( 0 , uPos );
                m_strReadBuf.erase( 0 , uPos + m_strLineSep.size() );
                bRet = TRUE;
                break;
            }
        }
    
        BYTE byBuf[8192];
        DWORD dwRead = 0;
        while ( FALSE != ReadFile( m_hFile , byBuf , sizeof(byBuf) , &dwRead , NULL ) && 0 < dwRead )
        {
            m_strReadBuf.append( (CONST CHAR *)byBuf , dwRead );
            size_t uPos = m_strReadBuf.find( m_strLineSep );
            if ( uPos != string::npos )
            {
                aData = m_strReadBuf.substr( 0 , uPos );
                m_strReadBuf.erase( 0 , uPos + m_strLineSep.size() );
                bRet = TRUE;
                break;
            }
        }
    } while ( 0 );
    
    return bRet;
}


VOID CFile::Flush()
{
    if ( NULL != m_hFile )
    {
        FlushFileBuffers( m_hFile );
    }
}

VOID CFile::Close()
{
    this->Flush();
    if ( NULL != m_hFile )
    {
        CloseHandle( m_hFile );
        m_hFile = NULL;
    }
    m_strLineSep.clear();
}





BOOL CCsv::WriteRow( CONST std::vector<std::string> & aColData , BOOL aAddQuote )
{
    BOOL bRet = FALSE;

    for ( UINT i = 0 ; i < aColData.size() ; i++ )
    {
        if ( aAddQuote )
        {
            this->Write( (CONST UCHAR *)"\"" , strlen("\"") );
        }
        this->Write( (CONST UCHAR *)aColData[i].c_str() , aColData[i].size() );
        if ( aAddQuote )
        {
            this->Write( (CONST UCHAR *)"\"" , strlen("\"") );
        }

        if ( i < aColData.size() - 1 )
        {
            this->Write( (CONST UCHAR *)"," , strlen(",") );
        }
    }
    this->WriteLine();

    return bRet;
}













#ifdef __cplusplus
}
#endif

}   //End of namespace CWUtils