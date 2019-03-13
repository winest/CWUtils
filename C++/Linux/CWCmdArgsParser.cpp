#include "stdafx.h"
#include "CWCmdArgsParser.h"
using namespace std;

namespace CWUtils
{
CCmdArgsParser * CCmdArgsParser::m_self = NULL;
typedef map<string, string>::iterator MIT;
typedef vector<string>::iterator VIT;



CCmdArgsParser * CCmdArgsParser::GetInstance( CONST CHAR * aStartter, CONST CHAR * aSplitter )
{
    if ( NULL == m_self )
    {
        m_self = new ( std::nothrow ) CCmdArgsParser( aStartter, aSplitter );
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

BOOL CCmdArgsParser::ParseArgs( CONST CHAR * aCmdLine, BOOL aHasBinaryPath )
{
    BOOL bRet = FALSE;
    m_strBinName.clear();
    m_mapNamedArgs.clear();
    m_vecUnnamedArgs.clear();

    do
    {
        CONST CHAR * szCmdLine = aCmdLine;
        if ( NULL == szCmdLine )
        {
            break;
        }

        vector<string> vecArgs;
        if ( aHasBinaryPath )
        {
            CONST CHAR * pQuoteEnd = strchr( szCmdLine + 1, '"' );
            string strBinaryPath( szCmdLine, pQuoteEnd - szCmdLine + 1 );
            vecArgs.push_back( strBinaryPath );
            szCmdLine = pQuoteEnd + 1;
        }

        if ( FALSE == this->SplitArgs( szCmdLine, vecArgs ) )
        {
            break;
        }
        if ( FALSE == this->ParseArgs( vecArgs, aHasBinaryPath ) )
        {
            break;
        }

        bRet = TRUE;
    } while ( 0 );

    return bRet;
}

BOOL CCmdArgsParser::ParseArgs( INT aArgc, CHAR * aArgv[], BOOL aHasBinaryPath )
{
    BOOL bRet = FALSE;
    m_strBinName.clear();
    m_mapNamedArgs.clear();
    m_vecUnnamedArgs.clear();

    do
    {
        vector<string> vecArgs;
        if ( 0 == aArgc )
        {
            bRet = this->ParseArgs();
            break;
        }
        else
        {
            for ( INT i = 0; i < aArgc; i++ )
            {
                vecArgs.push_back( aArgv[i] );
            }
        }

        if ( FALSE == this->ParseArgs( vecArgs, aHasBinaryPath ) )
        {
            break;
        }

        bRet = TRUE;
    } while ( 0 );

    return bRet;
}


BOOL CCmdArgsParser::ParseArgs( vector<string> & aArgs, BOOL aHasBinaryPath )
{
    BOOL bRet = FALSE;
    m_strBinName.clear();
    m_mapNamedArgs.clear();
    m_vecUnnamedArgs.clear();

    size_t uArgIndex = 0;
    if ( aHasBinaryPath && 0 < aArgs.size() )
    {
        m_strBinName = this->RemoveQuote( aArgs[0] );
        uArgIndex++;
    }

    typedef enum _PARSER_STATE
    {
        STATE_GET_NAME,
        STATE_GET_VALUE
    } PARSER_STATE;
    PARSER_STATE state = STATE_GET_NAME;
    string strIncompletePairName;

    //Purpose:
    //Input                              => <NamedKey , NamedValue , UnamedValue>
    //-data="111 222"                    => <data , 111 222 , NULL>
    //-"data content2"="222 333"         => <data content2 , 222 333 , NULL>
    //"-data content3=333 444"           => <NULL , NULL , -data content3=333 444>
    //"\"-data content4=444 555\""       => <NULL , NULL , "-data content4=444 555">
    //"-\"data content5\"=\"555 666\""   => <NULL , NULL , -"data content5"="555 666">
    for ( ; uArgIndex < aArgs.size(); uArgIndex++ )
    {
        switch ( state )
        {
            case STATE_GET_NAME:
            {
                if ( IsQuoted( aArgs[uArgIndex].c_str() ) )
                {
                    m_vecUnnamedArgs.push_back( RemoveQuote( aArgs[uArgIndex] ) );
                    break;
                }

                string strToken = aArgs[uArgIndex];
                if ( ( 2 <= strToken.length() ) &&
                     ( string::npos != m_strStartter.find_first_of( strToken.c_str(), 0, 1 ) ) )
                {
                    size_t posSplitter = strToken.find_first_of( m_strSplitter.c_str(), 2 );
                    if ( string::npos != posSplitter )
                    {
                        string strName = RemoveQuote( strToken.substr( 1, posSplitter - 1 ) );
                        std::transform( strName.begin(), strName.end(), strName.begin(), ::tolower );
                        string strValue = RemoveQuote( strToken.substr( posSplitter + 1 ) );
                        m_mapNamedArgs[strName] = strValue;
                    }
                    else
                    {
                        strIncompletePairName = RemoveQuote( strToken.substr( 1 ) );
                        std::transform( strIncompletePairName.begin(), strIncompletePairName.end(),
                                        strIncompletePairName.begin(), ::tolower );
                        state = STATE_GET_VALUE;
                    }
                }
                else
                {
                    m_vecUnnamedArgs.push_back( strToken );
                }
                break;
            }
            case STATE_GET_VALUE:
            {
                m_mapNamedArgs[strIncompletePairName] = RemoveQuote( aArgs[uArgIndex] );
                state = STATE_GET_NAME;
                break;
            }
            default:
                goto exit;
        }
    }

    //Check state machine
    if ( STATE_GET_NAME != state )
    {
        goto exit;
    }
    //Check necessary parameters are parsed
    for ( map<string, CmdArgProperty>::iterator itProp = m_mapNamedArgsProp.begin(); itProp != m_mapNamedArgsProp.end();
          itProp++ )
    {
        if ( TRUE == itProp->second.bMustExists && m_mapNamedArgs.end() == m_mapNamedArgs.find( itProp->first ) )
        {
            goto exit;
        }
    }
    bRet = TRUE;

exit:
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

BOOL CCmdArgsParser::HasArg( CONST CHAR * aName )
{
    assert( NULL != aName );
    MIT it = m_mapNamedArgs.find( aName );
    return ( it != m_mapNamedArgs.end() ) ? TRUE : FALSE;
}

BOOL CCmdArgsParser::GetArg( CONST CHAR * aName, string & aValue, CONST CHAR * aDefaultVal )
{
    assert( NULL != aName );
    MIT it = m_mapNamedArgs.find( aName );
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


BOOL CCmdArgsParser::GetArg( CONST CHAR * aName, INT32 & aValue, INT32 aDefaultVal )
{
    assert( NULL != aName );
    aValue = aDefaultVal;

    MIT it = m_mapNamedArgs.find( aName );
    if ( it != m_mapNamedArgs.end() )
    {
        UINT32 ulVal = (UINT32)strtoul( it->second.c_str(), NULL, 10 );
        if ( ERANGE != errno )
        {
            aValue = (INT32)ulVal;
            return TRUE;
        }
    }

    return FALSE;
}
BOOL CCmdArgsParser::GetArg( CONST CHAR * aName, UINT32 & aValue, UINT32 aDefaultVal )
{
    return GetArg( aName, (INT32 &)aValue, (INT32)aDefaultVal );
}

BOOL CCmdArgsParser::GetArg( CONST CHAR * aName, INT64 & aValue, INT64 aDefaultVal )
{
    assert( NULL != aName );
    aValue = aDefaultVal;

    MIT it = m_mapNamedArgs.find( aName );
    if ( it != m_mapNamedArgs.end() )
    {
        UINT64 ulVal = strtoull( it->second.c_str(), NULL, 10 );
        if ( ERANGE != errno )
        {
            aValue = (INT64)ulVal;
            return TRUE;
        }
    }

    return FALSE;
}
BOOL CCmdArgsParser::GetArg( CONST CHAR * aName, UINT64 & aValue, UINT64 aDefaultVal )
{
    return GetArg( aName, (INT64 &)aValue, (INT64)aDefaultVal );
}




size_t CCmdArgsParser::GetUnnamedArgsCount()
{
    return m_vecUnnamedArgs.size();
}

BOOL CCmdArgsParser::GetUnnamedArg( size_t aIndex, string & aValue, CONST CHAR * aDefaultVal )
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

BOOL CCmdArgsParser::GetUnnamedArg( size_t aIndex, INT32 & aValue, INT32 aDefaultVal )
{
    aValue = aDefaultVal;

    if ( aIndex < m_vecUnnamedArgs.size() )
    {
        UINT32 ulVal = (UINT32)strtoul( m_vecUnnamedArgs[aIndex].c_str(), NULL, 10 );
        if ( ERANGE != errno )
        {
            aValue = (INT32)ulVal;
            return TRUE;
        }
    }

    return FALSE;
}
BOOL CCmdArgsParser::GetUnnamedArg( size_t aIndex, UINT32 & aValue, UINT32 aDefaultVal )
{
    return GetUnnamedArg( aIndex, (INT32 &)aValue, (INT32)aDefaultVal );
}

BOOL CCmdArgsParser::GetUnnamedArg( size_t aIndex, INT64 & aValue, INT64 aDefaultVal )
{
    aValue = aDefaultVal;

    if ( aIndex < m_vecUnnamedArgs.size() )
    {
        UINT64 ulVal = strtoull( m_vecUnnamedArgs[aIndex].c_str(), NULL, 10 );
        if ( ERANGE != errno )
        {
            aValue = (INT64)ulVal;
            return TRUE;
        }
    }

    return FALSE;
}
BOOL CCmdArgsParser::GetUnnamedArg( size_t aIndex, UINT64 & aValue, UINT64 aDefaultVal )
{
    return GetUnnamedArg( aIndex, (INT64 &)aValue, (INT64)aDefaultVal );
}




BOOL CCmdArgsParser::SetUsage( CONST CHAR * aName, BOOL aMustExists, CONST CHAR * aUsageFormat, ... )
{
    BOOL bRet = FALSE;
    CmdArgProperty prop;
    prop.bMustExists = ( FALSE == aMustExists ) ? FALSE : TRUE;

    if ( NULL != aUsageFormat )
    {
        va_list args;
        va_start( args, aUsageFormat );
        SIZE_T len = vsnprintf( NULL, 0, aUsageFormat, args ) +
                     1;    //Get formatted string length and adding one for null-terminator
        if ( 1 < len )
        {
            CHAR * szBuf = new ( std::nothrow ) CHAR[len];
            if ( NULL != szBuf )
            {
                if ( vsnprintf( szBuf, len, aUsageFormat, args ) > 0 )
                {
                    prop.strUsage = szBuf;
                    bRet = TRUE;
                }
                delete[] szBuf;
            }
        }
        va_end( args );
    }

    if ( bRet )
    {
        if ( NULL == aName )
        {
            m_strGeneralUsage = prop.strUsage;
        }
        else
        {
            m_mapNamedArgsProp[string( aName )] = prop;
        }
    }
    return bRet;
}
VOID CCmdArgsParser::ShowUsage( CONST CHAR * aName )
{
    string strUsage = m_strGeneralUsage;
    if ( NULL == aName )
    {
        if ( 0 < m_mapNamedArgsProp.size() )
        {
            strUsage.append( "\n[Options]\n" );
            for ( map<string, CmdArgProperty>::iterator it = m_mapNamedArgsProp.begin(); it != m_mapNamedArgsProp.end();
                  it++ )
            {
                strUsage.push_back( '\n' );
                if ( 0 < m_strStartter.length() )
                {
                    strUsage.push_back( m_strStartter[0] );
                }
                strUsage.append( AddQuoteIfHaveSpace( it->first ) );
                if ( 0 < m_strSplitter.length() )
                {
                    strUsage.push_back( m_strSplitter[0] );
                }
                strUsage.append( it->second.strUsage );
            }
        }
    }
    else
    {
        string strName = aName;
        std::transform( strName.begin(), strName.end(), strName.begin(), ::tolower );
        map<string, CmdArgProperty>::iterator it = m_mapNamedArgsProp.find( strName );
        if ( m_mapNamedArgsProp.end() != it )
        {
            strUsage.push_back( '\n' );
            if ( 0 < m_strStartter.length() )
            {
                strUsage.push_back( m_strStartter[0] );
            }
            strUsage.append( AddQuoteIfHaveSpace( it->first ) );
            if ( 0 < m_strSplitter.length() )
            {
                strUsage.push_back( m_strSplitter[0] );
            }
            strUsage.append( it->second.strUsage );
        }
        else
        {
            strUsage.append( "\nNo usage found for \"" );
            strUsage.append( aName );
            strUsage.append( "\"" );
        };
    }
    strUsage.append( "\n\n" );

#ifdef _CONSOLE
    printf( "%s", strUsage.c_str() );
#else
    strUsage.insert( 0, "zenity --info --text=\"" );
    strUsage.push_back( '\"' );
    system( strUsage.c_str() );
#endif
}

VOID CCmdArgsParser::DumpArgs()
{
    INT nBufLen = 0;
    CHAR szBuf[4096] = { 0 };

#ifndef _CONSOLE
    nBufLen += snprintf( &szBuf[nBufLen], _countof( szBuf ) - nBufLen, "zenity --info --text=\"" );
#endif

    nBufLen += snprintf( &szBuf[nBufLen], _countof( szBuf ) - nBufLen, "Binary name: %s\n", m_strBinName.c_str() );

    for ( MIT itNamed = m_mapNamedArgs.begin(); itNamed != m_mapNamedArgs.end(); itNamed++ )
    {
        nBufLen += snprintf( &szBuf[nBufLen], _countof( szBuf ) - nBufLen, "name=%s, value=%s\n",
                             itNamed->first.c_str(), itNamed->second.c_str() );
    }
    for ( VIT itUnnamed = m_vecUnnamedArgs.begin(); itUnnamed != m_vecUnnamedArgs.end(); itUnnamed++ )
    {
        nBufLen += snprintf( &szBuf[nBufLen], _countof( szBuf ) - nBufLen, "unamed value=%s\n", itUnnamed->c_str() );
    }

#ifdef _CONSOLE
    printf( "%s", szBuf );
#else
    nBufLen += snprintf( &szBuf[nBufLen], _countof( szBuf ) - nBufLen, "\"" );
    system( szBuf );
#endif
}

BOOL CCmdArgsParser::IsQuoted( IN CONST string & aString )
{
    return ( 2 <= aString.length() && '"' == aString[0] && '"' == aString[aString.length() - 1] ) ? TRUE : FALSE;
}

string CCmdArgsParser::AddQuoteIfHaveSpace( IN CONST string & aString )
{
    if ( string::npos == aString.find_first_of( ' ' ) )
    {
        return aString;
    }
    else
    {
        string strResult;
        strResult.reserve( sizeof( '\"' ) + aString.length() + sizeof( '\"' ) );
        strResult.push_back( '\"' );
        strResult.append( aString );
        strResult.push_back( '\"' );
        return strResult;
    }
}

string CCmdArgsParser::RemoveQuote( IN CONST string & aString )
{
    string strResult;
    if ( IsQuoted( aString ) )
    {
        strResult = aString.substr( 1, aString.length() - 2 );
    }
    else
    {
        strResult = aString;
    }
    return strResult;
}

BOOL CCmdArgsParser::SplitArgs( IN CONST std::string & aCmdLine, OUT std::vector<std::string> & aOutput )
{
    BOOL bQuoting = FALSE;
    BOOL bEscaping = FALSE;

    //Flush buffer to aOutput only when whitespace or terminator is met
    string strTmp;
    for ( size_t i = 0; i < aCmdLine.length(); i++ )
    {
        if ( bEscaping )
        {
            strTmp.push_back( aCmdLine[i] );
            bEscaping = FALSE;
        }
        else if ( '\\' == aCmdLine[i] )
        {
            bEscaping = TRUE;
        }
        else if ( '"' == aCmdLine[i] )
        {
            strTmp.push_back( aCmdLine[i] );
            if ( FALSE == bQuoting )
            {
                bQuoting = TRUE;
            }
            else
            {
                bQuoting = FALSE;
            }
        }
        else if ( ' ' == aCmdLine[i] )
        {
            if ( 0 < strTmp.length() )
            {
                if ( FALSE == bQuoting )
                {
                    aOutput.push_back( strTmp );
                    strTmp.clear();
                }
                else
                {
                    strTmp.push_back( aCmdLine[i] );
                }
            }
        }
        else
        {
            strTmp.push_back( aCmdLine[i] );
        }
    }

    BOOL bGoodEnd = ( FALSE == bQuoting && FALSE == bEscaping ) ? TRUE : FALSE;
    if ( 0 < strTmp.length() && bGoodEnd )
    {
        aOutput.push_back( strTmp );
        strTmp.clear();
    }

    return bGoodEnd;
}


}    //End of namespace CWUtils
