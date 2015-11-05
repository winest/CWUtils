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
#include <map>
#include <string>
#include <vector>

#include "CWString.h"
#include "CWTree.h"

namespace CWUtils
{



typedef enum _HttpParserErr
{
    HTTP_PARSER_FAILURE = 0 ,             //General error
    HTTP_PARSER_SUCCESS = 1 ,
    HTTP_PARSER_NEED_MORE_DATA = 2 ,
    HTTP_PARSER_NOT_SUPPORTED = 3 ,
    HTTP_PARSER_DATA_EXCEED = 4 ,         //Stop processing because data is too big
    HTTP_PARSER_NO_MEMORY = 5 ,
    HTTP_PARSER_USER_ABORT = 6
} HttpParserErr;

typedef enum _HttpFieldId
{
    HTTP_FIELD_EMPTY = 0 ,
    HTTP_FIELD_METHOD_BEGIN = 1 , //Method begin
    HTTP_FIELD_METHOD_HEAD ,
    HTTP_FIELD_METHOD_GET ,
    HTTP_FIELD_METHOD_POST ,
    HTTP_FIELD_METHOD_PUT ,
    HTTP_FIELD_METHOD_DELETE ,
    HTTP_FIELD_METHOD_TRACE ,
    HTTP_FIELD_METHOD_OPTIONS ,
    HTTP_FIELD_METHOD_CONNECT ,
    HTTP_FIELD_METHOD_PATCH ,
    HTTP_FIELD_METHOD_END ,      //Method end
    HTTP_FIELD_HTTP_RESPONSE ,
    HTTP_FIELD_HOST ,
    HTTP_FIELD_PROXY_CONNECTION ,
    HTTP_FIELD_REFERER ,
    HTTP_FIELD_USER_AGENT ,
    HTTP_FIELD_ACCEPT ,
    HTTP_FIELD_ACCEPT_ENCODING ,
    HTTP_FIELD_ACCEPT_LANGUAGE ,
    HTTP_FIELD_ACCEPT_CHARSET ,
    HTTP_FIELD_ACCEPT_RANGES ,
    HTTP_FIELD_TRANSFER_ENCODING ,
    HTTP_FIELD_COOKIE ,
    HTTP_FIELD_SERVER ,
    HTTP_FIELD_CONNECTION ,
    HTTP_FIELD_KEEP_ALIVE ,
    HTTP_FIELD_DATE ,
    HTTP_FIELD_LAST_MODIFIED ,
    HTTP_FIELD_ETAG ,
    HTTP_FIELD_CACHE_CONTROL ,
    HTTP_FIELD_VARY ,
    HTTP_FIELD_VIA ,
    HTTP_FIELD_CONTENT_LENGTH ,
    HTTP_FIELD_CONTENT_TYPE ,
    HTTP_FIELD_CONTENT_ENCODING ,
    HTTP_FIELD_X_FLASH_VERSION ,
    HTTP_FIELD_X_SESSION_COOKIE ,
    HTTP_FIELD_X_POWERED_BY ,
    HTTP_FIELD_X_VARY_OPTION ,
    HTTP_FIELD_X_CACHE ,
    HTTP_FIELD_X_CACHE_LOOKUP ,
    HTTP_FIELD_INCOMPLETE ,             //for the case when buf is ending while doing Field keyword matching.
    HTTP_FIELD_UNKNOWN ,                //for a Field which is not included in table
    HTTP_FIELD_END                     //for the end of header
} HttpFieldId; 

typedef enum _HttpTransferEncodingType
{
    HTTP_TRANSFER_ENCODING_UNKNOWN = 0 ,
    HTTP_TRANSFER_ENCODING_CONTENT_LENGTH ,
    HTTP_TRANSFER_ENCODING_CHUNKED
} HttpTransferEncodingType;

typedef enum _HttpStatusCode
{
    HTTP_STATUS_CODE_UNKNOWN = 0 ,
    HTTP_STATUS_CODE_CONTINUE = 100 ,
    HTTP_STATUS_CODE_SWITCHING_PROTOCOLS  = 101 ,
    HTTP_STATUS_CODE_OK = 200 ,
    HTTP_STATUS_CODE_CREATED = 201 , 
    HTTP_STATUS_CODE_ACCEPTED = 202 , 
    HTTP_STATUS_CODE_NON_AUTHORITATIVE_INFORMATION = 203 , 
    HTTP_STATUS_CODE_NO_CONTENT = 204 , 
    HTTP_STATUS_CODE_RESET_CONTENT = 205 ,
    HTTP_STATUS_CODE_PARTIAL_CONTENT = 206 ,
    HTTP_STATUS_CODE_MULTIPLE_CHOICES = 300 ,
    HTTP_STATUS_CODE_MOVED_PERMANENTLY = 301 , 
    HTTP_STATUS_CODE_MOVED_TEMPORARILY = 302 , 
    HTTP_STATUS_CODE_SEE_OTHER = 303 ,
    HTTP_STATUS_CODE_NOT_MODIFIED = 304 , 
    HTTP_STATUS_CODE_USE_PROXY = 305 ,
    HTTP_STATUS_CODE_TEMPORARY_REDIRECT = 307 ,
    HTTP_STATUS_CODE_BAD_REQUEST = 400 , 
    HTTP_STATUS_CODE_UNAUTHORIZED = 401 , 
    HTTP_STATUS_CODE_PAYMENT_REQUIRED = 402 ,
    HTTP_STATUS_CODE_FORBIDDEN = 403 , 
    HTTP_STATUS_CODE_NOT_FOUND = 404 , 
    HTTP_STATUS_CODE_METHOD_NOT_ALLOWED = 405 ,
    HTTP_STATUS_CODE_NOT_ACCEPTABLE = 406 ,
    HTTP_STATUS_CODE_PROXY_AUTHENTICATION_REQUIRED = 407 ,
    HTTP_STATUS_CODE_REQUEST_TIMEOUT = 408 ,
    HTTP_STATUS_CODE_CONFLICT = 409 ,
    HTTP_STATUS_CODE_GONE = 410 ,
    HTTP_STATUS_CODE_LENGTH_REQUIRED = 411 ,
    HTTP_STATUS_CODE_PRECONDITION_FAILED = 412 ,
    HTTP_STATUS_CODE_REQUEST_ENTITY_TOO_LARGE = 413 , 
    HTTP_STATUS_CODE_REQUEST_URI_TOO_LONG = 414 ,
    HTTP_STATUS_CODE_UNSUPPORTED_MEDIA_TYPE = 415 ,
    HTTP_STATUS_CODE_REQUESTED_RANGE_NOT_SATISFIABLE = 416 ,
    HTTP_STATUS_CODE_EXPECTATION_FAILED = 417 ,
    HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR = 500 , 
    HTTP_STATUS_CODE_NOT_IMPLEMENTED = 501 , 
    HTTP_STATUS_CODE_BAD_GATEWAY = 502 , 
    HTTP_STATUS_CODE_SERVICE_UNAVAILABLE = 503 ,
    HTTP_STATUS_CODE_GATEWAY_TIMEOUT = 504 ,
    HTTP_STATUS_CODE_HTTP_VERSION_NOT_SUPPORTED = 505
} HttpStatusCode;


class CHttpFieldMgr
{
    public :
        CHttpFieldMgr() { this->Reset(); }
        virtual ~CHttpFieldMgr() {}
        virtual VOID Reset();

