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
