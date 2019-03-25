#include "stdafx.h"
#include "CWString.h"
using namespace std;

namespace CWUtils
{
#ifdef __cplusplus
extern "C" {
#endif

BOOL StringToWString( IN CONST std::string & aString, OUT std::wstring & aWString, DWORD aCodePage )
{
    BOOL bRet = FALSE;
    aWString.clear();

    DWORD dwFlag = ( CP_UTF8 == aCodePage ) ? MB_ERR_INVALID_CHARS : 0;
    WCHAR wzBuf[4096];
    INT nBuf = _countof( wzBuf );
    INT nBufCopied = MultiByteToWideChar( aCodePage, dwFlag, aString.c_str(), (INT)aString.size(), wzBuf, nBuf );
    if ( 0 != nBufCopied )
    {
        aWString.assign( wzBuf, nBufCopied );
        bRet = TRUE;
    }
    else if ( ERROR_INSUFFICIENT_BUFFER == GetLastError() )
    {
        nBuf = MultiByteToWideChar( aCodePage, dwFlag, aString.c_str(), (INT)aString.size(), nullptr, 0 );
        WCHAR * wzNewBuf = new ( std::nothrow ) WCHAR[nBuf];
        if ( nullptr != wzNewBuf )
        {
            nBufCopied = MultiByteToWideChar( aCodePage, dwFlag, aString.c_str(), (INT)aString.size(), wzNewBuf, nBuf );
            if ( 0 != nBufCopied )
            {
                aWString.assign( wzNewBuf, nBufCopied );
                bRet = TRUE;
            }
            delete[] wzNewBuf;
        }
    }
    else
    {
    }
    return bRet;
}

BOOL WStringToString( IN CONST std::wstring & aWString, OUT std::string & aString, DWORD aCodePage )
{
    BOOL bRet = FALSE;
    aString.clear();

#if ( _WIN32_WINNT_VISTA <= _WIN32_WINNT )
    DWORD dwFlag = ( CP_UTF8 == aCodePage ) ? WC_ERR_INVALID_CHARS : 0;
#else
    DWORD dwFlag = 0;
#endif
    CHAR szBuf[4096];
    INT nBuf = _countof( szBuf );
    INT nBufCopied = WideCharToMultiByte( aCodePage, dwFlag, aWString.c_str(), (INT)aWString.length(), szBuf, nBuf,
                                          nullptr, nullptr );
    if ( 0 != nBufCopied )
    {
        aString.assign( szBuf, nBufCopied );
        bRet = TRUE;
    }
    else if ( ERROR_INSUFFICIENT_BUFFER == GetLastError() )
    {
        nBuf = WideCharToMultiByte( aCodePage, dwFlag, aWString.c_str(), (INT)aWString.length(), nullptr, 0, nullptr,
                                    nullptr );
        CHAR * szNewBuf = new ( std::nothrow ) CHAR[nBuf];
        if ( nullptr != szNewBuf )
        {
            nBufCopied = WideCharToMultiByte( aCodePage, dwFlag, aWString.c_str(), (INT)aWString.length(), szNewBuf,
                                              nBuf, nullptr, nullptr );
            if ( 0 != nBufCopied )
            {
                aString.assign( szNewBuf, nBufCopied );
                bRet = TRUE;
            }
            delete[] szNewBuf;
        }
    }
    else
    {
    }
    return bRet;
}

VOID SplitStringA( CONST string & aSrcString, vector<string> & aOutput, CONST CHAR * aDelimiter )
{
    aOutput.clear();
    string::size_type lastPos = aSrcString.find_first_not_of( aDelimiter, 0 );    //Skip delimiters at beginning
    string::size_type pos = aSrcString.find_first_of( aDelimiter, lastPos );      //Find first "non-delimiter"

    while ( string::npos != pos || string::npos != lastPos )
    {
        aOutput.push_back( aSrcString.substr( lastPos, pos - lastPos ) );    //Found a token, add it to the vector
        lastPos = aSrcString.find_first_not_of( aDelimiter, pos );           //Skip delimiters. Note the "not_of"
        pos = aSrcString.find_first_of( aDelimiter, lastPos );               //Find next "non-delimiter"
    }
}

VOID SplitStringW( CONST wstring & aSrcString, vector<wstring> & aOutput, CONST WCHAR * aDelimiter )
{
    aOutput.clear();
    wstring::size_type lastPos = aSrcString.find_first_not_of( aDelimiter, 0 );    //Skip delimiters at beginning
    wstring::size_type pos = aSrcString.find_first_of( aDelimiter, lastPos );      //Find first "non-delimiter"

    while ( wstring::npos != pos || wstring::npos != lastPos )
    {
        aOutput.push_back( aSrcString.substr( lastPos, pos - lastPos ) );    //Found a token, add it to the vector
        lastPos = aSrcString.find_first_not_of( aDelimiter, pos );           //Skip delimiters. Note the "not_of"
        pos = aSrcString.find_first_of( aDelimiter, lastPos );               //Find next "non-delimiter"
    }
}

BOOL CDECL FormatStringA( OUT string & aOutString, IN CONST CHAR * aFormat, ... )
{
    BOOL bRet = FALSE;
    aOutString.clear();
    CHAR szBuf[4096];
    CHAR * pBuf = szBuf;

    if ( nullptr != aFormat )
    {
        va_list args = nullptr;
        va_start( args, aFormat );
        SIZE_T len =
            _vscprintf( aFormat, args ) + 1;    //Get formatted string length and adding one for null-terminator

        if ( _countof( szBuf ) < len )
        {
            pBuf = new ( std::nothrow ) CHAR[len];
        }
        if ( nullptr != pBuf )
        {
            if ( 0 < _vsnprintf_s( pBuf, len, _TRUNCATE, aFormat, args ) )
            {
                aOutString = pBuf;
            }

            if ( pBuf != szBuf )
            {
                delete[] pBuf;
            }
            bRet = TRUE;
        }

        va_end( args );
    }
    else
    {
        SetLastError( ERROR_INVALID_PARAMETER );
    }

    return bRet;
}

BOOL CDECL FormatStringW( OUT wstring & aOutString, IN CONST WCHAR * aFormat, ... )
{
    BOOL bRet = FALSE;
    aOutString.clear();
    WCHAR wzBuf[4096];
    WCHAR * pBuf = wzBuf;

    if ( nullptr != aFormat )
    {
        va_list args = nullptr;
        va_start( args, aFormat );
        SIZE_T len =
            _vscwprintf( aFormat, args ) + 1;    //Get formatted string length and adding one for null-terminator

        if ( _countof( wzBuf ) < len )
        {
            pBuf = new ( std::nothrow ) WCHAR[len];
        }
        if ( nullptr != pBuf )
        {
            if ( 0 < _vsnwprintf_s( pBuf, len, _TRUNCATE, aFormat, args ) )
            {
                aOutString = pBuf;
            }

            if ( pBuf != wzBuf )
            {
                delete[] pBuf;
            }
            bRet = TRUE;
        }

        va_end( args );
    }
    else
    {
        SetLastError( ERROR_INVALID_PARAMETER );
    }

    return bRet;
}

VOID ToLower( IN OUT std::string & aString )
{
    std::transform( aString.begin(), aString.end(), aString.begin(), std::tolower );
}

VOID ToUpper( IN OUT std::string & aString )
{
    std::transform( aString.begin(), aString.end(), aString.begin(), std::toupper );
}

INT ToInt( CONST CHAR * aStr, SIZE_T aStrLen )
{
    INT nRet = 0;
    for ( SIZE_T i = 0; i < aStrLen; ++i )
    {
        nRet = ( nRet * 10 ) + ( aStr[i] - '0' );
    }
    return nRet;
}

INT64 ToInt64( CONST CHAR * aStr, SIZE_T aStrLen )
{
    INT64 nRet = 0;
    for ( SIZE_T i = 0; i < aStrLen; ++i )
    {
        nRet = ( nRet * 10 ) + ( aStr[i] - '0' );
    }
    return nRet;
}

VOID ToHexString( CONST UCHAR * aInput, SIZE_T aInputSize, string & aOutput, CONST CHAR * aSplitter )
{
    aOutput.clear();
    UINT32 aryTable[256];
    for ( size_t i = 0; i < 256; i++ )
    {
        CHAR szChar[3];
        _snprintf_s( szChar, _TRUNCATE, "%02IX", i );
        aryTable[i] = ( (UINT32)szChar[0] ) + ( (UINT32)szChar[1] << 16 );
    }

    size_t uSplitterLen = strlen( aSplitter );
    aOutput.reserve( aInputSize * ( 2 + uSplitterLen ) );
    for ( size_t i = 0; i < aInputSize; i++ )
    {
        //INT nCurr = i * ( 2 + uSplitterLen );
        UINT32 uVal = aryTable[aInput[i]];
        aOutput.push_back( (CHAR)uVal );
        aOutput.push_back( ( CHAR )( uVal >> 16 ) );

        aOutput.append( aSplitter );
    }
    if ( aOutput.length() >= uSplitterLen )
    {
        aOutput.erase( aOutput.length() - uSplitterLen );
    }
}


VOID ToHexDump( CONST UCHAR * aInput,
                SIZE_T aInputSize,
                string & aOutput,
                CONST CHAR * aSplitter,
                size_t aBytesPerLine )
{
    aOutput.clear();
    UINT32 aryTable[256];
    for ( size_t i = 0; i < 256; i++ )
    {
        CHAR szChar[3];
        _snprintf_s( szChar, _TRUNCATE, "%02IX", i );
        aryTable[i] = ( (UINT32)szChar[0] ) + ( (UINT32)szChar[1] << 16 );
    }
    size_t uSplitterLen = strlen( aSplitter );
    string strLineSep = "\r\n";

    //<8 bytes address><aSplitter><aBytesPerLine's hex with (aBytesPerLine - 1)'s space><aSplitter><aBytesPerLine's ascii><2 bytes line separator>
    size_t uOneLineSize = 8 + uSplitterLen * 2 + aBytesPerLine * 4 - 1 + strLineSep.length();
    aOutput.reserve( (size_t)ceil( (double)aInputSize / (double)aBytesPerLine ) * uOneLineSize );
    for ( size_t i = 0; i < aInputSize; i += aBytesPerLine )
    {
        CHAR szAddr[8 + 16];    //Assume aSplitter has at most 15 bytes
        _snprintf_s( szAddr, _TRUNCATE, "%08IX%hs", i, aSplitter );
        aOutput.append( szAddr );
        for ( size_t j = 0; j < aBytesPerLine; j++ )
        {
            size_t nCurr = i + j;
            if ( nCurr >= aInputSize )
            {
                aOutput.append( ( aBytesPerLine - j ) * 3 - 1, ' ' );
                break;
            }
            UINT32 uVal = aryTable[aInput[nCurr]];
            aOutput.push_back( (CHAR)uVal );
            aOutput.push_back( ( CHAR )( uVal >> 16 ) );
            if ( j + 1 < aBytesPerLine )
            {
                aOutput.push_back( ' ' );
            }
        }

        aOutput.append( aSplitter );

        for ( size_t j = 0; j < aBytesPerLine; j++ )
        {
            size_t nCurr = i + j;
            if ( nCurr >= aInputSize )
            {
                aOutput.append( aBytesPerLine - j, ' ' );
                break;
            }

            if ( 0x1F < ( aInput[nCurr] & 0xFF ) && ( aInput[nCurr] & 0xFF ) < 0x7F )
            {
                aOutput.push_back( (char)aInput[nCurr] );
            }
            else
            {
                aOutput.push_back( '.' );
            }
        }
        aOutput.append( strLineSep );
    }

    if ( aOutput.length() >= strLineSep.length() )
    {
        aOutput.erase( aOutput.length() - strLineSep.length() );
    }
}

VOID JoinStringA( CONST IN std::vector<std::string> aVec,
                  std::string & aOutput,
                  CONST CHAR * aSplitter,
                  size_t aStartIndex,
                  size_t aEndIndex )
{
    size_t uEnd = min( aVec.size() - 1, aEndIndex );
    std::stringstream ss;
    for ( size_t i = aStartIndex; i <= uEnd; ++i )
    {
        if ( i != aStartIndex )
        {
            ss << aSplitter;
        }
        ss << aVec[i];
    }
    aOutput = ss.str();
}

VOID ReplaceStringW( IN OUT std::wstring & aString, IN CONST WCHAR * aOldString, IN CONST WCHAR * aNewString )
{
    if ( nullptr == aOldString || nullptr == aNewString )
    {
        return;
    }

    std::wstring::size_type sizeOld = ::wcslen( aOldString );
    std::wstring::size_type sizeNew = ::wcslen( aNewString );

    std::wstring::size_type sizeNextIndex = 0;

    while ( sizeNextIndex + sizeOld <= aString.size() )
    {
        sizeNextIndex = aString.find( aOldString, sizeNextIndex );
        if ( sizeNextIndex == std::wstring::npos )
        {
            break;
        }
        aString.replace( sizeNextIndex, sizeOld, aNewString );
        sizeNextIndex += sizeNew;
    }
}

BOOL UnEscapeStringW( IN CONST WCHAR * aEscapedStr, IN SIZE_T aEscapedStrLen, OUT std::wstring & aUnEscapedStr )
{
    aUnEscapedStr.clear();
    SIZE_T uFlushedLen = 0;
    BOOL bEscaping = FALSE;
    for ( SIZE_T i = 0; i < aEscapedStrLen; i++ )
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
                case L'r':
                    aUnEscapedStr.append( &aEscapedStr[uFlushedLen], i - uFlushedLen - 1 );
                    aUnEscapedStr.append( 1, '\r' );
                    break;
                case L'n':
                    aUnEscapedStr.append( &aEscapedStr[uFlushedLen], i - uFlushedLen - 1 );
                    aUnEscapedStr.append( 1, '\n' );
                    break;
                case L't':
                    aUnEscapedStr.append( &aEscapedStr[uFlushedLen], i - uFlushedLen - 1 );
                    aUnEscapedStr.append( 1, '\t' );
                    break;
                case L'\\':
                    aUnEscapedStr.append( &aEscapedStr[uFlushedLen], i - uFlushedLen - 1 );
                    aUnEscapedStr.append( 1, '\\' );
                    break;
                case L'0':
                    aUnEscapedStr.append( &aEscapedStr[uFlushedLen], i - uFlushedLen - 1 );
                    aUnEscapedStr.append( 1, '\0' );
                    break;
                default:
                    goto exit;
            }
            uFlushedLen = i + 1;
            bEscaping = FALSE;
        }
    }
    if ( 0 < aEscapedStrLen )
    {
        aUnEscapedStr.append( &aEscapedStr[uFlushedLen], aEscapedStrLen - uFlushedLen );
    }