    public :
        HttpParserErr GetFieldValue( HttpFieldId aFieldId , std::string & aData );
        virtual HttpParserErr SetField( HttpFieldId aFieldId , std::string & aData , UINT aDataSize , BOOL aAppend , BOOL aComplete );
        virtual HttpParserErr SetUnknownField( std::string aFieldName , std::string & aData , UINT aDataSize , BOOL aAppend , BOOL aComplete );

        VOID GetVersion( UINT & aMajorVer , UINT & aMinorVer );
        VOID SetVersion( UINT aMajorVer , UINT aMinorVer );

        BOOL IsHeaderEnd();
        VOID SetHeaderEnd( BOOL aEnd );
        UINT GetParsedHeadersCount();
        VOID GetParsedHeaders( std::vector<std::string> & aHeaders );

        BOOL IsBodyEnd();
        VOID SetBodyEnd( BOOL aEnd );
        HttpTransferEncodingType GetTransferEncodingType();
        virtual UINT GetContentLength();
        VOID ReserveBodySize( SIZE_T aSize );
        UINT GetCurrentBodyLength();
        const UCHAR * GetCurrentBodyPtr();
        HttpParserErr InsertBody( const UCHAR * aBuf , UINT aBufSize );

    protected :
        HttpParserErr ParseHttpVersion( const std::string & aVersion );
        HttpParserErr ParseContentLength();
        BOOL IsChunkedTransferEncoding();
        VOID StringTrim( std::string & aStr );

