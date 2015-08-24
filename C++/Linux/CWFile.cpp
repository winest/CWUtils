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


BOOL IsPathExist( CONST CHAR * aFullPath )
{
    return ( access( aFullPath , F_OK ) != -1 ) ? TRUE : FALSE;
}

BOOL IsFileExist( CONST CHAR * aFullPath )
{
    struct stat st;
    return ( lstat( aFullPath , &st ) != -1 && S_ISREG(st.st_mode) ) ? TRUE : FALSE;
}

BOOL IsDirExist( CONST CHAR * aDirPath )
{
    struct stat st;
    return ( lstat( aDirPath , &st ) != -1 && S_ISDIR(st.st_mode) ) ? TRUE : FALSE;
}





BOOL CFile::Open( CONST CHAR * aPath , CFileOpenAttr aOpenAttr , BOOL aMoveToEnd , BOOL aCanRead , BOOL aCanWrite )
{
    BOOL bRet = FALSE;

    do 
    {
        if ( NULL != m_hFile )
        {
            break;
        }

        DWORD dwAccess = 0;

        string strMode;
        switch ( aOpenAttr )
        {
            case FILE_OPEN_ATTR_CREATE_IF_NOT_EXIST :
            {
                strMode.push_back( 'a' );
                break;
            }
            case FILE_OPEN_ATTR_CREATE_ALWAYS :
            {
                strMode.push_back( 'w' );
                break;
            }
            case FILE_OPEN_ATTR_OPEN_EXISTING :
            {
                strMode.push_back( 'r' );
                break;
            }
        }
        if ( aCanWrite )
        {
            strMode.push_back( '+' );
        }
        FILE * hFile = fopen( aPath , strMode.c_str() );
        if ( NULL == hFile )
        {
            break;
        }

        if ( FALSE == aMoveToEnd )
        {
            fseek( hFile , 0 , SEEK_SET );
        }
        m_hFile = hFile;
        bRet = TRUE;
    } while ( 0 );

    return bRet;
}

BOOL CFile::Write( CONST UCHAR * aData , SIZE_T aDataSize )
{
    BOOL bRet;
    size_t uWritten;

    do 
    {
        if ( NULL == m_hFile )
        {
            break;
        }
        
        uWritten = fwrite( aData , 1 , aDataSize , m_hFile );
        if ( uWritten != aDataSize )
        {
            break;
        }
        bRet = TRUE;
    } while ( 0 );

    return bRet;
}

BOOL CFile::WriteLine( CONST UCHAR * aData , SIZE_T aDataSize )
{
    BOOL bRet = FALSE;
    size_t uWritten;

    do 
    {
        if ( NULL == m_hFile )
        {
            break;
        }
        
        uWritten = fwrite( aData , 1 , aDataSize , m_hFile );
        if ( uWritten != aDataSize )
        {
            break;
        }
        uWritten = fwrite( "\n" , 1 , 1 , m_hFile );
        if ( uWritten != 1 )
        {
            break;
        }
        bRet = TRUE;
    } while ( 0 );

    return bRet;
}

VOID CFile::Flush()
{
    if ( NULL != m_hFile )
    {
        fflush( m_hFile );
    }
}

VOID CFile::Close()
{
    this->Flush();
    if ( NULL != m_hFile )
    {
        fclose( m_hFile );
        m_hFile = NULL;
    }
}

#ifdef __cplusplus
}
#endif

}   //End of namespace CWUtils
