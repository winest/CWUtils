#pragma once

#include <wdm.h>

namespace KmUtils
{

#ifdef __cplusplus
extern "C" {
#endif

#ifndef TO_LOWER
    #define TO_LOWER( WC ) ( (L'A' <= WC && WC <= L'Z') ? (WC+L'a'-L'A') : WC )
#endif

#ifndef TO_UPPER
    #define TO_UPPER( WC ) ( (L'a' <= WC && WC <= L'z') ? (WC-L'a'+L'A') : WC )
#endif

//There is another undocumented function RtlCompareUnicodeStrings( aSrc , aSrcLenInWChar , aDst , aDstLenInWChar , aCaseInSensitive )
LONG UStringCompareUString( CONST PUNICODE_STRING aSrc , CONST PUNICODE_STRING aDst , ULONG aCompareLen , BOOLEAN aCaseSensitive = FALSE );

#ifdef __cplusplus
}
#endif

}   //End of namespace KmUtils
