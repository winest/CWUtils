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


BOOL _WStringToString( IN CONST std::wstring & aWString , OUT std::string & aString , DWORD aCodePage )
{
    UNREFERENCED_PARAMETER( aCodePage ); //Linux will use the locale in setlocale( LC_CTYPE , NULL )
    
    BOOL bRet = FALSE;
    aString.clear();

    CHAR szBuf[4096];
    size_t uBufSize = _countof( szBuf );
    size_t uBufCopied = wcstombs( szBuf , aWString.c_str() , uBufSize );
    if ( uBufCopied < uBufSize )
    {
        aString.assign( szBuf , uBufCopied );
        bRet = TRUE;
    }
    else
    {
        uBufSize = aWString.length() * 4;
        CHAR * szNewBuf = new (std::nothrow) CHAR[uBufSize];
        if ( NULL != szNewBuf )
        {
            uBufCopied = wcstombs( szNewBuf , aWString.c_str() , uBufSize );
            if ( uBufCopied < uBufSize )
            {
                aString.assign( szNewBuf , uBufCopied );
                bRet = TRUE;
            }            
            delete [] szNewBuf;
        }
    }
    return bRet;
}



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





BOOL CFile::Open( CONST CHAR * aPath , UINT32 aOpenAttr , CONST std::string & aLineSep )
{
    BOOL bRet = FALSE;

    do 
    {
        if ( NULL != m_hFile )
        {
            break;
        }

        string strMode;
        if ( aOpenAttr & FILE_OPEN_ATTR_CREATE_IF_NOT_EXIST )
        {
            strMode.push_back( 'a' );
            if ( aOpenAttr & FILE_OPEN_ATTR_READ )
            {
                strMode.push_back( '+' );
            }
        }
        else if ( aOpenAttr & FILE_OPEN_ATTR_CREATE_ALWAYS )
        {
            strMode.push_back( 'w' );
            if ( aOpenAttr & FILE_OPEN_ATTR_READ )
            {
                strMode.push_back( '+' );
            }
        }
        else if ( aOpenAttr & FILE_OPEN_ATTR_OPEN_EXISTING )
        {
            strMode.push_back( 'r' );
            if ( aOpenAttr & FILE_OPEN_ATTR_WRITE )
            {
                strMode.push_back( '+' );
            }
        }
        else
        {
            break;
        }
        
        if ( aOpenAttr & FILE_OPEN_ATTR_BINARY )
        {
            strMode.push_back( 'b' );
        }
        
        FILE * hFile = fopen( aPath , strMode.c_str() );
        if ( NULL == hFile )
        {
            break;
        }

        if ( aOpenAttr & FILE_OPEN_ATTR_MOVE_TO_END )
        {
            fseek( hFile , 0 , SEEK_SET );
        }
        m_hFile = hFile;
        m_strLineSep = aLineSep;
        bRet = TRUE;
    } while ( 0 );

    return bRet;
}

BOOL CFile::Open( CONST WCHAR * aPath , UINT32 aOpenAttr , CONST std::string & aLineSep )
{
    wstring wstrPath = aPath;
    string strPath;
    _WStringToString( wstrPath , strPath , 0 );
    return this->Open( strPath.c_str() , aOpenAttr , aLineSep );
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
