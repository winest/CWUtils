#pragma once

#include <cstring>
#include <string>

namespace CWUtils
{

#ifdef __cplusplus
    extern "C" {
#endif



VOID Base64Encode( CONST UCHAR * aInput , SIZE_T aInputSize , std::string & aOutput , CONST CHAR * aEncodeTable );
VOID Base64Decode( CONST UCHAR * aInput , SIZE_T aInputSize , std::string & aOutput , CONST CHAR * aDecodeTable );

VOID Rc4( CONST UCHAR * aInput , SIZE_T aInputSize , std::string & aOutput , CONST CHAR * aRc4Key );

#ifdef __cplusplus
    }
#endif

}   //End of namespace CWUtils
