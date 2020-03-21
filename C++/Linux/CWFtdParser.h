#pragma once

/*
 * Copyright (c) 2009-2020, ChienWei Hung <winestwinest@gmail.com>
 * CWUtils is published under the BSD-3-Clause license.
 *
 * CWUtils is a set of standalone APIs for developers to speed up their
 * programming. It should be very easy to port them to other projects or
 * learn how to implement things on different languages and platforms.
 *
 * The latest version can be found at https://github.com/winest/CWUtils
 */

#include "WinDef.h"
#include <map>
#include <string>
#include <vector>
#include <memory>

#include "CWString.h"
#include "CWNetwork.h"

typedef enum _FtdParserErr
{
    FTD_PARSER_FAILURE = 0,    //General error
    FTD_PARSER_SUCCESS,
    FTD_PARSER_ALREADY_PARSED,
    FTD_PARSER_NEED_MORE_DATA,
    FTD_PARSER_NOT_SUPPORTED,
    FTD_PARSER_DATA_EXCEED,    //Stop processing because data is too big
    FTD_PARSER_NO_MEMORY,
    FTD_PARSER_USER_ABORT
} FtdParserErr;


typedef enum _FtdPartDataType
{
    FTD_PART_DATA_TYPE_RAW = 0,
    FTD_PART_DATA_TYPE_NUM,
} FtdPartDataType;

typedef enum _FtdPartId
{
    FTD_PART_ID_UNKNOWN = 0,

    FTD_PART_ID_HDR_BEGIN,
    FTD_PART_ID_HDR_TYPE,
    FTD_PART_ID_HDR_EXTHDR_SIZE,
    FTD_PART_ID_HDR_FTDC_SIZE,
    FTD_PART_ID_HDR_END,

    FTD_PART_ID_EXTHDR_BEGIN,
    FTD_PART_ID_EXTHDR_TAG_TYPE,
    FTD_PART_ID_EXTHDR_TAG_SIZE,
    FTD_PART_ID_EXTHDR_TAG_CONTENT,
    FTD_PART_ID_EXTHDR_END,

    FTD_PART_ID_FTDCHDR1_BEGIN,
    FTD_PART_ID_FTDCHDR1_VERSION,
    FTD_PART_ID_FTDCHDR1_TYPE,
    FTD_PART_ID_FTDCHDR1_UNKNOWN1,
    FTD_PART_ID_FTDCHDR1_CHAIN_TYPE,
    FTD_PART_ID_FTDCHDR1_END,

    FTD_PART_ID_FTDCHDR2_BEGIN,
    FTD_PART_ID_FTDCHDR2_SEQ_SERIES,
    FTD_PART_ID_FTDCHDR2_SEQ_NUM,
    FTD_PART_ID_FTDCHDR2_UNKNOWN1,
    FTD_PART_ID_FTDCHDR2_FIELD_COUNT,
    FTD_PART_ID_FTDCHDR2_FIELD_SIZE,
    FTD_PART_ID_FTDCHDR2_REQ_ID,
    FTD_PART_ID_FTDCHDR2_END,

    FTD_PART_ID_FTDC_BEGIN,
    FTD_PART_ID_FTDC_FIELD_ID,
    FTD_PART_ID_FTDC_FIELD_SIZE,
    FTD_PART_ID_FTDC_FIELD_CONTENT,
    FTD_PART_ID_FTDC_END
} FtdPartId;

#define FTD_FLAG_NONE            0x0
#define FTD_FLAG_FTDC_COMPRESSED 0x1

