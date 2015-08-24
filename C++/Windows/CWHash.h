#pragma once

#include <Windows.h>
#include <Wincrypt.h>
#include <string>

namespace CWUtils
{

#ifdef __cplusplus
    extern "C" {
#endif

BOOL GetFileMd5( IN CONST WCHAR * aFilePath , OUT std::string & aMd5 );

//Refer to https://code.google.com/p/smhasher/w/list
VOID MurmurHash2_x64_64( CONST VOID * aMem , size_t aMemSize , UINT64 aSeed , UINT64 * a64BitsOutput );
VOID MurmurHash3_x86_32( CONST VOID * aMem , size_t aMemSize , UINT32 aSeed , UINT32 * a32BitsOutput );
VOID MurmurHash3_x86_128( CONST VOID * aMem , size_t aMemSize , UINT32 aSeed , VOID * a128BitsOutput );
VOID MurmurHash3_x64_128( CONST VOID * aMem , size_t aMemSize , UINT64 aSeed , VOID * a128BitsOutput );

#ifdef __cplusplus
    }
#endif

}   //End of namespace CWUtils
