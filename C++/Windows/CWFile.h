#pragma once

/*
 * Copyright (c) 2009-2015, ChienWei Hung <winestwinest@gmail.com>
 * CWUtils is published under the BSD-3-Clause license.
 *
 * CWUtils is a set of standalone APIs for developers to speed up their 
 * programming. It should be very easy to port them to other projects or 
 * learn how to implement things on different languages and platforms. 
 *
 * The latest version can be found at https://github.com/winest/CWUtils
 */

#include <Windows.h>
#include <string>
#include <vector>
#include <string>
#include <Psapi.h>
#include <VersionHelpers.h>

#pragma comment( lib, "Version.lib" )
#if ( _WIN32_WINNT_VISTA > _WIN32_WINNT )
#    pragma comment( lib, "Psapi.lib" )
#endif



namespace CWUtils
{
#ifdef __cplusplus
extern "C" {
#endif

typedef enum _VERSION_COMPARE
{
    VERSION_UNAVAILABLE = -2,
    VERSION_OLDER,
    VERSION_EQUAL,
    VERSION_NEWER
} VERSION_COMPARE;

BOOL RelativeToFullPath( CONST WCHAR * aRelativePath, std::wstring & aFullPath );

BOOL IsPathExist( CONST WCHAR * aFullPath );

BOOL IsFileExist( CONST WCHAR * aFullPath );

UINT64 GetFileSizeByPath( IN CONST WCHAR * aFullPath );

BOOL GetFileContent( IN CONST WCHAR * aFullPath, IN OUT std::string & aContent );

BOOL SaveToFile( IN CONST WCHAR * aSavePath, BOOL aAppend, IN CONST BYTE * aData, IN DWORD aDataSize );
BOOL SaveToFileEx( IN CONST WCHAR * aSavePath, BOOL aAppend, IN CONST CHAR * aFormat, ... );

BOOL ForceCopyFile( CONST WCHAR * aSrcPath, CONST WCHAR * aDstPath, BOOL aRemoveSrc = FALSE );
BOOL ForceMoveFile( CONST WCHAR * aSrcPath, CONST WCHAR * aDstPath );

VOID CreateTempFile( OUT std::wstring & aFilePath, IN CONST WCHAR * aPreferFolderPath = NULL );

BOOL GetFileVersion( CONST WCHAR * aFullPath, WORD * aMajor, WORD * aMinor, WORD * aRevision, WORD * aBuildNumber );
VERSION_COMPARE CompareFileVersion( CONST WCHAR * aNewFilePath, CONST WCHAR * aExistFilePath );

BOOL DevicePathToDrivePath( IN CONST WCHAR * aDevicePath, OUT std::wstring & aDrivePath );
BOOL GetFilePathFromHandle( IN HANDLE aFile, OUT std::wstring & aFilePath );
BOOL GetCurrentProcessPath( OUT std::wstring & aProcPath );


BOOL IsDirExist( CONST WCHAR * aDirPath );

BOOL IsEmptyDir( CONST WCHAR * aDirPath );

BOOL GetWindowsDir( std::wstring & aWinDir );

BOOL GetModuleDir( HMODULE aModule, std::wstring & aFolder );

BOOL GetFileDir( IN CONST WCHAR * aSearchPath, IN CONST WCHAR * aFindingFile, OUT std::wstring & aResult );

//Create the folder if it's not exists
BOOL CreateDir( CONST std::wstring & aDir );

//Delete the folder recursively
BOOL DeleteDir( std::wstring aDir );

//Create directory for the file if the directory doesn't exist
//If aFilePath="C:\123\456", if will create "C:\123"
//If aFilePath="C:\123\456\", if will create "C:\123\456"
//If aFilePath="456", if will not create anything
BOOL CreateFileDir( CONST std::wstring & aFileFullPath );



#ifndef FILE_LINE_SEP
    #define FILE_LINE_SEP "\r\n"
#endif
CONST SIZE_T FILE_BUF_SIZE = 4096;

CONST UINT32 FILE_OPEN_ATTR_NONE = 0x00000000;                   //Nothing
CONST UINT32 FILE_OPEN_ATTR_CREATE_IF_NOT_EXIST = 0x00000001;    //Open if exists, create if not exists
CONST UINT32 FILE_OPEN_ATTR_CREATE_ALWAYS = 0x00000002;          //Always create new file
CONST UINT32 FILE_OPEN_ATTR_OPEN_EXISTING = 0x00000004;          //Open if exists

CONST UINT32 FILE_OPEN_ATTR_BINARY = 0x00000008;         //Open the raw file directly
CONST UINT32 FILE_OPEN_ATTR_MOVE_TO_END = 0x00000010;    //Move file pointer to the end of file
CONST UINT32 FILE_OPEN_ATTR_READ = 0x00000020;           //Open for read
CONST UINT32 FILE_OPEN_ATTR_WRITE = 0x00000040;          //Open for write

class CFile
{
    public:
    CFile() : m_hFile( NULL ) , m_uReadPos(0) { m_strReadBuf.reserve( FILE_BUF_SIZE * 2 ); }
    virtual ~CFile() { this->Close(); }

    public:
    BOOL Open( CONST CHAR * aPath, UINT32 aOpenAttr = FILE_OPEN_ATTR_CREATE_IF_NOT_EXIST | FILE_OPEN_ATTR_READ, CONST std::string aLineSep = FILE_LINE_SEP );
    BOOL Open( CONST WCHAR * aPath, UINT32 aOpenAttr = FILE_OPEN_ATTR_CREATE_IF_NOT_EXIST | FILE_OPEN_ATTR_READ, CONST std::string aLineSep = FILE_LINE_SEP );
    BOOL Write( CONST CHAR * aData, SIZE_T aDataSize );
    BOOL Write( CONST UCHAR * aData, SIZE_T aDataSize );
    BOOL WriteLine();
    BOOL WriteLine( CONST CHAR * aData, SIZE_T aDataSize );
    BOOL WriteLine( CONST UCHAR * aData, SIZE_T aDataSize );

    BOOL Read( std::string & aData, SIZE_T aDataSize, BOOL aAppend = FALSE );
    BOOL ReadLine( std::string & aData, BOOL aAppend = FALSE );
    VOID Flush();
    VOID Close();

    HANDLE GetFileHandle() { return m_hFile; }

    protected:
    HANDLE m_hFile;
    std::string m_strLineSep;
    std::string m_strReadBuf;
    size_t m_uReadPos;

    std::string m_strTmp;
};

class CCsv : public CFile
{
    public:
    CCsv() : m_strColSep( "," ) {}
    CCsv( std::string aColSep ) : m_strColSep( aColSep ) {}
    virtual ~CCsv() { this->Close(); }

    public:
    BOOL WriteRow( CONST std::vector<std::string> & aColData, BOOL aAddQuote );
    BOOL ReadRow( std::vector<std::string> & aColData, BOOL aAppend = FALSE );
    BOOL ReadRow( std::vector<int> & aColData, BOOL aAppend = FALSE );
    BOOL ReadRow( std::vector<float> & aColData, BOOL aAppend = FALSE );
    BOOL ReadRow( std::vector<double> & aColData, BOOL aAppend = FALSE );

    protected:
    std::string m_strColSep;
};

#ifdef __cplusplus
}
#endif

}    //End of namespace CWUtils