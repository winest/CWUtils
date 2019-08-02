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

#pragma warning( push, 0 )
#include <Windows.h>
#include <string>
#include <list>
#include <map>
#pragma warning( pop )

namespace CWUtils
{
#ifdef __cplusplus
extern "C" {
#endif

BOOL GetIniSectionNames( CONST WCHAR * aIniPath, std::list<std::wstring> & aSectionNames );
BOOL GetIniSectionValues( const WCHAR * aIniPath,
                          const WCHAR * aSectionName,
                          std::map<std::wstring, std::wstring> & aKeyVal );

#ifdef __cplusplus
}
#endif

}    //End of namespace CWUtils