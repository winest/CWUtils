/*
    Author: winest

    Provide the same interface used by VMWare tool to communicate with VMWare.
*/

#include "stdafx.h"
#include "CWVmWare.h"

namespace CWUtils
{

#ifdef __cplusplus
extern "C" {
#endif



CVMWare::CVMWare()
{
    //Constructor
}
CVMWare::~CVMWare()
{
    //Destructor
}

//Return UTF-8 encoding in bytes excluding terminating NULL
//Return >0 if something is in clipboard buffer, 0 if clipboard is empty, -1 if failed
INT CVMWare::GetClipboardSize()
{
    BDOOR_REG_LB regLB;
    regLB.ecx.loWord = BDOOR_CMD_GETSELLENGTH;
    if ( MagicLB( &regLB ) )
    {
        if ( regLB.eax.dword == 0xFFFFFFFF )    //Empty
            return 0;
        else
            return (INT)regLB.eax.dword;
    }
    return -1;
}

BOOL CVMWare::GetClipboard( TCHAR * aBuf , INT aBufLen )
{
    int i;
    int size = GetClipboardSize();
    if ( size <= 0 )
    {
        aBuf[0] = (TCHAR)0;
        if ( size == 0 )
            return TRUE;
        else
            return FALSE;
    }


    BYTE * tmp;
#if ( defined UNICODE || defined _UNICODE )
    tmp = new BYTE[size + 1];
#else
    tmp = aBuf;
#endif    
    tmp[size] = (BYTE)0;

    int left = size;    //How many bytes needed to be copied from clipboard

    BDOOR_REG_LB regLB;
    regLB.ecx.loWord = BDOOR_CMD_GETNEXTPIECE;

    i = 0;
    while ( left > 0 )
    {
        MagicLB( &regLB );
        switch ( left )
        {
            case 1 :
                tmp[i] = regLB.eax.byte1;
                left -= 1;
                break;
            case 2 :
                tmp[i] = regLB.eax.byte1;
                tmp[i+1] = regLB.eax.byte2;
                left -= 2;
                break;
            case 3 :
                tmp[i] = regLB.eax.byte1;
                tmp[i+1] = regLB.eax.byte2;
                tmp[i+2] = regLB.eax.byte3;
                left -= 3;
                break;
            default :
                *( (UINT32 *)(tmp+i) ) = regLB.eax.dword;
                left -= 4;
                break;
        }
        i += 4;
    }
    
#if ( defined UNICODE || defined _UNICODE )
    MultiByteToWideChar( CP_UTF8 , 0 , (LPCSTR)tmp , size+1 , aBuf , aBufLen + 1 );
    delete [] tmp;
#endif
    
    return TRUE;
}


BOOL CVMWare::SetClipboard( TCHAR * aText , INT aTextLen )
{
    int i;

    //Clean clipboard buffer first
    int size = GetClipboardSize();
    if ( size < 0 )
    {
        return FALSE;
    }

    BDOOR_REG_LB regLB;
    regLB.ecx.loWord = BDOOR_CMD_GETNEXTPIECE;
    for ( i = 0 ; i < size ; i+=4 )
    {
        MagicLB( &regLB );
    }

    BYTE * tmp;
#if    ( defined UNICODE || defined _UNICODE )
    //Convert aText to in UTF-8
    size = WideCharToMultiByte( CP_UTF8 , 0 , aText , aTextLen , NULL , 0 , NULL , NULL );
    tmp = new BYTE[size + 1];
    WideCharToMultiByte( CP_UTF8 , 0 , aText , aTextLen , (LPSTR)tmp , size+1 , NULL , NULL );
#else
    //Assume it's already UTF-8 encoding
    size = aTextLen;
    tmp = aText;
#endif
    tmp[size] = (BYTE)0;
    
    //Set the new text size to clipboard    
    regLB.ecx.loWord = BDOOR_CMD_SETSELLENGTH;
    regLB.ebx.dword = size;
    if ( ! MagicLB( &regLB ) )
        return FALSE;

    //Set the new text to clipboard
    int left = size;
    regLB.ecx.loWord = BDOOR_CMD_SETNEXTPIECE;

    i = 0;
    while ( left > 0 )
    {
        switch ( left )
        {
            case 1 :
                regLB.ebx.byte1 = tmp[i];
                left -= 1;
                break;
            case 2 :
                regLB.ebx.byte1 = tmp[i];
                regLB.ebx.byte2 = tmp[i+1];
                left -= 2;
                break;
            case 3 :
                regLB.ebx.byte1 = tmp[i];
                regLB.ebx.byte2 = tmp[i+1];
                regLB.ebx.byte3 = tmp[i+2];
                left -= 3;
                break;
            default :
                regLB.ebx.dword = *( (UINT32 *)(tmp+i) );
                left -= 4;
                break;
        }
        MagicLB( &regLB );

        i += 4;
    }

#if    ( defined UNICODE || defined _UNICODE )
    delete [] tmp;
#endif
    return TRUE;
}









//0x4F4C4354 for in and RPCI_PROTOCOL_NUM for out
MSG_CHANNEL * CVMWare::MsgChannelOpen( UINT32 aProtocol )
{
    MSG_CHANNEL * channel;
    UINT32 flags;
    BDOOR_REG_LB regLB;

    channel = (MSG_CHANNEL *)malloc( sizeof(*channel) );
    if ( channel == NULL ) 
    {
        free( channel );
        channel = NULL;
        return NULL;
    }

    flags = GUESTMSG_FLAG_COOKIE;

retry :
    //IN: Type
    regLB.ecx.hiWord = MESSAGE_TYPE_OPEN;
    //IN: Magic number of the protocol and flags
    regLB.ebx.dword = aProtocol | flags;

    regLB.ecx.loWord = BDOOR_CMD_MESSAGE;
    MagicLB( &regLB );

    //OUT: Status
    if ( (regLB.ecx.hiWord & MESSAGE_STATUS_SUCCESS ) == 0)
    {
        if ( flags )
        {
            //Cookies not supported. Fall back to no cookie. --hpreg
            flags = 0;
            goto retry;
        }
        else
        {
            free( channel );
            channel = NULL;
            return NULL;    
        }
    }

    //OUT: Id and cookie
    channel->id = regLB.edx.hiWord;
    channel->cookieHigh = regLB.esi.dword;
    channel->cookieLow = regLB.edi.dword;

    //Initialize the channel
    channel->data = NULL;
    channel->dataAlloc = 0;

    return channel;
}

BOOL CVMWare::MsgChannelSend( MSG_CHANNEL * aChannel , UINT8 * aCmd , INT aCmdLen )
{
    const UINT8 * buf;
    size_t bufLen;
    BDOOR_REG_LB regLB;

retry :
    buf = aCmd;
    bufLen = aCmdLen;

    //Send the size.

    //IN: Type
    regLB.ecx.hiWord = MESSAGE_TYPE_SENDSIZE;
    //IN: Id and cookie
    regLB.edx.hiWord = aChannel->id;
    regLB.esi.dword = aChannel->cookieHigh;
    regLB.edi.dword = aChannel->cookieLow;
    //IN: Size
    regLB.ebx.dword = bufLen;

    regLB.ecx.loWord = BDOOR_CMD_MESSAGE;
    MagicLB( &regLB );

    //OUT: Status
    if ( ( regLB.ecx.hiWord & MESSAGE_STATUS_SUCCESS ) == 0 )
    {
        return FALSE;
    }

    if ( regLB.ecx.hiWord & MESSAGE_STATUS_HB )
    {
        //High-bandwidth backdoor port supported. Send the message in one backdoor operation. --hpreg

        if ( bufLen ) 
        {
            BDOOR_REG_HB regHB;

            regHB.ebx.loWord = BDOORHB_CMD_MESSAGE;
            regHB.ebx.hiWord = MESSAGE_STATUS_SUCCESS;
            regHB.edx.hiWord = aChannel->id;
            regHB.ebp.dword = aChannel->cookieHigh;
            regHB.edi.dword = aChannel->cookieLow;
            regHB.ecx.dword = bufLen;
            regHB.esi.dword = (uintptr_t)buf;

            MagicHBOut( &regHB );
            if ( ( regHB.ebx.hiWord & MESSAGE_STATUS_SUCCESS ) == 0 ) 
            {
                if ( ( regHB.ebx.hiWord & MESSAGE_STATUS_CPT ) != 0) 
                {
                    //A checkpoint occurred. Retry the operation. --hpreg
                    goto retry;
                }
                return FALSE;
            }
        }
    } 
    else 
    {
        //High-bandwidth backdoor port not supported. Send the message, 4 bytes at a time. --hpreg
        
        while ( bufLen > 0 )
        {
             //IN: Type
             regLB.ecx.hiWord = MESSAGE_TYPE_SENDPAYLOAD;
             //IN: Id and cookie
             regLB.edx.hiWord = aChannel->id;
             regLB.esi.dword = aChannel->cookieHigh;
             regLB.edi.dword = aChannel->cookieLow;
             //IN: Piece of message

             //Beware in case we are not allowed to read extra bytes beyond the end of the buffer.
             switch ( bufLen ) 
             {
                 case 1 :
                     regLB.ebx.dword = buf[0];
                     bufLen -= 1;
                     break;
                 case 2 :
                     regLB.ebx.dword = buf[0] | buf[1] << 8;
                     bufLen -= 2;
                     break;
                 case 3 :
                     regLB.ebx.dword = buf[0] | buf[1] << 8 | buf[2] << 16;
                     bufLen -= 3;
                     break;
                 default :
                     regLB.ebx.dword = *( (UINT32 *)buf );
                     bufLen -= 4;
                     break;
             }

             regLB.ecx.loWord = BDOOR_CMD_MESSAGE;
             MagicLB( &regLB );

             //OUT: Status
             if ( ( regLB.ecx.hiWord & MESSAGE_STATUS_SUCCESS ) == 0 )
             {
                 if ( (regLB.ecx.hiWord & MESSAGE_STATUS_CPT) != 0 )
                 {
                     //A checkpoint occurred. Retry the operation. --hpreg
                     goto retry;
                 }
                 return FALSE;
             }

             buf += 4;
        }
    }

    return TRUE;
}
BOOL CVMWare::MsgChannelReceive( MSG_CHANNEL * aChannel , UINT8 * aBuf , INT * aSizeReceived  )
{
    BDOOR_REG_LB regLB;
    size_t sizeReceived;
    UINT8 * buf;

retry : 

    //Is there a message waiting for our retrieval?

    //IN: Type
    regLB.ecx.hiWord = MESSAGE_TYPE_RECVSIZE;
    //IN: Id and cookie
    regLB.edx.hiWord = aChannel->id;
    regLB.esi.dword = aChannel->cookieHigh;
    regLB.edi.dword = aChannel->cookieLow;

    regLB.ecx.loWord = BDOOR_CMD_MESSAGE;
    MagicLB( &regLB );

    //OUT: Status
    if ( ( regLB.ecx.hiWord & MESSAGE_STATUS_SUCCESS ) == 0 ) 
    {
        return FALSE;
    }

    if ( ( regLB.ecx.hiWord & MESSAGE_STATUS_DORECV ) == 0 ) 
    {
        //No message to retrieve
        *aSizeReceived = 0;
        return TRUE;
    }


    //Receive the size.

    //OUT: Type
    if ( regLB.edx.hiWord != MESSAGE_TYPE_SENDSIZE )
    {
        return FALSE;
    }

    //OUT: Size
    sizeReceived = regLB.ebx.dword;


    //Allocate an extra byte for a trailing NULL character. The code that will
    //deal with this message may not know about binary strings, and may expect
    //a C string instead. --hpreg

    if ( sizeReceived + 1 > aChannel->dataAlloc )
    {
        buf = (UINT8 *)realloc( aChannel->data , sizeReceived + 1 );
        if ( buf == NULL )
        {
            goto error_quit;
        }
        
        aChannel->data = buf;
        aChannel->dataAlloc = sizeReceived + 1;
    }
    *aSizeReceived = sizeReceived;
    buf = aBuf = aChannel->data;

    if ( regLB.ecx.hiWord & MESSAGE_STATUS_HB )
    {
        //High-bandwidth backdoor port supported. Receive the message in one backdoor operation. --hpreg
        
        if ( sizeReceived )
        {
            BDOOR_REG_HB regHB;
            
            regHB.ebx.loWord = BDOORHB_CMD_MESSAGE;
            regHB.ebx.hiWord = MESSAGE_STATUS_SUCCESS;
            regHB.edx.hiWord = aChannel->id;
            regHB.esi.dword = aChannel->cookieHigh;
            regHB.ebp.dword = aChannel->cookieLow;
            regHB.ecx.dword = sizeReceived;
            regHB.edi.dword = (uintptr_t)buf;

            MagicHBIn( &regHB );
            if ( ( regHB.ebx.hiWord & MESSAGE_STATUS_SUCCESS ) == 0 )
            {
                if ( ( regHB.ebx.hiWord & MESSAGE_STATUS_CPT ) != 0 )
                {
                    //A checkpoint occurred. Retry the operation. --hpreg
                    goto retry;
                }
                goto error_quit;
            }
        }
    } 
    else 
    {
        //High-bandwidth backdoor port not supported. Receive the message, 4 bytes at a time. --hpreg
       
        while ( sizeReceived > 0 ) 
        {
            //IN: Type
            regLB.ecx.hiWord = MESSAGE_TYPE_RECVPAYLOAD;
            //IN: Id and cookie
            regLB.edx.hiWord = aChannel->id;
            regLB.esi.dword = aChannel->cookieHigh;
            regLB.edi.dword = aChannel->cookieLow;
            //IN: Status for the previous request (that succeeded)
            regLB.ebx.dword = MESSAGE_STATUS_SUCCESS;

            regLB.ecx.loWord = BDOOR_CMD_MESSAGE;
            MagicLB( &regLB );

            //OUT: Status
            if ( ( regLB.ecx.hiWord & MESSAGE_STATUS_SUCCESS ) == 0 ) 
            {
                if ( ( regLB.ecx.hiWord & MESSAGE_STATUS_CPT ) != 0 ) 
                {
                    //A checkpoint occurred. Retry the operation. --hpreg
                    goto retry;
                }

                goto error_quit;
            }
            
            //OUT: Type
            if ( regLB.edx.hiWord != MESSAGE_TYPE_SENDPAYLOAD ) 
            {
                goto error_quit;
            }

            //OUT: Piece of message
            
            //Beware in case we are not allowed to write extra bytes beyond the end of the buffer. --hpreg
            switch ( sizeReceived ) 
            {
                case 1:
                    buf[0] = regLB.ebx.byte1;
                    sizeReceived -= 1;
                    break;
                case 2:
                    buf[0] = regLB.ebx.byte1;
                    buf[1] = regLB.ebx.byte2;
                    sizeReceived -= 2;
                    break;
                case 3:
                    buf[0] = regLB.ebx.byte1;
                    buf[1] = regLB.ebx.byte2;
                    buf[2] = regLB.ebx.byte3;
                    sizeReceived -= 3;
                    break;
                default:
                    *( (UINT32 *)buf ) = regLB.ebx.dword;
                    sizeReceived -= 4;
                    break;
            }

             buf += 4;
        }
    }

    //Write a trailing NULL just after the message. --hpreg
    aChannel->data[*aSizeReceived] = '\0';

    //IN: Type
    regLB.ecx.hiWord = MESSAGE_TYPE_RECVSTATUS;
    //IN: Id and cookie
    regLB.edx.hiWord = aChannel->id;
    regLB.esi.dword = aChannel->cookieHigh;
    regLB.edi.dword = aChannel->cookieLow;
    //IN: Status for the previous request (that succeeded)
    regLB.ebx.dword = MESSAGE_STATUS_SUCCESS;

    regLB.ecx.loWord = BDOOR_CMD_MESSAGE;
    MagicLB( &regLB );

    //OUT: Status
    if ( ( regLB.ecx.hiWord & MESSAGE_STATUS_SUCCESS ) == 0 ) 
    {
        if ( ( regLB.ecx.hiWord & MESSAGE_STATUS_CPT ) != 0 )
        {
            //A checkpoint occurred. Retry the operation. --hpreg
            goto retry;
        }
        goto error_quit;
    }

    return TRUE;

error_quit :
    //IN: Type
    if ( sizeReceived == 0 ) 
    {
        regLB.ecx.hiWord = MESSAGE_TYPE_RECVSTATUS;
    } 
    else 
    {
        regLB.ecx.hiWord = MESSAGE_TYPE_RECVPAYLOAD;
    }
    //IN: Id and cookie
    regLB.edx.hiWord = aChannel->id;
    regLB.esi.dword = aChannel->cookieHigh;
    regLB.edi.dword = aChannel->cookieLow;
    //IN: Status for the previous request (that failed)
    regLB.ebx.dword = 0;

    regLB.ecx.loWord = BDOOR_CMD_MESSAGE;
    MagicLB( &regLB );

    //OUT: Status
    if ( ( regLB.ecx.hiWord & MESSAGE_STATUS_SUCCESS ) == 0 )
    {
        return FALSE;
    }

    return FALSE;
}
BOOL CVMWare::MsgChannelClose( MSG_CHANNEL * aChannel )
{
    BDOOR_REG_LB regLB;

    //IN: Type
    regLB.ecx.hiWord = MESSAGE_TYPE_CLOSE;
    //IN: Id and cookie
    regLB.edx.hiWord = aChannel->id;
    regLB.esi.dword = aChannel->cookieHigh;
    regLB.edi.dword = aChannel->cookieLow;

    regLB.ecx.loWord = BDOOR_CMD_MESSAGE;
    MagicLB( &regLB );

    //OUT: Status
    if ( ( regLB.ecx.hiWord & MESSAGE_STATUS_SUCCESS ) == 0 )
    {
        return FALSE;
    }
    else{}

    free( aChannel->data );
    aChannel->data = NULL;

    free( aChannel );

    return TRUE;
}

INT CVMWare::CastMagic( INT aMagicNum , TCHAR * aParam1 , TCHAR * aParam2 , TCHAR * aOutput )
{
    BDOOR_REG_LB regLB;
    regLB.ecx.loWord = aMagicNum;
    int nParam1 = _ttoi( aParam1 );
    int nParam2 = _ttoi( aParam2 );

    switch ( aMagicNum )
    {
        case BDOOR_CMD_GETMHZ :
            break;
        case BDOOR_CMD_APMFUNCTION :
            break;
        case BDOOR_CMD_GETDISKGEO :
            break;
        case BDOOR_CMD_GETPTRLOCATION :
            break;
        case BDOOR_CMD_SETPTRLOCATION :
            break;
        case BDOOR_CMD_GETSELLENGTH :
            if ( MagicLB( &regLB ) )
                _stprintf( aOutput , _T("Left length: %d\r\n") , regLB.eax );
            break;
        case BDOOR_CMD_GETNEXTPIECE :
            if ( MagicLB( &regLB ) && regLB.eax.dword != 0 )
                _stprintf( aOutput , _T("Content: %#x %#x %#x %#x\r\n") , regLB.eax.byte1 , regLB.eax.byte2 , regLB.eax.byte3 , regLB.eax.byte4 );
            break;
        case BDOOR_CMD_SETSELLENGTH :    //Need to read all bytes left in clipboard at first
            regLB.ebx.dword = nParam1;    //Text length
            if ( MagicLB( &regLB ) )
                _stprintf( aOutput , _T("Set length to: %d\r\n") , regLB.ebx );
            break;
        case BDOOR_CMD_SETNEXTPIECE :
            if ( aParam1[0] != 0 )
            {
                regLB.ebx.byte1 = aParam1[0];
                if ( aParam1[1] != 0 )
                {
                    regLB.ebx.byte2 = aParam1[1];
                    if ( aParam1[2] != 0 )
                    {
                        regLB.ebx.byte3 = aParam1[2];
                        if ( aParam1[3] != 0 )
                        {
                            regLB.ebx.byte4 = aParam1[3];
                        }
                    }
                }
            }

            if ( MagicLB( &regLB ) )
                _stprintf( aOutput , _T("Set %#x %#x %#x %#x\r\n") , regLB.ebx.byte1 , regLB.ebx.byte2 , regLB.ebx.byte3 , regLB.ebx.byte4 );
            
            break;
        case BDOOR_CMD_GETVERSION :    //10
            regLB.ebx.dword = ~BDOOR_MAGIC;
            if ( MagicLB( &regLB ) )
                _stprintf( aOutput , _T("Version: %d, Product type: %d\r\n") , regLB.eax , regLB.ecx );
            break;
        case BDOOR_CMD_GETDEVICELISTELEMENT :
            break;
        case BDOOR_CMD_TOGGLEDEVICE :
            break;
        case BDOOR_CMD_GETGUIOPTIONS :
            break;
        case BDOOR_CMD_SETGUIOPTIONS :
            break;
        case BDOOR_CMD_GETSCREENSIZE :
            break;
        case BDOOR_CMD_MONITOR_CONTROL :
            break;
        case BDOOR_CMD_GETHWVERSION :
            break;
        case BDOOR_CMD_OSNOTFOUND :
            break;
        case BDOOR_CMD_GETUUID :
            break;
        case BDOOR_CMD_GETMEMSIZE :    //20
            break;
        case BDOOR_CMD_HOSTCOPY : /* Devel only */
            break;
        /* BDOOR_CMD_GETOS2INTCURSOR, :, is very old and defunct. Reuse. */
        case BDOOR_CMD_GETTIME : /* Deprecated. Use GETTIMEFULL. */
            break;
        case BDOOR_CMD_STOPCATCHUP :
            break;
        case BDOOR_CMD_PUTCHR : /* Devel only */
            break;
        case BDOOR_CMD_ENABLE_MSG : /* Devel only */
            break;
        case BDOOR_CMD_GOTO_TCL : /* Devel only */
            break;
        case BDOOR_CMD_INITPCIOPROM :
            break;
        case BDOOR_CMD_INT13 :
            break;
        case BDOOR_CMD_MESSAGE :    //30
            break;
        case BDOOR_CMD_RSVD0 :
            break;
        case BDOOR_CMD_RSVD1 :
            break;
        case BDOOR_CMD_RSVD2 :
            break;
        case BDOOR_CMD_ISACPIDISABLED :
            break;
        case BDOOR_CMD_TOE : /* Not in use */
            break;
        /* BDOOR_CMD_INITLSIOPROM, :, was merged with :. Reuse. */
        case BDOOR_CMD_PATCH_SMBIOS_STRUCTS :
            break;
        case BDOOR_CMD_MAPMEM : /* Devel only */
            break;
        case BDOOR_CMD_ABSPOINTER_DATA :
            break;
        case BDOOR_CMD_ABSPOINTER_STATUS :    //40
            break;
        case BDOOR_CMD_ABSPOINTER_COMMAND :
            break;
        case BDOOR_CMD_TIMER_SPONGE :
            break;
        case BDOOR_CMD_PATCH_ACPI_TABLES :
            break;
        /* Catch-all to allow synchronous tests */
        case BDOOR_CMD_DEVEL_FAKEHARDWARE : /* Debug only - needed in beta */
            break;
        case BDOOR_CMD_GETHZ :
            break;
        case BDOOR_CMD_GETTIMEFULL :
            break;
        case BDOOR_CMD_STATELOGGER :
            break;
        case BDOOR_CMD_CHECKFORCEBIOSSETUP :
            break;
        case BDOOR_CMD_LAZYTIMEREMULATION :
            break;
        case BDOOR_CMD_BIOSBBS :    //50
            break;
        case BDOOR_CMD_VASSERT :
            break;
        case BDOOR_CMD_ISGOSDARWIN :
            break;
        case BDOOR_CMD_DEBUGEVENT :
            break;
        case BDOOR_CMD_OSNOTMACOSXSERVER :
            break;
        case BDOOR_CMD_MAX :    //55
            break;
        default :
            return -1;
    }
    return 0;
}

BOOL CVMWare::TestMagic( INT aMagicNum , TCHAR * aOutput )
{
    BDOOR_REG_LB regLB;    
    
    regLB.ecx.loWord = aMagicNum;    //Function number

    regLB.ebx.dword = ~BDOOR_MAGIC;    //Make sure ebx does not contain BDOOR_MAGIC

    if( MagicLB( &regLB ) )
    {
        _stprintf( aOutput , _T("eax: %#x, ebx: %#x, ecx: %#x, edx: %#x, edi: %#x, esi: %#x\r\n") ,
                   regLB.eax , regLB.ebx , regLB.ecx , regLB.edx , regLB.edi , regLB.esi );
        return TRUE;
    }
    return FALSE;
}


BOOL CVMWare::MagicLB( BDOOR_REG_LB * aBackdoorReg )
{
    aBackdoorReg->eax.dword = BDOOR_MAGIC;
    aBackdoorReg->edx.dword = BDOOR_PORT;

    BOOL result = TRUE;
    __try
    {
        __asm
        {
            mov        edi , aBackdoorReg
            push    edi    //Save address of aBackdoorReg
            
            mov        eax , [edi]
            mov        ebx , [edi + 4]
            mov        ecx , [edi + 8]
            mov        edx , [edi + 12]
            mov        esi , [edi + 16]
            mov        edi , [edi + 20]

            in        eax , dx

            pop        edi    //Get address of aBackdoorReg
            
            mov        [edi] , eax
            mov        [edi + 4] , ebx
            mov        [edi + 8] , ecx
            mov        [edi + 12] , edx
            mov        [edi + 16] , esi
            mov        [edi + 20] , edi
        }
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
        result = FALSE;
    }

    return result;
}

BOOL CVMWare::MagicHBIn( BDOOR_REG_HB * aBackdoorRegHb )
{
    aBackdoorRegHb->eax.dword = BDOOR_MAGIC;
    aBackdoorRegHb->edx.dword = BDOORHB_PORT;
    aBackdoorRegHb->ecx.hiWord = 0xFFFF;    //Make sure ecx does not contain any known VMX type

    BOOL result = TRUE;
    __try
    {
        __asm
        {
            mov        edi , aBackdoorRegHb
            push    edi    //Save address of aBackdoorRegHb
            
            mov        eax , [edi]
            mov        ebx , [edi + 4]
            mov        ecx , [edi + 8]
            mov        edx , [edi + 12]
            mov        esi , [edi + 16]
            mov        ebp , [edi + 24]
            mov        edi , [edi + 20]            

            cld
            rep insb

            pop        edi    //Get address of aBackdoorRegHb
            
            mov        [edi] , eax
            mov        [edi + 4] , ebx
            mov        [edi + 8] , ecx
            mov        [edi + 12] , edx
            mov        [edi + 16] , esi
            mov        [edi + 24] , ebp
            mov        [edi + 20] , edi            
        }
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
        result = FALSE;
    }

    return result;
}

BOOL CVMWare::MagicHBOut( BDOOR_REG_HB * aBackdoorRegHb )
{
    aBackdoorRegHb->eax.dword = BDOOR_MAGIC;
    aBackdoorRegHb->edx.dword = BDOORHB_PORT;
    aBackdoorRegHb->ecx.hiWord = 0xFFFF;    //Make sure ecx does not contain any known VMX type

    BOOL result = TRUE;
    __try
    {
        __asm
        {
            mov        edi , aBackdoorRegHb
            push    edi    //Save address of aBackdoorRegHb
            
            mov        eax , [edi]
            mov        ebx , [edi + 4]
            mov        ecx , [edi + 8]
            mov        edx , [edi + 12]
            mov        esi , [edi + 16]
            mov        ebp , [edi + 24]
            mov        edi , [edi + 20]
            

            cld
            rep outsb

            pop        edi    //Get address of aBackdoorRegHb
            
            mov        [edi] , eax
            mov        [edi + 4] , ebx
            mov        [edi + 8] , ecx
            mov        [edi + 12] , edx
            mov        [edi + 16] , esi
            mov        [edi + 24] , ebp
            mov        [edi + 20] , edi            
        }
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
        result = FALSE;
    }

    return result;
}



#ifdef __cplusplus
    }
#endif

}   //End of namespace CWUtils