    protected :
        UINT m_uMajorVer;
        UINT m_uMinorVer;

        BOOL m_bHeaderEnd;
        std::vector<std::string> m_vecParsedHeader;        
        std::map<HttpFieldId , std::string> m_mapFields;
        std::map<std::string , std::string> m_mapUnknownFields;

        BOOL m_bBodyEnd;
        HttpTransferEncodingType m_uTransferEncodingType;
        UINT m_uContentLength;
        vector<UCHAR> m_vecBody;
};

class CHttpReqFieldMgr : public CHttpFieldMgr
{
    public :
        CHttpReqFieldMgr() { this->Reset(); }
        virtual ~CHttpReqFieldMgr() {}
        virtual VOID Reset();

    public :
        virtual HttpParserErr SetField( HttpFieldId aFieldId , std::string & aData , UINT aDataSize , BOOL aAppend , BOOL aComplete );
        HttpFieldId GetMethodId();
        const CHAR * GetUri();
        VOID GetRequestLine( std::string & aRequestLine );
        VOID GetUserAgent( std::string & aAgent );
        VOID GetHost( std::string & aHost );
        VOID GetReferer( std::string & aReferer );

    protected :
        HttpParserErr ParseRequestVersionAndUri( HttpFieldId aFieldId );

    protected :
        BOOL m_bHasMethod;
        HttpFieldId m_uMethodId;
        std::string m_strUri;
        std::string m_strRequestLine;
        std::string m_strAgent;
        std::string m_strHost;
        std::string m_strReferer;
};

class CHttpRspFieldMgr : public CHttpFieldMgr
{
    public :
        CHttpRspFieldMgr() { this->Reset(); }
        virtual ~CHttpRspFieldMgr() {}
        virtual VOID Reset();

    public :
        virtual HttpParserErr SetField( HttpFieldId aFieldId , std::string & aData , UINT aDataSize , BOOL aAppend , BOOL aComplete );
        HttpStatusCode GetStatusCode();
        VOID GetStatusLine( std::string & aStatusLine );

    protected :
        HttpParserErr ParseResponseVersionAndCode();

    protected :
        BOOL m_bIsChunked;
        HttpStatusCode m_uStatusCode;
        std::string m_strStatusLine;
        std::string m_strUri;
}; 














//Return whether to parse the remaining part continually
typedef BOOL (CALLBACK *ParserUpdateCallback)( IN VOID * aContext , IN BOOL aConnOut , IN HttpParserErr aParseRet , OUT INT * aCallbackRet );

class CHttpParser
{
    public :
        CHttpParser() { this->Reset(); }
        virtual ~CHttpParser() { this->UnInit(); }
        
        BOOL Init( VOID * pCbkCtx , ParserUpdateCallback pfnHeaderCbk , ParserUpdateCallback pfnBodyCbk ,
                   UINT aMaxHeaderCnt = INFINITE , UINT aMaxBodySize = INFINITE );
        VOID Reset();
        VOID UnInit();

    private :
        typedef enum _HttpChunkState
        {
            HTTP_CHUNK_STATE_UNKNOWN = 0,    //Never parse length or body of this chunk
            HTTP_CHUNK_STATE_LENGTH ,        //Partially length parsed
            HTTP_CHUNK_STATE_BODY ,          //Partially body received
            HTTP_CHUNK_STATE_END             //Last chunk
        } HttpChunkState;

