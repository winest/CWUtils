#pragma once

#include <wdm.h>

namespace KmUtils
{

#ifdef __cplusplus
extern "C" {
#endif

//For path definition, please refer to http://msdn.microsoft.com/en-us/library/windows/desktop/aa365247(v=vs.85).aspx#namespaces
//If the output buffer is not enough, it will return STATUS_BUFFER_TOO_SMALL and set required bytes to output UNICODE_STRING's MaximumLength
NTSTATUS DevicePathToFilePath( IN CONST PUNICODE_STRING aDevicePath , OUT PUNICODE_STRING aFilePath );
NTSTATUS FilePathToDevicePath( IN CONST PUNICODE_STRING aFilePath , OUT PUNICODE_STRING aDevicePath );

#ifdef __cplusplus
}
#endif

}   //End of namespace KmUtils
