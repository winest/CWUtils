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

#include "WinDef.h"
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include <errno.h>
#include <string>
#include <vector>
#include <algorithm>

namespace CWUtils
{

#ifdef __cplusplus
    extern "C" {
#endif



typedef enum _StringType    //Lower index generally has higher restriction
{
    STRING_TYPE_START = 0 ,
    STRING_TYPE_NUM  ,          //isdigit()
    STRING_TYPE_ALPHA ,         //isalpha()
    STRING_TYPE_ALPHANUM ,      //isalnum()
    STRING_TYPE_BASE64 ,        
    STRING_TYPE_PRINTABLE ,     //isprint()
    STRING_TYPE_READABLE ,      //isprint() + isspace()
    STRING_TYPE_BINARY ,
    STRING_TYPE_END
} StringType;

BOOL StringToWString( IN CONST std::string & aString , OUT std::wstring & aWString , DWORD aCodePage = 0 );
BOOL WStringToString( IN CONST std::wstring & aWString , OUT std::string & aString , DWORD aCodePage = 0 );

VOID SplitStringA( CONST std::string & aSrcString , std::vector<std::string> & aOutput , CONST CHAR * aDelimiter );
VOID SplitStringW( CONST std::wstring & aSrcString , std::vector<std::wstring> & aOutput , CONST WCHAR * aDelimiter );
BOOL CDECL FormatStringA( OUT std::string & aOutString , IN CONST CHAR * aFormat , ... );
BOOL CDECL FormatStringW( OUT std::wstring & aOutString , IN CONST WCHAR * aFormat , ... );
VOID ToLower( IN OUT std::string & aString );
VOID ToUpper( IN OUT std::string & aString );
VOID ToHexString( CONST UCHAR * aInput , SIZE_T aInputSize , std::string & aOutput , CONST CHAR * aSplitter );
VOID ToHexDump( CONST UCHAR * aInput , SIZE_T aInputSize , std::string & aOutput , CONST CHAR * aSplitter , size_t aBytesPerLine );

INT ToInt( CONST CHAR * aStr , SIZE_T aStrLen );
INT64 ToInt64( CONST CHAR * aStr , SIZE_T aStrLen );

VOID ReplaceStringW( IN OUT std::wstring & aString , IN CONST WCHAR * aOldString , IN CONST WCHAR * aNewString );
BOOL UnEscapeStringW( IN CONST WCHAR * aEscapedStr , IN SIZE_T aEscapedStrLen , OUT std::wstring & aUnEscapedStr );
VOID TrimStringA( IN OUT std::string & aString , CONST CHAR * aTrimChars );

BOOL IsBase64SpecialCharA( CHAR aChar );
BOOL IsBase64CharA( CHAR aChar );
BOOL IsBase64StringA( CONST CHAR * aStr , SIZE_T aStrLen );
StringType GetStringTypeA( CONST CHAR * aStr , SIZE_T aStrLen );
BOOL WildcardMatchW( IN CONST WCHAR * aString , IN CONST WCHAR * aWildcardPattern );

CONST WCHAR * GetPathBaseNameW( IN CONST WCHAR * aFullPath );

//Encode full URL include parameters after "?"
BOOL EncodeUrlA( IN CONST std::string & aOldUrl , OUT std::string & aNewUrl );

class CUrlStringPtr
{
    public :
        CUrlStringPtr() { Clean(); }
        CUrlStringPtr( CONST CHAR * pUrl ) { Clean(); this->SetUrl( pUrl , strlen(pUrl) ); }
        CUrlStringPtr( CONST CHAR * pUrl , SIZE_T uLen ) { Clean(); this->SetUrl( pUrl , uLen ); }
        virtual ~CUrlStringPtr() { Clean(); }

    public :
        BOOL SetUrl( CONST CHAR * pUrl );
        BOOL SetUrl( CONST CHAR * pUrl , SIZE_T uLen );
        CONST CHAR * GetUrl() { return m_pUrl; }
        SIZE_T GetUrlLen() { return m_uUrlLen; }  //URL length without terminating null

        ULONG GetScheme() { return m_ulScheme; }
        VOID GetScheme( std::string & aScheme ) { if ( m_uSchemeLen ) { aScheme.assign( &m_pUrl[m_uSchemeOffset] , m_uSchemeLen ); } }
        SIZE_T GetSchemeOffset() { return m_uSchemeOffset; }
        SIZE_T GetSchemeLen() { return m_uSchemeLen; }

        VOID GetHost( std::string & aHost ) { aHost.assign( &m_pUrl[m_uHostOffset] , m_uHostLen ); }
        SIZE_T GetHostOffset() { return m_uHostOffset; }
        SIZE_T GetHostLen() { return m_uHostLen; }        

        ULONG GetPort()   { return m_ulPort; }
        VOID GetPort( std::string & aPort ) { if ( m_uPortLen ) { aPort.assign( &m_pUrl[m_uPortOffset] , m_uPortLen ); } }
        SIZE_T GetPortOffset() { return m_uPortOffset; }
        SIZE_T GetPortLen() { return m_uPortLen; }

        VOID GetUri( std::string & aUri )  { if ( m_uUrlLen ) { aUri.assign( &m_pUrl[m_uUriOffset] , m_uUriLen ); } }
        SIZE_T GetUriOffset() { return m_uUriOffset; }
        SIZE_T GetUriLen() { return m_uUriLen; }

        VOID GetParams( std::string & aParams )  { if ( m_uParamsLen ) { aParams.assign( &m_pUrl[m_uParamsOffset] , m_uParamsLen ); } }
        SIZE_T GetParamsOffset() { return m_uParamsOffset; }
        SIZE_T GetParamsLen() { return m_uParamsLen; }

        VOID GetFragment( std::string & aFragment )  { if ( m_uFragLen ) { aFragment.assign( &m_pUrl[m_uFragOffset] , m_uFragLen ); } }
        SIZE_T GetFragmentOffset() { return m_uFragOffset; }
        SIZE_T GetFragmentLen() { return m_uFragLen; }

    private :
        VOID Clean();

    private :
        CONST CHAR * m_pUrl;
        SIZE_T m_uUrlLen;   //URL length without terminating null
        
        ULONG m_ulScheme;
        SIZE_T m_uSchemeOffset , m_uSchemeLen;
        SIZE_T m_uHostOffset , m_uHostLen;
        ULONG m_ulPort;
        SIZE_T m_uPortOffset , m_uPortLen;
        SIZE_T m_uUriOffset , m_uUriLen;
        SIZE_T m_uParamsOffset , m_uParamsLen;
        SIZE_T m_uFragOffset , m_uFragLen;
};

#ifdef __cplusplus
    }
#endif

}   //End of namespace CWUtils
