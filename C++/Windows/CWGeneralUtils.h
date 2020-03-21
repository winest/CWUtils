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

#include <Windows.h>

namespace CWUtils
{
#ifdef __cplusplus
extern "C" {
#endif

#define LIKELY
#define UNLIKELY
#define ATOMIC_READ( X )      InterlockedCompareExchange( reinterpret_cast<volatile LONG *>( &X ), 0, 0 )
#define ATOMIC_ASSIGN( X, Y ) InterlockedExchange( reinterpret_cast<volatile LONG *>( &X ), Y )
#define ATOMIC_INC( X )       InterlockedIncrement( reinterpret_cast<volatile LONG *>( &X ) )
#define ATOMIC_DEC( X )       InterlockedDecrement( reinterpret_cast<volatile LONG *>( &X ) )
//#define __VA_ARGS_NUM__ (...)       ( ( sizeof((size_t[]){__VA_ARGS__}) / sizeof(size_t) ) - 1 )
#define NUM_TO_TEXTA( aNum ) ( #aNum )
#define NUM_TO_TEXTW( aNum ) ( L#aNum )
#define NUM_TEXT_PAIRA( aNum ) \
    {                          \
        aNum, #aNum            \
    }
#define NUM_TEXT_PAIRW( aNum ) \
    {                          \
        aNum, L#aNum           \
    }
#define NULL_STRA( aPtr ) ( LIKELY( aPtr != nullptr ) ? aPtr : "<NULL>" )
#define NULL_STRW( aPtr ) ( LIKELY( aPtr != nullptr ) ? aPtr : L"<NULL>" )
#if ( defined UNICODE || defined _UNICODE )
#    define NUM_TO_TEXT NUM_TO_TEXTW
#    define NULL_STR    NULL_STRW
#else
#    define NUM_TO_TEXT NUM_TO_TEXTA
#    define NULL_STR    NULL_STRA
#endif

#ifndef CP_BIG5
#    define CP_BIG5 950
#endif

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
__inline HINSTANCE GetModuleHInstance()
{
    return (HINSTANCE)&__ImageBase;
}

#define DBG_LOG_PATH L"C:\\temp\\DebugMsg.txt"
VOID ShowDebugMsg( CONST WCHAR * aReason = L"Reason", BOOL aShowInRelease = FALSE );
VOID WriteDebugMsg( CONST IN CHAR * aFormat, ... );

UINT BitRange( UINT aNum, INT aIndexStart, INT aIndexEnd );


#ifdef __cplusplus
}
#endif

}    //End of namespace CWUtils
