#include "stdafx.h"
#include "CWFtdParser.h"
using namespace std;
using namespace CWUtils;

#if defined( USE_WPP )
#    include "_GenerateTmh.h"
#    include "CWFtdParser.tmh"
#elif defined( USE_G3LOG )
#    include <g3log/g3log.hpp>
#    include <g3log/logworker.hpp>
#    include "G3LogLevel.h"
#endif



namespace CWUtils
{
//================================================================================
//================================ CFtdPartMgr =================================
//================================================================================
VOID CFtdPartMgr::Reset()
{
    m_dwFlags = FTD_FLAG_NONE;
    m_mapRawParts.clear();
    m_mapNumParts.clear();
    m_vecFtdExtHdrTags.clear();
    m_vecFtdcFields.clear();
}

FtdParserErr CFtdPartMgr::GetRawPartValue( FtdPartId aPartId, string & aPartVal ) CONST
{
    FtdParserErr uParserRet = FTD_PARSER_FAILURE;
    auto it = m_mapRawParts.find( aPartId );
    if ( it != m_mapRawParts.end() )
    {
        aPartVal = it->second;
        uParserRet = FTD_PARSER_SUCCESS;
    }
    return uParserRet;
}

FtdParserErr CFtdPartMgr::SetRawPartValue( FtdPartId aPartId, CONST UCHAR * aPartVal, UINT aPartValSize )
{
    m_mapRawParts[aPartId] = string( (CONST CHAR *)aPartVal, (size_t)aPartValSize );
    return FTD_PARSER_SUCCESS;
}

FtdParserErr CFtdPartMgr::SetRawPartValue( FtdPartId aPartId, CONST vector<UCHAR> & aPartVal )
{
    m_mapRawParts[aPartId] = string( aPartVal.begin(), aPartVal.end() );
    return FTD_PARSER_SUCCESS;
}

FtdParserErr CFtdPartMgr::SetRawPartValue( FtdPartId aPartId, CONST string & aPartVal )
{
    m_mapRawParts[aPartId] = aPartVal;
    return FTD_PARSER_SUCCESS;
}

FtdParserErr CFtdPartMgr::RemoveRawPartValue( FtdPartId aPartId )
{
    FtdParserErr uParserRet = FTD_PARSER_FAILURE;
    auto it = m_mapRawParts.find( aPartId );
    if ( it != m_mapRawParts.end() )
    {
        m_mapRawParts.erase( it );
        uParserRet = FTD_PARSER_SUCCESS;
    }
    return uParserRet;
}

FtdParserErr CFtdPartMgr::GetNumPartValue( FtdPartId aPartId, UINT8 & aPartVal ) CONST
{
    FtdParserErr uParserRet = FTD_PARSER_FAILURE;
    auto it = m_mapNumParts.find( aPartId );
    if ( it != m_mapNumParts.end() )
    {
        aPartVal = (UINT8)it->second;
        uParserRet = FTD_PARSER_SUCCESS;
    }
    else
    {
        aPartVal = 0;
    }
    return uParserRet;
}

FtdParserErr CFtdPartMgr::GetNumPartValue( FtdPartId aPartId, UINT16 & aPartVal ) CONST
{
    FtdParserErr uParserRet = FTD_PARSER_FAILURE;
    auto it = m_mapNumParts.find( aPartId );
    if ( it != m_mapNumParts.end() )
    {
        aPartVal = (UINT16)it->second;
        uParserRet = FTD_PARSER_SUCCESS;
    }
    else
    {
        aPartVal = 0;
    }
    return uParserRet;
}

FtdParserErr CFtdPartMgr::GetNumPartValue( FtdPartId aPartId, UINT32 & aPartVal ) CONST
{
    FtdParserErr uParserRet = FTD_PARSER_FAILURE;
    auto it = m_mapNumParts.find( aPartId );
    if ( it != m_mapNumParts.end() )
    {
        aPartVal = (UINT32)it->second;
        uParserRet = FTD_PARSER_SUCCESS;
    }
    else
    {
        aPartVal = 0;
    }
    return uParserRet;
}

FtdParserErr CFtdPartMgr::SetNumPartValue( FtdPartId aPartId, UINT32 aPartVal )
{
    m_mapNumParts[aPartId] = aPartVal;
    return FTD_PARSER_SUCCESS;
}

FtdParserErr CFtdPartMgr::RemoveNumPartValue( FtdPartId aPartId )
{
    FtdParserErr uParserRet = FTD_PARSER_FAILURE;
    auto it = m_mapNumParts.find( aPartId );
    if ( it != m_mapNumParts.end() )
    {
        m_mapNumParts.erase( it );
        uParserRet = FTD_PARSER_SUCCESS;
    }
    return uParserRet;
}

std::shared_ptr<CONST std::vector<FtdExtHdrTag>> CFtdPartMgr::GetFtdExtHdrTags() CONST
{
    return std::make_shared<std::vector<FtdExtHdrTag>>( m_vecFtdExtHdrTags );
}

std::shared_ptr<CONST std::vector<FtdcField>> CFtdPartMgr::GetFtdcFields() CONST
{
    return std::make_shared<std::vector<FtdcField>>( m_vecFtdcFields );
}



DWORD CFtdPartMgr::GetFlags()
{
    return m_dwFlags;
}

BOOL CFtdPartMgr::IsFlagSet( DWORD aFlag )
{
    return ( m_dwFlags & aFlag ) ? TRUE : FALSE;
}

DWORD CFtdPartMgr::AddFlag( DWORD aFlag )
{
    m_dwFlags |= aFlag;
    return m_dwFlags;
}

DWORD CFtdPartMgr::RemoveFlag( DWORD aFlag )
{
    m_dwFlags ^= aFlag;
    return m_dwFlags;
}

VOID CFtdPartMgr::AddFtdExtHdrTag( CONST FtdExtHdrTag & aTag )
{
    m_vecFtdExtHdrTags.push_back( aTag );
}

VOID CFtdPartMgr::AddFtdcField( CONST FtdcField & aField )
{
    m_vecFtdcFields.push_back( aField );
}






//================================================================================
//================================= CFtdParser ==================================
//================================================================================
std::map<FtdPartId, FtdPartIdInfo> CFtdParser::m_mapFtdPartIdInfo;

BOOL CFtdParser::Init( FtdRecordParsedCbk aRecordParsedCbk, VOID * aRecordParsedCbkCtx )
{
    BOOL bRet = FALSE;
    do
    {
        m_mapFtdPartIdInfo[FTD_PART_ID_HDR_TYPE] =
            FtdPartIdInfo( FTD_PART_ID_HDR_TYPE, "FTD_PART_ID_HDR_TYPE", FTD_PART_DATA_TYPE_NUM,
                           sizeof( FtdHdr::uFtdType ), FTD_PART_ID_HDR_EXTHDR_SIZE );
        m_mapFtdPartIdInfo[FTD_PART_ID_HDR_EXTHDR_SIZE] =
            FtdPartIdInfo( FTD_PART_ID_HDR_EXTHDR_SIZE, "FTD_PART_ID_HDR_EXTHDR_SIZE", FTD_PART_DATA_TYPE_NUM,
                           sizeof( FtdHdr::uFtdExtHdrSize ), FTD_PART_ID_HDR_FTDC_SIZE );
        m_mapFtdPartIdInfo[FTD_PART_ID_HDR_FTDC_SIZE] =
            FtdPartIdInfo( FTD_PART_ID_HDR_FTDC_SIZE, "FTD_PART_ID_HDR_FTDC_SIZE", FTD_PART_DATA_TYPE_NUM,
                           sizeof( FtdHdr::uFtdcSize ), FTD_PART_ID_EXTHDR_TAG_TYPE );

        m_mapFtdPartIdInfo[FTD_PART_ID_EXTHDR_TAG_TYPE] =
            FtdPartIdInfo( FTD_PART_ID_EXTHDR_TAG_TYPE, "FTD_PART_ID_EXTHDR_TAG_TYPE", FTD_PART_DATA_TYPE_NUM,
                           sizeof( FtdExtHdrTag::uFtdTagType ), FTD_PART_ID_EXTHDR_TAG_SIZE );
        m_mapFtdPartIdInfo[FTD_PART_ID_EXTHDR_TAG_SIZE] =
            FtdPartIdInfo( FTD_PART_ID_EXTHDR_TAG_SIZE, "FTD_PART_ID_EXTHDR_TAG_SIZE", FTD_PART_DATA_TYPE_NUM,
                           sizeof( FtdExtHdrTag::uFtdTagSize ), FTD_PART_ID_EXTHDR_TAG_CONTENT );
        m_mapFtdPartIdInfo[FTD_PART_ID_EXTHDR_TAG_CONTENT] =
            FtdPartIdInfo( FTD_PART_ID_EXTHDR_TAG_CONTENT, "FTD_PART_ID_EXTHDR_TAG_CONTENT", FTD_PART_DATA_TYPE_RAW, -1,
                           FTD_PART_ID_EXTHDR_TAG_TYPE );

        m_mapFtdPartIdInfo[FTD_PART_ID_FTDCHDR1_VERSION] =
            FtdPartIdInfo( FTD_PART_ID_FTDCHDR1_VERSION, "FTD_PART_ID_FTDCHDR1_VERSION", FTD_PART_DATA_TYPE_NUM,
                           sizeof( FtdcHdr1::uVersion ), FTD_PART_ID_FTDCHDR1_TYPE );
        m_mapFtdPartIdInfo[FTD_PART_ID_FTDCHDR1_TYPE] =
            FtdPartIdInfo( FTD_PART_ID_FTDCHDR1_TYPE, "FTD_PART_ID_FTDCHDR1_TYPE", FTD_PART_DATA_TYPE_NUM,
                           sizeof( FtdcHdr1::uType ), FTD_PART_ID_FTDCHDR1_UNKNOWN1 );
        m_mapFtdPartIdInfo[FTD_PART_ID_FTDCHDR1_UNKNOWN1] =
            FtdPartIdInfo( FTD_PART_ID_FTDCHDR1_UNKNOWN1, "FTD_PART_ID_FTDCHDR1_UNKNOWN1", FTD_PART_DATA_TYPE_NUM,
                           sizeof( FtdcHdr1::uUnknown1 ), FTD_PART_ID_FTDCHDR1_CHAIN_TYPE );
        m_mapFtdPartIdInfo[FTD_PART_ID_FTDCHDR1_CHAIN_TYPE] =
            FtdPartIdInfo( FTD_PART_ID_FTDCHDR1_CHAIN_TYPE, "FTD_PART_ID_FTDCHDR1_CHAIN_TYPE", FTD_PART_DATA_TYPE_NUM,
                           sizeof( FtdcHdr1::uChainType ), FTD_PART_ID_FTDCHDR2_SEQ_SERIES );

        m_mapFtdPartIdInfo[FTD_PART_ID_FTDCHDR2_SEQ_SERIES] =
            FtdPartIdInfo( FTD_PART_ID_FTDCHDR2_SEQ_SERIES, "FTD_PART_ID_FTDCHDR2_SEQ_SERIES", FTD_PART_DATA_TYPE_NUM,
                           sizeof( FtdcHdr2::uSeqSeries ), FTD_PART_ID_FTDCHDR2_SEQ_NUM );
        m_mapFtdPartIdInfo[FTD_PART_ID_FTDCHDR2_SEQ_NUM] =
            FtdPartIdInfo( FTD_PART_ID_FTDCHDR2_SEQ_NUM, "FTD_PART_ID_FTDCHDR2_SEQ_NUM", FTD_PART_DATA_TYPE_NUM,
                           sizeof( FtdcHdr2::uSeqNum ), FTD_PART_ID_FTDCHDR2_UNKNOWN1 );
        m_mapFtdPartIdInfo[FTD_PART_ID_FTDCHDR2_UNKNOWN1] =
            FtdPartIdInfo( FTD_PART_ID_FTDCHDR2_UNKNOWN1, "FTD_PART_ID_FTDCHDR2_UNKNOWN1", FTD_PART_DATA_TYPE_NUM,
                           sizeof( FtdcHdr2::uUnknown1 ), FTD_PART_ID_FTDCHDR2_FIELD_COUNT );
        m_mapFtdPartIdInfo[FTD_PART_ID_FTDCHDR2_FIELD_COUNT] =
            FtdPartIdInfo( FTD_PART_ID_FTDCHDR2_FIELD_COUNT, "FTD_PART_ID_FTDCHDR2_FIELD_COUNT", FTD_PART_DATA_TYPE_NUM,
                           sizeof( FtdcHdr2::uFieldCount ), FTD_PART_ID_FTDCHDR2_FIELD_SIZE );
        m_mapFtdPartIdInfo[FTD_PART_ID_FTDCHDR2_FIELD_SIZE] =
            FtdPartIdInfo( FTD_PART_ID_FTDCHDR2_FIELD_SIZE, "FTD_PART_ID_FTDCHDR2_FIELD_SIZE", FTD_PART_DATA_TYPE_NUM,
                           sizeof( FtdcHdr2::uFieldSize ), FTD_PART_ID_FTDCHDR2_REQ_ID );
        m_mapFtdPartIdInfo[FTD_PART_ID_FTDCHDR2_REQ_ID] =
            FtdPartIdInfo( FTD_PART_ID_FTDCHDR2_REQ_ID, "FTD_PART_ID_FTDCHDR2_REQ_ID", FTD_PART_DATA_TYPE_NUM,
                           sizeof( FtdcHdr2::uReqId ), FTD_PART_ID_FTDC_FIELD_ID );

        m_mapFtdPartIdInfo[FTD_PART_ID_FTDC_FIELD_ID] =
            FtdPartIdInfo( FTD_PART_ID_FTDC_FIELD_ID, "FTD_PART_ID_FTDC_FIELD_ID", FTD_PART_DATA_TYPE_NUM,
                           sizeof( FtdcField::uFieldId ), FTD_PART_ID_FTDC_FIELD_SIZE );
        m_mapFtdPartIdInfo[FTD_PART_ID_FTDC_FIELD_SIZE] =
            FtdPartIdInfo( FTD_PART_ID_FTDC_FIELD_SIZE, "FTD_PART_ID_FTDC_FIELD_SIZE", FTD_PART_DATA_TYPE_NUM,
                           sizeof( FtdcField::uFieldSize ), FTD_PART_ID_FTDC_FIELD_CONTENT );
        m_mapFtdPartIdInfo[FTD_PART_ID_FTDC_FIELD_CONTENT] =
            FtdPartIdInfo( FTD_PART_ID_FTDC_FIELD_CONTENT, "FTD_PART_ID_FTDC_FIELD_CONTENT", FTD_PART_DATA_TYPE_RAW, -1,
                           FTD_PART_ID_FTDC_FIELD_ID );

        m_ReqMainPartParsedInfo.Reset();
        m_ReqSubPartParsedInfo.Reset();
        m_ReqPartMgr.Reset();

        m_RspMainPartParsedInfo.Reset();
        m_RspSubPartParsedInfo.Reset();
        m_RspPartMgr.Reset();

        m_pfnRecordParsedCbk = aRecordParsedCbk;
        m_pRecordParsedCbkCtx = aRecordParsedCbkCtx;

        bRet = TRUE;
    } while ( 0 );

    return bRet;
}

VOID CFtdParser::Reset()
{
    m_ReqMainPartParsedInfo.Reset();
    m_ReqSubPartParsedInfo.Reset();
    m_ReqPartMgr.Reset();
    m_ReqFtdExtHdrTag.Reset();

    m_RspMainPartParsedInfo.Reset();
    m_RspSubPartParsedInfo.Reset();
    m_RspPartMgr.Reset();
    m_RspFtdExtHdrTag.Reset();
    m_uLastParserErr = FTD_PARSER_NEED_MORE_DATA;
}

VOID CFtdParser::UnInit()
{
    m_pfnRecordParsedCbk = NULL;
    m_pRecordParsedCbkCtx = NULL;
}

CONST FtdPartIdInfo * CFtdParser::GetFtdPartIdInfo( FtdPartId aPartId )
{
    auto it = m_mapFtdPartIdInfo.find( aPartId );
    if ( it != m_mapFtdPartIdInfo.end() )
    {
        return &it->second;
    }
    else
    {
        return NULL;
    }
}

FtdParserErr CFtdParser::Input( BOOL aConnOut, const UCHAR * aBuf, UINT aBufSize, INT & aCallbackRet )
{
    DbgOut( VERB, DBG_PROTOHANDLER, "Enter. aConnOut=%d", aConnOut );

    FtdParserErr uParserRet = m_uLastParserErr;

    do
    {
        if ( NULL == aBuf || 0 == aBufSize )
        {
            DbgOut( WARN, DBG_PROTOHANDLER, "Empty buffer. aBuf=0x%p, aBufSize=%u", aBuf, aBufSize );
            break;
        }

        if ( FTD_PARSER_SUCCESS != m_uLastParserErr && FTD_PARSER_NEED_MORE_DATA != m_uLastParserErr )
        {
            DbgOut( WARN, DBG_PROTOHANDLER,
                    "Stop parsing because of previous state disable parsing. m_uLastParserErr=%u", m_uLastParserErr );
            break;
        }

        ParseParam parseParam;
        parseParam.bConnOut = aConnOut;
        parseParam.pBuf = aBuf;
        parseParam.uBufSize = aBufSize;
        if ( aConnOut )
        {
            parseParam.pMainPartParsedInfo = &m_ReqMainPartParsedInfo;
            parseParam.pSubPartParsedInfo = &m_ReqSubPartParsedInfo;
            parseParam.pFieldMgr = &m_ReqPartMgr;
            parseParam.pPartialFtdExtHdrTag = &m_ReqFtdExtHdrTag;
            parseParam.pPartialFtdcField = &m_ReqFtdcField;
        }
        else
        {
            parseParam.pMainPartParsedInfo = &m_RspMainPartParsedInfo;
            parseParam.pSubPartParsedInfo = &m_RspSubPartParsedInfo;
            parseParam.pFieldMgr = &m_RspPartMgr;
            parseParam.pPartialFtdExtHdrTag = &m_RspFtdExtHdrTag;
            parseParam.pPartialFtdcField = &m_RspFtdcField;
        }


        //For part that is already parsed, it will return FTD_PARSER_ALREADY_PARSED
        //For part that is fully parsed in this round, it will return FTD_PARSER_SUCCESS
        while ( 0 < parseParam.uBufSize )
        {
            uParserRet = ParseFtdHdr( &parseParam );
            if ( FTD_PARSER_SUCCESS != uParserRet && FTD_PARSER_ALREADY_PARSED != uParserRet )
            {
                break;
            }

            uParserRet = ParseFtdExtHdrTags( &parseParam );
            if ( FTD_PARSER_SUCCESS != uParserRet && FTD_PARSER_ALREADY_PARSED != uParserRet )
            {
                break;
            }

            uParserRet = ParseFtdcHdr1( &parseParam );
            if ( FTD_PARSER_SUCCESS != uParserRet && FTD_PARSER_ALREADY_PARSED != uParserRet )
            {
                break;
            }

            uParserRet = ParseFtdcHdr2( &parseParam );
            if ( FTD_PARSER_SUCCESS != uParserRet && FTD_PARSER_ALREADY_PARSED != uParserRet )
            {
                break;
            }

            uParserRet = ParseFtdcFields( &parseParam );
            if ( FTD_PARSER_SUCCESS != uParserRet && FTD_PARSER_ALREADY_PARSED != uParserRet )
            {
                break;
            }
        }
    } while ( 0 );

    m_uLastParserErr = uParserRet;
    DbgOut( VERB, DBG_PROTOHANDLER, "Leave. uParserRet=%u", uParserRet );
    return uParserRet;
}

UINT32 CFtdParser::Ntoh( CONST UCHAR * aBuf, UINT aBufSize )
{
    if ( NULL == aBuf || 0 == aBufSize || 4 < aBufSize )
    {
        return 0;
    }
    UINT uRet = 0;
    switch ( aBufSize )
    {
        case 1:
        {
            uRet = aBuf[0];
            break;
        }
        case 2:
        {
            uRet = aBuf[0] << 8 | aBuf[1];
            break;
        }
        case 3:
        {
            uRet = aBuf[0] << 16 | aBuf[1] << 8 | aBuf[2];
            break;
        }
        case 4:
        {
            uRet = aBuf[0] << 24 | aBuf[1] << 16 | aBuf[2] << 8 | aBuf[3];
            break;
        }
    }
    return uRet;
}

FtdParserErr CFtdParser::ParseFtdHdr( ParseParam * aParam )
{
    assert( aParam && aParam->pBuf && aParam->pFieldMgr && aParam->pMainPartParsedInfo && aParam->pSubPartParsedInfo );
    DbgOut( VERB, DBG_PROTOHANDLER, "Enter. uPartId=%u", aParam->pSubPartParsedInfo->uPartId );

    FtdParserErr uParserRet = FTD_PARSER_NEED_MORE_DATA;

    do
    {
        if ( aParam->pSubPartParsedInfo->uPartId > FTD_PART_ID_HDR_END )
        {
            uParserRet = FTD_PARSER_ALREADY_PARSED;
            break;
        }
        else if ( aParam->pSubPartParsedInfo->uPartId < FTD_PART_ID_HDR_BEGIN )
        {
            DbgOut( ERRO, DBG_PROTOHANDLER, "Unexpected part. uPartId=%u", aParam->pSubPartParsedInfo->uPartId );
            uParserRet = FTD_PARSER_FAILURE;
            break;
        }
        else
        {
        }

        UINT uFtdHdrSize = sizeof( FtdHdr );
        uParserRet = this->DoParse( aParam, uFtdHdrSize, FTD_PART_ID_HDR_BEGIN, FTD_PART_ID_HDR_END, FALSE );
    } while ( 0 );

    DbgOut( VERB, DBG_PROTOHANDLER, "Leave. uParserRet=%u, uPartId=%u", uParserRet,
            aParam->pSubPartParsedInfo->uPartId );
    return uParserRet;
}

FtdParserErr CFtdParser::ParseFtdExtHdrTags( ParseParam * aParam )
{
    assert( aParam && aParam->pBuf && aParam->pFieldMgr && aParam->pMainPartParsedInfo && aParam->pSubPartParsedInfo );
    DbgOut( VERB, DBG_PROTOHANDLER, "Enter. uPartId=%u", aParam->pSubPartParsedInfo->uPartId );

    FtdParserErr uParserRet = FTD_PARSER_NEED_MORE_DATA;

    do
    {
        if ( aParam->pSubPartParsedInfo->uPartId > FTD_PART_ID_EXTHDR_END )
        {
            uParserRet = FTD_PARSER_ALREADY_PARSED;
            break;
        }
        else if ( aParam->pSubPartParsedInfo->uPartId < FTD_PART_ID_EXTHDR_BEGIN )
        {
            DbgOut( ERRO, DBG_PROTOHANDLER, "Unexpected part. uPartId=%u", aParam->pSubPartParsedInfo->uPartId );
            uParserRet = FTD_PARSER_FAILURE;
            break;
        }
        else
        {
        }

        UINT8 uExtHdrSize = 0;
        aParam->pFieldMgr->GetNumPartValue( FTD_PART_ID_HDR_EXTHDR_SIZE, uExtHdrSize );

        uParserRet = this->DoParse( aParam, uExtHdrSize, FTD_PART_ID_EXTHDR_BEGIN, FTD_PART_ID_EXTHDR_END, FALSE );
    } while ( 0 );

    DbgOut( VERB, DBG_PROTOHANDLER, "Leave. uParserRet=%u, uPartId=%u", uParserRet,
            aParam->pSubPartParsedInfo->uPartId );
    return uParserRet;
}

FtdParserErr CFtdParser::ParseFtdcHdr1( ParseParam * aParam )
{
    assert( aParam && aParam->pBuf && aParam->pFieldMgr && aParam->pMainPartParsedInfo && aParam->pSubPartParsedInfo );
    DbgOut( VERB, DBG_PROTOHANDLER, "Enter. uPartId=%u", aParam->pSubPartParsedInfo->uPartId );

    FtdParserErr uParserRet = FTD_PARSER_NEED_MORE_DATA;

    do
    {
        if ( aParam->pSubPartParsedInfo->uPartId > FTD_PART_ID_FTDCHDR1_END )
        {
            uParserRet = FTD_PARSER_ALREADY_PARSED;
            break;
        }
        else if ( aParam->pSubPartParsedInfo->uPartId < FTD_PART_ID_FTDCHDR1_BEGIN )
        {
            DbgOut( ERRO, DBG_PROTOHANDLER, "Unexpected part. uPartId=%u", aParam->pSubPartParsedInfo->uPartId );
            uParserRet = FTD_PARSER_FAILURE;
            break;
        }
        else
        {
        }

        UINT16 uFtdcSize = 0;
        aParam->pFieldMgr->GetNumPartValue( FTD_PART_ID_HDR_FTDC_SIZE, uFtdcSize );

        UINT uFtdcHdr1Size = ( 0 < uFtdcSize ) ? sizeof( FtdcHdr1 ) : 0;
        uParserRet =
            this->DoParse( aParam, uFtdcHdr1Size, FTD_PART_ID_FTDCHDR1_BEGIN, FTD_PART_ID_FTDCHDR1_END, FALSE );

        //It seems that if it's a response, then all later data are compressed
        if ( FTD_PARSER_SUCCESS == uParserRet )
        {
            UINT8 uType;
            aParam->pFieldMgr->GetNumPartValue( FTD_PART_ID_FTDCHDR1_TYPE, uType );
            if ( uType == 3 )
            {
                aParam->pFieldMgr->AddFlag( FTD_FLAG_FTDC_COMPRESSED );
            }
        }
    } while ( 0 );

    DbgOut( VERB, DBG_PROTOHANDLER, "Leave. uParserRet=%u, uPartId=%u", uParserRet,
            aParam->pSubPartParsedInfo->uPartId );
    return uParserRet;
}

FtdParserErr CFtdParser::ParseFtdcHdr2( ParseParam * aParam )
{
    assert( aParam && aParam->pBuf && aParam->pFieldMgr && aParam->pMainPartParsedInfo && aParam->pSubPartParsedInfo );
    DbgOut( VERB, DBG_PROTOHANDLER, "Enter. uPartId=%u", aParam->pSubPartParsedInfo->uPartId );

    FtdParserErr uParserRet = FTD_PARSER_NEED_MORE_DATA;

    do
    {
        if ( aParam->pSubPartParsedInfo->uPartId > FTD_PART_ID_FTDCHDR2_END )
        {
            uParserRet = FTD_PARSER_ALREADY_PARSED;
            break;
        }
        else if ( aParam->pSubPartParsedInfo->uPartId < FTD_PART_ID_FTDCHDR2_BEGIN )
        {
            DbgOut( ERRO, DBG_PROTOHANDLER, "Unexpected part. uPartId=%u", aParam->pSubPartParsedInfo->uPartId );
            uParserRet = FTD_PARSER_FAILURE;
            break;
        }
        else
        {
        }



        UINT16 uFtdcSize = 0;
        aParam->pFieldMgr->GetNumPartValue( FTD_PART_ID_HDR_FTDC_SIZE, uFtdcSize );

        //Decompress all FTDC data if needed
        UINT16 uFtdcCompressedSize =
            ( 0 < uFtdcSize ) ? uFtdcSize - sizeof( FtdcHdr1 ) : 0;    //Since FtdcHdr1 won't be compressed
        while ( 0 < aParam->uBufSize && aParam->pFieldMgr->IsFlagSet( FTD_FLAG_FTDC_COMPRESSED ) &&
                aParam->pMainPartParsedInfo->uCompressionParsedSize < uFtdcCompressedSize )
        {
            UCHAR uCurr = aParam->pBuf[0];
            if ( aParam->pMainPartParsedInfo->bIsEscaping )
            {
                aParam->pMainPartParsedInfo->vecDecompressedBuf.push_back( uCurr );
                aParam->pMainPartParsedInfo->bIsEscaping = FALSE;
            }
            else if ( 0xe0 == uCurr )
            {
                aParam->pMainPartParsedInfo->bIsEscaping = TRUE;
            }
            else if ( 0xe1 <= uCurr && uCurr <= 0xef )
            {
                for ( size_t i = 0; i < ( uCurr - 0xe0 ); i++ )
                {
                    aParam->pMainPartParsedInfo->vecDecompressedBuf.push_back( 0x00 );
                }
            }
            else
            {
                aParam->pMainPartParsedInfo->vecDecompressedBuf.push_back( uCurr );
            }

            aParam->pBuf++;
            aParam->uBufSize--;
            aParam->pMainPartParsedInfo->uCompressionParsedSize++;
        }
        if ( aParam->pFieldMgr->IsFlagSet( FTD_FLAG_FTDC_COMPRESSED ) && 0 < uFtdcCompressedSize )
        {
#if defined( USE_WPP )
            DbgOut( VERB, DBG_PROTOHANDLER, "Decompressed 0x%X/0x%X: %!HEXDUMP!",
                    aParam->pMainPartParsedInfo->uCompressionParsedSize, uFtdcCompressedSize,
                    WppHexDump( aParam->pMainPartParsedInfo->vecDecompressedBuf.data(),
                                aParam->pMainPartParsedInfo->vecDecompressedBuf.size() ) );
#elif defined( USE_G3LOG )
            string strHexDump;
            CWUtils::ToHexDump( aParam->pMainPartParsedInfo->vecDecompressedBuf.data(),
                                aParam->pMainPartParsedInfo->vecDecompressedBuf.size(), strHexDump, "|", 32 );
            DbgOut( VERB, DBG_PROTOHANDLER, "Decompressed 0x%X/0x%X:\n%s",
                    aParam->pMainPartParsedInfo->uCompressionParsedSize, uFtdcCompressedSize, strHexDump.c_str() );
#endif
        }
        if ( aParam->pMainPartParsedInfo->uCompressionParsedSize >= uFtdcSize )
        {
            aParam->pFieldMgr->RemoveFlag( FTD_FLAG_FTDC_COMPRESSED );
            aParam->pMainPartParsedInfo->uCompressionParsedSize = 0;
        }




        UINT uFtdcHdr2Size = ( 0 < uFtdcSize ) ? sizeof( FtdcHdr2 ) : 0;

        //Parse decompressed data
        uParserRet = this->DoParse( aParam, uFtdcHdr2Size, FTD_PART_ID_FTDCHDR2_BEGIN, FTD_PART_ID_FTDCHDR2_END, TRUE );
        if ( FTD_PARSER_NEED_MORE_DATA != uParserRet )
        {
            break;
        }

        //Should only hit here if data is not compressed
        uParserRet =
            this->DoParse( aParam, uFtdcHdr2Size, FTD_PART_ID_FTDCHDR2_BEGIN, FTD_PART_ID_FTDCHDR2_END, FALSE );
    } while ( 0 );

    DbgOut( VERB, DBG_PROTOHANDLER, "Leave. uParserRet=%u, uPartId=%u", uParserRet,
            aParam->pSubPartParsedInfo->uPartId );
    return uParserRet;
}


FtdParserErr CFtdParser::ParseFtdcFields( ParseParam * aParam )
{
    assert( aParam && aParam->pBuf && aParam->pFieldMgr && aParam->pMainPartParsedInfo && aParam->pSubPartParsedInfo );
    DbgOut( VERB, DBG_PROTOHANDLER, "Enter. uPartId=%u", aParam->pSubPartParsedInfo->uPartId );

    FtdParserErr uParserRet = FTD_PARSER_NEED_MORE_DATA;

    do
    {
        if ( aParam->pSubPartParsedInfo->uPartId > FTD_PART_ID_FTDC_END )
        {
            uParserRet = FTD_PARSER_ALREADY_PARSED;
            break;
        }
        else if ( aParam->pSubPartParsedInfo->uPartId < FTD_PART_ID_FTDC_BEGIN )
        {
            DbgOut( ERRO, DBG_PROTOHANDLER, "Unexpected part. uPartId=%u", aParam->pSubPartParsedInfo->uPartId );
            uParserRet = FTD_PARSER_FAILURE;
            break;
        }
        else
        {
        }




        UINT16 uFtdcSize = 0;
        aParam->pFieldMgr->GetNumPartValue( FTD_PART_ID_HDR_FTDC_SIZE, uFtdcSize );

        //Decompress all FTDC data if needed
        UINT16 uFtdcCompressedSize =
            ( 0 < uFtdcSize ) ? uFtdcSize - sizeof( FtdcHdr1 ) : 0;    //Since FtdcHdr1 won't be compressed
        while ( 0 < aParam->uBufSize && aParam->pFieldMgr->IsFlagSet( FTD_FLAG_FTDC_COMPRESSED ) &&
                aParam->pMainPartParsedInfo->uCompressionParsedSize < uFtdcCompressedSize )
        {
            UCHAR uCurr = aParam->pBuf[0];
            if ( aParam->pMainPartParsedInfo->bIsEscaping )
            {
                aParam->pMainPartParsedInfo->vecDecompressedBuf.push_back( uCurr );
                aParam->pMainPartParsedInfo->bIsEscaping = FALSE;
            }
            else if ( 0xe0 == uCurr )
            {
                aParam->pMainPartParsedInfo->bIsEscaping = TRUE;
            }
            else if ( 0xe1 <= uCurr && uCurr <= 0xef )
            {
                for ( size_t i = 0; i < ( uCurr - 0xe0 ); i++ )
                {
                    aParam->pMainPartParsedInfo->vecDecompressedBuf.push_back( 0x00 );
                }
            }
            else
            {
                aParam->pMainPartParsedInfo->vecDecompressedBuf.push_back( uCurr );
            }

            aParam->pBuf++;
            aParam->uBufSize--;
            aParam->pMainPartParsedInfo->uCompressionParsedSize++;
        }
        if ( aParam->pFieldMgr->IsFlagSet( FTD_FLAG_FTDC_COMPRESSED ) && 0 < uFtdcCompressedSize )
        {
#if defined( USE_WPP )
            DbgOut( VERB, DBG_PROTOHANDLER, "Decompressed 0x%X/0x%X: %!HEXDUMP!",
                    aParam->pMainPartParsedInfo->uCompressionParsedSize, uFtdcCompressedSize,
                    WppHexDump( aParam->pMainPartParsedInfo->vecDecompressedBuf.data(),
                                aParam->pMainPartParsedInfo->vecDecompressedBuf.size() ) );
#elif defined( USE_G3LOG )
            string strHexDump;
            CWUtils::ToHexDump( aParam->pMainPartParsedInfo->vecDecompressedBuf.data(),
                                aParam->pMainPartParsedInfo->vecDecompressedBuf.size(), strHexDump, "|", 32 );
            DbgOut( VERB, DBG_PROTOHANDLER, "Decompressed 0x%X/0x%X:\n%s",
                    aParam->pMainPartParsedInfo->uCompressionParsedSize, uFtdcCompressedSize, strHexDump.c_str() );
#endif
        }
        if ( aParam->pMainPartParsedInfo->uCompressionParsedSize >= uFtdcSize )
        {
            aParam->pFieldMgr->RemoveFlag( FTD_FLAG_FTDC_COMPRESSED );
            aParam->pMainPartParsedInfo->uCompressionParsedSize = 0;
        }



        UINT16 uFtdFieldSize;
        aParam->pFieldMgr->GetNumPartValue( FTD_PART_ID_FTDCHDR2_FIELD_SIZE, uFtdFieldSize );

        //Parse decompressed data
        uParserRet = this->DoParse( aParam, uFtdFieldSize, FTD_PART_ID_FTDC_BEGIN, FTD_PART_ID_FTDC_END, TRUE );
        if ( FTD_PARSER_NEED_MORE_DATA != uParserRet )
        {
            break;
        }

        //Should only hit here if data is not compressed
        uParserRet = this->DoParse( aParam, uFtdFieldSize, FTD_PART_ID_FTDC_BEGIN, FTD_PART_ID_FTDC_END, FALSE );
    } while ( 0 );

    DbgOut( VERB, DBG_PROTOHANDLER, "Leave. uParserRet=%u, uPartId=%u", uParserRet,
            aParam->pSubPartParsedInfo->uPartId );
    return uParserRet;
}


FtdParserErr CFtdParser::DoParse( ParseParam * aParam,
                                  UINT aMainPartSize,
                                  FtdPartId aBeginPartId,
                                  FtdPartId aEndPartId,
                                  BOOL aUseDecompessedBuf )
{
    assert( aParam && aParam->pFieldMgr && aParam->pPartialFtdExtHdrTag );
    DbgOut( VERB, DBG_PROTOHANDLER, "Enter" );

    FtdParserErr uParserRet = FTD_PARSER_NEED_MORE_DATA;

    CONST UCHAR * pCurrBuf;
    UINT uCurrBufSize;
    if ( aUseDecompessedBuf )
    {
        pCurrBuf = aParam->pMainPartParsedInfo->vecDecompressedBuf.data();
        uCurrBufSize = aParam->pMainPartParsedInfo->vecDecompressedBuf.size();
    }
    else
    {
        pCurrBuf = aParam->pBuf;
        uCurrBufSize = aParam->uBufSize;
    }
    DbgOut( VERB, DBG_PROTOHANDLER, "aUseDecompessedBuf=%d, pCurrBuf=0x%p, uCurrBufSize=0x%X", aUseDecompessedBuf,
            pCurrBuf, uCurrBufSize );


    do
    {
        while ( 0 < uCurrBufSize && aParam->pMainPartParsedInfo->uMainPartParsedSize < aMainPartSize )
        {
            if ( aParam->pSubPartParsedInfo->uPartId < aBeginPartId ||
                 aParam->pSubPartParsedInfo->uPartId > aEndPartId )
            {
                break;
            }

#if defined( USE_WPP )
            DbgOut( VERB, DBG_PROTOHANDLER, "Parsing %!FTDPARTID!|%!FTDPARTID!|%!FTDPARTID!=0x%X/0x%X", aBeginPartId,
                    aParam->pSubPartParsedInfo->uPartId, aEndPartId, aParam->pMainPartParsedInfo->uMainPartParsedSize,
                    aMainPartSize );
#elif defined( USE_G3LOG )
            DbgOut( VERB, DBG_PROTOHANDLER, "Parsing %u|%u|%u=0x%X/0x%X", aBeginPartId,
                    aParam->pSubPartParsedInfo->uPartId, aEndPartId, aParam->pMainPartParsedInfo->uMainPartParsedSize,
                    aMainPartSize );
#endif


            UINT uLeftSize = aParam->pSubPartParsedInfo->uSubPartSize - aParam->pSubPartParsedInfo->vecBuf.size();
            if ( 0 == uLeftSize || 0 == uCurrBufSize )
            {
                //Skip
            }
            else if ( uLeftSize <= uCurrBufSize )
            {
                aParam->pSubPartParsedInfo->vecBuf.insert( aParam->pSubPartParsedInfo->vecBuf.end(), pCurrBuf,
                                                           pCurrBuf + uLeftSize );

                FtdPartId uPartId;
                for ( uPartId = ( FtdPartId )( aBeginPartId + 1 ); uPartId < aEndPartId;
                      uPartId = ( FtdPartId )( uPartId + 1 ) )
                {
                    if ( aParam->pSubPartParsedInfo->uPartId == uPartId )
                    {
                        CONST FtdPartIdInfo * pInfo = &m_mapFtdPartIdInfo[uPartId];
                        if ( pInfo->uPartDataType == FTD_PART_DATA_TYPE_NUM )
                        {
                            UINT uData = Ntoh( aParam->pSubPartParsedInfo->vecBuf.data(),
                                               aParam->pSubPartParsedInfo->vecBuf.size() );
                            aParam->pFieldMgr->SetNumPartValue( pInfo->uPartId, uData );
#if defined( USE_WPP )
                            DbgOut( VERB, DBG_PROTOHANDLER, "Found %!FTDPARTID!, Value=0x%X", pInfo->uPartId, uData );
#elif defined( USE_G3LOG )
                            DbgOut( VERB, DBG_PROTOHANDLER, "Found %u, Value=0x%X", pInfo->uPartId, uData );
#endif
                        }
                        else
                        {
                            aParam->pFieldMgr->SetRawPartValue( pInfo->uPartId, aParam->pSubPartParsedInfo->vecBuf );
#if defined( USE_WPP )
                            DbgOut( VERB, DBG_PROTOHANDLER, "Found %!FTDPARTID!, Value=%!HEXDUMP!", pInfo->uPartId,
                                    WppHexDump( aParam->pSubPartParsedInfo->vecBuf.data(),
                                                aParam->pSubPartParsedInfo->vecBuf.size() ) );
#elif defined( USE_G3LOG )
                            string strHexDump;
                            CWUtils::ToHexDump( aParam->pSubPartParsedInfo->vecBuf.data(),
                                                aParam->pSubPartParsedInfo->vecBuf.size(), strHexDump, "|", 32 );
                            DbgOut( VERB, DBG_PROTOHANDLER, "Found %u, Value=%s", pInfo->uPartId, strHexDump.c_str() );
#endif
                        }

                        //Handling dynamic-size part
                        if ( aParam->pSubPartParsedInfo->uPartId == FTD_PART_ID_EXTHDR_TAG_CONTENT )
                        {
                            FtdExtHdrTag tag;
                            aParam->pFieldMgr->GetNumPartValue( FTD_PART_ID_EXTHDR_TAG_TYPE, (UINT8 &)tag.uFtdTagType );
                            aParam->pFieldMgr->GetNumPartValue( FTD_PART_ID_EXTHDR_TAG_SIZE, tag.uFtdTagSize );
                            if ( 0 < tag.uFtdTagSize )
                            {
                                aParam->pFieldMgr->GetRawPartValue( FTD_PART_ID_EXTHDR_TAG_CONTENT,
                                                                    tag.strFtdTagContent );
                            }

                            aParam->pFieldMgr->AddFtdExtHdrTag( tag );
                            aParam->pFieldMgr->RemoveNumPartValue( FTD_PART_ID_EXTHDR_TAG_SIZE );
                        }
                        else if ( aParam->pSubPartParsedInfo->uPartId == FTD_PART_ID_FTDC_FIELD_CONTENT )
                        {
                            FtdcField field;
                            aParam->pFieldMgr->GetNumPartValue( FTD_PART_ID_FTDC_FIELD_ID, field.uFieldId );
                            aParam->pFieldMgr->GetNumPartValue( FTD_PART_ID_FTDC_FIELD_SIZE, field.uFieldSize );
                            if ( 0 < field.uFieldSize )
                            {
                                aParam->pFieldMgr->GetRawPartValue( FTD_PART_ID_FTDC_FIELD_CONTENT,
                                                                    field.strFieldContent );
                            }

                            aParam->pFieldMgr->AddFtdcField( field );
                            aParam->pFieldMgr->RemoveNumPartValue( FTD_PART_ID_FTDC_FIELD_SIZE );
                        }
                        else
                        {
                        }


                        //Start to parse next part
                        if ( aParam->pSubPartParsedInfo->uPartId == FTD_PART_ID_EXTHDR_TAG_SIZE )
                        {
                            UINT8 uExtHdrTagSize;
                            aParam->pFieldMgr->GetNumPartValue( pInfo->uPartId, uExtHdrTagSize );
                            aParam->pSubPartParsedInfo->SwitchPart( pInfo->uNextPartId, uExtHdrTagSize );
                        }
                        else if ( aParam->pSubPartParsedInfo->uPartId == FTD_PART_ID_FTDC_FIELD_SIZE )
                        {
                            UINT16 uFtdcFieldSize;
                            aParam->pFieldMgr->GetNumPartValue( pInfo->uPartId, uFtdcFieldSize );
                            aParam->pSubPartParsedInfo->SwitchPart( pInfo->uNextPartId, uFtdcFieldSize );
                        }
                        else
                        {
                            aParam->pSubPartParsedInfo->SwitchPart( pInfo->uNextPartId );
                        }
                        break;
                    }
                }
                if ( uPartId == aEndPartId )
                {
                    DbgOut( ERRO, DBG_PROTOHANDLER, "Part information not implemented" );
                    uParserRet = FTD_PARSER_FAILURE;
                    break;
                }

                aParam->pSubPartParsedInfo->vecBuf.clear();
                aParam->pMainPartParsedInfo->uMainPartParsedSize += uLeftSize;
                pCurrBuf += uLeftSize;
                uCurrBufSize -= uLeftSize;
            }
            else
            {
                aParam->pSubPartParsedInfo->vecBuf.insert( aParam->pSubPartParsedInfo->vecBuf.end(), pCurrBuf,
                                                           pCurrBuf + uCurrBufSize );
                aParam->pMainPartParsedInfo->uMainPartParsedSize += uCurrBufSize;
                pCurrBuf += uCurrBufSize;
                uCurrBufSize -= uCurrBufSize;
                uParserRet = FTD_PARSER_NEED_MORE_DATA;
                break;
            }
        }

        //Removed parsed part from aParam->pMainPartParsedInfo->vecDecompressedBuf
        if ( aUseDecompessedBuf )
        {
            UINT uCurrParsedSize = pCurrBuf - aParam->pMainPartParsedInfo->vecDecompressedBuf.data();
            if ( 0 < uCurrParsedSize )
            {
                aParam->pMainPartParsedInfo->vecDecompressedBuf.erase(
                    aParam->pMainPartParsedInfo->vecDecompressedBuf.begin(),
                    aParam->pMainPartParsedInfo->vecDecompressedBuf.begin() + uCurrParsedSize );
            }
        }
        else
        {
            UINT uCurrParsedSize = pCurrBuf - aParam->pBuf;
            if ( 0 < uCurrParsedSize )
            {
                aParam->pBuf += uCurrParsedSize;
                aParam->uBufSize -= uCurrParsedSize;
            }
        }



        if ( aParam->pSubPartParsedInfo->uPartId < aBeginPartId || aParam->pSubPartParsedInfo->uPartId > aEndPartId )
        {
            aParam->pMainPartParsedInfo->uMainPartParsedSize = 0;
            uParserRet = FTD_PARSER_SUCCESS;
            break;
        }



        //Handling the case no such part exists or all dynamic-size parts are parsed
        if ( aParam->pMainPartParsedInfo->uMainPartParsedSize >= aMainPartSize )
        {
            switch ( aParam->pSubPartParsedInfo->uPartId )
            {
                case FTD_PART_ID_EXTHDR_TAG_TYPE:
                {
                    aParam->pSubPartParsedInfo->SwitchPart( FTD_PART_ID_FTDCHDR1_VERSION );
                    break;
                }
                case FTD_PART_ID_EXTHDR_TAG_CONTENT:
                {
                    //Handle the case the size is 0
                    FtdExtHdrTag tag;
                    aParam->pFieldMgr->GetNumPartValue( FTD_PART_ID_EXTHDR_TAG_TYPE, (UINT8 &)tag.uFtdTagType );
                    aParam->pFieldMgr->GetNumPartValue( FTD_PART_ID_EXTHDR_TAG_SIZE, tag.uFtdTagSize );
                    aParam->pFieldMgr->AddFtdExtHdrTag( tag );

                    aParam->pSubPartParsedInfo->SwitchPart( FTD_PART_ID_FTDCHDR1_VERSION );
                    break;
                }
                case FTD_PART_ID_FTDCHDR1_VERSION:
                {
                    aParam->pSubPartParsedInfo->SwitchPart( FTD_PART_ID_FTDCHDR2_SEQ_SERIES );
                    break;
                }
                case FTD_PART_ID_FTDCHDR2_SEQ_SERIES:
                {
                    aParam->pSubPartParsedInfo->SwitchPart( FTD_PART_ID_FTDC_FIELD_ID );
                    break;
                }
                case FTD_PART_ID_FTDC_FIELD_ID:
                {
                    if ( m_pfnRecordParsedCbk )
                    {
                        m_pfnRecordParsedCbk( m_pRecordParsedCbkCtx, aParam->bConnOut, this );
                    }


                    aParam->pSubPartParsedInfo->SwitchPart( FTD_PART_ID_HDR_TYPE );
                    aParam->pMainPartParsedInfo->Reset();
                    aParam->pFieldMgr->Reset();
                    break;
                }
                case FTD_PART_ID_FTDC_FIELD_CONTENT:
                {
                    //Handle the case the size is 0
                    FtdcField field;
                    aParam->pFieldMgr->GetNumPartValue( FTD_PART_ID_FTDC_FIELD_ID, field.uFieldId );
                    aParam->pFieldMgr->GetNumPartValue( FTD_PART_ID_FTDC_FIELD_SIZE, field.uFieldSize );
                    aParam->pFieldMgr->AddFtdcField( field );

                    aParam->pSubPartParsedInfo->SwitchPart( FTD_PART_ID_HDR_TYPE );
                    aParam->pMainPartParsedInfo->Reset();
                    aParam->pFieldMgr->Reset();
                    break;
                }
                default:
                {
                    break;
                }
            }

            aParam->pMainPartParsedInfo->uMainPartParsedSize = 0;
            uParserRet = FTD_PARSER_SUCCESS;
            break;
        }
    } while ( 0 );

#if defined( USE_WPP )
    DbgOut( VERB, DBG_PROTOHANDLER, "Leave. uParserRet=%!FTDPARSERERR!, uPartId=%!FTDPARTID!, uMainPartParsedSize=0x%X",
            uParserRet, aParam->pSubPartParsedInfo->uPartId, aParam->pMainPartParsedInfo->uMainPartParsedSize );
#elif defined( USE_G3LOG )
    DbgOut( VERB, DBG_PROTOHANDLER, "Leave. uParserRet=%u, uPartId=%u, uMainPartParsedSize=0x%X", uParserRet,
            aParam->pSubPartParsedInfo->uPartId, aParam->pMainPartParsedInfo->uMainPartParsedSize );
#endif
    return uParserRet;
}


CONST CFtdPartMgr * CFtdParser::GetPartMgr( BOOL aConnOut ) CONST
{
    if ( aConnOut )
    {
        return &m_ReqPartMgr;
    }
    else
    {
        return &m_RspPartMgr;
    }
}

}    //End of namespace CWUtils
