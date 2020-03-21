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

/*
 * Define some structure or constants used in VMWare open source thus make it easy to comprehend.
 */

namespace CWUtils
{
#ifdef __cplusplus
extern "C" {
#endif



//Type added by winest
#pragma pack( push )
#pragma pack( 1 )
typedef union
{
    struct
    {
        union
        {
            struct
            {
                UINT8 byte1;
                UINT8 byte2;
            };
            UINT16 loWord;
        };
        union
        {
            struct
            {
                UINT8 byte3;
                UINT8 byte4;
            };
            UINT16 hiWord;
        };
    };
    UINT32 dword;
} REG32;

typedef union
{
    struct
    {
        REG32 loDword;
        REG32 hiDword;
    };
    UINT64 uint64;
} REG64;
#pragma pack( pop )

#ifndef X86_64
#    define REG REG32
#else
#    define REG REG64
#endif

typedef struct
{
    REG eax;
    REG ebx;
    REG ecx;
    REG edx;
    REG esi;
    REG edi;
} BDOOR_REG_LB;

typedef struct
{
    REG eax;
    REG ebx;
    REG ecx;
    REG edx;
    REG esi;
    REG edi;
    REG ebp;
} BDOOR_REG_HB;

//The channel object used in high bandwidth backdoor
typedef struct
{
    //Identifier
    UINT16 id;

    //Data buffer
    unsigned char * data;

    //Allocated size
    size_t dataAlloc;

    /* The cookie */
    UINT32 cookieHigh;
    UINT32 cookieLow;
} MSG_CHANNEL;

/*
 * open-vm-tools-7.8.5-00000\lib\include\backdoor_def.h
 *
 *        This contains backdoor defines that can be included from
 *        an assembly language file.
 */

//If you want to add a new low-level backdoor call for a guest userland
//application, please consider using the GuestRpc mechanism instead. --hpreg

#define BDOOR_MAGIC 0x564D5868
//Low-bandwidth backdoor port. --hpreg
#define BDOOR_PORT 0x5658

#define BDOOR_CMD_GETMHZ               1
#define BDOOR_CMD_APMFUNCTION          2    //BDOOR_CMD_APMFUNCTION is used by FrobOS code or old BIOS code --hpreg
#define BDOOR_CMD_GETDISKGEO           3
#define BDOOR_CMD_GETPTRLOCATION       4
#define BDOOR_CMD_SETPTRLOCATION       5
#define BDOOR_CMD_GETSELLENGTH         6
#define BDOOR_CMD_GETNEXTPIECE         7
#define BDOOR_CMD_SETSELLENGTH         8
#define BDOOR_CMD_SETNEXTPIECE         9
#define BDOOR_CMD_GETVERSION           10
#define BDOOR_CMD_GETDEVICELISTELEMENT 11
#define BDOOR_CMD_TOGGLEDEVICE         12
#define BDOOR_CMD_GETGUIOPTIONS        13
#define BDOOR_CMD_SETGUIOPTIONS        14
#define BDOOR_CMD_GETSCREENSIZE        15
#define BDOOR_CMD_MONITOR_CONTROL      16
#define BDOOR_CMD_GETHWVERSION         17
#define BDOOR_CMD_OSNOTFOUND           18
#define BDOOR_CMD_GETUUID              19
#define BDOOR_CMD_GETMEMSIZE           20
#define BDOOR_CMD_HOSTCOPY             21    //Devel only
//BDOOR_CMD_GETOS2INTCURSOR, 22, is very old and defunct. Reuse.
#define BDOOR_CMD_GETTIME        23    //Deprecated. Use GETTIMEFULL.
#define BDOOR_CMD_STOPCATCHUP    24
#define BDOOR_CMD_PUTCHR         25    //Devel only
#define BDOOR_CMD_ENABLE_MSG     26    //Devel only
#define BDOOR_CMD_GOTO_TCL       27    //Devel only
#define BDOOR_CMD_INITPCIOPROM   28
#define BDOOR_CMD_INT13          29
#define BDOOR_CMD_MESSAGE        30
#define BDOOR_CMD_RSVD0          31
#define BDOOR_CMD_RSVD1          32
#define BDOOR_CMD_RSVD2          33
#define BDOOR_CMD_ISACPIDISABLED 34
#define BDOOR_CMD_TOE            35    //Not in use
//BDOOR_CMD_INITLSIOPROM, 36, was merged with 28. Reuse.
#define BDOOR_CMD_PATCH_SMBIOS_STRUCTS 37
#define BDOOR_CMD_MAPMEM               38    //Devel only
#define BDOOR_CMD_ABSPOINTER_DATA      39
#define BDOOR_CMD_ABSPOINTER_STATUS    40
#define BDOOR_CMD_ABSPOINTER_COMMAND   41
#define BDOOR_CMD_TIMER_SPONGE         42
#define BDOOR_CMD_PATCH_ACPI_TABLES    43

//Catch-all to allow synchronous tests
#define BDOOR_CMD_DEVEL_FAKEHARDWARE  44    //Debug only - needed in beta
#define BDOOR_CMD_GETHZ               45
#define BDOOR_CMD_GETTIMEFULL         46
#define BDOOR_CMD_STATELOGGER         47
#define BDOOR_CMD_CHECKFORCEBIOSSETUP 48
#define BDOOR_CMD_LAZYTIMEREMULATION  49
#define BDOOR_CMD_BIOSBBS             50
#define BDOOR_CMD_VASSERT             51
#define BDOOR_CMD_ISGOSDARWIN         52
#define BDOOR_CMD_DEBUGEVENT          53
#define BDOOR_CMD_OSNOTMACOSXSERVER   54
#define BDOOR_CMD_MAX                 55

//IMPORTANT NOTE: When modifying the behavior of an existing backdoor command,
//you must adhere to the semantics expected by the oldest Tools who use that
//command. Specifically, do not alter the way in which the command modifies
//the registers. Otherwise backwards compatibility will suffer.


//High-bandwidth backdoor port. --hpreg
#define BDOORHB_PORT 0x5659

#define BDOORHB_CMD_MESSAGE 0
#define BDOORHB_CMD_VASSERT 1
#define BDOORHB_CMD_MAX     2


//There is another backdoor which allows access to certain TSC-related
//values using otherwise illegal PMC indices when the pseudo_perfctr
//control flag is set.
#define BDOOR_PMC_HW_TSC      0x10000
#define BDOOR_PMC_REAL_NS     0x10001
#define BDOOR_PMC_APPARENT_NS 0x10002

#define IS_BDOOR_PMC( index ) ( ( ( index ) | 3 ) == 0x10003 )
#define BDOOR_CMD( ecx )      ( (ecx)&0xFFFF )




/*
 * open-vm-tools-7.8.5-00000\lib\include\guest_msg_def.h
 *
 *        Second layer of the internal communication channel between guest
 *        applications and VMWare
 *
 */

//Basic request types
typedef enum
{
    MESSAGE_TYPE_OPEN,
    MESSAGE_TYPE_SENDSIZE,
    MESSAGE_TYPE_SENDPAYLOAD,
    MESSAGE_TYPE_RECVSIZE,
    MESSAGE_TYPE_RECVPAYLOAD,
    MESSAGE_TYPE_RECVSTATUS,
    MESSAGE_TYPE_CLOSE,
} MessageType;


//Reply statuses
//The basic request succeeded
#define MESSAGE_STATUS_SUCCESS 0x0001
//VMWare has a message available for its party
#define MESSAGE_STATUS_DORECV 0x0002
//The channel has been closed
#define MESSAGE_STATUS_CLOSED 0x0004
//VMWare removed the message before the party fetched it
#define MESSAGE_STATUS_UNSENT 0x0008
//A checkpoint occurred
#define MESSAGE_STATUS_CPT 0x0010
//An underlying device is powering off
#define MESSAGE_STATUS_POWEROFF 0x0020
//VMWare has detected a timeout on the channel
#define MESSAGE_STATUS_TIMEOUT 0x0040
//VMWare supports high-bandwidth for sending and receiving the payload
#define MESSAGE_STATUS_HB 0x0080



//This mask defines the status bits that the guest is allowed to set;
//we use this to mask out all other bits when receiving the status
//from the guest. Otherwise, the guest can manipulate VMX state by
//setting status bits that are only supposed to be changed by the
//VMX. See bug 45385.
#define MESSAGE_STATUS_GUEST_MASK MESSAGE_STATUS_SUCCESS


//Max number of channels.
//Unfortunately this has to be public because the monitor part
//of the backdoor needs it for its trivial-case optimization. [greg]
#define GUESTMSG_MAX_CHANNEL 8

//Flags to open a channel. --hpreg
#define GUESTMSG_FLAG_COOKIE 0x80000000
#define GUESTMSG_FLAG_ALL    GUESTMSG_FLAG_COOKIE


//Maximum size of incoming message. This is to prevent denial of host service
//attacks from guest applications.
#define GUESTMSG_MAX_IN_SIZE ( 64 * 1024 )



//rpcout.h: Remote Procedure Call between VMware and guest applications
#define RPCI_PROTOCOL_NUM 0x49435052    //RPCI in memory



#ifdef __cplusplus
}
#endif

}    //End of namespace CWUtils