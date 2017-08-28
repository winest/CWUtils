#pragma once

#include <sys/types.h>



namespace CWUtils
{

#ifdef __cplusplus
extern "C" {
#endif



BOOL GetCrc32( CONST UCHAR * aBuf , SIZE_T aBufSize , UINT32 & aCrc32 );



#ifdef __cplusplus
}
#endif

}   //End of namespace CWUtils
