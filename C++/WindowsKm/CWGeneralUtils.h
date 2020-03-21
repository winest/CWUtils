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
#include <ntstrsafe.h>



#ifndef MAX_PATH
#    define MAX_PATH 260
#endif

#ifndef _countof
#    define _countof( aArray ) ( sizeof( aArray ) / sizeof( aArray[0] ) )
#endif

#ifndef CW_MEM_TAG_UTILS
#    define CW_MEM_TAG_UTILS 'tUWC'
#endif

#define ATOMIC_READ( X )      InterlockedCompareExchange( reinterpret_cast<volatile LONG *>( &X ), 0, 0 )
#define ATOMIC_ASSIGN( X, Y ) InterlockedExchange( reinterpret_cast<volatile LONG *>( &X ), Y )

#define SHOW_NULL( aString ) ( ( NULL != aString ) ? aString : L"<NULL>" )

#ifdef MY_DRIVER_NAME_A
#    define DEBUG_MSG( aMsg, ... )                                                          \
        do                                                                                  \
        {                                                                                   \
            DbgPrint( "[" MY_DRIVER_NAME_A "][" __FUNCTION__ "] " aMsg "\n", __VA_ARGS__ ); \
        } while ( 0 )
#endif



VOID * __cdecl operator new( size_t aSize );
VOID __cdecl operator delete( VOID * aVoid );


namespace KmUtils
{
#ifdef __cplusplus
extern "C" {
#endif

VOID CppInit();
VOID CppUnInit();

#ifdef __cplusplus
}
#endif

}    //End of namespace KmUtils
