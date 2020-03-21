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

//For path definition, please refer to http://msdn.microsoft.com/en-us/library/windows/desktop/aa365247(v=vs.85).aspx#namespaces
//If the output buffer is not enough, it will return STATUS_BUFFER_TOO_SMALL and set required bytes to output UNICODE_STRING's MaximumLength
NTSTATUS DevicePathToFilePath( IN CONST PUNICODE_STRING aDevicePath, OUT PUNICODE_STRING aFilePath );
NTSTATUS FilePathToDevicePath( IN CONST PUNICODE_STRING aFilePath, OUT PUNICODE_STRING aDevicePath );

#ifdef __cplusplus
}
#endif

}    //End of namespace KmUtils
