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
#include <stdint.h>
#include <netinet/in.h>
#include <string>
#include <list>

namespace CWUtils
{
#ifdef __cplusplus
extern "C" {
#endif

#ifndef CP_UTF16LE
#    define CP_UTF16LE 1200
#endif





#ifndef MAC_ADDR_SIZE
#    define MAC_ADDR_SIZE 6
#endif

typedef union _MacAddress
{
    uint8_t u8[MAC_ADDR_SIZE / sizeof( uint8_t )];
    uint16_t u16[MAC_ADDR_SIZE / sizeof( uint16_t )];
} __attribute__( ( __packed__ ) ) MacAddress;

#ifndef ETHERNET_TYPE_IPV4
#    define ETHERNET_TYPE_IPV4 0x0800
#endif
#ifndef ETHERNET_TYPE_IPV6
#    define ETHERNET_TYPE_IPV6 0x86DD
#endif
#ifndef ETHERNET_TYPE_VLAN
#    define ETHERNET_TYPE_VLAN 0x8100
#endif
#ifndef ETHERNET_TYPE_ARP
#    define ETHERNET_TYPE_ARP 0x0806
#endif
#ifndef ETHERNET_TYPE_REVARP
#    define ETHERNET_TYPE_REVARP 0x8035
#endif
#ifndef ETHERNET_TYPE_WOL
#    define ETHERNET_TYPE_WOL 0x0842
#endif
#ifndef ETHERNET_TYPE_PPPOE_SESSION
#    define ETHERNET_TYPE_PPPOE_SESSION 0x8864
#endif
typedef struct _EthernetHdr    //Ethernet Header ( 14 bytes )
{
    MacAddress DstMacAddr;
    MacAddress SrcMacAddr;
    uint16_t EtherType;
} __attribute__( ( __packed__ ) ) EthernetHdr;


typedef struct _PppoeSessionHdr    //PPPoE Session Header ( 2 bytes )
{
#ifdef CWUTILS_BIG_ENDIAN
    uint8_t Ver : 4;
    uint8_t Type : 4;
#else
    uint8_t Type : 4;
    uint8_t Ver : 4;
#endif
    uint8_t Code;
    uint16_t SessionId;
    uint16_t Length;
} __attribute__( ( __packed__ ) ) PppoeSessionHdr;

#ifndef PPP_TYPE_IPV4
#    define PPP_TYPE_IPV4 0x0021
#endif
#ifndef PPP_TYPE_IPV6
#    define PPP_TYPE_IPV6 0x0057
#endif
#ifndef PPP_TYPE_ICMP
#    define PPP_TYPE_ICMP 0x8021
#endif

typedef struct _PppHdr    //PPPoE Session Header ( 2 bytes )
{
    uint16_t PppType;
} __attribute__( ( __packed__ ) ) PppHdr;


#ifndef IPV4_ADDR_SIZE
#    define IPV4_ADDR_SIZE 4
#endif
typedef union _Ipv4Address    //IPv4 Header ( 20 bytes )
{
    BOOL operator==( const _Ipv4Address & aAddr ) const { return ( this->u32 == aAddr.u32 ); }
    BOOL operator!=( const _Ipv4Address & aAddr ) const { return ! ( this->u32 == aAddr.u32 ); }
    BOOL operator<( const _Ipv4Address & aAddr ) const { return ( this->u32 < aAddr.u32 ); }
    uint8_t u8[IPV4_ADDR_SIZE / sizeof( uint8_t )];
    uint32_t u32;
} __attribute__( ( __packed__ ) ) Ipv4Address;
typedef struct _Ipv4Hdr
{
#ifdef CWUTILS_BIG_ENDIAN
    uint8_t Ver : 4;    //Version ( 4 bits ) + Header length ( 4 bits )
    uint8_t HdrLen : 4;
#else
    uint8_t HdrLen : 4;    //Version ( 4 bits ) + Header length ( 4 bits )
    uint8_t Ver : 4;
#endif
    uint8_t Tos;          //Type of service ( 8 bits )
    uint16_t TotalLen;    //Total length ( 16 bits )
    uint16_t Id;          //Identifier ( 16 bits )
#ifdef CWUTILS_BIG_ENDIAN
    uint16_t Flags : 3;    //Flags ( 3 bits) + Fragment offset ( 13 bits )
    uint16_t Offset : 13;
#else
    uint16_t Offset : 13;    //Flags ( 3 bits) + Fragment offset ( 13 bits )
    uint16_t Flags : 3;
#endif
    uint8_t Ttl;            //Time to live ( 8 bits )
    uint8_t Proto;          //Protocol ( 8 bits )
    uint16_t Chksum;        //Header checksum ( 16 bits )
    Ipv4Address SrcAddr;    //Source address ( 32 bits )
    Ipv4Address DstAddr;    //Destination address ( 32 bits )
} __attribute__( ( __packed__ ) ) Ipv4Hdr;

#ifndef IPV6_ADDR_SIZE
#    define IPV6_ADDR_SIZE 16
#endif
typedef union _Ipv6Address
{
    BOOL operator==( const _Ipv6Address & aAddr ) const
    {
        for ( SIZE_T i = 0; i < _countof( u32 ); i++ )
        {
            if ( this->u32[i] != aAddr.u32[i] )
            {
                return FALSE;
            }
        }
        return TRUE;
    }
    BOOL operator!=( const _Ipv6Address & aAddr ) const { return ! ( *this == aAddr ); }
    BOOL operator<( const _Ipv6Address & aAddr ) const
    {
        for ( SIZE_T i = 0; i < _countof( u32 ); i++ )
        {
            if ( this->u32[i] < aAddr.u32[i] )
            {
                return TRUE;
            }
        }
        return FALSE;
    }
    uint8_t u8[IPV6_ADDR_SIZE / sizeof( uint8_t )];
    uint16_t u16[IPV6_ADDR_SIZE / sizeof( uint16_t )];
    uint32_t u32[IPV6_ADDR_SIZE / sizeof( uint32_t )];
} __attribute__( ( __packed__ ) ) Ipv6Address;
typedef struct _Ipv6Hdr    //IPv6 Header ( 40 bytes )
{
    uint32_t VerPrioLabel;    //Version ( 4 bits ) + Priority ( 8 bits ) + Flow Label ( 20 bits )
    uint16_t PayLen;          //Payload length ( 16 bits )
    uint8_t NextHdr;          //Next header ( 8 bits )
    uint8_t HopLimit;         //Hop Limit ( 8 bits )
    Ipv6Address SrcAddr;      //Source address ( 128 bits )
    Ipv6Address DstAddr;      //Destination address ( 128 bits )
} __attribute__( ( __packed__ ) ) Ipv6Hdr;



#ifndef TCP_UDP_PORT_SIZE
#    define TCP_UDP_PORT_SIZE 2
#endif
typedef struct _TcpHdr    //TCP Header ( 20 bytes )
{
    uint16_t SrcPort;    //Source port ( 16 bits )
    uint16_t DstPort;    //Destination port ( 16 bits )
    uint32_t SeqNum;     //Sequence number ( 32 bits )
    uint32_t AckNum;     //Acknowledgement number ( 32 bits )
#ifdef CWUTILS_BIG_ENDIAN
    uint8_t HdrLen : 4;    //Header length ( 4 bits ) + Reserved ( 6 bits )
    uint8_t Reserved : 4;
#else
    uint8_t Reserved : 4;    //Header length ( 4 bits ) + Reserved ( 6 bits )
    uint8_t HdrLen : 4;
#endif
    //uint8_t  Flags;             //Flags ( 8 bits )
    union
    {
        struct
        {
#ifdef CWUTILS_BIG_ENDIAN
            uint8_t CWR : 1;    //Congestion Window Reduced Flag
            uint8_t ECN : 1;    //ECN-Echo Flag
            uint8_t URG : 1;    //Urgent Flag
            uint8_t ACK : 1;    //Acknowledgement Flag
            uint8_t PSH : 1;    //Push Flag
            uint8_t RST : 1;    //Reset Flag
            uint8_t SYN : 1;    //Synchronize Flag
            uint8_t FIN : 1;    //Finish Flag
#else
            uint8_t FIN : 1;    //Finish Flag
            uint8_t SYN : 1;    //Synchronize Flag
            uint8_t RST : 1;    //Reset Flag
            uint8_t PSH : 1;    //Push Flag
            uint8_t ACK : 1;    //Acknowledgement Flag
            uint8_t URG : 1;    //Urgent Flag
            uint8_t ECN : 1;    //ECN-Echo Flag
            uint8_t CWR : 1;    //Congestion Window Reduced Flag
#endif
        };
        uint8_t u8;
    } Flags;

    uint16_t RecvWindow;    //Receive window ( 16 bits )
    uint16_t Chksum;        //Checksum ( 16 bits )
    uint16_t Urg;           //Urg data pointer ( 16 bits )
} __attribute__( ( __packed__ ) ) TcpHdr;

typedef struct _UdpHdr    //UDP Header ( 8 bytes )
{
    uint16_t SrcPort;    //Source port ( 16 bits )
    uint16_t DstPort;    //Destination port ( 16 bits )
    uint16_t SegLen;     //UDP segment length ( 16 bits )
    uint16_t Chksum;     //Checksum ( 16 bits )
} __attribute__( ( __packed__ ) ) UdpHdr;



#ifdef __cplusplus
}
#endif

}    //End of namespace CWUtils
