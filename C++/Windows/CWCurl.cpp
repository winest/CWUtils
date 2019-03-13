#include "stdafx.h"
#include "CWCurl.h"
using std::vector;
using std::string;

// #include "_GenerateTmh.h"
// #include "CCurl.tmh"

namespace CWUtils
{
#ifdef __cplusplus
extern "C" {
#endif



BOOL _GetFileContent( IN CONST WCHAR * aFullPath, IN OUT std::string & aContent )
{
    HANDLE hFile =
        CreateFileW( aFullPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
    if ( INVALID_HANDLE_VALUE != hFile )
    {
        LARGE_INTEGER size;
        if ( 0 != GetFileSizeEx( hFile, &size ) )
        {
            aContent.reserve( (SIZE_T)size.QuadPart );
        }

        BYTE byBuf[8192];
        DWORD dwRead = 0;
        while ( FALSE != ReadFile( hFile, byBuf, sizeof( byBuf ), &dwRead, NULL ) && 0 < dwRead )
        {
            aContent.append( (CONST CHAR *)byBuf, dwRead );
        }

        CloseHandle( hFile );
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}



CCurl::CCurl()
{
    m_pCurl = curl_easy_init();
    if ( NULL == m_pCurl )
    {
        wprintf_s( L"curl_easy_init() fail\n" );
    }
}

CCurl::~CCurl()
{
    if ( NULL != m_pCurl )
    {
        curl_easy_cleanup( m_pCurl );
    }
}


CURLcode CCurl::Get( IN CONST CHAR * aUrl,
                     IN CONST CHAR * aParams,
                     OUT CurlResponse * aResponse,
                     OPTIONAL IN UINT aTimeout )
{
    wprintf_s( L"aUrl=%hs, aGetFileds=%hs, aResponse=0x%p, aTimeout=%u\n", aUrl, aParams, aResponse, aTimeout );
    _ASSERT( aUrl );

    string strRequest = aUrl;
    if ( NULL != aParams )
    {
        CHAR * szGetFields = curl_easy_escape( m_pCurl, aParams, (INT)strlen( aParams ) );
        if ( NULL != szGetFields )
        {
            strRequest += "?";
            strRequest += szGetFields;
            curl_free( szGetFields );
        }
    }

    if ( NULL != aResponse )
    {
        aResponse->strData.clear();
    }
    curl_easy_reset( m_pCurl );

    curl_easy_setopt( m_pCurl, CURLOPT_URL, strRequest.c_str() );
    curl_easy_setopt( m_pCurl, CURLOPT_TIMEOUT_MS, aTimeout );
    if ( NULL != aResponse )
    {
        curl_easy_setopt( m_pCurl, CURLOPT_WRITEHEADER, aResponse );
        curl_easy_setopt( m_pCurl, CURLOPT_HEADERFUNCTION, CCurl::OnHdrReceive );

        curl_easy_setopt( m_pCurl, CURLOPT_WRITEDATA, aResponse );
        curl_easy_setopt( m_pCurl, CURLOPT_WRITEFUNCTION, CCurl::OnDataReceive );
    }

    //Perform the request
    CURLcode curlResponse = CURL_LAST;
    curlResponse = curl_easy_perform( m_pCurl );
    if ( CURLE_OK != curlResponse )
    {
        wprintf_s( L"curl_easy_perform() error in GET method. curlResponse=%d\n", curlResponse );
    }
    return curlResponse;
}


CURLcode CCurl::Post( IN CONST CHAR * aUrl,
                      IN CONST CHAR * aParams,
                      IN INT aParamSize,
                      OUT CurlResponse * aResponse,
                      OPTIONAL IN UINT aTimeout )
{
    wprintf_s( L"aUrl=%hs, aParams=%hs, aParamSize=%d, aResponse=0x%p, aTimeout=%u\n", aUrl, aParams, aParamSize,
               aResponse, aTimeout );
    _ASSERT( aUrl );
    if ( NULL != aResponse )
    {
        aResponse->strData.clear();
    }

    curl_easy_reset( m_pCurl );
    curl_easy_setopt( m_pCurl, CURLOPT_URL, aUrl );
    curl_easy_setopt( m_pCurl, CURLOPT_TIMEOUT_MS, aTimeout );
    curl_easy_setopt( m_pCurl, CURLOPT_POST, TRUE );
    if ( NULL != aParams && 0 < aParamSize )
    {
        curl_easy_setopt( m_pCurl, CURLOPT_POSTFIELDS, aParams );
        curl_easy_setopt( m_pCurl, CURLOPT_POSTFIELDSIZE, aParamSize );
    }
    if ( NULL != aResponse )
    {
        curl_easy_setopt( m_pCurl, CURLOPT_WRITEHEADER, aResponse );
        curl_easy_setopt( m_pCurl, CURLOPT_HEADERFUNCTION, CCurl::OnHdrReceive );

        curl_easy_setopt( m_pCurl, CURLOPT_WRITEDATA, aResponse );
        curl_easy_setopt( m_pCurl, CURLOPT_WRITEFUNCTION, CCurl::OnDataReceive );
    }

    //Perform the request
    CURLcode curlResponse = CURL_LAST;
    curlResponse = curl_easy_perform( m_pCurl );
    if ( CURLE_OK != curlResponse )
    {
        wprintf_s( L"curl_easy_perform() error in POST method. curlResponse=%d\n", curlResponse );
        if ( NULL != aResponse )
        {
            aResponse->strData.clear();
        }
    }

    return curlResponse;
}

CURLcode CCurl::Post( IN CONST CHAR * aUrl,
                      IN CONST CHAR * aParams,
                      OUT CurlResponse * aResponse,
                      OPTIONAL IN UINT aTimeout )
{
    return this->Post( aUrl, aParams, -1, aResponse, aTimeout );
}


CURLcode CCurl::UploadFile( IN CONST CHAR * aUrl,
                            IN CONST CHAR * aFilePath,
                            OUT CurlResponse * aResponse,
                            OPTIONAL IN UINT aTimeout )
{
    WCHAR wzFilePath[INTERNET_MAX_URL_LENGTH] = { 0 };
    if ( 0 < MultiByteToWideChar( CP_ACP, 0, aFilePath, -1, wzFilePath, _countof( wzFilePath ) ) )
    {
        return this->UploadFile( aUrl, wzFilePath, aResponse, aTimeout );
    }
    else
    {
        wprintf_s( L"MultiByteToWideChar() failed. GetLastError()=%!WINERROR!\n", GetLastError() );
        return CURLE_BAD_FUNCTION_ARGUMENT;
    }
}

CURLcode CCurl::UploadFile( IN CONST CHAR * aUrl,
                            IN CONST WCHAR * aFilePath,
                            OUT CurlResponse * aResponse,
                            OPTIONAL IN UINT aTimeout )
{
    wprintf_s( L"aUrl=%hs, aFilePath=%ws, aResponse=0x%p, aTimeout=%u\n", aUrl, aFilePath, aResponse, aTimeout );
    _ASSERT( aUrl && aFilePath );

    curl_easy_reset( m_pCurl );
    curl_easy_setopt( m_pCurl, CURLOPT_URL, aUrl );
    curl_easy_setopt( m_pCurl, CURLOPT_TIMEOUT_MS, aTimeout );

    std::string strContent;
    if ( FALSE == _GetFileContent( aFilePath, strContent ) )
    {
        wprintf_s( L"Failed to get content of %ws. GetLastError()=%!WINERROR!\n", aFilePath, GetLastError() );
        return CURLE_READ_ERROR;
    }

    curl_httppost *post = NULL, *last = NULL;
    curl_formadd( &post, &last, CURLFORM_COPYNAME, "file", CURLFORM_BUFFER, "file", CURLFORM_BUFFERLENGTH,
                  strContent.size(), CURLFORM_BUFFERPTR, strContent.c_str(), CURLFORM_END );
    curl_easy_setopt( m_pCurl, CURLOPT_HTTPPOST, post );
    if ( NULL != aResponse )
    {
        curl_easy_setopt( m_pCurl, CURLOPT_WRITEHEADER, aResponse );
        curl_easy_setopt( m_pCurl, CURLOPT_HEADERFUNCTION, CCurl::OnHdrReceive );

        curl_easy_setopt( m_pCurl, CURLOPT_WRITEDATA, aResponse );
        curl_easy_setopt( m_pCurl, CURLOPT_WRITEFUNCTION, CCurl::OnDataReceive );
    }

    //Perform the request
    CURLcode curlResponse = CURL_LAST;
    curlResponse = curl_easy_perform( m_pCurl );
    if ( CURLE_OK != curlResponse )
    {
        wprintf_s( L"curl_easy_perform() error when uploading file. curlResponse=%d\n", curlResponse );
        if ( NULL != aResponse )
        {
            aResponse->strData.clear();
        }
    }

    return curlResponse;
}

size_t CCurl::OnHdrReceive( CHAR * aData, size_t aHdrBlockSize, size_t aHdrBlockCount, void * aResponse )
{
    CurlResponse * response = (CurlResponse *)aResponse;
    size_t sizeResponse = ( aHdrBlockSize * aHdrBlockCount );
    string hdr( aData, sizeResponse );
    response->vecHeaders.push_back( hdr );

    return sizeResponse;    //Return how many bytes are processed. This should return how many size we received or CURL_WRITEFUNC_PAUSE
}

size_t CCurl::OnDataReceive( CHAR * aData, size_t aDataBlockCount, size_t aDataBlockSize, void * aResponse )
{
    CurlResponse * response = (CurlResponse *)aResponse;
    size_t sizeResponse = ( aDataBlockSize * aDataBlockCount );
    response->strData.append( aData, sizeResponse );

    return sizeResponse;    //Return how many bytes are processed. This should return how many size we received or CURL_WRITEFUNC_PAUSE
}


#ifdef __cplusplus
}
#endif

}    //End of namespace CWUtils