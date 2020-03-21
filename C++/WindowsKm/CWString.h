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

#include <wdm.h>

namespace KmUtils
{
#ifdef __cplusplus
extern "C" {
#endif

#ifndef TO_LOWER
#    define TO_LOWER( WC ) ( ( L'A' <= WC && WC <= L'Z' ) ? ( WC + L'a' - L'A' ) : WC )
#endif

#ifndef TO_UPPER
#    define TO_UPPER( WC ) ( ( L'a' <= WC && WC <= L'z' ) ? ( WC - L'a' + L'A' ) : WC )
#endif

//There is another undocumented function RtlCompareUnicodeStrings( aSrc , aSrcLenInWChar , aDst , aDstLenInWChar , aCaseInSensitive )
LONG UStringCompareUString( CONST PUNICODE_STRING aSrc,
                            CONST PUNICODE_STRING aDst,
                            ULONG aCompareLen,
                            BOOLEAN aCaseSensitive = FALSE );

#ifdef __cplusplus
}
#endif

}    //End of namespace KmUtils
