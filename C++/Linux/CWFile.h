#pragma once
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <vector>
#include <string>

namespace CWUtils
{

#ifdef __cplusplus
extern "C" {
#endif

BOOL IsPathExist( CONST CHAR * aFullPath );

BOOL IsFileExist( CONST CHAR * aFullPath );

BOOL IsDirExist( CONST CHAR * aDirPath );

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
        BOOL Open( CONST CHAR * aPath , CFileOpenAttr aOpenAttr , BOOL aMoveToEnd , BOOL aCanRead , BOOL aCanWrite );
        BOOL Write( CONST UCHAR * aData , SIZE_T aDataSize );
        BOOL WriteLine( CONST UCHAR * aData , SIZE_T aDataSize );
        VOID Flush();
        VOID Close();

        FILE * GetFileHandle() { return m_hFile; }

    private :
        FILE * m_hFile;
};

#ifdef __cplusplus
}
#endif

}   //End of namespace CWUtils
