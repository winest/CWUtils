#pragma once
#include <stdint.h>
#include <cstdint>

#include <Windows.h>
#include <string>
#include <list>
#include <WS2tcpip.h>
#include <IPHlpApi.h>

namespace CWUtils
{

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CP_UTF16LE
    #define CP_UTF16LE  1200
#endif
    
#pragma pack( push , 1 )
#ifndef MAC_ADDR_SIZE
    #define MAC_ADDR_SIZE    6
#endif
typedef union _MacAddress
{
    uint8_t  u8[MAC_ADDR_SIZE / sizeof(uint8_t)];
    uint16_t u16[MAC_ADDR_SIZE / sizeof(uint16_t)];
} MacAddress;

#ifndef ETHERNET_TYPE_IPV4
    #define ETHERNET_TYPE_IPV4    0x0800
#endif
#ifndef ETHERNET_TYPE_IPV6
    #define ETHERNET_TYPE_IPV6    0x86DD
#endif
#ifndef ETHERNET_TYPE_ARP
    #define ETHERNET_TYPE_ARP     0x0806
#endif
#ifndef ETHERNET_TYPE_REVARP
    #define ETHERNET_TYPE_REVARP  0x8035
#endif
#ifndef ETHERNET_TYPE_WOL
    #define ETHERNET_TYPE_WOL     0x0842
#endif
typedef struct _EthernetHdr     //Ethernet Header ( 14 bytes )
{
    MacAddress DstMacAddr;
    MacAddress SrcMacAddr;
    uint16_t   EtherType;
} EthernetHdr;


#ifndef IPV4_ADDR_SIZE
    #define IPV4_ADDR_SIZE 4
#endif
typedef union _Ipv4Address      //IPv4 Header ( 20 bytes )
{
    BOOL operator==( const _Ipv4Address & aAddr ) const
    {
        return ( this->u32 == aAddr.u32 );
    }
    BOOL operator!=( const _Ipv4Address & aAddr ) const
    {
        return !( this->u32 == aAddr.u32 );
    }
    BOOL operator<( const _Ipv4Address & aAddr ) const
    {
        return ( this->u32 < aAddr.u32 );
    }
    uint8_t  u8[IPV4_ADDR_SIZE / sizeof(uint8_t)];
    uint32_t u32;
} Ipv4Address;
typedef struct _Ipv4Hdr
{
#ifdef BIG_ENDIAN
    uint8_t      Ver : 4;       //Version ( 4 bits ) + Header length ( 4 bits )
    uint8_t      HdrLen : 4;    
#else
    uint8_t      HdrLen : 4;    //Version ( 4 bits ) + Header length ( 4 bits )
    uint8_t      Ver : 4;       
#endif
    uint8_t      Tos;           //Type of service ( 8 bits )
    uint16_t     TotalLen;      //Total length ( 16 bits )
    uint16_t     Id;            //Identifier ( 16 bits )
#ifdef BIG_ENDIAN
    uint16_t     Flags : 3;     //Flags ( 3 bits) + Fragment offset ( 13 bits )
    uint16_t     Offset : 13;   
#else
    uint16_t     Offset : 13;   //Flags ( 3 bits) + Fragment offset ( 13 bits )
    uint16_t     Flags : 3;
#endif
    uint8_t      Ttl;           //Time to live ( 8 bits )
    uint8_t      Proto;         //Protocol ( 8 bits )
    uint16_t     Chksum;        //Header checksum ( 16 bits )
    Ipv4Address  SrcAddr;       //Source address ( 32 bits )
    Ipv4Address  DstAddr;       //Destination address ( 32 bits )
} Ipv4Hdr;

#ifndef IPV6_ADDR_SIZE
    #define IPV6_ADDR_SIZE   16
#endif
typedef union _Ipv6Address
{
    BOOL operator==( const _Ipv6Address & aAddr ) const
    {
        for ( SIZE_T i = 0 ; i < _countof(u32) ; i++ )
        {
            if ( this->u32[i] != aAddr.u32[i] )
            {
                return FALSE;
            }
        }
        return TRUE;
    }
    BOOL operator!=( const _Ipv6Address & aAddr ) const
    {
        return !( *this == aAddr );
    }
    BOOL operator<( const _Ipv6Address & aAddr ) const
    {
        for ( SIZE_T i = 0 ; i < _countof(u32) ; i++ )
        {
            if ( this->u32[i] < aAddr.u32[i] )
            {
                return TRUE;
            }
        }
        return FALSE;
    }
    uint8_t  u8[IPV6_ADDR_SIZE / sizeof(uint8_t)];
    uint16_t u16[IPV6_ADDR_SIZE / sizeof(uint16_t)];
    uint32_t u32[IPV6_ADDR_SIZE / sizeof(uint32_t)];
} Ipv6Address;
typedef struct _Ipv6Hdr         //IPv6 Header ( 40 bytes )
{
    uint32_t    VerPrioLabel;   //Version ( 4 bits ) + Priority ( 8 bits ) + Flow Label ( 20 bits )
    uint16_t    PayLen;         //Payload length ( 16 bits )
    uint8_t     NextHdr;        //Next header ( 8 bits )
    uint8_t     HopLimit;       //Hop Limit ( 8 bits )
    Ipv6Address SrcAddr;        //Source address ( 128 bits )
    Ipv6Address DstAddr;        //Destination address ( 128 bits )
} Ipv6Hdr;



#ifndef TCP_UDP_PORT_SIZE
    #define TCP_UDP_PORT_SIZE   2
#endif
typedef struct _TcpHdr          //TCP Header ( 20 bytes )
{
    uint16_t SrcPort;           //Source port ( 16 bits )
    uint16_t DstPort;           //Destination port ( 16 bits )
    uint32_t SeqNum;            //Sequence number ( 32 bits )
    uint32_t AckNum;            //Acknowledgement number ( 32 bits )
#ifdef BIG_ENDIAN
    uint8_t  HdrLen : 4;        //Header length ( 4 bits ) + Reserved ( 6 bits )
    uint8_t  Reserved : 4;
#else
    uint8_t  Reserved : 4;      //Header length ( 4 bits ) + Reserved ( 6 bits )
    uint8_t  HdrLen : 4;        
#endif
    //uint8_t  Flags;             //Flags ( 8 bits )
    union 
    {
        struct
        {
#ifdef BIG_ENDIAN
            uint8_t CWR : 1;          //Congestion Window Reduced Flag
            uint8_t ECN : 1;          //ECN-Echo Flag
            uint8_t URG : 1;          //Urgent Flag
            uint8_t ACK : 1;          //Acknowledgement Flag
            uint8_t PSH : 1;          //Push Flag
            uint8_t RST : 1;          //Reset Flag
            uint8_t SYN : 1;          //Synchronize Flag
            uint8_t FIN : 1;          //Finish Flag
#else
            uint8_t FIN : 1;          //Finish Flag
            uint8_t SYN : 1;          //Synchronize Flag
            uint8_t RST : 1;          //Reset Flag
            uint8_t PSH : 1;          //Push Flag
            uint8_t ACK : 1;          //Acknowledgement Flag
            uint8_t URG : 1;          //Urgent Flag
            uint8_t ECN : 1;          //ECN-Echo Flag
            uint8_t CWR : 1;          //Congestion Window Reduced Flag
#endif
        };
        uint8_t u8;
    } Flags;    

    uint16_t RecvWindow;        //Receive window ( 16 bits )
    uint16_t Chksum;            //Checksum ( 16 bits )
    uint16_t Urg;               //Urg data pointer ( 16 bits )
} TcpHdr;

typedef struct _UdpHdr          //UDP Header ( 8 bytes )
{
    uint16_t SrcPort;           //Source port ( 16 bits )
    uint16_t DstPort;           //Destination port ( 16 bits )
    uint16_t SegLen;            //UDP segment length ( 16 bits )
    uint16_t Chksum;            //Checksum ( 16 bits )
} UdpHdr;

#pragma pack( pop )








typedef struct _CW_NETWORK_INTERFACE_INFO {
    _CW_NETWORK_INTERFACE_INFO() : ulIfType(IF_TYPE_OTHER) , nOperStatus(IfOperStatusUnknown) , u64UploadSpeed(0) , u64DownloadSpeed(0)
                                   { ZeroMemory(&byMacAddr,sizeof(byMacAddr)); }
    std::string strGuid;
    std::wstring wstrName;
    std::wstring wstrDescription;
    std::wstring wstrDnsSuffix;
    UCHAR byMacAddr[6];
    IFTYPE ulIfType;
    IF_OPER_STATUS nOperStatus;
    ULONG64 u64UploadSpeed;
    ULONG64 u64DownloadSpeed;
    
    std::list<sockaddr_in> lsUnicastIp4;
    std::list<sockaddr_in6> lsUnicastIp6;
    std::list<sockaddr_in> lsAnycastIp4;
    std::list<sockaddr_in6> lsAnycastIp6;
    std::list<sockaddr_in> lsMulticastIp4;
    std::list<sockaddr_in6> lsMulticastIp6;
    std::list<sockaddr_in> lsPrefixIp4;
    std::list<ULONG> lsPrefixIp4Bit;
    std::list<sockaddr_in6> lsPrefixIp6;
    std::list<ULONG> lsPrefixIp6Bit;
    std::list<sockaddr_in> lsGatewayIp4;
    std::list<sockaddr_in6> lsGatewayIp6;
    std::list<sockaddr_in> lsDnsIp4;
    std::list<sockaddr_in6> lsDnsIp6;
} CW_NETWORK_INTERFACE_INFO;

BOOL GetNetworkInterfaceInfo( DWORD aFamily , std::list<CW_NETWORK_INTERFACE_INFO> & aInfos );


//Return ERROR_SUCCESS if no error happen. Other return code will cause the server to stop the connection
typedef DWORD (CALLBACK *SocketRecvCbk)( IN DWORD aSkt , IN VOID * aUserCtx , CONST CHAR * aRecvBuf , INT aRecvSize );
class CServerSock
{
    public :
        CServerSock() : m_bWsaStarted(FALSE) , m_dwCodePage(CP_ACP) , m_hEvtExit(NULL) ,
                        m_sktListen(INVALID_SOCKET) , m_sktAccpet(INVALID_SOCKET) ,
                        m_hSenderThread(NULL) , m_hReceiverThread(NULL) , m_cbkRecv(NULL) , m_pUserCtx(NULL) {}
        ~CServerSock() {}
    public :
        BOOL Init( SocketRecvCbk aRecvCbk , VOID * aUserCtx , DWORD aCodePage = CP_ACP );
        BOOL CloseSockets();
        BOOL UnInit();

        BOOL BindListen( CONST WCHAR * aIp , USHORT aPort , DWORD aFamily = AF_INET , DWORD aType = SOCK_STREAM , DWORD aProto = IPPROTO_TCP );
        BOOL Accept();
        BOOL SendRawData( CONST CHAR * aBuf , INT aBufSize );
        BOOL SendString( CONST WCHAR * aString , INT aStringLen , BOOL aUnEscapeChar );   //Will convert to the corresponding codepage before sent
        SOCKET GetAcceptSocket() { return m_sktAccpet; }
        HANDLE GetExitEvent() { return m_hEvtExit; }

    protected :
        static UINT CALLBACK SenderThread( VOID * aArgs );
        UINT CALLBACK DoSender();
        static UINT CALLBACK ReceiverThread( VOID * aArgs );
        UINT CALLBACK DoReceiver();

    private :
        BOOL m_bWsaStarted;
        DWORD m_dwCodePage;
        HANDLE m_hEvtExit;
        SOCKET m_sktListen , m_sktAccpet;
        HANDLE m_hSenderThread , m_hReceiverThread;
        SocketRecvCbk m_cbkRecv;
        VOID * m_pUserCtx;
};


#ifdef __cplusplus
}
#endif

}   //End of namespace CWUtils