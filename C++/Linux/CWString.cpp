#include "stdafx.h"
#include "CWString.h"
using namespace std;

namespace CWUtils
{

#ifdef __cplusplus
    extern "C" {
#endif

BOOL StringToWString( IN CONST std::string & aString , OUT std::wstring & aWString , DWORD aCodePage )
{
    UNREFERENCED_PARAMETER( aCodePage ); //Linux will use the locale in setlocale( LC_CTYPE , NULL )
    
    BOOL bRet = FALSE;
    aWString.clear();

    WCHAR wzBuf[4096];
    size_t uBufSize = _countof( wzBuf );
    size_t uBufCopied = mbstowcs( wzBuf , aString.c_str() , uBufSize );
    if ( uBufCopied < uBufSize )
    {
        aWString.assign( wzBuf , uBufCopied );
        bRet = TRUE;
    }
    else
    {
        uBufSize = aString.length() * 2;
        WCHAR * wzNewBuf = new (std::nothrow) WCHAR[uBufSize];
        if ( NULL != wzNewBuf )
        {
            uBufCopied = mbstowcs( wzNewBuf , aString.c_str() , uBufSize );
            if ( uBufCopied < uBufSize )
            {
                aWString.assign( wzNewBuf , uBufCopied );
                bRet = TRUE;
            }
            delete [] wzNewBuf;
        }
    }
    return bRet;
}

BOOL WStringToString( IN CONST std::wstring & aWString , OUT std::string & aString , DWORD aCodePage )
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

VOID SplitStringA( CONST string & aSrcString , vector<string> & aOutput , CONST CHAR * aDelimiter )
{    
    string::size_type lastPos = aSrcString.find_first_not_of( aDelimiter , 0 );   //Skip delimiters at beginning    
    string::size_type pos     = aSrcString.find_first_of( aDelimiter , lastPos ); //Find first "non-delimiter"

    while ( string::npos != pos || string::npos != lastPos )
    {        
        aOutput.push_back( aSrcString.substr(lastPos , pos - lastPos) );   //Found a token, add it to the vector        
        lastPos = aSrcString.find_first_not_of( aDelimiter , pos );        //Skip delimiters. Note the "not_of"        
        pos = aSrcString.find_first_of( aDelimiter , lastPos );            //Find next "non-delimiter"
    }
}

VOID SplitStringW( CONST wstring & aSrcString , vector<wstring> & aOutput , CONST WCHAR * aDelimiter )
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

BOOL CDECL FormatStringA( OUT string & aOutString , IN CONST CHAR * aFormat , ... )
{
    BOOL bRet = FALSE;
    aOutString.clear();
    CHAR szBuf[4096];
    CHAR * pBuf = szBuf;
    SIZE_T uBufLen = _countof(szBuf);
    INT nCopiedLen = 0;

    if ( NULL != aFormat )
    {
        va_list args;
        va_start( args , aFormat );

        do
        {
            nCopiedLen = vsnprintf( pBuf , uBufLen , aFormat , args ) + 1;    //Get formatted string length and adding one for null-terminator

            if ( 0 < nCopiedLen && nCopiedLen < uBufLen )
            {
                aOutString.assign( pBuf , (SIZE_T)nCopiedLen );
                break;
            }

            if ( pBuf != szBuf )
            {
                delete [] pBuf;
            }
            uBufLen = uBufLen * 2;
            pBuf = new (std::nothrow) CHAR[uBufLen];
        } while ( pBuf );

        if ( pBuf != szBuf )
        {
            delete [] pBuf;
        }

        va_end( args );
    }
    else
    {
        errno = EINVAL;
    }

    return bRet; 
}

BOOL CDECL FormatStringW( OUT wstring & aOutString , IN CONST WCHAR * aFormat , ... )
{
    BOOL bRet = FALSE;
    aOutString.clear();
    WCHAR wzBuf[4096];
    WCHAR * pBuf = wzBuf;
    SIZE_T uBufLen = _countof(wzBuf);
    INT nCopiedLen = 0;

    if ( NULL != aFormat )
    {
        va_list args;
        va_start( args , aFormat );

        do
        {
            nCopiedLen = vswprintf( pBuf , uBufLen , aFormat , args ) + 1;    //Get formatted string length and adding one for null-terminator

            if ( 0 < nCopiedLen && nCopiedLen < uBufLen )
            {
                aOutString.assign( pBuf , (SIZE_T)nCopiedLen );
                break;
            }

            if ( pBuf != wzBuf )
            {
                delete [] pBuf;
            }
            uBufLen = uBufLen * 2;
            pBuf = new (std::nothrow) WCHAR[uBufLen];
        } while ( pBuf );

        if ( pBuf != wzBuf )
        {
            delete [] pBuf;
        }

        va_end( args );
    }
    else
    {
        errno = EINVAL;
    }

    return bRet; 
}

VOID ToLower( IN OUT std::string & aString )
{
    std::transform( aString.begin() , aString.end() , aString.begin() , (int (*)(int))std::tolower );
}

VOID ToUpper( IN OUT std::string & aString )
{
    std::transform( aString.begin() , aString.end() , aString.begin() , (int (*)(int))std::toupper );
}

VOID ToHexString( CONST UCHAR * aInput , SIZE_T aInputSize , string & aOutput , CONST CHAR * aSplitter )
{
    aOutput.clear();
    UINT32 aryTable[256];
    for ( size_t i = 0 ; i < 256 ; i++ )
    {
        CHAR szChar[3];
        snprintf( szChar , sizeof(szChar) , "%02zX" , i );
        aryTable[i] = ( (UINT32)szChar[0] ) + ( (UINT32)szChar[1] << 16 );
    }

    size_t uSplitterLen = strlen( aSplitter );
    aOutput.reserve( aInputSize * (2 + uSplitterLen) );
    for ( size_t i = 0 ; i < aInputSize ; i++ )
    {
        INT nCurr = i * ( 2 + uSplitterLen );
        UINT32 uVal = aryTable[aInput[i]];
        aOutput.push_back( (CHAR)uVal );
        aOutput.push_back( (CHAR)( uVal >> 16 ) );

        aOutput.append( aSplitter );
    }
    if ( aOutput.length() >= uSplitterLen )
    {
        aOutput.erase( aOutput.length() - uSplitterLen );
    }
}


VOID ToHexDump( CONST UCHAR * aInput , SIZE_T aInputSize , string & aOutput , CONST CHAR * aSplitter , size_t aBytesPerLine )
{
    aOutput.clear();
    UINT32 aryTable[256];
    for ( size_t i = 0 ; i < 256 ; i++ )
    {
        CHAR szChar[3];
        snprintf( szChar , sizeof(szChar) , "%02zX" , i );
        aryTable[i] = ( (UINT32)szChar[0] ) + ( (UINT32)szChar[1] << 16 );
    }
    size_t uSplitterLen = strlen( aSplitter );
    string strLineSep = "\r\n";

    //<8 bytes address><aSplitter><aBytesPerLine's hex with (aBytesPerLine - 1)'s space><aSplitter><aBytesPerLine's ascii><2 bytes line separator>
    size_t uOneLineSize = 8 + uSplitterLen * 2 + aBytesPerLine * 4 - 1 + strLineSep.length();
    aOutput.reserve( (size_t)ceil((double)aInputSize/(double)aBytesPerLine) * uOneLineSize );
    for ( int i = 0 ; i < aInputSize ; i += aBytesPerLine )
    {
        CHAR szAddr[8+16];  //Assume aSplitter has at most 15 bytes
        snprintf( szAddr , sizeof(szAddr) , "%08X%s" , i , aSplitter );
        aOutput.append( szAddr );
        for ( int j = 0 ; j < aBytesPerLine ; j++ )
        {
            int nCurr = i + j;
            if ( nCurr >= aInputSize )
            {
                aOutput.append( (aBytesPerLine - j) * 3 - 1 , ' ' );
                break;
            }
            UINT32 uVal = aryTable[aInput[nCurr]];
            aOutput.push_back( (CHAR)uVal );
            aOutput.push_back( (CHAR)(uVal >> 16) );
            if ( j + 1 < aBytesPerLine )
            {
                aOutput.push_back( ' ' );
            }
        }

        aOutput.append( aSplitter );

        for ( int j = 0 ; j < aBytesPerLine ; j++ )
        {
            int nCurr = i + j;
            if ( nCurr >= aInputSize )
            {
                aOutput.append( aBytesPerLine - j , ' ' );
                break;
            }
            aOutput.push_back( iscntrl(aInput[nCurr]&0xFF) ? '.' : (char)aInput[nCurr] );
        }
        aOutput.append( strLineSep );
    }

    if ( aOutput.length() >= strLineSep.length() )
    {
        aOutput.erase( aOutput.length() - strLineSep.length() );
    }
}

VOID ReplaceStringW( IN OUT std::wstring & aString , IN CONST WCHAR * aOldString , IN CONST WCHAR * aNewString )
{
    if ( NULL == aOldString || NULL == aNewString )
        return;

    std::wstring::size_type sizeOld = ::wcslen( aOldString );
    std::wstring::size_type sizeNew = ::wcslen( aNewString );

    std::wstring::size_type sizeNextIndex = 0;

    while ( sizeNextIndex + sizeOld <= aString.size() )
    {
        sizeNextIndex = aString.find( aOldString , sizeNextIndex );
        if ( sizeNextIndex == std::wstring::npos)
        {
            break;
        }
        aString.replace(sizeNextIndex, sizeOld, aNewString);
        sizeNextIndex += sizeNew;
    }
}

BOOL UnEscapeStringW( IN CONST WCHAR * aEscapedStr , IN SIZE_T aEscapedStrLen , OUT std::wstring & aUnEscapedStr )
{
    aUnEscapedStr.clear();
    SIZE_T uFlushedLen = 0;
    BOOL bEscaping = FALSE;
    for ( SIZE_T i = 0 ; i < aEscapedStrLen ; i++ )
    {
        if ( FALSE == bEscaping )
        {
            if ( L'\\' == aEscapedStr[i] )
            {
                bEscaping = TRUE;
            }
        }
        else
        {
            switch ( aEscapedStr[i] )
            {
                case L'r' :
                    aUnEscapedStr.append( &aEscapedStr[uFlushedLen] , i - uFlushedLen - 1 );
                    aUnEscapedStr.append( 1 , '\r' );
                    break;
                case L'n' :
                    aUnEscapedStr.append( &aEscapedStr[uFlushedLen] , i - uFlushedLen - 1 );
                    aUnEscapedStr.append( 1 , '\n' );
                    break;
                case L't' :
                    aUnEscapedStr.append( &aEscapedStr[uFlushedLen] , i - uFlushedLen - 1 );
                    aUnEscapedStr.append( 1 , '\t' );
                    break;
                case L'\\' :
                    aUnEscapedStr.append( &aEscapedStr[uFlushedLen] , i - uFlushedLen - 1 );
                    aUnEscapedStr.append( 1 , '\\' );
                    break;
                case L'0' :
                    aUnEscapedStr.append( &aEscapedStr[uFlushedLen] , i - uFlushedLen - 1 );
                    aUnEscapedStr.append( 1 , '\0' );
                    break;
                default :
                    goto exit;
            }
            uFlushedLen = i + 1;
            bEscaping = FALSE;
        }
    }
    if ( 0 < aEscapedStrLen )
    {
        aUnEscapedStr.append( &aEscapedStr[uFlushedLen] , aEscapedStrLen - uFlushedLen );
    }

exit :
    return ( FALSE == bEscaping ) ? TRUE : FALSE;
}

VOID TrimStringA( IN OUT std::string & aString , CONST CHAR * aTrimChars )
{
    SIZE_T pos = aString.find_last_not_of( aTrimChars );
    if ( std::string::npos != pos )
    {
        aString.erase( pos + 1 );
    }
    pos = aString.find_first_not_of( aTrimChars );
    if ( std::string::npos != pos )
    {
        aString.erase( 0 , pos );
    }
}


BOOL IsBase64SpecialCharA( CHAR aChar )
{
    return ( '+' == aChar ) || ( '/' == aChar ) || ( '=' == aChar );
}

BOOL IsBase64CharA( CHAR aChar )
{
    return ( isdigit(aChar) || isalpha(aChar) || IsBase64SpecialCharA(aChar) );
}

BOOL IsBase64StringA( CONST CHAR * aStr , SIZE_T aStrLen )
{
    BOOL bRet = FALSE;
    SIZE_T uPosEnd = aStrLen;

    //aStrLen should be the multiple of 4
    if ( 4 > aStrLen || 0 != aStrLen % 4 )
    {
        goto exit;
    }

    //Find ending '=', at most 2 bytes is possible
    for ( SIZE_T i = 0 ; i < 2 ; i++ )
    {
        if ( '=' == aStr[uPosEnd-1] ) 
        {
            uPosEnd--;
        }
    }

    //Check other characters
    for ( SIZE_T i = 0 ; i < uPosEnd ; i++ )
    {
        if ( FALSE == IsBase64CharA(aStr[i]) || '=' == aStr[i] )
        {
            goto exit;
        }
    }
    bRet = TRUE;

exit :
    return bRet;
}


StringType GetStringTypeA( CONST CHAR * aStr , SIZE_T aStrLen )
{
    StringType type = STRING_TYPE_START;

    for ( SIZE_T i = 0 ; i < aStrLen ; i++ )
    {
        INT c = ( aStr[i] & 0xFF );
        switch ( type )
        {
            case STRING_TYPE_NUM :
            {
                if ( false == isdigit(c) )
                {
                    if ( isalpha(c) )
                    {
                        type = STRING_TYPE_ALPHANUM;
                    }
                    else if ( IsBase64SpecialCharA(c) )
                    {
                        type = STRING_TYPE_BASE64;
                    }
                    else if ( isprint(c) )
                    {
                        type = STRING_TYPE_PRINTABLE;
                    }
                    else if ( isspace(c) )
                    {
                        type = STRING_TYPE_READABLE;
                    }
                    else
                    {
                        type = STRING_TYPE_BINARY;
                        goto exit;
                    }
                }
                break;
            }
            case STRING_TYPE_ALPHA :
            {
                if ( false == isalpha(c) )
                {
                    if ( isdigit(c) )
                    {
                        type = STRING_TYPE_ALPHANUM;
                    }
                    else if ( IsBase64SpecialCharA(c) )
                    {
                        type = STRING_TYPE_BASE64;
                    }
                    else if ( isprint(c) )
                    {
                        type = STRING_TYPE_PRINTABLE;
                    }
                    else if ( isspace(c) )
                    {
                        type = STRING_TYPE_READABLE;
                    }
                    else
                    {
                        type = STRING_TYPE_BINARY;
                        goto exit;
                    }
                }
                break;
            }
            case STRING_TYPE_ALPHANUM :
            {
                if ( false == isalnum(c) )
                {
                    if ( IsBase64SpecialCharA(c) )
                    {
                        type = STRING_TYPE_BASE64;
                    }
                    else if ( isprint(c) )
                    {
                        type = STRING_TYPE_PRINTABLE;
                    }
                    else if ( isspace(c) )
                    {
                        type = STRING_TYPE_READABLE;
                    }
                    else
                    {
                        type = STRING_TYPE_BINARY;
                        goto exit;
                    }
                }
                break;
            }
            case STRING_TYPE_BASE64 :
            {
                if ( false == IsBase64CharA(c) )
                {
                    if ( isprint(c) )
                    {
                        type = STRING_TYPE_PRINTABLE;
                    }
                    else if ( isspace(c) )
                    {
                        type = STRING_TYPE_READABLE;
                    }
                    else
                    {
                        type = STRING_TYPE_BINARY;
                        goto exit;
                    }
                }
                break;
            }
            case STRING_TYPE_PRINTABLE :
            {
                if ( false == isprint(c) )
                {
                    if ( isspace(c) )
                    {
                        type = STRING_TYPE_READABLE;
                    }
                    else
                    {
                        type = STRING_TYPE_BINARY;
                        goto exit;
                    }
                }
                break;
            }
            case STRING_TYPE_READABLE :
            {
                if ( false == isprint(c) && false == isspace(c) )
                {
                    type = STRING_TYPE_BINARY;
                    goto exit;
                }
                break;
            }
            case STRING_TYPE_START :
            {
                if ( isdigit(c) )
                {
                    type = STRING_TYPE_NUM;
                }
                else if ( isalpha(c) )
                {
                    type = STRING_TYPE_ALPHA;
                }
                else if ( IsBase64SpecialCharA(c) )
                {
                    type = STRING_TYPE_BASE64;
                }
                else if ( isprint(c) )
                {
                    type = STRING_TYPE_PRINTABLE;
                }
                else if ( isspace(c) )
                {
                    type = STRING_TYPE_READABLE;
                }
                else
                {
                    type = STRING_TYPE_BINARY;
                    goto exit;
                }
                break;
            }
            default :
            {
                type = STRING_TYPE_BINARY;
                goto exit;
            }
        }
    }

    //Special handling for base64 to check it follows the rule
    if ( STRING_TYPE_BASE64 == type && FALSE == IsBase64StringA(aStr , aStrLen) )
    {
        type = STRING_TYPE_PRINTABLE;
    }

exit :
    return type;
}

BOOL WildcardMatchW( IN CONST WCHAR * aString , IN CONST WCHAR * aWildcardPattern )
{
    enum State {
        Exact ,          //Exact match
        Any ,            //?
        AnyRepeat       //*
    } state = Exact;

    CONST WCHAR * wzTmp = NULL;

    BOOL bMatch = TRUE;

    while ( bMatch && *aWildcardPattern )
    {
        switch ( *aWildcardPattern )
        {
            case L'*' :
                state = AnyRepeat;
                wzTmp = aWildcardPattern + 1;
                break;
            case L'?' :
                state = Any;
                break;
            default :
                state = Exact;
                break;
        }        

        if ( L'\0' == *aString )
            break;

        switch ( state )
        {
            case Exact :
                bMatch = ( *aString == *aWildcardPattern );
                aString++;
                aWildcardPattern++;
                break;
            case Any :
                bMatch = TRUE;
                aString++;
                aWildcardPattern++;
                break;
            case AnyRepeat :
                bMatch = TRUE;
                aString++;
                if ( *aString == *wzTmp )
                {
                    aWildcardPattern++;
                }
                break;
        }
    }

    switch ( state )
    {
        case AnyRepeat :
            return ( *aString == *wzTmp );
        case Any :
            return ( *aString == *aWildcardPattern );
        default :
            return ( bMatch && ( *aString == *aWildcardPattern ) );
    }
}


CONST WCHAR * GetPathBaseNameW( IN CONST WCHAR * aFullPath )
{
    CONST WCHAR * wzBase = wcsrchr( aFullPath , L'\\' );
    return ( wzBase ) ? ( wzBase + 1 ) : aFullPath;
}


//Encode full URL include parameters after "?"
BOOL EncodeUrlA( IN CONST std::string & aOldUrl , OUT std::string & aNewUrl )
{
    CHAR szBuf[INTERNET_MAX_URL_LENGTH] = { 0 };
    INT nLen = 0;
    BOOL bParam = FALSE;
    for ( SIZE_T i = 0 ; i < aOldUrl.length() && nLen < INTERNET_MAX_PATH_LENGTH ; i++ )
    {
        if ( isalnum( (UCHAR)aOldUrl[i] ) )
        {
            nLen += snprintf( &szBuf[nLen] , 2 , "%c" , aOldUrl[i] );
        }        
        else if ( '/' == aOldUrl[i] || '.' == aOldUrl[i] || ':' == aOldUrl[i] )
        {
            nLen += snprintf( &szBuf[nLen] , 4 , (bParam) ? "%%%02X" : "%c" , aOldUrl[i] );
        }
        else if ( '?' == aOldUrl[i] )
        {
            nLen += snprintf( &szBuf[nLen] , 4 , (bParam) ? "%%%02X" : "%c" , aOldUrl[i] );
            bParam = TRUE;
        }
        else if ( '=' == aOldUrl[i] )
        {
            nLen += snprintf( &szBuf[nLen] , 4 , (bParam) ? "%c" : "%%%02X" , aOldUrl[i] );
            bParam = TRUE;
        }
        else
        {
            nLen += snprintf( &szBuf[nLen] , 4 , "%%%02X" , (UCHAR)aOldUrl[i] );
        }
    }
    if ( INTERNET_MAX_PATH_LENGTH <= nLen )
    {
        aNewUrl.clear();
        return FALSE;
    }
    else
    {
        aNewUrl.assign( szBuf , nLen );
        return TRUE;
    }
}





VOID CUrlStringPtr::Clean()
{
    m_pUrl = NULL;
    m_uUrlLen = 0;

    m_ulScheme = INTERNET_SCHEME_HTTP;
    m_uSchemeOffset = 0;
    m_uSchemeLen = 0;
    m_uHostOffset = 0;
    m_uHostLen = 0;
    m_ulPort = 80;
    m_uPortOffset = 0;
    m_uPortLen = 0;
    m_uUriOffset = 0;
    m_uUriLen = 0;
    m_uParamsOffset = 0;
    m_uParamsLen = 0;
    m_uFragOffset = 0;
    m_uFragLen = 0;
}


BOOL CUrlStringPtr::SetUrl( CONST CHAR * pUrl )
{
    return this->SetUrl( pUrl , strlen(pUrl) );
}

BOOL CUrlStringPtr::SetUrl( CONST CHAR * pUrl , SIZE_T uLen )
{
    if ( NULL == pUrl )
    {
        return FALSE;
    }

    m_pUrl = pUrl;
    m_uUrlLen = uLen;

    //Parse scheme
    CONST CHAR * pHostStart = m_pUrl;
    if ( strncasecmp(m_pUrl , "http://" , sizeof("http://")-1) == 0 )
    {
        pHostStart += sizeof("http://") - 1;
        m_uSchemeLen = sizeof("http") - 1;
    }
    else if ( strncasecmp(m_pUrl , "https://" , sizeof("https://")-1) == 0 )
    {
        pHostStart += sizeof("https://") - 1;
        m_ulScheme = INTERNET_SCHEME_HTTPS;
        m_uSchemeLen = sizeof("https") - 1;
        m_ulPort = 443;
    }
    else {}

    //Parse relative URI
    CONST CHAR * pUri = strchr( pHostStart , '/' );
    if ( NULL == pUri )
    {
        m_uUriOffset = m_uUrlLen;
        pUri = m_pUrl + m_uUriOffset;
    }
    else
    {
        m_uUriOffset = pUri - pUrl;
        m_uUriLen = m_uUrlLen - m_uUriOffset;
    }

    //Parse query parameters
    CONST CHAR * pParams = strchr( pUri , '?' );
    if ( NULL == pParams )
    {
        m_uParamsOffset = m_uUrlLen;
        pParams = m_pUrl + m_uParamsOffset;
    }
    else
    {
        pParams++;  //Ignore the '?'
        m_uParamsOffset = pParams - pUrl;
        m_uParamsLen = m_uUrlLen - m_uParamsOffset;
        m_uUriLen = pParams - pUri - 1;
    }

    //Parse fragment
    CONST CHAR * pFrag = strchr( pParams , '#' );
    if ( NULL == pFrag )
    {
        m_uFragOffset = m_uUrlLen;
        pFrag = m_pUrl + m_uFragOffset;
    }
    else
    {
        pFrag++;  //Ignore the '#'
        m_uFragOffset = pFrag - pUrl;
        m_uFragLen = m_uUrlLen - m_uFragOffset;
        m_uParamsLen = pFrag - pParams - 1;
    }

    //Parse host and port
    m_uHostOffset = pHostStart - m_pUrl;
    m_uHostLen = pUri - pHostStart;
    CONST CHAR * pHostEnd = ( pHostStart != pUri ) ? pUri - 1 : pUri;
    for ( CONST CHAR * pTmp = pHostEnd ; pTmp > pHostStart ; pTmp-- )
    {
        if ( ':' == *pTmp )
        {
            CONST CHAR * pPortStart = pTmp + 1;

            //Care IPv6 address literal, such as [fe80:1234::1]:80
            if ( ! ( '[' == pHostStart[0] && ']' != *(pTmp-1) ) )
            {
                if ( pPortStart < pUri )
                {
                    CHAR * pPortEnd = NULL;
                    m_ulPort = strtoul( pPortStart , &pPortEnd , 10 );
                    m_uPortOffset = pPortStart - m_pUrl;
                    m_uPortLen = pPortEnd - pPortStart;
                }
                m_uHostLen = pTmp - pHostStart;
            }
            break;
        }
    }
    
    return TRUE;
}


#ifdef __cplusplus
    }
#endif

}   //End of namespace CWUtils