exit:
    return ( FALSE == bEscaping ) ? TRUE : FALSE;
}

VOID TrimStringA( IN OUT std::string & aString, CONST CHAR * aTrimChars )
{
    SIZE_T pos = aString.find_last_not_of( aTrimChars );
    if ( std::string::npos != pos )
    {
        aString.erase( pos + 1 );
    }
    pos = aString.find_first_not_of( aTrimChars );
    if ( std::string::npos != pos )
    {
        aString.erase( 0, pos );
    }
}


BOOL IsBase64SpecialCharA( CHAR aChar )
{
    return ( '+' == aChar ) || ( '/' == aChar ) || ( '=' == aChar );
}

BOOL IsBase64CharA( CHAR aChar )
{
    return ( isdigit( aChar ) || isalpha( aChar ) || IsBase64SpecialCharA( aChar ) );
}

BOOL IsBase64StringA( CONST CHAR * aStr, SIZE_T aStrLen )
{
    BOOL bRet = FALSE;
    SIZE_T uPosEnd = aStrLen;

    //aStrLen should be the multiple of 4
    if ( 4 > aStrLen || 0 != aStrLen % 4 )
    {
        goto exit;
    }

    //Find ending '=', at most 2 bytes is possible
    for ( SIZE_T i = 0; i < 2; i++ )
    {
        if ( '=' == aStr[uPosEnd - 1] )
        {
            uPosEnd--;
        }
    }

    //Check other characters
    for ( SIZE_T i = 0; i < uPosEnd; i++ )
    {
        if ( FALSE == IsBase64CharA( aStr[i] ) || '=' == aStr[i] )
        {
            goto exit;
        }
    }
    bRet = TRUE;

exit:
    return bRet;
}


