#include "stdafx.h"
#include "CWHttpParser.h"

#include <algorithm>
#include <string>
#include <map>

using namespace std;
using namespace CWUtils;

#include "_GenerateTmh.h"
#include "CWHttpParser.tmh"


#define MAX_MAP_TABLE_VALUE       0x7F //map table handles 0x00~0x7F

namespace CWUtils
{



typedef struct _HttpFieldMap
{
    CHAR szFieldName[32];
    UINT uFieldId;
} HttpFieldMap;

HttpFieldMap g_FieldTable[] =
{
    { "HEAD " ,               HTTP_FIELD_METHOD_HEAD } ,
    { "GET " ,                HTTP_FIELD_METHOD_GET } ,
    { "POST " ,               HTTP_FIELD_METHOD_POST } ,
    { "PUT " ,                HTTP_FIELD_METHOD_PUT } ,
    { "DELETE " ,             HTTP_FIELD_METHOD_DELETE } ,
    { "TRACE " ,              HTTP_FIELD_METHOD_TRACE } ,
    { "OPTIONS " ,            HTTP_FIELD_METHOD_OPTIONS } ,
    { "CONNECT " ,            HTTP_FIELD_METHOD_CONNECT } ,
    { "PATCH " ,              HTTP_FIELD_METHOD_PATCH } ,
    { "HTTP/" ,               HTTP_FIELD_HTTP_RESPONSE } ,
    { "Host:" ,               HTTP_FIELD_HOST } ,
    { "Proxy-Connection:" ,   HTTP_FIELD_PROXY_CONNECTION } ,
    { "Referer:" ,            HTTP_FIELD_REFERER } ,
    { "User-Agent:" ,         HTTP_FIELD_USER_AGENT } ,
    { "Accept:" ,             HTTP_FIELD_ACCEPT } ,
    { "Accept-Encoding:" ,    HTTP_FIELD_ACCEPT_ENCODING } ,
    { "Accept-Language:" ,    HTTP_FIELD_ACCEPT_LANGUAGE } ,
    { "Accept-Charset:" ,     HTTP_FIELD_ACCEPT_CHARSET } ,
    { "Accept-Ranges:" ,      HTTP_FIELD_ACCEPT_RANGES } ,
    { "Transfer-Encoding:" ,  HTTP_FIELD_TRANSFER_ENCODING } ,
    { "Cookie:" ,             HTTP_FIELD_COOKIE } ,
    { "Server:" ,             HTTP_FIELD_SERVER } ,
    { "Connection:" ,         HTTP_FIELD_CONNECTION } ,
    { "Keep-Alive:" ,         HTTP_FIELD_KEEP_ALIVE } ,
    { "Date:" ,               HTTP_FIELD_DATE } ,
    { "Last-Modified:" ,      HTTP_FIELD_LAST_MODIFIED } ,
    { "ETag:" ,               HTTP_FIELD_ETAG } ,
    { "Cache-Control:" ,      HTTP_FIELD_CACHE_CONTROL } ,
    { "Vary:" ,               HTTP_FIELD_VARY } ,
    { "Via:" ,                HTTP_FIELD_VIA } ,
    { "Content-Length:" ,     HTTP_FIELD_CONTENT_LENGTH } ,
    { "Content-Type:" ,       HTTP_FIELD_CONTENT_TYPE } ,
    { "Content-Encoding:" ,   HTTP_FIELD_CONTENT_ENCODING } ,
    { "x-flash-version:" ,    HTTP_FIELD_X_FLASH_VERSION } ,
    { "x-sessioncookie:" ,    HTTP_FIELD_X_SESSION_COOKIE } ,
    { "X-Powered-By:" ,       HTTP_FIELD_X_POWERED_BY } ,
    { "X-Vary-Option:" ,      HTTP_FIELD_X_VARY_OPTION } ,
    { "X-Cache:" ,            HTTP_FIELD_X_CACHE } ,
    { "X-Cache-Lookup:" ,     HTTP_FIELD_X_CACHE_LOOKUP } ,
    { "\r\n" ,                HTTP_FIELD_END } ,        //check header end
    { "\n" ,                  HTTP_FIELD_END }         //check header end
}; 



//================================================================================
//================================ CHttpFieldMgr =================================
//================================================================================
VOID CHttpFieldMgr::Reset()
{
    m_uMajorVer = 0;
    m_uMinorVer = 0;
    m_bHeaderEnd = FALSE;
    m_vecParsedHeader.clear();
    m_mapFields.clear();
    m_mapUnknownFields.clear();
    m_bBodyEnd = FALSE;
    m_uTransferEncodingType = HTTP_TRANSFER_ENCODING_UNKNOWN;
    m_uContentEncodingType = HTTP_CONTENT_ENCODING_IDENTITY;
    m_uContentLength = 0;
    m_vecBody.clear();
}

HttpParserErr CHttpFieldMgr::GetFieldValue( HttpFieldId aFieldId , std::string & aData )
{
    std::map<HttpFieldId , std::string>::iterator it = m_mapFields.find( aFieldId );
    if ( m_mapFields.end() != it )
    {
        aData.assign( it->second.c_str() , it->second.size() );
        return HTTP_PARSER_SUCCESS;
    }
    else
    {
        return HTTP_PARSER_FAILURE;
    }
}

HttpParserErr CHttpFieldMgr::SetField( HttpFieldId aFieldId , std::string & aData , UINT aDataSize , BOOL aAppend , BOOL aComplete )
{
    map<HttpFieldId , string>::iterator it = m_mapFields.find( aFieldId );
    if ( aAppend && ( m_mapFields.end() != it ) )
    {
        it->second.append( aData.c_str() , aDataSize );
    }
    else
    {
        if ( m_mapFields.end() == it )
        {
            pair<map<HttpFieldId , string>::iterator , BOOL> p = m_mapFields.insert( make_pair(aFieldId , aData) );
            it = p.first;
        }
        else
        {
            it->second.assign( aData.c_str() , aDataSize );
        }
    }

    if ( aComplete )    //Trim data
    {
        CWUtils::TrimStringA( it->second , " \r" );
    }
    return HTTP_PARSER_SUCCESS;
}

HttpParserErr CHttpFieldMgr::SetUnknownField( std::string aFieldName , std::string & aData , UINT aDataSize , BOOL aAppend , BOOL aComplete )
{
    map<string , string>::iterator it = m_mapUnknownFields.find( aFieldName );
    if ( aAppend && ( m_mapUnknownFields.end() != it ) )
    {
        it->second.append( aData.c_str() , aDataSize );
    }
    else
    {
        if ( m_mapUnknownFields.end() == it )
        {
            pair<map<string , string>::iterator , BOOL> p = m_mapUnknownFields.insert( make_pair(aFieldName , aData) );
            it = p.first;
        }
        else
        {
            it->second.assign( aData.c_str() , aDataSize );
        }
    }

    if ( aComplete )    //Trim data
    {
        CWUtils::TrimStringA( it->second , " \r" );
    }
    return HTTP_PARSER_SUCCESS;
}

VOID CHttpFieldMgr::GetVersion( UINT & aMajorVer , UINT & aMinorVer )
{
    aMajorVer = m_uMajorVer;
    aMinorVer = m_uMinorVer;
}

VOID CHttpFieldMgr::SetVersion( UINT aMajorVer , UINT aMinorVer )
{
    m_uMajorVer = aMajorVer;
    m_uMinorVer = aMinorVer;
}

BOOL CHttpFieldMgr::IsHeaderEnd()
{
    return m_bHeaderEnd;
}

VOID CHttpFieldMgr::SetHeaderEnd( BOOL aEnd )
{
    m_bHeaderEnd = aEnd;
}

UINT CHttpFieldMgr::GetParsedHeadersCount()
{
    return (UINT)m_vecParsedHeader.size();
}

VOID CHttpFieldMgr::GetParsedHeaders( std::vector<std::string> & aHeaders )
{
    aHeaders = m_vecParsedHeader;
}



BOOL CHttpFieldMgr::IsBodyEnd()
{
    return m_bBodyEnd;
}

VOID CHttpFieldMgr::SetBodyEnd( BOOL aEnd )
{
    m_bBodyEnd = aEnd;
}

HttpTransferEncodingType CHttpFieldMgr::GetTransferEncodingType()
{
    return m_uTransferEncodingType;
}

HttpContentEncodingType CHttpFieldMgr::GetContentEncodingType()
{
    return m_uContentEncodingType;
}


UINT CHttpFieldMgr::GetContentLength()
{
    //We do not use length in field map.
    //Because in some cases(e.g. method is HEAD), the content length is always 0 no matter what value is.
    return m_uContentLength;
}

VOID CHttpFieldMgr::ReserveBodySize( SIZE_T aSize )
{
    m_vecBody.reserve( aSize );
}


UINT CHttpFieldMgr::GetCurrentBodyLength()
{
    return m_vecBody.size();
}

const UCHAR * CHttpFieldMgr::GetCurrentBodyPtr()
{
    return m_vecBody.data();
}

HttpParserErr CHttpFieldMgr::InsertBody( const UCHAR * aBuf , UINT aBufSize )
{
    DbgOut( VERB , DBG_PROTOHANDLER , "Enter. m_vecBody=0x%p (%Iu/%Iu)" , m_vecBody.data() , m_vecBody.size() , m_vecBody.capacity() );

    HttpParserErr uParserRet = HTTP_PARSER_FAILURE;

    do 
    {
        if ( NULL == aBuf || aBufSize == 0 )
        {
            DbgOut( WARN , DBG_PROTOHANDLER , "Skip empty buffer" );
            uParserRet = HTTP_PARSER_SUCCESS;
            break;
        }

        m_vecBody.insert( m_vecBody.end() , aBuf , aBuf + aBufSize );
        uParserRet = HTTP_PARSER_SUCCESS;
    } while ( 0 );
    

    DbgOut( VERB , DBG_PROTOHANDLER , "Leave. m_vecBody=0x%p (%Iu/%Iu)" , m_vecBody.data() , m_vecBody.size() , m_vecBody.capacity() );
    return uParserRet;
}

HttpParserErr CHttpFieldMgr::ParseHttpVersion( const std::string & aVersion )
{
    if ( 3 <= aVersion.length() && isdigit(aVersion[0]) && ('.' == aVersion[1]) && isdigit(aVersion[2]) )
    {
        SetVersion( (UINT)(aVersion[0] - '0') , (UINT)(aVersion[2] - '0') );
        return HTTP_PARSER_SUCCESS;
    }
    return HTTP_PARSER_FAILURE;
}

HttpParserErr CHttpFieldMgr::ParseContentLength()
{
    std::string data;
    if ( HTTP_PARSER_SUCCESS == GetFieldValue( HTTP_FIELD_CONTENT_LENGTH , data ) && 0 < data.length() )
    {
        m_uContentLength = strtoul( data.c_str() , NULL , 10 );
        return HTTP_PARSER_SUCCESS;
    }
    return HTTP_PARSER_FAILURE;
}

BOOL CHttpFieldMgr::IsChunkedTransferEncoding()
{
    std::string data;
    if ( HTTP_PARSER_SUCCESS == GetFieldValue( HTTP_FIELD_TRANSFER_ENCODING , data ) && std::string::npos != data.find( "chunked" ) )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

VOID CHttpFieldMgr::UpdateContentEncodingType()
{
    std::string data;
    if ( HTTP_PARSER_SUCCESS == GetFieldValue( HTTP_FIELD_CONTENT_ENCODING , data ) )
    {
        if ( data == "gzip" )
        {
            m_uContentEncodingType = HTTP_CONTENT_ENCODING_GZIP;
        }
        else if ( data == "deflate" )
        {
            m_uContentEncodingType = HTTP_CONTENT_ENCODING_DEFLATE;
        }
        else if ( data == "compress" )
        {
            m_uContentEncodingType = HTTP_CONTENT_ENCODING_COMPRESS;
        }
        else if ( data == "identity" )
        {
            m_uContentEncodingType = HTTP_CONTENT_ENCODING_IDENTITY;
        }
        else if ( data.length() > 0 )
        {
            m_uContentEncodingType = HTTP_CONTENT_ENCODING_UNKNOWN_NEW;
        }
        else
        {
            m_uContentEncodingType = HTTP_CONTENT_ENCODING_IDENTITY;
        }
    }
}













//================================================================================
//=============================== CHttpReqFieldMgr ===============================
//================================================================================
VOID CHttpReqFieldMgr::Reset()
{
    CHttpFieldMgr::Reset();
    m_bHasMethod = FALSE;
    m_uMethodId = HTTP_FIELD_EMPTY;
    m_strUri.clear();
    m_strRequestLine.clear();
    m_strAgent.clear();
    m_strHost.clear();
    m_strReferer.clear();
}

HttpParserErr CHttpReqFieldMgr::SetField( HttpFieldId aFieldId , std::string & aData , UINT aDataSize , BOOL aAppend , BOOL aComplete )
{
    DbgOut( VERB , DBG_PROTOHANDLER , "Enter. aFieldId=%u, aData=%hs, aAppend=%d, aComplete=%d" , aFieldId , aData.c_str() , aAppend , aComplete );
    HttpParserErr uParserRet = CHttpFieldMgr::SetField( aFieldId , aData , aDataSize , aAppend , aComplete );
    if ( (HTTP_PARSER_SUCCESS == uParserRet) && aComplete )
    {
        std::string strFieldName;
        for ( SIZE_T i = 0 ; i < _countof(g_FieldTable) ; i++ )
        {
            if ( g_FieldTable[i].uFieldId == aFieldId )
            {
                strFieldName.assign( g_FieldTable[i].szFieldName );
                break;
            }
        }

        if ( HTTP_FIELD_METHOD_BEGIN < aFieldId && aFieldId < HTTP_FIELD_METHOD_END )
        {
            m_bHasMethod = TRUE;
            m_uMethodId = aFieldId;
            ParseRequestVersionAndUri( aFieldId );
            m_strRequestLine = strFieldName + aData;
        }
        else
        {
            switch ( aFieldId )
            {
                case HTTP_FIELD_TRANSFER_ENCODING :
                {
                    if ( IsChunkedTransferEncoding() )
                    {
                        m_uTransferEncodingType = HTTP_TRANSFER_ENCODING_CHUNKED;
                    }
                    break;
                }
                case HTTP_FIELD_CONTENT_ENCODING :
                {
                    UpdateContentEncodingType();
                    break;
                }
                case HTTP_FIELD_CONTENT_LENGTH :
                {
                    if ( HTTP_FIELD_METHOD_HEAD == m_uMethodId ) //When method is HEAD, content length always equals 0
                    {
                        m_uContentLength = 0;
                    }
                    else
                    {
                        this->ParseContentLength();
                    }

                    //According to RFC 2616 section 4.4, chunked encoding has higher priority than content-length
                    //So we only change it if the transfer encoding type is still unknown
                    if ( HTTP_TRANSFER_ENCODING_UNKNOWN == m_uTransferEncodingType )
                    {
                        m_uTransferEncodingType = HTTP_TRANSFER_ENCODING_CONTENT_LENGTH;
                    }
                    break;
                }
                case HTTP_FIELD_USER_AGENT :
                {
                    //GetFieldValue(HTTP_FIELD_USER_AGENT, m_strAgent);
                    m_strAgent = aData;
                    CWUtils::TrimStringA( m_strAgent , " " );
                    break;
                }
                case HTTP_FIELD_HOST :
                {
                    //GetFieldValue(HTTP_FIELD_HOST, m_strHost);
                    m_strHost = aData;
                    CWUtils::TrimStringA( m_strHost , " " );
                    break;
                }
                case HTTP_FIELD_REFERER :
                {
                    m_strReferer = aData;
                    CWUtils::TrimStringA( m_strReferer , " " );
                    break;
                }
                case HTTP_FIELD_HTTP_RESPONSE :    //Incorrect state
                {
                    uParserRet = HTTP_PARSER_FAILURE;
                    break;
                }
                default :
                {
                    break;
                }
            }
        }

        m_vecParsedHeader.push_back( strFieldName + aData );
    }

    DbgOut( VERB , DBG_PROTOHANDLER , "Leave. uParserRet=%u" , uParserRet );
    return uParserRet;
}

const CHAR * CHttpReqFieldMgr::GetUri()
{
    return m_strUri.c_str();
}

HttpFieldId CHttpReqFieldMgr::GetMethodId()
{
    return m_uMethodId;
}

VOID CHttpReqFieldMgr::GetRequestLine( std::string & aRequestLine )
{
    aRequestLine = m_strRequestLine;
}

VOID CHttpReqFieldMgr::GetUserAgent( std::string & aAgent )
{
    aAgent = m_strAgent;
}

VOID CHttpReqFieldMgr::GetHost( std::string & aHost )
{
    aHost = m_strHost;
}

VOID CHttpReqFieldMgr::GetReferer( std::string & aReferer )
{
    aReferer = m_strReferer;
}


HttpParserErr CHttpReqFieldMgr::ParseRequestVersionAndUri( HttpFieldId aFieldId )
{
    HttpParserErr uParserRet = HTTP_PARSER_FAILURE;

    do 
    {
        std::string strUri;
        const CHAR * szRequestVerMark = " HTTP/";

        if ( HTTP_PARSER_SUCCESS == GetFieldValue( aFieldId , strUri ) )
        {
            std::string strUriUpper = strUri;
            std::transform( strUri.begin() , strUri.end() , strUriUpper.begin() , ::toupper );
            SIZE_T pos = strUriUpper.rfind( szRequestVerMark );
            if ( std::string::npos != pos )
            {
                std::string ver = strUri.substr( pos + strlen(szRequestVerMark) );
                if ( HTTP_PARSER_SUCCESS != ParseHttpVersion( ver ) )
                {
                    break;
                }

                strUri.resize( pos );
                CWUtils::TrimStringA( strUri , " " );
                m_strUri = strUri;
                uParserRet = HTTP_PARSER_SUCCESS;
            }
        }
    } while ( 0 );
    
    return uParserRet;
}





//================================================================================
//=============================== CHttpRspFieldMgr ===============================
//================================================================================
VOID CHttpRspFieldMgr::Reset()
{
    CHttpFieldMgr::Reset();
    m_uContentLength = INFINITE;    //To compatible with HTTP 1.0 
    m_bIsChunked = FALSE;
    m_uStatusCode = HTTP_STATUS_CODE_UNKNOWN;
}

HttpParserErr CHttpRspFieldMgr::SetField( HttpFieldId aFieldId , std::string & aData , UINT aDataSize , BOOL aAppend , BOOL aComplete )
{
    DbgOut( VERB , DBG_PROTOHANDLER , "Enter. aFieldId=%u, aData=%hs, aAppend=%d, aComplete=%d" , aFieldId , aData.c_str() , aAppend , aComplete );
    HttpParserErr uParserRet = CHttpFieldMgr::SetField( aFieldId , aData , aDataSize , aAppend , aComplete );
    if ( (HTTP_PARSER_SUCCESS == uParserRet) && aComplete )
    {
        std::string strFieldName;
        for ( SIZE_T i = 0 ; i < _countof(g_FieldTable) ; i++ )
        {
            if ( g_FieldTable[i].uFieldId == aFieldId )
            {
                strFieldName.assign( g_FieldTable[i].szFieldName );
                break;
            }
        }

        if ( HTTP_FIELD_METHOD_BEGIN < aFieldId && aFieldId < HTTP_FIELD_METHOD_END )
        {
            uParserRet = HTTP_PARSER_FAILURE;
        }
        else
        {
            switch ( aFieldId )
            {
                case HTTP_FIELD_TRANSFER_ENCODING :
                {
                    if ( IsChunkedTransferEncoding() )
                    {
                        m_uTransferEncodingType = HTTP_TRANSFER_ENCODING_CHUNKED;
                    }
                    break;
                }
                case HTTP_FIELD_CONTENT_ENCODING :
                {
                    UpdateContentEncodingType();
                    break;
                }
                case HTTP_FIELD_HTTP_RESPONSE :
                {
                    m_strStatusLine = strFieldName + aData;
                    uParserRet = ParseResponseVersionAndCode();
                    break;
                }
                case HTTP_FIELD_CONTENT_LENGTH :
                {
                    if ( (HTTP_STATUS_CODE_OK > m_uStatusCode) || (HTTP_STATUS_CODE_NO_CONTENT == m_uStatusCode) || (HTTP_STATUS_CODE_NOT_MODIFIED == m_uStatusCode) )
                    {
                        m_uContentLength = 0;
                    }
                    else
                    {
                        ParseContentLength();
                    }

                    //According to RFC 2616 section 4.4, chunked encoding has higher priority than content-length
                    //So we only change it if the transfer encoding type is still unknown
                    if ( HTTP_TRANSFER_ENCODING_UNKNOWN == m_uTransferEncodingType )
                    {
                        m_uTransferEncodingType = HTTP_TRANSFER_ENCODING_CONTENT_LENGTH;
                    }
                    break;
                }
                default :
                {
                    break;
                }
            }

            m_vecParsedHeader.push_back( strFieldName + aData );
        }
    }

    DbgOut( VERB , DBG_PROTOHANDLER , "Leave. uParserRet=%u" , uParserRet );
    return uParserRet;
}

HttpStatusCode CHttpRspFieldMgr::GetStatusCode()
{
    return m_uStatusCode;
};


VOID CHttpRspFieldMgr::GetStatusLine( std::string & aStatusLine )
{
    aStatusLine = m_strStatusLine;
}

HttpParserErr CHttpRspFieldMgr::ParseResponseVersionAndCode()
{
    HttpParserErr uParserRet = HTTP_PARSER_FAILURE;

    do 
    {
        std::string data;
        if ( HTTP_PARSER_SUCCESS == GetFieldValue( HTTP_FIELD_HTTP_RESPONSE , data ) )
        {
            if ( 7 > data.length() )    //Version length + space + status code length
            {
                break;
            }

            if ( ParseHttpVersion( data ) != HTTP_PARSER_SUCCESS )
            {
                break;
            }

            if ( !isdigit(data[4]) || !isdigit(data[5]) || !isdigit(data[6]) )
            {
                break;
            }

            m_uStatusCode = (HttpStatusCode)( ((data[4] - '0') * 100) + ((data[5] - '0') * 10) + (data[6] - '0') );
            DbgOut( INFO , DBG_PROTOHANDLER , "m_uStatusCode=%u" , m_uStatusCode );
            
            //All 1xx, 204, and 304 responses must not include a body
            if ( (HTTP_STATUS_CODE_OK > m_uStatusCode) || (HTTP_STATUS_CODE_NO_CONTENT == m_uStatusCode) || (HTTP_STATUS_CODE_NOT_MODIFIED == m_uStatusCode) )
            {
                m_uContentLength = 0;
            }

            uParserRet = HTTP_PARSER_SUCCESS;
        }
    } while ( 0 );
    return uParserRet;
}




















//================================================================================
//================================= CHttpParser ==================================
//================================================================================
BOOL CHttpParser::Init( VOID * aCbkCtx , ParserUpdateCallback aHeaderCbk , ParserUpdateCallback aBodyCbk ,
                        UINT aMaxHeaderCnt , UINT aMaxBodySize )
{
    BOOL bRet = FALSE;
    do 
    {
        std::map<std::string , UINT> mapFields;
        for ( INT i = 0 ; i < _countof(g_FieldTable) ; i++ )
        {
            mapFields[g_FieldTable[i].szFieldName] = g_FieldTable[i].uFieldId;
        }
        m_FieldMatcher = new (std::nothrow) CPrefixTree<UINT>( FALSE , mapFields );
        if ( NULL == m_FieldMatcher )
        {
            break;
        }

        m_pCbkCtx = aCbkCtx;
        m_pfnHeaderCbk = aHeaderCbk;
        m_pfnBodyCbk = aBodyCbk;

        m_uMaxHeaderCnt = aMaxHeaderCnt;
        m_uMaxBodySize = aMaxBodySize;

        bRet = TRUE;
    } while ( 0 );
    
    return bRet;
}

VOID CHttpParser::Reset()
{
    m_ReqFieldMgr.Reset();
    m_ReqIncompleteHeaderState.Reset();
    m_ReqChunkState.Reset();
    m_RspFieldMgr.Reset();
    m_RspIncompleteHeaderState.Reset();
    m_RspChunkState.Reset();
    m_uLastParserErr = HTTP_PARSER_NEED_MORE_DATA;
}

VOID CHttpParser::UnInit()
{
    m_pCbkCtx = NULL;
    m_pfnHeaderCbk = NULL;
    m_pfnBodyCbk = NULL;

    if ( NULL != m_FieldMatcher )
    {
        delete m_FieldMatcher;
        m_FieldMatcher = NULL;
    }
}

HttpParserErr CHttpParser::Input( BOOL aConnOut , const UCHAR * aBuf , UINT aBufSize , INT & aCallbackRet )
{
    DbgOut( VERB , DBG_PROTOHANDLER , "Enter. aConnOut=%d" , aConnOut );

    HttpParserErr uParserRet = m_uLastParserErr;

    do 
    {
        if ( NULL == aBuf || 0 == aBufSize )
        {
            DbgOut( WARN , DBG_PROTOHANDLER , "Empty buffer. aBuf=0x%p, aBufSize=%lu" , aBuf , aBufSize );
            break;
        }

        if ( HTTP_PARSER_SUCCESS != m_uLastParserErr && HTTP_PARSER_NEED_MORE_DATA != m_uLastParserErr )
        {
            DbgOut( WARN , DBG_PROTOHANDLER , "Stop parsing because of previous state disable parsing. m_uLastParserErr=%u" , m_uLastParserErr );
            break;
        }

        ParseParam parseParam;
        parseParam.bConnOut = aConnOut;
        parseParam.pBuf = aBuf;
        parseParam.uBufSize = aBufSize;
        if ( aConnOut )
        {
            parseParam.pFieldMgr = &m_ReqFieldMgr;
            parseParam.pIncompleteState = &m_ReqIncompleteHeaderState;
            parseParam.pChunkState = &m_ReqChunkState;
        }
        else
        {
            parseParam.pFieldMgr = &m_RspFieldMgr;
            parseParam.pIncompleteState = &m_RspIncompleteHeaderState;
            parseParam.pChunkState = &m_RspChunkState;
        }

        if ( parseParam.pFieldMgr->IsBodyEnd() )    //Handle any special cases that doesn't have body
        {
            parseParam.pFieldMgr->Reset();
        }

        if ( parseParam.pFieldMgr->IsHeaderEnd() ) //Parse body
        {
            uParserRet = (HttpParserErr)ParseBody( &parseParam );
            if ( m_pfnBodyCbk )
            {
                if ( FALSE == m_pfnBodyCbk( m_pCbkCtx , aConnOut , uParserRet , &aCallbackRet ) )
                {
                    DbgOut( INFO , DBG_PROTOHANDLER , "Stop parsing because of callback. uParserRet=%u, nCallbackRet=%d" , uParserRet , aCallbackRet );
                    uParserRet = HTTP_PARSER_USER_ABORT;
                    break;
                }
            }
        }
        else
        {
            uParserRet = (HttpParserErr)ParseHeader( &parseParam );
            if ( m_pfnHeaderCbk )
            {
                if ( FALSE == m_pfnHeaderCbk( m_pCbkCtx , aConnOut , uParserRet , &aCallbackRet ) )
                {
                    DbgOut( INFO , DBG_PROTOHANDLER , "Stop parsing because of callback. uParserRet=%u, nCallbackRet=%d" , uParserRet , aCallbackRet );
                    uParserRet = HTTP_PARSER_USER_ABORT;
                    break;
                }
            }


            if ( parseParam.pFieldMgr->IsHeaderEnd() )
            {
                uParserRet = (HttpParserErr)ParseBody( &parseParam );
                if ( m_pfnBodyCbk )
                {
                    if ( FALSE == m_pfnBodyCbk( m_pCbkCtx , aConnOut , uParserRet , &aCallbackRet ) )
                    {
                        DbgOut( INFO , DBG_PROTOHANDLER , "Stop parsing because of callback. uParserRet=%u, nCallbackRet=%d" , uParserRet , aCallbackRet );
                        uParserRet = HTTP_PARSER_USER_ABORT;
                        break;
                    }
                }
            }
        }
    } while ( 0 );
    
    m_uLastParserErr = uParserRet;
    DbgOut( VERB , DBG_PROTOHANDLER , "Leave. uParserRet=%u, nCallbackRet=%d" , uParserRet , aCallbackRet );
    return uParserRet;
}

HttpParserErr CHttpParser::ParseHeader( ParseParam * aParam )
{
    DbgOut( VERB , DBG_PROTOHANDLER , "Enter" );
    
    HttpParserErr uParserRet = HTTP_PARSER_NEED_MORE_DATA;

    do 
    {
        if ( NULL == aParam->pBuf || 0 == aParam->uBufSize ||
             NULL == aParam->pFieldMgr || NULL == aParam->pIncompleteState )
        {
            DbgOut( WARN , DBG_PROTOHANDLER , "Unexpected parameter. pBuf=0x%p, uBufSize=%u, pFieldMgr=0x%p, pIncompleteState=0x%p" ,
                                              aParam->pBuf , aParam->uBufSize , aParam->pFieldMgr , aParam->pIncompleteState );
            uParserRet = HTTP_PARSER_FAILURE;
            break;
        }

        //If current Field id is HTTP_FIELD_INCOMPLETE, we don't need call ParseIncompleteLine function
        //because DoFieldSearch function will complete matching as normal case
        //ParseIncompleteLine() handle either known-complete field with incomplete data or unknown field
        if ( (HTTP_FIELD_EMPTY != aParam->pIncompleteState->uFieldId) && (HTTP_FIELD_INCOMPLETE != aParam->pIncompleteState->uFieldId) )
        {
            uParserRet = ParseIncompleteLine( aParam );
            if ( HTTP_PARSER_SUCCESS != uParserRet )
            {
                DbgOut( ERRO , DBG_PROTOHANDLER , "ParseIncompleteLine() failed" );
                break;
            }
        }

        //Clear previous state
        if ( HTTP_FIELD_INCOMPLETE != aParam->pIncompleteState->uFieldId )
        {
            aParam->pIncompleteState->Reset();
        }

        //Check max header count
        if ( m_uMaxHeaderCnt < aParam->pFieldMgr->GetParsedHeadersCount() )
        {
            DbgOut( WARN , DBG_PROTOHANDLER , "MaxHeaderCnt exceed. Current/Max=%u/%u" , aParam->pFieldMgr->GetParsedHeadersCount() , m_uMaxHeaderCnt );
            uParserRet = HTTP_PARSER_DATA_EXCEED;
            break;
        }

        //Parse start from field
        const UCHAR * pBufBeg = aParam->pBuf;
        const UCHAR * pBufEnd = aParam->pBuf + aParam->uBufSize;
        DbgOut( INFO , DBG_PROTOHANDLER , "pBufBeg=0x%p, pBufEnd=0x%p, uBufSize=%u" , pBufBeg , pBufEnd , aParam->uBufSize );

        UCHAR * pLineEnd = NULL;
        HttpFieldId uFieldId = HTTP_FIELD_EMPTY;
        UINT uFieldSize = 0;
        UINT uLineSize = 0;
        UINT uMatcherRet = CPrefixTree<UINT>::MISS;
        std::string strFieldData;

        while ( pBufBeg < pBufEnd )
        {
            DbgOut( INFO , DBG_PROTOHANDLER , "pBufBeg=0x%p, pBufEnd=0x%p, uBufSize=%u" , pBufBeg , pBufEnd , aParam->uBufSize );

            uMatcherRet = DoFieldSearch( *m_FieldMatcher , pBufBeg , (UINT)(pBufEnd - pBufBeg) , aParam->pIncompleteState , &uFieldId , &uFieldSize );

            DbgOut( INFO , DBG_PROTOHANDLER , "DoFieldSearch(). uMatcherRet=%u, uFieldId=%u, uFieldSize=%u" , uMatcherRet , uFieldId , uFieldSize );

            if ( CPrefixTree<UINT>::HIT == uMatcherRet ) //HIT
            {
                pBufBeg += uFieldSize;  //Move to the end position of Field
                if ( HTTP_FIELD_END == uFieldId )
                {
                    aParam->uBufSize = pBufEnd - pBufBeg;
                    DbgOut( INFO , DBG_PROTOHANDLER , "aParam->uBufSize = 0x%p - 0x%p = %u" , pBufEnd , pBufBeg , aParam->uBufSize );

                    aParam->pBuf = (UCHAR *)pBufBeg;
                    aParam->pFieldMgr->SetHeaderEnd( TRUE );
                    uParserRet = HTTP_PARSER_SUCCESS;
                    break;
                }

                if ( HTTP_PARSER_SUCCESS == FindLineEndDelimiter( pBufBeg , (UINT)(pBufEnd - pBufBeg) , &pLineEnd , &uLineSize ) )
                {
                    strFieldData.assign( (const CHAR *)pBufBeg , uLineSize );
                    aParam->pFieldMgr->SetField( uFieldId , strFieldData , uLineSize , FALSE , TRUE );

                    uParserRet = HTTP_PARSER_NEED_MORE_DATA;
                    pBufBeg = pLineEnd; //Move to next line
                }
                else //Known-complete field name with incomplete field data
                {
                    aParam->pIncompleteState->bIsIncompleteFieldName = FALSE;
                    aParam->pIncompleteState->uFieldId = uFieldId;
                    uLineSize = (pBufEnd - pBufBeg);
                    strFieldData.assign( (const CHAR *)pBufBeg , uLineSize );
                    aParam->pFieldMgr->SetField( uFieldId , strFieldData , uLineSize , FALSE , FALSE );

                    uParserRet = HTTP_PARSER_NEED_MORE_DATA;
                    pBufBeg = pLineEnd;
                    break;
                }
            }
            else if ( CPrefixTree<UINT>::MORE_ONE == uMatcherRet ) //Known-partially-complete field name
            {
                aParam->pIncompleteState->uFieldId = HTTP_FIELD_INCOMPLETE;
                aParam->pIncompleteState->strUnknownFieldName.append( (const CHAR *)pBufBeg , (pBufEnd - pBufBeg) );    //In case nothing matched finally
                uParserRet = HTTP_PARSER_NEED_MORE_DATA;
                pBufBeg = pBufEnd;
                break;
            }
            else //Nothing matched
            {
                DbgOut( INFO , DBG_PROTOHANDLER , "Encounter a nothing matched area!!!" );

                CHAR * pFieldEnd = (CHAR *)memchr( pBufBeg , ':' , (pBufEnd - pBufBeg) );
                if ( pFieldEnd != NULL )
                {
                    aParam->pIncompleteState->bIsIncompleteFieldName = FALSE;
                    uLineSize = (UINT)(pFieldEnd - (CHAR *)pBufBeg); //record field name till ':'
                    std::string strFieldName( (const CHAR *)pBufBeg , uLineSize );
                    DbgOut( WARN , DBG_PROTOHANDLER , "Unknown field. strFieldName=%hs" , strFieldName.c_str() );

                    pBufBeg = (UCHAR *)pFieldEnd + 1;
                    if ( HTTP_PARSER_SUCCESS == FindLineEndDelimiter( pBufBeg , (UINT)(pBufEnd - pBufBeg) , &pLineEnd , &uLineSize ) )
                    {
                        strFieldData.assign( (const CHAR *)pBufBeg , uLineSize );
                        aParam->pFieldMgr->SetUnknownField( strFieldName , strFieldData , uLineSize , FALSE , TRUE );

                        uParserRet = HTTP_PARSER_NEED_MORE_DATA;
                        pBufBeg = pLineEnd;
                    }
                    else //Unknown-complete field name with incomplete field data
                    {
                        aParam->pIncompleteState->uFieldId = HTTP_FIELD_UNKNOWN;
                        uLineSize = (pBufEnd - pBufBeg);
                        strFieldData.assign( (const CHAR *)pBufBeg , uLineSize );
                        aParam->pFieldMgr->SetUnknownField( strFieldName , strFieldData , uLineSize , FALSE , FALSE );

                        uParserRet = HTTP_PARSER_NEED_MORE_DATA;
                        pBufBeg = pLineEnd;
                        break;
                    }
                }
                else    //Unknown-incomplete field name
                {
                    aParam->pIncompleteState->bIsIncompleteFieldName = TRUE;
                    aParam->pIncompleteState->strUnknownFieldName.assign( (const CHAR *)pBufBeg , (pBufEnd - pBufBeg) );
                    aParam->pIncompleteState->uFieldId = HTTP_FIELD_UNKNOWN;
                    DbgOut( WARN , DBG_PROTOHANDLER , "Incomplete unknown field. strUnknownFieldName=%hs" , aParam->pIncompleteState->strUnknownFieldName.c_str() );
                    
                    uParserRet = HTTP_PARSER_NEED_MORE_DATA;
                    pBufBeg = pBufEnd;
                    break;
                }
            }

            //Check max header count
            if ( m_uMaxHeaderCnt < aParam->pFieldMgr->GetParsedHeadersCount() )
            {
                DbgOut( WARN , DBG_PROTOHANDLER , "MaxHeaderCnt exceed. Current/Max=%u/%u" , aParam->pFieldMgr->GetParsedHeadersCount() , m_uMaxHeaderCnt );
                uParserRet = HTTP_PARSER_DATA_EXCEED;
                break;
            }
        }

        aParam->pBuf = pBufBeg;
        aParam->uBufSize = pBufEnd - pBufBeg;

        //Check max header count
        if ( m_uMaxHeaderCnt < aParam->pFieldMgr->GetParsedHeadersCount() )
        {
            DbgOut( WARN , DBG_PROTOHANDLER , "MaxHeaderCnt exceed. Current/Max=%u/%u" , aParam->pFieldMgr->GetParsedHeadersCount() , m_uMaxHeaderCnt );
            uParserRet = HTTP_PARSER_DATA_EXCEED;
            break;
        }
    } while ( 0 );
    
    DbgOut( VERB , DBG_PROTOHANDLER , "Leave. uParserRet=%u" , uParserRet );
    return uParserRet;
}

HttpParserErr CHttpParser::ParseBody( ParseParam * aParam )
{
    DbgOut( VERB , DBG_PROTOHANDLER , "Enter. pBuf=0x%p, uBufSize=%u" , aParam->pBuf , aParam->uBufSize );
    
    HttpParserErr uParserRet = HTTP_PARSER_FAILURE;

    do 
    {
        //We don't check buf length because it could be 0 after header parse
        if ( NULL == aParam->pFieldMgr || NULL == aParam->pIncompleteState )
        {
            DbgOut( ERRO , DBG_PROTOHANDLER , "Unexpected parameter. pBuf=0x%p, uBufSize=%u, pFieldMgr=0x%p, pIncompleteState=0x%p" ,
                                              aParam->pBuf , aParam->uBufSize , aParam->pFieldMgr , aParam->pIncompleteState );
            uParserRet = HTTP_PARSER_FAILURE;
            break;
        }

        //Handle the case that should not have body
        if ( FALSE == aParam->bConnOut )
        {
            HttpFieldId uMethod = m_ReqFieldMgr.GetMethodId();
            HttpStatusCode uStatus = m_RspFieldMgr.GetStatusCode();
            if ( HTTP_FIELD_METHOD_HEAD == uMethod || HTTP_STATUS_CODE_OK > uStatus ||
                 HTTP_STATUS_CODE_NO_CONTENT == uStatus || HTTP_STATUS_CODE_NOT_MODIFIED == uStatus )
            {
                DbgOut( INFO , DBG_PROTOHANDLER , "Skip body because of header value. Method=%u, Status=%u" , uMethod , uStatus );
                uParserRet = HTTP_PARSER_SUCCESS;
                aParam->pFieldMgr->SetBodyEnd( TRUE );
                break;
            }
        }

        HttpTransferEncodingType uTransferEncodingType = aParam->pFieldMgr->GetTransferEncodingType();
        DbgOut( INFO , DBG_PROTOHANDLER , "uTransferEncodingType=%u" , uTransferEncodingType );


        //Do actions according to Transfer-Encoding type
        if ( HTTP_TRANSFER_ENCODING_CHUNKED == uTransferEncodingType )
        {
            uParserRet = ParseChunkedBody( aParam );
            break;
        }
        else if ( HTTP_TRANSFER_ENCODING_CONTENT_LENGTH == uTransferEncodingType )
        {
            UINT uTotalLen = aParam->pFieldMgr->GetContentLength();
            UINT uCurrentLen = aParam->pFieldMgr->GetCurrentBodyLength();
            DbgOut( INFO , DBG_PROTOHANDLER , "uCurrentLen/uTotalLen=%u/%u" , uCurrentLen , uTotalLen );

            //Check max body size
            if ( m_uMaxBodySize < uTotalLen )
            {
                DbgOut( WARN , DBG_PROTOHANDLER , "MaxBodySize exceed. Total/Max=%u/%u" , uTotalLen , m_uMaxBodySize );
                uParserRet = HTTP_PARSER_DATA_EXCEED;
                break;
            }

            //Skip empty content
            if ( 0 == uTotalLen ) 
            {
                aParam->pFieldMgr->SetBodyEnd( TRUE );
                uParserRet = HTTP_PARSER_SUCCESS;
                break;
            }

            //Reserve the buffer at the beginning to prevent memory reallocation
            if ( 0 == uCurrentLen )
            {
                aParam->pFieldMgr->ReserveBodySize( uTotalLen );
            }
            
            //Body only receive at most uTotalLen bytes specified in Content-Length field
            UINT uBufSize = 0;
            if ( uTotalLen > (uCurrentLen + aParam->uBufSize) )
            {
                uBufSize = aParam->uBufSize;
            }
            else
            {
                uBufSize = uTotalLen - uCurrentLen;
            }

            DbgOut( INFO , DBG_PROTOHANDLER , "uTotalLen=%lu, uCurrentLen=%lu, aParam->uBufSize=%lu, uBufSize=%lu" , uTotalLen , uCurrentLen , aParam->uBufSize , uBufSize );

            aParam->pFieldMgr->InsertBody( aParam->pBuf , uBufSize );
            uCurrentLen = aParam->pFieldMgr->GetCurrentBodyLength();
            aParam->pBuf += uBufSize;
            aParam->uBufSize -= uBufSize;
            if ( uTotalLen == uCurrentLen ) //Whole body received
            {
                aParam->pFieldMgr->SetBodyEnd( TRUE );
                uParserRet = HTTP_PARSER_SUCCESS;
                break;
            }
            else if ( uTotalLen > uCurrentLen )
            {
                uParserRet = HTTP_PARSER_NEED_MORE_DATA;
                break;
            }
            else
            {
                DbgOut( ERRO , DBG_PROTOHANDLER , "Content-Length is less than current sent length" );
                uParserRet = HTTP_PARSER_FAILURE;
                break;
            }
        }
        else    //HTTP_TRANSFER_ENCODING_UNKNOWN
        {
            //RFC 1945 section 7.2.2
            //If a Content-Length header field is present, its value in bytes represents the length of the
            //Entity-Body. Otherwise, the body length is determined by the closing of the connection by the server.
            //HTTP/1.0 requests containing an entity body must include a valid Content-Length header field
            if ( FALSE == aParam->bConnOut )
            {
                CHttpRspFieldMgr * pRspFieldMgr = (CHttpRspFieldMgr *)aParam->pFieldMgr;
                DbgOut( ERRO , DBG_PROTOHANDLER , "Cannot determine content length. status=%u" , pRspFieldMgr->GetStatusCode() );
                //Check max body size
                if ( m_uMaxBodySize < pRspFieldMgr->GetCurrentBodyLength() + aParam->uBufSize )
                {
                    DbgOut( WARN , DBG_PROTOHANDLER , "MaxBodySize exceed. Total/Max=%u/%u" , pRspFieldMgr->GetCurrentBodyLength() + aParam->uBufSize , m_uMaxBodySize );
                    uParserRet = HTTP_PARSER_DATA_EXCEED;
                }
                else
                {
                    aParam->pFieldMgr->InsertBody( aParam->pBuf , aParam->uBufSize );
                    uParserRet = HTTP_PARSER_NEED_MORE_DATA;    //Receive until connection terminated
                }
            }
            else
            {
                CHttpReqFieldMgr * pReqFieldMgr = (CHttpReqFieldMgr *)aParam->pFieldMgr;
                DbgOut( INFO , DBG_PROTOHANDLER , "Cannot determine content length, set to end. method=%u" , pReqFieldMgr->GetMethodId() );
                pReqFieldMgr->SetBodyEnd( TRUE );
                uParserRet = HTTP_PARSER_SUCCESS;
            }
            break;
        }
    } while ( 0 );

    DbgOut( VERB , DBG_PROTOHANDLER , "Leave. uParserRet=%u" , uParserRet );
    return uParserRet;
}

HttpParserErr CHttpParser::ParseChunkedBody( ParseParam * pParam )
{
    DbgOut( VERB , DBG_PROTOHANDLER , "Enter" );

    HttpParserErr uParserRet = HTTP_PARSER_NEED_MORE_DATA;
    const UCHAR * pBufEnd = pParam->pBuf + pParam->uBufSize;
    UCHAR * pLineEnd = NULL;
    UINT uLineSize = 0;

    while ( pParam->pBuf < pBufEnd )
    {
        DbgOut( INFO , DBG_PROTOHANDLER , "ChunkState=%d" , pParam->pChunkState->uState );
        pLineEnd = NULL;
        uLineSize = 0;
        if ( HTTP_CHUNK_STATE_UNKNOWN == pParam->pChunkState->uState ||
             HTTP_CHUNK_STATE_LENGTH == pParam->pChunkState->uState ) 
        {
            BOOL bAppend = ( HTTP_CHUNK_STATE_UNKNOWN == pParam->pChunkState->uState ) ? FALSE : TRUE;
            if ( HTTP_PARSER_SUCCESS == FindLineEndDelimiter( pParam->pBuf , pParam->uBufSize , &pLineEnd , &uLineSize ) )
            {
                pParam->pChunkState->uState = HTTP_CHUNK_STATE_BODY;
                if ( bAppend )
                {
                    pParam->pChunkState->strChunkLength.append( (const CHAR *)pParam->pBuf , uLineSize );
                }
                else
                {
                    pParam->pChunkState->strChunkLength.assign( (const CHAR *)pParam->pBuf , uLineSize );
                }
                CWUtils::TrimStringA( pParam->pChunkState->strChunkLength , " \r\n" );
                pParam->pChunkState->uChunkLength = strtoul( pParam->pChunkState->strChunkLength.c_str() , NULL , 16 );
                if ( 0 == pParam->pChunkState->uChunkLength ) //Last chunk
                {
                    pParam->pChunkState->uState = HTTP_CHUNK_STATE_END;
                }
                else
                {
                    //Check max body size
                    if ( m_uMaxBodySize < pParam->pFieldMgr->GetCurrentBodyLength() + pParam->pChunkState->uChunkLength )
                    {
                        DbgOut( WARN , DBG_PROTOHANDLER , "MaxBodySize exceed. New/Max=%u/%u" , pParam->pFieldMgr->GetCurrentBodyLength() + pParam->pChunkState->uChunkLength , m_uMaxBodySize );
                        uParserRet = HTTP_PARSER_DATA_EXCEED;
                        break;
                    }
                }

                pParam->uBufSize = pBufEnd - pLineEnd;
                pParam->pBuf = pLineEnd;
            }
            else //Cannot find line end delimiter
            {
                uLineSize = pParam->uBufSize;
                pParam->pChunkState->uState = HTTP_CHUNK_STATE_LENGTH;
                if ( bAppend )
                {
                    pParam->pChunkState->strChunkLength.append( (const CHAR *)pParam->pBuf , uLineSize );
                }
                else
                {
                    pParam->pChunkState->strChunkLength.assign( (const CHAR *)pParam->pBuf , uLineSize );
                }

                pParam->uBufSize = pBufEnd - pLineEnd;
                pParam->pBuf = pLineEnd;
                break;
            }
        }
        else if ( HTTP_CHUNK_STATE_BODY == pParam->pChunkState->uState )
        {
            uLineSize = pParam->pChunkState->uChunkLength - pParam->pChunkState->uReceivedChunkSize;
            if ( uLineSize <= pParam->uBufSize )    //Full chunk received
            {
                pParam->pFieldMgr->InsertBody( pParam->pBuf , uLineSize );

                pParam->pChunkState->uReceivedChunkSize = pParam->pChunkState->uChunkLength;
                pParam->pBuf += uLineSize;
                pParam->uBufSize = pBufEnd - pParam->pBuf;

                //Parse end delimiter
                if ( HTTP_PARSER_SUCCESS == FindLineEndDelimiter( pParam->pBuf , pParam->uBufSize , &pLineEnd , &uLineSize ) )
                {
                    //Clear state for next chunk
                    pParam->pChunkState->uState = HTTP_CHUNK_STATE_UNKNOWN;
                    pParam->pChunkState->uReceivedChunkSize = 0;

                    pParam->pBuf = pLineEnd;
                    pParam->uBufSize = pBufEnd - pParam->pBuf;
                }
                else //Cannot find line end delimiter
                {
                    pParam->pBuf = pLineEnd;
                    pParam->uBufSize = pBufEnd - pParam->pBuf;
                    uParserRet = HTTP_PARSER_NEED_MORE_DATA;
                    break;
                }
            }
            else //Received buffer is not enough
            {
                pParam->pFieldMgr->InsertBody( pParam->pBuf , pParam->uBufSize );
                pParam->pChunkState->uReceivedChunkSize += pParam->uBufSize;
                uParserRet = HTTP_PARSER_NEED_MORE_DATA;
                break;
            }
        }
        else if ( HTTP_CHUNK_STATE_END == pParam->pChunkState->uState )
        {
            if ( HTTP_PARSER_SUCCESS == FindLineEndDelimiter( pParam->pBuf , pParam->uBufSize , &pLineEnd , &uLineSize ) )
            {
                DbgOut( INFO , DBG_PROTOHANDLER , "Full chunked received. TotalSize=%u" , pParam->pFieldMgr->GetCurrentBodyLength() );
                pParam->pFieldMgr->SetBodyEnd( TRUE );
                uParserRet = HTTP_PARSER_SUCCESS;
                break;
            }
            else // cannot find line end delimiter.
            {
                uParserRet = HTTP_PARSER_NEED_MORE_DATA;
                break;
            }
        }
        else
        {
            DbgOut( ERRO , DBG_PROTOHANDLER , "Unknown chunked state=%u" , pParam->pChunkState->uState );
            uParserRet = HTTP_PARSER_FAILURE;
            break;
        }
    }

    DbgOut( VERB , DBG_PROTOHANDLER , "Leave" );
    return uParserRet;
}

UINT CHttpParser::DoFieldSearch( CPrefixTree<UINT> & aMatcher , const UCHAR * aBuf , UINT aBufSize , CIncompleteHeaderState * aIncompleteState , HttpFieldId * aFieldId , UINT * aFieldSize )
{
    if ( NULL == aBuf || 0 == aBufSize || NULL == aIncompleteState || NULL == aFieldId || NULL == aFieldSize )
    {
        return CPrefixTree<UINT>::MISS;
    }

    INT uNodeId = -1;
    SIZE_T uFoundSize;
    UINT uFieldId;

    // re-use previous position for search
    UINT uMatcherRet = aMatcher.SearchShortest( aIncompleteState->pPrevNodePos , &aIncompleteState->pPrevNodePos , (const CHAR *)aBuf , aBufSize , uFoundSize , uFieldId );
    switch ( uMatcherRet )
    {
        case CPrefixTree<UINT>::HIT :
        {
            aIncompleteState->pPrevNodePos = NULL;
            if ( aFieldSize )
            {
                *aFieldSize = uFoundSize;
            }
            if ( aFieldId )
            {
                *aFieldId = (HttpFieldId)uFieldId;
            }
            break;
        }
        case CPrefixTree<UINT>::MISS :
        {
            aIncompleteState->pPrevNodePos = NULL;
            break;
        }
        case CPrefixTree<UINT>::MORE_ONE:
        {
            break;
        }
        default :
        {
            aIncompleteState->pPrevNodePos = NULL;
            break;
        }
    }

    return uMatcherRet;
}

HttpParserErr CHttpParser::FindLineEndDelimiter( const UCHAR * aBuf , UINT aBufSize , UCHAR ** aLineEndPos , UINT * aLineSize )
{
    HttpParserErr uParserRet = HTTP_PARSER_FAILURE;

    do 
    {
        if ( NULL == aLineEndPos || NULL == aLineSize )
        {
            DbgOut( ERRO , DBG_PROTOHANDLER , "Invalid parameter" );
            break;
        }

        (*aLineEndPos) = const_cast<UCHAR *>(aBuf) + aBufSize;
        (*aLineSize) = aBufSize;
        if ( NULL == aBuf || 0 == aBufSize )
        {
            uParserRet = HTTP_PARSER_NEED_MORE_DATA;
            break;
        }

        UCHAR * pPos = NULL;
        pPos = (UCHAR *)memchr( (CHAR *)aBuf , '\n' , aBufSize );
        if ( NULL != pPos )
        {
            (*aLineSize) = (UINT)(pPos - aBuf);
            (*aLineEndPos) = pPos + 1;
            if ( (*aLineSize > 0) && (*(pPos - 1) == '\r') ) //Could be'\r\n' or '\n', check pPos - 1
            {
                (*aLineSize) -= 1; //Line size decrease 1 because line end with "\r\n"
            }
            uParserRet = HTTP_PARSER_SUCCESS;
        }
    } while ( 0 );
    
    return uParserRet;
}

HttpParserErr CHttpParser::ParseIncompleteLine( ParseParam * aParam )
{
    HttpParserErr uParserRet = HTTP_PARSER_FAILURE;
    const UCHAR * pBufBeg = aParam->pBuf;
    const UCHAR * pBufEnd = pBufBeg + aParam->uBufSize;
    CHttpFieldMgr * pFieldMgr = aParam->pFieldMgr;
    CIncompleteHeaderState * pIncompleteState = aParam->pIncompleteState;
    UCHAR * pLineEnd = NULL;
    UINT uFieldSize = 0;
    UINT uLineSize = 0;
    std::string strFieldData;

    do
    {
        //Field name
        if ( pIncompleteState->bIsIncompleteFieldName )
        {
            CHAR * pFieldEnd = (CHAR *)memchr( (CHAR *)pBufBeg , ':' , aParam->uBufSize );
            if ( NULL != pFieldEnd ) //Field name is completely received. Don't break to keep parsing field data
            {
                pIncompleteState->strUnknownFieldName.append( (CHAR *)pBufBeg , ((UCHAR *)pFieldEnd - pBufBeg + 1) );
                pIncompleteState->bIsIncompleteFieldName = FALSE;
                pBufBeg = (UCHAR *)pFieldEnd + 1; //Move to the begin position of Field data
                aParam->pBuf = pBufBeg;
                aParam->uBufSize = ( pBufEnd - pBufBeg );
            }
            else //Field name not complete
            {
                if ( HTTP_PARSER_SUCCESS == FindLineEndDelimiter( pBufBeg , aParam->uBufSize , &pLineEnd , &uLineSize ) )
                {
                    pIncompleteState->strUnknownFieldName.append( (CHAR *)pBufBeg , uLineSize );
                    CWUtils::TrimStringA( pIncompleteState->strUnknownFieldName , " \r\n" );
                    if ( 0 == pIncompleteState->strUnknownFieldName.length() ) //A space filled string, ignore
                    {
                        pBufBeg = (UCHAR *)pFieldEnd + 1; //Move to the begin position of Field data
                        aParam->pBuf = pBufBeg;
                        aParam->uBufSize = ( pBufEnd - pBufBeg );
                        uParserRet = HTTP_PARSER_SUCCESS;
                        break;
                    }
                    else    //Skip the strange line
                    {
                        pBufBeg = pLineEnd;
                        aParam->pBuf = pBufBeg;
                        aParam->uBufSize = ( pBufEnd - pBufBeg );
                        uParserRet = HTTP_PARSER_NOT_SUPPORTED;
                        break;
                    }
                }
                else
                {
                    pIncompleteState->strUnknownFieldName.append( (CHAR *)pBufBeg , aParam->uBufSize );
                    pBufBeg = pLineEnd;
                    aParam->pBuf = pBufBeg;
                    aParam->uBufSize = ( pBufEnd - pBufBeg );
                    uParserRet = HTTP_PARSER_NEED_MORE_DATA;
                    break;
                }
            }
        }

        //Field data
        if ( HTTP_PARSER_SUCCESS == FindLineEndDelimiter( pBufBeg , aParam->uBufSize , &pLineEnd , &uLineSize ) )
        {
            strFieldData.assign( (const CHAR *)pBufBeg , uLineSize );
            if ( HTTP_FIELD_UNKNOWN != pIncompleteState->uFieldId )
            {
                pFieldMgr->SetField( pIncompleteState->uFieldId , strFieldData , uLineSize , TRUE , TRUE );
            }
            else
            {
                pFieldMgr->SetUnknownField( pIncompleteState->strUnknownFieldName , strFieldData , uLineSize , TRUE , TRUE );
            }
            pBufBeg = pLineEnd + 1; //Move to next line
            aParam->pBuf = pBufBeg;
            aParam->uBufSize = ( pBufEnd - pBufBeg );
            pIncompleteState->uFieldId = HTTP_FIELD_EMPTY;
        }
        else //Cannot find end position, buffer may not complete
        {
            strFieldData.assign( (const CHAR *)pBufBeg , aParam->uBufSize );
            if ( HTTP_FIELD_UNKNOWN != pIncompleteState->uFieldId )
            {
                pFieldMgr->SetField( pIncompleteState->uFieldId , strFieldData , aParam->uBufSize , TRUE , FALSE );
            }
            else
            {
                pFieldMgr->SetUnknownField( pIncompleteState->strUnknownFieldName , strFieldData , aParam->uBufSize , TRUE , FALSE );
            }

            pBufBeg = pLineEnd;
            aParam->pBuf = pBufBeg;
            aParam->uBufSize = ( pBufEnd - pBufBeg );
            uParserRet = HTTP_PARSER_NEED_MORE_DATA;
            break;
        }
        
        uParserRet = HTTP_PARSER_SUCCESS;
    } while ( 0 );
    
    return uParserRet;
}

CHttpFieldMgr * CHttpParser::GetFieldMgr( BOOL aConnOut )
{
    if ( aConnOut )
    {
        return &m_ReqFieldMgr;
    }
    else
    {
        return &m_RspFieldMgr;
    }
}

BOOL CHttpParser::IsHeaderEnd( BOOL aConnOut )
{
    if ( aConnOut )
    {
        return m_ReqFieldMgr.IsHeaderEnd();
    }
    else
    {
        return m_RspFieldMgr.IsHeaderEnd();
    }
}

HttpFieldId CHttpParser::GetMethodId()
{
    return m_ReqFieldMgr.GetMethodId();
}

BOOL CHttpParser::GetMethod( std::string & aMethod )
{
    BOOL bRet = FALSE;

    HttpFieldId uMethod = m_ReqFieldMgr.GetMethodId();
    for ( SIZE_T i = 0 ; i < _countof(g_FieldTable) ; i++ )
    {
        if ( g_FieldTable[i].uFieldId == uMethod )
        {
            aMethod.assign( g_FieldTable[i].szFieldName );
            CWUtils::TrimStringA( aMethod , " " );
            bRet = TRUE;
            break;
        }
    }
    
    return bRet;
}

const CHAR * CHttpParser::GetUri()
{
    return m_ReqFieldMgr.GetUri();
}

VOID CHttpParser::GetUrl( std::string & aUrl )
{
    const CHAR * tmpUri = this->GetUri();

    std::string strUri;
    std::string strHost;
    std::string strMethod;

    if ( NULL != tmpUri )
    {
        strUri = tmpUri;
    }
    else
    {
        DbgOut( ERRO , DBG_PROTOHANDLER , "tmpUri is NULL" );
    }

    this->GetHost( strHost );
    this->GetMethod( strMethod );

    if ( strUri.length() && (strUri[0] == '/' || strUri == "*") )
    {
        aUrl = "http://";
        if ( strHost.length() > 0 )
        {
            aUrl += strHost;
        }
        else
        {
            DbgOut( ERRO , DBG_PROTOHANDLER , "Host header not included" );
        }

        //RFC2616 section 5.1.2
        //The asterisk "*" means that the request does not apply to a particular resource, but to the server itself,
        //and is only allowed when the method used does not necessarily apply to a resource.
        //One example would be OPTIONS * HTTP/1.1
        if ( strUri != "*" )
        {
            aUrl += strUri;
        }
    }
    else if ( strMethod == "CONNECT" )    //Authority
    {
        aUrl = "http://";
        aUrl += strUri;
    }
    else                                  //AbsoluteUri
    {
        aUrl = strUri;
    }
}




VOID CHttpParser::GetRequestLine( std::string & aRequestLine )
{
    m_ReqFieldMgr.GetRequestLine( aRequestLine );
}

VOID CHttpParser::GetHost( std::string & aHost )
{
    m_ReqFieldMgr.GetHost( aHost );
}

VOID CHttpParser::GetReferer( std::string & aReferer )
{
    m_ReqFieldMgr.GetReferer( aReferer );
}

VOID CHttpParser::GetUserAgent( std::string & aUserAgent )
{
    m_ReqFieldMgr.GetUserAgent( aUserAgent );
}

VOID CHttpParser::GetReqHeaderRawData( std::vector<std::string> & aReqHeaderRawData )
{
    m_ReqFieldMgr.GetParsedHeaders( aReqHeaderRawData );
}

VOID CHttpParser::GetStatusLine( std::string & aStatusLine )
{
    m_RspFieldMgr.GetStatusLine( aStatusLine );
}

VOID CHttpParser::GetRspHeaderRawData( std::vector<std::string> & aRspHeaderRawData )
{
    m_RspFieldMgr.GetParsedHeaders( aRspHeaderRawData );
}

BOOL CHttpParser::IsBodyEnd( BOOL aConnOut )
{
    if ( aConnOut )
    {
        return m_ReqFieldMgr.IsBodyEnd();
    }
    else
    {
        return m_RspFieldMgr.IsBodyEnd();
    }
}



}   //End of namespace CWUtils
