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
#include <WinInet.h>
#include <vector>
#include <string>
#include <curl/curl.h>

#pragma comment( lib , "libcurl.lib" )


namespace CWUtils
{

#ifdef __cplusplus
    extern "C" {
#endif

#define CURL_DEFAULT_GET_TIMEOUT      ( 10 * 1000 )   //In milli-seconds, specify 0 to use curl's default timeout
#define CURL_DEFAULT_POST_TIMEOUT     ( 10 * 1000 )   //In milli-seconds, specify 0 to use curl's default timeout
#define CURL_DEFAULT_UPLOAD_TIMEOUT   ( 0 )           //In milli-seconds, specify 0 to use curl's default timeout
#define CURL_DEFAULT_DOWNLOAD_TIMEOUT ( 0 )           //In milli-seconds, specify 0 to use curl's default timeout


typedef struct _CurlResponse
{
    std::vector<std::string> vecHeaders;
    std::string strData;
} CurlResponse;

class CCurl
{
    public : 
        CCurl();
        ~CCurl();

    private :
        CURL * m_pCurl;

    public :
        //aUrl should already be encoded/escaped
        //Get method will finally be formatted to "aUrl + "?" + Escape(aParams)"
        CURLcode Get( IN CONST CHAR * aUrl , IN CONST CHAR * aParams , OUT CurlResponse * aResponse , OPTIONAL IN UINT aTimeout = CURL_DEFAULT_GET_TIMEOUT );
        
        //aUrl should already be encoded/escaped
        //Specify aParamSize to a negative number if aParams is a null-terminated string
        CURLcode Post( IN CONST CHAR * aUrl , IN CONST CHAR * aParams , IN INT aParamSize , OUT CurlResponse * aResponse , IN UINT aTimeout = CURL_DEFAULT_POST_TIMEOUT );
        CURLcode Post( IN CONST CHAR * aUrl , IN CONST CHAR * aParams , OUT CurlResponse * aResponse , IN UINT aTimeout = CURL_DEFAULT_POST_TIMEOUT );
        
        //aUrl should already be encoded/escaped
        CURLcode UploadFile( IN CONST CHAR * aUrl , IN CONST CHAR * aFilePath , OUT CurlResponse * aResponse , IN UINT aTimeout = CURL_DEFAULT_UPLOAD_TIMEOUT );
        CURLcode UploadFile( IN CONST CHAR * aUrl , IN CONST WCHAR * aFilePath , OUT CurlResponse * aResponse , IN UINT aTimeout = CURL_DEFAULT_UPLOAD_TIMEOUT );

    private :
        static size_t OnHdrReceive( CHAR * aData , size_t aHdrBlockSize , size_t aHdrBlockCount , void * aResponse );
        static size_t OnDataReceive( CHAR * aData , size_t aDataBlockSize , size_t aDataBlockCount , void * aResponse );
};

#ifdef __cplusplus
    }
#endif

}   //End of namespace CWUtils