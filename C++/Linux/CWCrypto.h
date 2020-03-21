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

#include <cstring>
#include <string>

namespace CWUtils
{
#ifdef __cplusplus
extern "C" {
#endif



VOID Base64Encode( CONST UCHAR * aInput, SIZE_T aInputSize, std::string & aOutput, CONST CHAR * aEncodeTable );
VOID Base64Decode( CONST UCHAR * aInput, SIZE_T aInputSize, std::string & aOutput, CONST CHAR * aDecodeTable );

VOID Rc4( CONST UCHAR * aInput, SIZE_T aInputSize, std::string & aOutput, CONST CHAR * aRc4Key );

#ifdef __cplusplus
}
#endif

}    //End of namespace CWUtils