StringType GetStringTypeA( CONST CHAR * aStr, SIZE_T aStrLen )
{
    StringType type = STRING_TYPE_START;

    for ( SIZE_T i = 0; i < aStrLen; i++ )
    {
        INT c = ( aStr[i] & 0xFF );
        switch ( type )
        {
            case STRING_TYPE_NUM:
            {
                if ( false == isdigit( c ) )
                {
                    if ( isalpha( c ) )
                    {
                        type = STRING_TYPE_ALPHANUM;
                    }
                    else if ( IsBase64SpecialCharA( static_cast<CHAR>( c ) ) )
                    {
                        type = STRING_TYPE_BASE64;
                    }
                    else if ( isprint( c ) )
                    {
                        type = STRING_TYPE_PRINTABLE;
                    }
                    else if ( isspace( c ) )
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
            case STRING_TYPE_ALPHA:
            {
                if ( false == isalpha( c ) )
                {
                    if ( isdigit( c ) )
                    {
                        type = STRING_TYPE_ALPHANUM;
                    }
                    else if ( IsBase64SpecialCharA( static_cast<CHAR>( c ) ) )
                    {
                        type = STRING_TYPE_BASE64;
                    }
                    else if ( isprint( c ) )
                    {
                        type = STRING_TYPE_PRINTABLE;
                    }
                    else if ( isspace( c ) )
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
            case STRING_TYPE_ALPHANUM:
            {
                if ( false == isalnum( c ) )
                {
                    if ( IsBase64SpecialCharA( static_cast<CHAR>( c ) ) )
                    {
                        type = STRING_TYPE_BASE64;
                    }
                    else if ( isprint( c ) )
                    {
                        type = STRING_TYPE_PRINTABLE;
                    }
                    else if ( isspace( c ) )
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
            case STRING_TYPE_BASE64:
            {
                if ( false == IsBase64CharA( static_cast<CHAR>( c ) ) )
                {
                    if ( isprint( c ) )
                    {
                        type = STRING_TYPE_PRINTABLE;
                    }
                    else if ( isspace( c ) )
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
            case STRING_TYPE_PRINTABLE:
            {
                if ( false == isprint( c ) )
                {
                    if ( isspace( c ) )
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
            case STRING_TYPE_READABLE:
            {
                if ( false == isprint( c ) && false == isspace( c ) )
                {
                    type = STRING_TYPE_BINARY;
                    goto exit;
                }
                break;
            }
            case STRING_TYPE_START:
            {
                if ( isdigit( c ) )
                {
                    type = STRING_TYPE_NUM;
                }
                else if ( isalpha( c ) )
                {
                    type = STRING_TYPE_ALPHA;
                }
                else if ( IsBase64SpecialCharA( static_cast<CHAR>( c ) ) )
                {
                    type = STRING_TYPE_BASE64;
                }
                else if ( isprint( c ) )
                {
                    type = STRING_TYPE_PRINTABLE;
                }
                else if ( isspace( c ) )
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
            default:
            {
                type = STRING_TYPE_BINARY;
                goto exit;
            }
        }
    }

    //Special handling for base64 to check it follows the rule
    if ( STRING_TYPE_BASE64 == type && FALSE == IsBase64StringA( aStr, aStrLen ) )
    {
        type = STRING_TYPE_PRINTABLE;
    }

exit:
    return type;
}

BOOL WildcardMatchW( IN CONST WCHAR * aString, IN CONST WCHAR * aWildcardPattern )
{
    enum State
    {
        Exact,       //Exact match
        Any,         //?
        AnyRepeat    //*
    } state = Exact;

    CONST WCHAR * wzTmp = nullptr;

    BOOL bMatch = TRUE;

    while ( bMatch && *aWildcardPattern )
    {
        switch ( *aWildcardPattern )
        {
            case L'*':
                state = AnyRepeat;
                wzTmp = aWildcardPattern + 1;
                break;
            case L'?':
                state = Any;
                break;
            default:
                state = Exact;
                break;
        }

        if ( L'\0' == *aString )
            break;

        switch ( state )
        {
            case Exact:
                bMatch = ( *aString == *aWildcardPattern );
                aString++;
                aWildcardPattern++;
                break;
            case Any:
                bMatch = TRUE;
                aString++;
                aWildcardPattern++;
                break;
            case AnyRepeat:
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
        case AnyRepeat:
            return ( *aString == *wzTmp );
        case Any:
            return ( *aString == *aWildcardPattern );
        default:
            return ( bMatch && ( *aString == *aWildcardPattern ) );
    }
}


BOOL GetLastErrorStringW( IN OUT std::wstring & aString, WORD aLang )
{
    BOOL bRet = FALSE;
    WCHAR * wzMsgBuf;
    SIZE_T sizeLen = (SIZE_T)FormatMessageW( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, nullptr,
                                             GetLastError(), aLang, (WCHAR *)&wzMsgBuf, 0, nullptr );
    if ( 0 < sizeLen )
    {
        //Remove \r\n at the end of the message
        CONST SIZE_T sizeCRLF = _countof( L"\r\n" ) - 1;
        if ( sizeCRLF <= sizeLen && 0 == wcsncmp( &wzMsgBuf[sizeLen - sizeCRLF], L"\r\n", sizeCRLF ) )
        {
            sizeLen -= sizeCRLF;
        }

        aString.assign( wzMsgBuf, sizeLen );
        LocalFree( wzMsgBuf );    //Free the buffer
        bRet = TRUE;
    }
    return bRet;
}

BOOL GetHResultStringW( HRESULT aResultCode, IN OUT std::wstring & aString, WORD aLang )
{
    BOOL bRet = FALSE;
    WCHAR * wzMsgBuf;

    if ( FACILITY_WINDOWS == HRESULT_FACILITY( aResultCode ) )
    {
        aResultCode = HRESULT_CODE( aResultCode );
    }

    SIZE_T sizeLen = (SIZE_T)FormatMessageW( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, nullptr,
                                             aResultCode, aLang, (WCHAR *)&wzMsgBuf, 0, nullptr );
    if ( 0 < sizeLen )
    {
        //Remove \r\n at the end of the message
        CONST SIZE_T sizeCRLF = _countof( L"\r\n" ) - 1;
        if ( sizeCRLF <= sizeLen && 0 == wcsncmp( &wzMsgBuf[sizeLen - sizeCRLF], L"\r\n", sizeCRLF ) )
        {
            sizeLen -= sizeCRLF;
        }

        aString.assign( wzMsgBuf, sizeLen );
        LocalFree( wzMsgBuf );    //Free the buffer
        bRet = TRUE;
    }
    return bRet;
}

CONST WCHAR * GetPathBaseNameW( IN CONST WCHAR * aFullPath )
{
    CONST WCHAR * wzBase = wcsrchr( aFullPath, L'\\' );
    return ( wzBase ) ? ( wzBase + 1 ) : aFullPath;
}

//Default will add slash for the ease of read
BOOL GuidToStringW( IN CONST GUID aGuid, OUT wstring & aGuidString, IN OPTIONAL BOOL aAddSlash )
{
    CONST WCHAR * wzFormat = ( aAddSlash ) ? L"%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X"
                                           : L"%08X%04X%04X%02X%02X%02X%02X%02X%02X%02X%02X";
    return FormatStringW( aGuidString, wzFormat, aGuid.Data1, aGuid.Data2, aGuid.Data3, aGuid.Data4[0], aGuid.Data4[1],
                          aGuid.Data4[2], aGuid.Data4[3], aGuid.Data4[4], aGuid.Data4[5], aGuid.Data4[6],
                          aGuid.Data4[7] );
}

//GUID should be one of the formats: 11223344556677889900AABBCCDDEEFF, 11223344-5566-7788-9900AABBCCDDEEFF, or 11223344-5566-7788-9900-AABBCCDDEEFF
BOOL StringToGuidW( IN CONST wstring & aGuidString, OUT GUID & aGuid )
{
    vector<wstring> vecGuid;
    SplitStringW( aGuidString, vecGuid, L"-" );

    INT nFirst, nSecond, nThird, nFourth, nFifth;    //Keep index of each GUID part
    switch ( vecGuid.size() )
    {
        case 1:    //The form of AABBCCDDAABBAABBAABBCCDDAABBCCDD
        {
            if ( vecGuid[0].length() != 32 )
            {
                SetLastError( ERROR_INVALID_PARAMETER );
                return FALSE;
            }
            else
            {
                nFirst = 0;
                nSecond = 8;
                nThird = 12;
                nFourth = 16;
                nFifth = 20;
            }
            break;
        }
        case 4:    //The form of AABBCCDD-AABB-AABB-AABBCCDDAABBCCDD
        {
            if ( vecGuid[0].length() != 8 || vecGuid[1].length() != 4 || vecGuid[2].length() != 4 ||
                 vecGuid[3].length() != 16 )
            {
                SetLastError( ERROR_INVALID_PARAMETER );
                return FALSE;
            }
            else
            {
                nFirst = 0;
                nSecond = 9;
                nThird = 14;
                nFourth = 19;
                nFifth = 23;
            }
            break;
        }
        case 5:    //The form of AABBCCDD-AABB-AABB-AABB-CCDDAABBCCDD
        {
            if ( vecGuid[0].length() != 8 || vecGuid[1].length() != 4 || vecGuid[2].length() != 4 ||
                 vecGuid[4].length() != 4 || vecGuid[5].length() != 12 )
            {
                SetLastError( ERROR_INVALID_PARAMETER );
                return FALSE;
            }
            else
            {
                nFirst = 0;
                nSecond = 9;
                nThird = 14;
                nFourth = 19;
                nFifth = 24;
            }
            break;
        }
        default:
        {
            SetLastError( ERROR_INVALID_PARAMETER );
            return FALSE;
        }
    }
    WCHAR wzBuf[10] = { 0 };

    wcsncpy_s( wzBuf, _countof( wzBuf ), &aGuidString[nFirst], 8 );
    wzBuf[8] = 0;
    aGuid.Data1 = wcstoul( wzBuf, nullptr, 16 );

    wcsncpy_s( wzBuf, _countof( wzBuf ), &aGuidString[nSecond], 4 );
    wzBuf[4] = 0;
    aGuid.Data2 = (USHORT)wcstoul( wzBuf, nullptr, 16 );

    wcsncpy_s( wzBuf, _countof( wzBuf ), &aGuidString[nThird], 4 );
    wzBuf[4] = 0;
    aGuid.Data3 = (USHORT)wcstoul( wzBuf, nullptr, 16 );

    for ( INT i = 0; i < 2; i++ )
    {
        wcsncpy_s( wzBuf, _countof( wzBuf ), &aGuidString[nFourth + ( 2 * i )], 2 );
        wzBuf[2] = 0;
        aGuid.Data4[i] = (UCHAR)wcstoul( wzBuf, nullptr, 16 );
    }

    for ( INT i = 0; i < 6; i++ )
    {
        wcsncpy_s( wzBuf, _countof( wzBuf ), &aGuidString[nFifth + ( 2 * i )], 2 );
        wzBuf[2] = 0;
        aGuid.Data4[i + 2] = (UCHAR)wcstoul( wzBuf, nullptr, 16 );
    }
    return TRUE;
}

//Encode full URL include parameters after "?"
BOOL EncodeUrlA( IN CONST std::string & aOldUrl, OUT std::string & aNewUrl )
{
    CHAR szBuf[INTERNET_MAX_URL_LENGTH] = { 0 };
    INT nLen = 0;
    BOOL bParam = FALSE;
    for ( SIZE_T i = 0; i < aOldUrl.length() && nLen < INTERNET_MAX_PATH_LENGTH; i++ )
    {
        if ( isalnum( (UCHAR)aOldUrl[i] ) )
        {
            nLen += _snprintf_s( &szBuf[nLen], 2, 1, "%c", aOldUrl[i] );
        }
        else if ( '/' == aOldUrl[i] || '.' == aOldUrl[i] || ':' == aOldUrl[i] )
        {
            nLen += _snprintf_s( &szBuf[nLen], 4, 3, ( bParam ) ? "%%%02X" : "%c", aOldUrl[i] );
        }
        else if ( '?' == aOldUrl[i] )
        {
            nLen += _snprintf_s( &szBuf[nLen], 4, 3, ( bParam ) ? "%%%02X" : "%c", aOldUrl[i] );
            bParam = TRUE;
        }
        else if ( '=' == aOldUrl[i] )
        {
            nLen += _snprintf_s( &szBuf[nLen], 4, 3, ( bParam ) ? "%c" : "%%%02X", aOldUrl[i] );
            bParam = TRUE;
        }
        else
        {
            nLen += _snprintf_s( &szBuf[nLen], 4, 3, "%%%02X", (UCHAR)aOldUrl[i] );
        }
    }
    if ( INTERNET_MAX_PATH_LENGTH <= nLen )
    {
        aNewUrl.clear();
        return FALSE;
    }
    else
    {
        aNewUrl.assign( szBuf, nLen );
        return TRUE;
    }
}






VOID CUrlStringPtr::Clean()
{
    m_pUrl = nullptr;
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
    return this->SetUrl( pUrl, strlen( pUrl ) );
}

BOOL CUrlStringPtr::SetUrl( CONST CHAR * pUrl, SIZE_T uLen )
{
    if ( nullptr == pUrl )
    {
        return FALSE;
    }

    m_pUrl = pUrl;
    m_uUrlLen = uLen;

    //Parse scheme
    CONST CHAR * pHostStart = m_pUrl;
    if ( _strnicmp( m_pUrl, "http://", sizeof( "http://" ) - 1 ) == 0 )
    {
        pHostStart += sizeof( "http://" ) - 1;
        m_uSchemeLen = sizeof( "http" ) - 1;
    }
    else if ( _strnicmp( m_pUrl, "https://", sizeof( "https://" ) - 1 ) == 0 )
    {
        pHostStart += sizeof( "https://" ) - 1;
        m_ulScheme = INTERNET_SCHEME_HTTPS;
        m_uSchemeLen = sizeof( "https" ) - 1;
        m_ulPort = 443;
    }
    else
    {
    }

    //Parse relative URI
    CONST CHAR * pUri = strchr( pHostStart, '/' );
    if ( nullptr == pUri )
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
    CONST CHAR * pParams = strchr( pUri, '?' );
    if ( nullptr == pParams )
    {
        m_uParamsOffset = m_uUrlLen;
        pParams = m_pUrl + m_uParamsOffset;
    }
    else
    {
        pParams++;    //Ignore the '?'
        m_uParamsOffset = pParams - pUrl;
        m_uParamsLen = m_uUrlLen - m_uParamsOffset;
        m_uUriLen = pParams - pUri - 1;
    }

    //Parse fragment
    CONST CHAR * pFrag = strchr( pParams, '#' );
    if ( nullptr == pFrag )
    {
        m_uFragOffset = m_uUrlLen;
        pFrag = m_pUrl + m_uFragOffset;
    }
    else
    {
        pFrag++;    //Ignore the '#'
        m_uFragOffset = pFrag - pUrl;
        m_uFragLen = m_uUrlLen - m_uFragOffset;
        m_uParamsLen = pFrag - pParams - 1;
    }

    //Parse host and port
    m_uHostOffset = pHostStart - m_pUrl;
    m_uHostLen = pUri - pHostStart;
    CONST CHAR * pHostEnd = ( pHostStart != pUri ) ? pUri - 1 : pUri;
    for ( CONST CHAR * pTmp = pHostEnd; pTmp > pHostStart; pTmp-- )
    {
        if ( ':' == *pTmp )
        {
            CONST CHAR * pPortStart = pTmp + 1;

            //Care IPv6 address literal, such as [fe80:1234::1]:80
            if ( !( '[' == pHostStart[0] && ']' != *( pTmp - 1 ) ) )
            {
                if ( pPortStart < pUri )
                {
                    CHAR * pPortEnd = nullptr;
                    m_ulPort = strtoul( pPortStart, &pPortEnd, 10 );
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


CString::CString() : m_nLen( 0 ), m_szData( nullptr ) {}

CString::CString( const CHAR * aData, INT aLen )
{
    m_nLen = ( aLen < 0 ) ? strlen( aData ) : aLen;
    m_szData = new CHAR[m_nLen + 1];
    memcpy( m_szData, aData, m_nLen );
    m_szData[m_nLen] = 0;
}

CString::CString( const CString & aRhs ) : m_nLen( 0 ), m_szData( nullptr )
{
    this->Copy( aRhs );
}

CString::CString( CString && aRhs ) : m_nLen( 0 ), m_szData( nullptr )
{
    //Use std::swap() because of noexcept
    std::swap( m_nLen, aRhs.m_nLen );
    std::swap( m_szData, aRhs.m_szData );
}

CString::~CString()
{
    if ( m_szData != nullptr )
    {
        delete[] m_szData;
    }
}

std::ostream & operator<<( std::ostream & aOutput, const CString & aRhs )
{
    aOutput << aRhs.m_szData;
    return aOutput;
}

CString CString::operator+( const CString & aRhs ) const
{
    return std::move( CString( *this ) += aRhs );
}

CString & CString::operator=( const CString & aRhs )
{
    this->Copy( aRhs );
    return *this;
}

CString & CString::operator=( CString && aRhs )
{
    std::swap( m_nLen, aRhs.m_nLen );
    std::swap( m_szData, aRhs.m_szData );
    return *this;
}

CString & CString::operator+=( const CString & aRhs )
{
    CString str;
    str.m_nLen = m_nLen + aRhs.m_nLen;
    str.m_szData = new CHAR[str.m_nLen + 1];
    memcpy( str.m_szData, m_szData, m_nLen );
    memcpy( &str.m_szData[m_nLen], aRhs.m_szData, aRhs.m_nLen );
    str.m_szData[str.m_nLen] = 0;

    *this = std::move( str );
    return *this;
}

CHAR & CString::operator[]( const INT aIdx )
{
    return m_szData[aIdx];
}

const CHAR & CString::operator[]( const INT aIdx ) const
{
    return m_szData[aIdx];
}

INT CString::Size() const
{
    return m_nLen;
}

void CString::Copy( const CString & aRhs )
{
    if ( this != &aRhs )    //Prevent self assignment
    {
        if ( m_szData != nullptr )
        {
            delete[] this->m_szData;
        }

        this->m_nLen = aRhs.m_nLen;
        this->m_szData = new CHAR[m_nLen + 1];
        memcpy( m_szData, aRhs.m_szData, m_nLen );
        m_szData[m_nLen] = 0;
    }
}

#ifdef __cplusplus
}
#endif

}    //End of namespace CWUtils