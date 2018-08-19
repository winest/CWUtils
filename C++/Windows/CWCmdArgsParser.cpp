#include "stdafx.h"
#include "CWCmdArgsParser.h"
using namespace std;

namespace CWUtils
{

CCmdArgsParser * CCmdArgsParser::m_self = NULL;

BOOL _WStringToString( IN CONST std::wstring & aWString , OUT std::string & aString , DWORD aCodePage )
{    
    BOOL bRet = FALSE;
    DWORD dwFlag = ( CP_UTF8 == aCodePage ) ? MB_ERR_INVALID_CHARS : 0;
    CHAR szBuf[4096];
    INT nBuf = _countof( szBuf );
    INT nBufCopied = WideCharToMultiByte( aCodePage , 0 , aWString.c_str() , (INT)aWString.length()  , szBuf , nBuf , NULL , NULL );
    if ( 0 != nBufCopied )
    {
        aString.assign( szBuf , nBufCopied );
        bRet = TRUE;
    }
    else if ( ERROR_INSUFFICIENT_BUFFER == GetLastError() )
    {
        nBuf = WideCharToMultiByte( aCodePage , 0 , aWString.c_str() , (INT)aWString.length() , NULL , 0 , NULL , NULL );
        CHAR * szNewBuf = new (std::nothrow) CHAR[nBuf];
        if ( NULL != szNewBuf )
        {
            nBufCopied = WideCharToMultiByte( aCodePage , 0 , aWString.c_str() , (INT)aWString.length() , szNewBuf , nBuf , NULL , NULL );
            if ( 0 != nBufCopied )
            {
                aString.assign( szNewBuf , nBufCopied );
                bRet = TRUE;
            }            
            delete [] szNewBuf;
        }
    }
    else{}
    return bRet;
}





CCmdArgsParser * CCmdArgsParser::GetInstance( CONST WCHAR * aStartter , CONST WCHAR * aSplitter )
{
    if ( NULL == m_self )
    {
        m_self = new (std::nothrow) CCmdArgsParser( aStartter , aSplitter );
    }
    return m_self;
}

VOID CCmdArgsParser::Clear()
{
    if ( NULL != m_self )
    {
        delete m_self;
        m_self = NULL;
    }
}

BOOL CCmdArgsParser::ParseArgs( CONST WCHAR * aCmdLine , BOOL aHasBinaryPath )
{
    BOOL bRet = FALSE;
    m_wstrBinName.clear();
    m_mapNamedArgs.clear();
    m_vecUnnamedArgs.clear();

    do 
    {
        CONST WCHAR * wzCmdLine = aCmdLine;
        if ( NULL == wzCmdLine )
        {
            wzCmdLine = GetCommandLineW();
            aHasBinaryPath = TRUE;
        }

        vector<wstring> vecArgs;
        if ( aHasBinaryPath )
        {
            CONST WCHAR * pQuoteEnd = wcschr( wzCmdLine + 1 , L'"' );
            wstring wstrBinaryPath( wzCmdLine , pQuoteEnd - wzCmdLine + 1 );
            vecArgs.push_back( wstrBinaryPath );
            wzCmdLine = pQuoteEnd + 1;
        }

        if ( FALSE == this->SplitArgs( wzCmdLine , vecArgs ) )
        {
            break;
        }
        if ( FALSE == this->ParseArgs( vecArgs , aHasBinaryPath ) )
        {
            break;
        }

        bRet = TRUE;
    } while ( 0 );
    
    return bRet;
}

BOOL CCmdArgsParser::ParseArgs( INT aArgc , WCHAR * aArgv[] , BOOL aHasBinaryPath )
{
    BOOL bRet = FALSE;
    m_wstrBinName.clear();
    m_mapNamedArgs.clear();
    m_vecUnnamedArgs.clear();

    do 
    {
        vector<wstring> vecArgs;
        if ( 0 == aArgc )
        {
            bRet = this->ParseArgs();
            break;
        }
        else
        {
            for ( INT i = 0 ; i < aArgc ; i++ )
            {
                vecArgs.push_back( aArgv[i] );
            }
        }

        if ( FALSE == this->ParseArgs( vecArgs , aHasBinaryPath ) )
        {
            break;
        }

        bRet = TRUE;
    } while ( 0 );
    
    return bRet;
}


BOOL CCmdArgsParser::ParseArgs( vector<wstring> & aArgs , BOOL aHasBinaryPath )
{
    BOOL bRet = FALSE;
    m_wstrBinName.clear();
    m_mapNamedArgs.clear();
    m_vecUnnamedArgs.clear();

    size_t uArgIndex = 0;
    if ( aHasBinaryPath && 0 < aArgs.size() )
    {
        m_wstrBinName = this->RemoveQuote( aArgs[0] );
        uArgIndex++;
    }

    typedef enum  _PARSER_STATE
    {
        STATE_GET_NAME ,
        STATE_GET_VALUE
    } PARSER_STATE;
    PARSER_STATE state = STATE_GET_NAME;
    wstring wstrIncompletePairName;

    //Purpose:
    //Input                              => <NamedKey , NamedValue , UnamedValue>
    //-data="111 222"                    => <data , 111 222 , NULL>
    //-"data content2"="222 333"         => <data content2 , 222 333 , NULL>
    //"-data content3=333 444"           => <NULL , NULL , -data content3=333 444>
    //"\"-data content4=444 555\""       => <NULL , NULL , "-data content4=444 555">
    //"-\"data content5\"=\"555 666\""   => <NULL , NULL , -"data content5"="555 666">
    for ( ; uArgIndex < aArgs.size() ; uArgIndex++ )
    {
        switch ( state )
        {
            case STATE_GET_NAME :
            {
                if ( IsQuoted( aArgs[uArgIndex].c_str() ) )
                {
                    m_vecUnnamedArgs.push_back( RemoveQuote( aArgs[uArgIndex] ) );
                    break;
                }

                wstring wstrToken = aArgs[uArgIndex];
                if ( ( 2 <= wstrToken.length() ) && 
                     ( wstring::npos != m_wstrStartter.find_first_of( wstrToken.c_str() , 0 , 1 ) ) )
                {
                    size_t posSplitter = wstrToken.find_first_of( m_wstrSplitter.c_str() , 2 );                    
                    if ( wstring::npos != posSplitter )
                    {
                        wstring wstrName = RemoveQuote( wstrToken.substr( 1 , posSplitter - 1 ) );
                        wstring wstrValue = RemoveQuote( wstrToken.substr( posSplitter + 1 ) );
                        m_mapNamedArgs[wstrName] = wstrValue;
                    }
                    else
                    {
                        wstrIncompletePairName = RemoveQuote( wstrToken.substr( 1 ) );                                          
                        state = STATE_GET_VALUE;
                    }
                }
                else
                {
                    m_vecUnnamedArgs.push_back( wstrToken );
                }
                break;
            }
            case STATE_GET_VALUE :
            {
                m_mapNamedArgs[ wstrIncompletePairName ] = RemoveQuote( aArgs[uArgIndex] ); 
                state = STATE_GET_NAME;
                break;
            }
            default :
                goto exit;
        }        
    }

    //Check state machine
    if ( STATE_GET_NAME != state )
    {
        goto exit;
    }
    //Check necessary parameters are parsed
    for ( auto itProp = m_mapNamedArgsProp.begin() ; itProp != m_mapNamedArgsProp.end() ; itProp++ )
    {
        if ( TRUE == itProp->second.bMustExists && 
             m_mapNamedArgs.end() == m_mapNamedArgs.find( itProp->first ) )
        {
            goto exit;
        }
    }
    bRet = TRUE;

exit :
    return bRet;
}

size_t CCmdArgsParser::GetTotalArgsCount()
{
    return m_mapNamedArgs.size() + m_vecUnnamedArgs.size();
}



size_t CCmdArgsParser::GetNamedArgsCount()
{
    return m_mapNamedArgs.size();
}

BOOL CCmdArgsParser::HasArg( CONST WCHAR * aName )
{
    _ASSERT( NULL != aName );
    auto it = m_mapNamedArgs.find( aName );
    return ( it != m_mapNamedArgs.end() ) ? TRUE : FALSE;
}

BOOL CCmdArgsParser::GetArg( CONST WCHAR * aName , string & aValue , CONST CHAR * aDefaultVal )
{
    wstring wstrValue;
    BOOL bRet = GetArg( aName , wstrValue , NULL );
    if ( FALSE != bRet )
    {
        bRet = _WStringToString( wstrValue , aValue , CP_ACP );
        if ( FALSE == bRet && NULL != aDefaultVal )
        {
            aValue = aDefaultVal;
        }
    }
    return bRet;
}

BOOL CCmdArgsParser::GetArg( CONST WCHAR * aName , wstring & aValue , CONST WCHAR * aDefaultVal )
{
    _ASSERT( NULL != aName );
    auto it = m_mapNamedArgs.find( aName );
    if ( it != m_mapNamedArgs.end() )
    {
        aValue = it->second;
        return TRUE;
    }    

    if ( NULL != aDefaultVal )
    {
        aValue = aDefaultVal;
    }
    return FALSE;
}


BOOL CCmdArgsParser::GetArg( CONST WCHAR * aName , INT32 & aValue , INT32 aDefaultVal )
{
    _ASSERT( NULL != aName );
    aValue = aDefaultVal;

    auto it = m_mapNamedArgs.find( aName );
    if ( it != m_mapNamedArgs.end() )
    {
        UINT32 ulVal = (UINT32)wcstoul( it->second.c_str() , NULL , 10 );
        if ( ERANGE != errno )
        {
            aValue = (INT)ulVal;
            return TRUE;
        }
    }

    return FALSE;
}
BOOL CCmdArgsParser::GetArg( CONST WCHAR * aName , UINT32 & aValue , UINT32 aDefaultVal )
{
    return GetArg( aName , (INT32 &)aValue , (INT32)aDefaultVal );
}

BOOL CCmdArgsParser::GetArg( CONST WCHAR * aName , INT64 & aValue , INT64 aDefaultVal )
{
    _ASSERT( NULL != aName );
    aValue = aDefaultVal;

    auto it = m_mapNamedArgs.find( aName );
    if ( it != m_mapNamedArgs.end() )
    {
        UINT64 ulVal = _wcstoui64( it->second.c_str() , NULL , 10 );
        if ( ERANGE != errno )
        {
            aValue = (INT64)ulVal;
            return TRUE;
        }
    }

    return FALSE;
}
BOOL CCmdArgsParser::GetArg( CONST WCHAR * aName , UINT64 & aValue , UINT64 aDefaultVal )
{
    return GetArg( aName , (INT64 &)aValue , (INT64)aDefaultVal );
}

BOOL CCmdArgsParser::GetArg( CONST WCHAR * aName , DOUBLE & aValue , DOUBLE aDefaultVal )
{
    _ASSERT( NULL != aName );
    aValue = aDefaultVal;

    auto it = m_mapNamedArgs.find( aName );
    if ( it != m_mapNamedArgs.end() )
    {
        DOUBLE fVal = wcstod( it->second.c_str() , NULL );
        if ( ERANGE != errno )
        {
            aValue = (DOUBLE)fVal;
            return TRUE;
        }
    }

    return FALSE;
}




size_t CCmdArgsParser::GetUnnamedArgsCount()
{
    return m_vecUnnamedArgs.size();
}

BOOL CCmdArgsParser::GetUnnamedArg( size_t aIndex , string & aValue , CONST CHAR * aDefaultVal )
{
    wstring wstrValue;
    BOOL bRet = GetUnnamedArg( aIndex , wstrValue , NULL );
    if ( FALSE != bRet )
    {
        bRet = _WStringToString( wstrValue , aValue , CP_ACP );
        if ( FALSE == bRet && NULL != aDefaultVal )
        {
            aValue = aDefaultVal;
        }
    }
    return bRet;
}

BOOL CCmdArgsParser::GetUnnamedArg( size_t aIndex , wstring & aValue , CONST WCHAR * aDefaultVal )
{
    if ( aIndex < m_vecUnnamedArgs.size() )
    {
        aValue = m_vecUnnamedArgs[aIndex];
        return TRUE;
    }

    if ( NULL != aDefaultVal )
    {
        aValue = aDefaultVal;
    }
    return FALSE;
}

BOOL CCmdArgsParser::GetUnnamedArg( size_t aIndex , INT32 & aValue , INT32 aDefaultVal )
{
    aValue = aDefaultVal;

    if ( aIndex < m_vecUnnamedArgs.size() )
    {
        UINT32 ulVal = (UINT32)wcstoul( m_vecUnnamedArgs[aIndex].c_str() , NULL , 10 );
        if ( ERANGE != errno )
        {
            aValue = (INT32)ulVal;
            return TRUE;
        }
    }

    return FALSE;

}
BOOL CCmdArgsParser::GetUnnamedArg( size_t aIndex , UINT32 & aValue , UINT32 aDefaultVal )
{
    return GetUnnamedArg( aIndex , (INT32 &)aValue , (INT32)aDefaultVal );
}

BOOL CCmdArgsParser::GetUnnamedArg( size_t aIndex , INT64 & aValue , INT64 aDefaultVal )
{
    aValue = aDefaultVal;

    if ( aIndex < m_vecUnnamedArgs.size() )
    {
        UINT64 ulVal = _wcstoui64( m_vecUnnamedArgs[aIndex].c_str() , NULL , 10 );
        if ( ERANGE != errno )
        {
            aValue = (INT64)ulVal;
            return TRUE;
        }
    }

    return FALSE;
}
BOOL CCmdArgsParser::GetUnnamedArg( size_t aIndex , UINT64 & aValue , UINT64 aDefaultVal )
{
    return GetUnnamedArg( aIndex , (INT64 &)aValue , (INT64)aDefaultVal );
}

BOOL CCmdArgsParser::GetUnnamedArg( size_t aIndex , DOUBLE & aValue , DOUBLE aDefaultVal )
{
    aValue = aDefaultVal;

    if ( aIndex < m_vecUnnamedArgs.size() )
    {
        DOUBLE fVal = wcstod( m_vecUnnamedArgs[aIndex].c_str() , NULL );
        if ( ERANGE != errno )
        {
            aValue = (DOUBLE)fVal;
            return TRUE;
        }
    }

    return FALSE;
}




BOOL CCmdArgsParser::SetUsage( CONST WCHAR * aName , BOOL aMustExists , CONST WCHAR * aUsageFormat , ... )
{
    BOOL bRet = FALSE;
    CmdArgProperty prop;
    prop.bMustExists = ( FALSE == aMustExists ) ? FALSE : TRUE;

    if ( NULL != aUsageFormat )
    {
        va_list args = NULL;
        va_start( args , aUsageFormat );
        size_t len = _vscwprintf( aUsageFormat , args ) + 1;    //Get formatted string length and adding one for null-terminator
        if ( 1 < len )
        {
            WCHAR * wzBuf = new (std::nothrow) WCHAR[len];
            if ( NULL != wzBuf )
            {
                if ( _vsnwprintf_s( wzBuf , len , _TRUNCATE , aUsageFormat , args ) > 0 )
                {
                    prop.wstrUsage = wzBuf;
                    bRet = TRUE;
                }
                delete [] wzBuf;
            }
        }
        va_end( args );
    }

    if ( bRet )
    {
        if ( NULL == aName )
        {
            m_wstrGeneralUsage = prop.wstrUsage;
        }
        else
        {
            m_mapNamedArgsProp[wstring(aName)] = prop;
        }
    }
    return bRet;
}
VOID CCmdArgsParser::ShowUsage( CONST WCHAR * aName )
{
    wstring wstrUsage = m_wstrGeneralUsage;
    if ( NULL == aName )
    {
        if ( 0 < m_mapNamedArgsProp.size() )
        {
            wstrUsage.append( L"\n[Options]\n" );
            for ( auto it = m_mapNamedArgsProp.begin() ; it != m_mapNamedArgsProp.end() ; it++ )
            {
                wstrUsage.push_back( L'\n' );
                if ( 0 < m_wstrStartter.length() )
                {
                    wstrUsage.push_back( m_wstrStartter[0] );
                }
                wstrUsage.append( AddQuoteIfHaveSpace( it->first ) );
                if ( 0 < m_wstrSplitter.length() )
                {
                    wstrUsage.push_back( m_wstrSplitter[0] );
                }
                wstrUsage.append( it->second.wstrUsage );
            }
        }
    }
    else
    {
        auto it = m_mapNamedArgsProp.find( wstring(aName) );
        if ( m_mapNamedArgsProp.end() != it )
        {
            wstrUsage.push_back( L'\n' );
            if ( 0 < m_wstrStartter.length() )
            {
                wstrUsage.push_back( m_wstrStartter[0] );
            }
            wstrUsage.append( AddQuoteIfHaveSpace( it->first ) );
            if ( 0 < m_wstrSplitter.length() )
            {
                wstrUsage.push_back( m_wstrSplitter[0] );
            }
            wstrUsage.append( it->second.wstrUsage );
        }
        else
        {
            wstrUsage.append( L"\nNo usage found for \"" );
            wstrUsage.append( aName );
            wstrUsage.append( L"\"" );
        };
    }
    wstrUsage.append( L"\n\n" );

    #ifdef _CONSOLE
        wprintf_s( wstrUsage.c_str() );
    #else
        MessageBoxW( NULL , wstrUsage.c_str() , L"Usage" , MB_ICONINFORMATION | MB_OK );
    #endif
}

VOID CCmdArgsParser::DumpArgs()
{
    INT nBufLen = 0;
    WCHAR wzBuf[4096] = { 0 };

    nBufLen += _snwprintf_s( &wzBuf[nBufLen] , _countof(wzBuf)-nBufLen , _TRUNCATE , L"Binary name: %ws\n" , m_wstrBinName.c_str() );
    
    for ( auto itNamed = m_mapNamedArgs.begin() ; itNamed != m_mapNamedArgs.end() ; itNamed++ )
    {
        nBufLen += _snwprintf_s( &wzBuf[nBufLen] , _countof(wzBuf)-nBufLen , _TRUNCATE , L"name=%ws, value=%ws\n" , itNamed->first.c_str() , itNamed->second.c_str() );
    }
    for ( auto itUnnamed = m_vecUnnamedArgs.begin() ; itUnnamed != m_vecUnnamedArgs.end() ; itUnnamed++ )
    {
        nBufLen += _snwprintf_s( &wzBuf[nBufLen] , _countof(wzBuf)-nBufLen , _TRUNCATE , L"unamed value=%ws\n" , itUnnamed->c_str() );
    }

    #ifdef _CONSOLE
        wprintf_s( wzBuf );
    #else
        MessageBoxW( NULL , wzBuf , L"Arguments" , MB_ICONINFORMATION | MB_OK );
    #endif
}

BOOL CCmdArgsParser::IsQuoted( IN CONST wstring & aString )
{
    return ( 2 <= aString.length() && L'"' == aString[0] && L'"' == aString[aString.length()-1] ) ? TRUE : FALSE;
}

wstring CCmdArgsParser::AddQuoteIfHaveSpace( IN CONST wstring & aString )
{
    if ( wstring::npos == aString.find_first_of( L' ' ) )
    {
        return aString;
    }
    else
    {
        wstring wstrResult;
        wstrResult.reserve( sizeof(L'\"') + aString.length() + sizeof(L'\"') );
        wstrResult.push_back( L'\"' );
        wstrResult.append( aString );
        wstrResult.push_back( L'\"' );
        return wstrResult;
    }
}

wstring CCmdArgsParser::RemoveQuote( IN CONST wstring & aString )
{
    wstring wstrResult;
    if ( IsQuoted( aString ) )
    {
        wstrResult = aString.substr( 1 , aString.length() - 2 );
    }
    else
    {
        wstrResult = aString;
    }    
    return wstrResult;
}

BOOL CCmdArgsParser::SplitArgs( IN CONST std::wstring & aCmdLine , OUT std::vector<std::wstring> & aOutput )
{
    BOOL bQuoting = FALSE;
    BOOL bEscaping = FALSE;

    //Flush buffer to aOutput only when whitespace or terminator is met
    wstring wstrTmp;
    for ( size_t i = 0 ; i < aCmdLine.length() ; i++ )
    {
        if ( bEscaping )
        {
            wstrTmp.push_back( aCmdLine[i] );
            bEscaping = FALSE;
        }
        else if ( L'\\' == aCmdLine[i] )
        {
            bEscaping = TRUE;
        }
        else if ( L'"' == aCmdLine[i] )
        {
            wstrTmp.push_back( aCmdLine[i] );
            if ( FALSE == bQuoting )
            {
                bQuoting = TRUE;
            }
            else
            {
                bQuoting = FALSE;
            }
        }
        else if ( L' ' == aCmdLine[i] )
        {
            if ( 0 < wstrTmp.length() )
            {
                if ( FALSE == bQuoting )
                {
                    aOutput.push_back( wstrTmp );
                    wstrTmp.clear();
                }
                else
                {
                    wstrTmp.push_back( aCmdLine[i] );
                }
            }
        }
        else
        {
            wstrTmp.push_back( aCmdLine[i] );
        }
    }

    BOOL bGoodEnd = ( FALSE == bQuoting && FALSE == bEscaping ) ? TRUE : FALSE;
    if ( 0 < wstrTmp.length() && bGoodEnd )
    {
        aOutput.push_back( wstrTmp );
        wstrTmp.clear();
    }

    return bGoodEnd;
}


}    //End of namespace CWUtils