#pragma once

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
