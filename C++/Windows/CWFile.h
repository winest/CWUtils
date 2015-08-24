#pragma once

#include <Windows.h>
#include <string>

namespace CWUtils
{

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _VERSION_COMPARE
{
    VERSION_UNAVAILABLE = -2 ,
    VERSION_OLDER ,
    VERSION_EQUAL ,
    VERSION_NEWER
} VERSION_COMPARE;

BOOL RelativeToFullPath( CONST WCHAR * aRelativePath , std::wstring & aFullPath );

BOOL IsPathExist( CONST WCHAR * aFullPath );

BOOL IsFileExist( CONST WCHAR * aFullPath );

UINT64 GetFileSizeByPath( IN CONST WCHAR * aFullPath );

BOOL GetFileContent( IN CONST WCHAR * aFullPath , IN OUT std::string & aContent );

BOOL SaveToFile( IN CONST WCHAR * aSavePath , BOOL aAppend , IN CONST BYTE * aData , IN DWORD aDataSize );
BOOL SaveToFileEx( IN CONST WCHAR * aSavePath , BOOL aAppend , IN CONST CHAR * aFormat , ... );

BOOL ForceCopyFile( CONST WCHAR * aSrcPath , CONST WCHAR * aDstPath , BOOL aRemoveSrc = FALSE );
BOOL ForceMoveFile( CONST WCHAR * aSrcPath , CONST WCHAR * aDstPath );

VOID CreateTempFile( OUT std::wstring & aFilePath , IN CONST WCHAR * aPreferFolderPath = NULL );

BOOL GetFileVersion( CONST WCHAR * aFullPath , WORD * aMajor , WORD * aMinor , WORD * aRevision , WORD * aBuildNumber );
VERSION_COMPARE CompareFileVersion( CONST WCHAR * aNewFilePath , CONST WCHAR * aExistFilePath );

BOOL DevicePathToDrivePath( IN CONST WCHAR * aDevicePath , OUT std::wstring & aDrivePath );
BOOL GetFilePathFromHandle( IN HANDLE aFile , OUT std::wstring & aFilePath );
BOOL GetCurrentProcessPath( OUT std::wstring & aProcPath );


BOOL IsDirExist( CONST WCHAR * aDirPath );

BOOL IsEmptyDir( CONST WCHAR * aDirPath );

BOOL GetWindowsDir( std::wstring & aWinDir );

BOOL GetModuleDir( HMODULE aModule , std::wstring & aFolder );

BOOL GetFileDir( IN CONST WCHAR * aSearchPath , IN CONST WCHAR * aFindingFile , OUT std::wstring & aResult );

//Create the folder if it's not exists
BOOL CreateDir( CONST std::wstring & aDir );

//Delete the folder recursively
BOOL DeleteDir( std::wstring aDir );

//Create directory for the file if the directory doesn't exist
//If aFilePath="C:\123\456", if will create "C:\123"
//If aFilePath="C:\123\456\", if will create "C:\123\456"
//If aFilePath="456", if will not create anything
BOOL CreateFileDir( CONST std::wstring & aFileFullPath );




typedef enum _CFileOpenAttr
{
    FILE_OPEN_ATTR_CREATE_IF_NOT_EXIST = 0 ,    //Open if exists, create if not exists
    FILE_OPEN_ATTR_CREATE_ALWAYS ,              //Always create new file
    FILE_OPEN_ATTR_OPEN_EXISTING ,              //Open if exists
} CFileOpenAttr;

class CFile
{
    public :
        CFile() : m_hFile(NULL) {}
        virtual ~CFile() { this->Close(); }

    public :
        BOOL Open( CONST WCHAR * aPath , CFileOpenAttr aOpenAttr , BOOL aMoveToEnd , BOOL aCanRead , BOOL aCanWrite );
        BOOL Write( CONST UCHAR * aData , SIZE_T aDataSize );
        VOID Flush();
        VOID Close();

        HANDLE GetFileHandle() { return m_hFile; }

    private :
        HANDLE m_hFile;
};

#ifdef __cplusplus
}
#endif

}   //End of namespace CWUtils