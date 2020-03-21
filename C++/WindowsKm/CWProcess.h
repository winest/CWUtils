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

#include <ntifs.h>

namespace KmUtils
{
#ifdef __cplusplus
extern "C" {
#endif

NTSTATUS Sleep( ULONG aMilliSeconds );
NTSTATUS GetProcessPathByPid( IN HANDLE aPid, IN OUT PUNICODE_STRING aProcessPath );
NTSTATUS GetProcessPathByHandle( IN HANDLE aProcessHandle, IN OUT PUNICODE_STRING aProcessPath );


NTSTATUS GetPidByProcessName( IN CONST PUNICODE_STRING aImagePath, IN OUT PHANDLE aPid );

//If aPidAry is NULL, aPidAryCnt will get the existing matching process count and return code is STATUS_INSUFFICIENT_BUFFER
//If aPidAry is not NULL, aPidAryCnt will be the PID count copied to aPidAry. Return code will be STATUS_SUCCESS or STATUS_INSUFFICIENT_BUFFER depends on
//whether aPidAry can hold all matching process
NTSTATUS GetPidAryByProcessName( IN CONST PUNICODE_STRING aImagePath,
                                 IN OUT PHANDLE aPidAry,
                                 IN OUT PULONG aPidAryCnt );

#ifdef __cplusplus
}
#endif

}    //End of namespace KmUtils
