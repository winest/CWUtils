#pragma once

/*
 * Copyright (c) 2009-2015, ChienWei Hung <winestwinest@gmail.com>
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

NTSTATUS GetRegFlag( IN CONST VOID * aRoot , IN CONST WCHAR * aKeyPath , IN CONST WCHAR * aFlagName , OUT ULONG32 * aFlagVal );



#ifdef __cplusplus
}
#endif

}   //End of namespace KmUtils
