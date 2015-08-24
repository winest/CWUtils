/*
    Author: winest

    Provide the same interface used by VMWare tool to communicate with VMWare.
*/

#pragma once

#include <Windows.h>
#include "CWGeneralUtils.h"
#include "CWVmWareBackdoor.h"

namespace CWUtils
{

#ifdef __cplusplus
extern "C" {
#endif



class CVMWare
{
    public :
        CVMWare();
        ~CVMWare();

    public :
        //Return UTF-8 encoding in bytes excluding terminating NULL
        //Return >0 if something is in clipboard buffer, 0 if clipboard is empty, -1 if failed
        static INT GetClipboardSize();
        static BOOL GetClipboard( TCHAR * aBuf , INT aBufLen );
        static BOOL SetClipboard( TCHAR * aText , INT aTextLen );

        static MSG_CHANNEL * MsgChannelOpen( UINT32 aProtocol );    //0x4F4C4354 for in and RPCI_PROTOCOL_NUM for out
        static BOOL MsgChannelSend( MSG_CHANNEL * aChannel , UINT8 * aCmd , INT aCmdLen );
        static BOOL MsgChannelReceive( MSG_CHANNEL * aChannel , UINT8 * aBuf , INT * aBufLen  );
        static BOOL MsgChannelClose( MSG_CHANNEL * aChannel );

        static INT CastMagic( INT aMagicNum , TCHAR * aParam1 , TCHAR * aParam2 , TCHAR * aOutput );
        static INT TestMagic( INT aMagicNum , TCHAR * aOutput );

    private :
        static BOOL MagicLB( BDOOR_REG_LB * aBackdoorReg );
        static BOOL MagicHBIn( BDOOR_REG_HB * aBackdoorRegHb );
        static BOOL MagicHBOut( BDOOR_REG_HB * aBackdoorRegHb );
};



#ifdef __cplusplus
    }
#endif

}   //End of namespace CWUtils