namespace CWUtils
{
CONST UINT16 FTDC_FIELD_ID_ORDER_STATUS = 0x2508;



#pragma pack( push, 1 )
typedef enum _FtdType :
    UINT8
{
    FTD_TYPE_NONE = 0,
    FTD_TYPE_FTDC,
    FTD_TYPE_COMPRESSED
} FtdType;

typedef struct _FtdHdr
{
    FtdType uFtdType;
    UINT8 uFtdExtHdrSize;
    UINT16 uFtdcSize;
} FtdHdr;



typedef enum _FtdTagType :
    UINT8
{
    FTD_TAG_TYPE_NONE = 0,
    FTD_TAG_TYPE_DATETIME,
    FTD_TAG_TYPE_COMPRESS_METHOD,
    FTD_TAG_TYPE_SESSION_STATE,
    FTD_TAG_TYPE_KEEP_ALIVE,
    FTD_TAG_TYPE_TRADE_DATE,
    FTD_TAG_TYPE_TARGET,
    FTD_TAG_TYPE_UNKNOWN
} FtdTagType;

typedef struct _FtdExtHdrTag
{
    VOID Reset()
    {
        uFtdTagType = FTD_TAG_TYPE_NONE;
        uFtdTagSize = 0;
        strFtdTagContent.clear();
    }
    FtdTagType uFtdTagType;
    UINT8 uFtdTagSize;
    std::string strFtdTagContent;
} FtdExtHdrTag;

//Split FtdcHdr into two parts because the second part can be compressed
typedef struct _FtdcHdr1
{
    UINT8 uVersion;
    UINT8 uType;
    UINT8 uUnknown1;
    UINT8 uChainType;
} FtdcHdr1;

typedef struct _FtdcHdr2
{
    UINT16 uSeqSeries;
    UINT32 uSeqNum;
    UINT32 uUnknown1;    //Transaction ID?
    UINT16 uFieldCount;
    UINT16 uFieldSize;
    UINT32 uReqId;
} FtdcHdr2;


typedef struct _FtdcField
{
    UINT16 uFieldId;
    UINT16 uFieldSize;
    std::string strFieldContent;
} FtdcField;

typedef struct _FtdOrderField
{
    CHAR szTradingDay[9];
    CHAR szSettlementGroupId[9];
    CHAR szSettlementId[4];
    CHAR szOrderSysId[13];
    CHAR szParticipantId[11];
    CHAR szClientId[11];
    CHAR szUserId[16];
    CHAR szInstrumentId[31];
    CHAR szOrderPriceType[1];
    CHAR szDirection[1];
    CHAR szCombOffsetFlag[5];
    CHAR szCombHedgeFlag[5];
    CHAR szLimitPrice[8];
    CHAR szVolumeTotalOriginal[4];
    CHAR szTimeCondition[1];
    CHAR szGTDDate[9];
    CHAR szVolumeCondition[1];
    CHAR szMinVolume[4];
    CHAR szContingentCondition[1];
    CHAR szStopPrice[8];
    CHAR szForceCloseReason[1];
    CHAR szOrderLocalId[13];
    CHAR szIsAutoSuspend[4];
    CHAR szOrderSource[1];
    CHAR szOrderStatus[1];
    CHAR szOrderType[1];
    CHAR szVolumeTraded[4];
    CHAR szVolumeTotal[4];
    CHAR szInsertDate[9];
    CHAR szInsertTime[9];
    CHAR szActiveTime[9];
    CHAR szSuspendTime[9];
    CHAR szUpdateTime[9];
    CHAR szCancelTime[9];
    CHAR szActiveUserId[16];
    CHAR szPriority[4];
    CHAR szTimeSortId[4];
    CHAR szClearingPartId[11];
    CHAR szBusinessUnit[21];
} FtdOrderField;
#pragma pack( pop )



typedef struct _FtdPartIdInfo
{
    _FtdPartIdInfo() {}
    _FtdPartIdInfo( FtdPartId aPartId,
                    std::string aPartIdDesc,
                    FtdPartDataType aPartDataType,
                    UINT aPartSize,
                    FtdPartId aNextPartId ) :
        uPartId( aPartId ),
        strPartIdDesc( aPartIdDesc ),
        uPartDataType( aPartDataType ),
        uPartSize( aPartSize ),
        uNextPartId( aNextPartId )
    {
    }
    FtdPartId uPartId;
    std::string strPartIdDesc;

    FtdPartDataType uPartDataType;
    UINT uPartSize;
    FtdPartId uNextPartId;
} FtdPartIdInfo;





class CFtdPartMgr
{
    public:
    CFtdPartMgr() { this->Reset(); }
    virtual ~CFtdPartMgr() {}
    virtual VOID Reset();

    public:
    FtdParserErr GetRawPartValue( FtdPartId aPartId, std::string & aPartVal ) CONST;
    FtdParserErr SetRawPartValue( FtdPartId aPartId, CONST UCHAR * aPartVal, UINT aPartValSize );
    FtdParserErr SetRawPartValue( FtdPartId aPartId, CONST std::string & aPartVal );
    FtdParserErr SetRawPartValue( FtdPartId aPartId, CONST std::vector<UCHAR> & aPartVal );
    FtdParserErr RemoveRawPartValue( FtdPartId aPartId );
    FtdParserErr GetNumPartValue( FtdPartId aPartId, UINT8 & aPartVal ) CONST;
    FtdParserErr GetNumPartValue( FtdPartId aPartId, UINT16 & aPartVal ) CONST;
    FtdParserErr GetNumPartValue( FtdPartId aPartId, UINT32 & aPartVal ) CONST;
    FtdParserErr SetNumPartValue( FtdPartId aPartId, UINT32 aPartVal );
    FtdParserErr RemoveNumPartValue( FtdPartId aPartId );

    std::shared_ptr<CONST std::vector<FtdExtHdrTag>> GetFtdExtHdrTags() CONST;
    std::shared_ptr<CONST std::vector<FtdcField>> GetFtdcFields() CONST;

    DWORD GetFlags();
    BOOL IsFlagSet( DWORD aFlag );
    DWORD AddFlag( DWORD aFlag );
    DWORD RemoveFlag( DWORD aFlag );

    VOID AddFtdExtHdrTag( CONST FtdExtHdrTag & aTag );
    VOID AddFtdcField( CONST FtdcField & aField );

    protected:
    DWORD m_dwFlags;

    std::map<FtdPartId, std::string> m_mapRawParts;
    std::map<FtdPartId, UINT32> m_mapNumParts;