        class CIncompleteHeaderState
        {
            public :
                CIncompleteHeaderState() { this->Reset(); }
                VOID Reset()
                {
                    pPrevNodePos = NULL;
                    uFieldId = HTTP_FIELD_EMPTY;
                    bIsIncompleteFieldName = FALSE;
                    strUnknownFieldName.clear();
                }
            public :
                VOID * pPrevNodePos; //Current position on Field tree.
                HttpFieldId uFieldId; //Current Field id,used for setting field value.
                BOOL bIsIncompleteFieldName; //An unknown field name is incomplete,if the field name is known, it should be hold in Tree pos.
                std::string strUnknownFieldName; //Current unknown Field name. for identifying case, it is used to store field value;
        };

        class CChunkState
        {
            public :
                CChunkState() { this->Reset(); }
                VOID Reset()
                {
                    uState = HTTP_CHUNK_STATE_UNKNOWN;
                    uChunkLength = 0;
                    strChunkLength.clear();
                    uReceivedChunkSize = 0;
                }
            public :
                HttpChunkState uState;
                UINT uChunkLength;
                std::string strChunkLength;
                UINT uReceivedChunkSize;
        };

        typedef struct _ParseParam
        {
            _ParseParam() : bConnOut(FALSE) , pBuf(NULL) , uBufSize(0) , pFieldMgr(NULL) , pIncompleteState(NULL) , pChunkState(NULL) {}
            BOOL bConnOut;
            const UCHAR * pBuf;
            UINT uBufSize;
            CHttpFieldMgr * pFieldMgr;
            CIncompleteHeaderState * pIncompleteState;
            CChunkState * pChunkState;
        } ParseParam;

    public :
        HttpParserErr Input( BOOL aConnOut , const UCHAR * aBuf , UINT aBufSize , INT & aCallbackRet );

        CHttpFieldMgr * GetFieldMgr( BOOL aConnOut );

        BOOL IsHeaderEnd( BOOL aConnOut );

        HttpFieldId GetMethodId();
        BOOL GetMethod( std::string & aMethod );
        const CHAR * GetUri();
        VOID GetUrl( std::string & aUrl );
        VOID GetRequestLine( std::string & saRequestLine );
        VOID GetHost( std::string & aHost );
        VOID GetReferer( std::string & aReferer );
        VOID GetUserAgent( std::string & aUserAgent );
        VOID GetReqHeaderRawData( std::vector<std::string> & aReqHeaderRawData );

        VOID GetStatusLine( std::string & aStatusLine );
        VOID GetRspHeaderRawData( std::vector<std::string> & aRspHeaderRawData );

        BOOL IsBodyEnd( BOOL aConnOut );

    protected :
        HttpParserErr ParseHeader( ParseParam * aParam );
        HttpParserErr ParseBody( ParseParam * aParam );
        HttpParserErr ParseChunkedBody( ParseParam * pParam );

        UINT DoFieldSearch( CWUtils::CPrefixTree<UINT> & aMatcher , const UCHAR * aBuf , UINT aBufSize , CIncompleteHeaderState * aIncompleteState , HttpFieldId * aFieldId , UINT * aFieldSize );
        HttpParserErr ParseIncompleteLine( ParseParam * aParam );        
        //aLineSize doesn't include "\r\n"
        //aLineEndPos indicates the position of ('\n' + 1) or (last character + 1)
        HttpParserErr FindLineEndDelimiter( const UCHAR * aBuf , UINT aBufSize , UCHAR ** aLineEndPos , UINT * aLineSize );

        VOID StringTrim( std::string & aStr );

    protected :
        CWUtils::CPrefixTree<UINT> * m_FieldMatcher;

        CHttpReqFieldMgr m_ReqFieldMgr;
        CIncompleteHeaderState m_ReqIncompleteHeaderState;
        CChunkState m_ReqChunkState;

        CHttpRspFieldMgr m_RspFieldMgr;
        CIncompleteHeaderState m_RspIncompleteHeaderState;
        CChunkState m_RspChunkState;

        HttpParserErr m_uLastParserErr;

        VOID * m_pCbkCtx;
        ParserUpdateCallback m_pfnHeaderCbk;
        ParserUpdateCallback m_pfnBodyCbk;

        UINT m_uMaxHeaderCnt;
        UINT m_uMaxBodySize;
}; 



}   //End of namespace CWUtils