    std::vector<FtdExtHdrTag> m_vecFtdExtHdrTags;
    std::vector<FtdcField> m_vecFtdcFields;
};






//Return whether to save the FtdcField in parser's memory
class CFtdParser;
typedef BOOL( CALLBACK * FtdRecordParsedCbk )( IN VOID * aContext,
                                               IN BOOL aConnOut,
                                               IN CONST CWUtils::CFtdParser * aFtdParser );

class CFtdParser
{
    public:
    CFtdParser() { this->Reset(); }
    virtual ~CFtdParser() { this->UnInit(); }

    BOOL Init( FtdRecordParsedCbk aRecordParsedCbk, VOID * aRecordParsedCbkCtx );
    VOID Reset();
    VOID UnInit();

    private:
    typedef struct _MainPartParsedInfo
    {
        _MainPartParsedInfo() : uMainPartParsedSize( 0 ) {}
        VOID Reset()
        {
            uMainPartParsedSize = 0;
            uCompressionParsedSize = 0;
            vecDecompressedBuf.clear();
            bIsEscaping = FALSE;
        }
        UINT uMainPartParsedSize;

        UINT16 uCompressionParsedSize;
        std::vector<UCHAR> vecDecompressedBuf;
        BOOL bIsEscaping;
    } MainPartParsedInfo;

    class CSubPartParsedInfo
    {
        public:
        VOID Reset() { this->SwitchPart( FTD_PART_ID_HDR_TYPE ); }
        VOID SwitchPart( FtdPartId aPartId )
        {
            vecBuf.clear();

            this->uPartId = aPartId;
            CONST FtdPartIdInfo * pInfo = CFtdParser::GetFtdPartIdInfo( aPartId );
            if ( NULL != pInfo && -1 != pInfo->uPartSize )
            {
                this->uSubPartSize = pInfo->uPartSize;
            }
        }
        VOID SwitchPart( FtdPartId aPartId, UINT aStatePartSize )
        {
            vecBuf.clear();

            this->uPartId = aPartId;
            this->uSubPartSize = aStatePartSize;
        }

        FtdPartId uPartId;
        UINT uSubPartSize;
        std::vector<UCHAR> vecBuf;    //Partially parsed buffer that is waiting for more data
    };

    typedef struct _ParseParam
    {
        _ParseParam() :
            bConnOut( FALSE ),
            pBuf( NULL ),
            uBufSize( 0 ),
            pMainPartParsedInfo( NULL ),
            pSubPartParsedInfo( NULL ),
            pFieldMgr( NULL )
        {
        }
        BOOL bConnOut;
        CONST UCHAR * pBuf;    //Buffer to current incoming data
        UINT uBufSize;

        MainPartParsedInfo * pMainPartParsedInfo;
        CSubPartParsedInfo * pSubPartParsedInfo;
        CFtdPartMgr * pFieldMgr;

        FtdExtHdrTag * pPartialFtdExtHdrTag;
        FtdcField * pPartialFtdcField;
    } ParseParam;

    public:
    static CONST FtdPartIdInfo * GetFtdPartIdInfo( FtdPartId aPartId );
    FtdParserErr Input( BOOL aConnOut, const UCHAR * aBuf, UINT aBufSize, INT & aCallbackRet );

    CONST CFtdPartMgr * GetPartMgr( BOOL aConnOut ) CONST;

    protected:
    UINT32 Ntoh( CONST UCHAR * aBuf, UINT aBufSize );

    FtdParserErr ParseFtdHdr( ParseParam * aParam );
    FtdParserErr ParseFtdExtHdrTags( ParseParam * aParam );
    FtdParserErr ParseFtdcHdr1( ParseParam * aParam );
    FtdParserErr ParseFtdcHdr2( ParseParam * aParam );
    FtdParserErr ParseFtdcFields( ParseParam * aParam );

    FtdParserErr DoParse( ParseParam * aParam,
                          UINT aMainPartSize,
                          FtdPartId aBeginState,
                          FtdPartId aEndState,
                          BOOL aUseDecompessedBuf );

    protected:
    static std::map<FtdPartId, FtdPartIdInfo> m_mapFtdPartIdInfo;

    MainPartParsedInfo m_ReqMainPartParsedInfo;
    CSubPartParsedInfo m_ReqSubPartParsedInfo;
    CFtdPartMgr m_ReqPartMgr;
    FtdExtHdrTag m_ReqFtdExtHdrTag;
    FtdcField m_ReqFtdcField;

    MainPartParsedInfo m_RspMainPartParsedInfo;
    CSubPartParsedInfo m_RspSubPartParsedInfo;
    CFtdPartMgr m_RspPartMgr;
    FtdExtHdrTag m_RspFtdExtHdrTag;
    FtdcField m_RspFtdcField;

    FtdParserErr m_uLastParserErr;

    FtdRecordParsedCbk m_pfnRecordParsedCbk;
    VOID * m_pRecordParsedCbkCtx;
};



}    //End of namespace CWUtils
