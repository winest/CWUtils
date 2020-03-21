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

#pragma warning( push, 0 )
#include <Windows.h>
#include <string>
#include <vector>
using std::wstring;
using std::vector;
#pragma warning( pop )

namespace CWUtils
{
#ifdef __cplusplus
extern "C" {
#endif

BOOL IsRegKeyExists( HKEY aRegKey, CONST WCHAR * aSection );

BOOL IsRegValueExists( HKEY aRegKey, CONST WCHAR * aSection, CONST WCHAR * aValueName );

BOOL DeleteRegValue( HKEY aRegKey, CONST WCHAR * aSection, CONST WCHAR * aValueName );

BOOL SetRegString( HKEY aRegKey,
                   CONST WCHAR * aSection,
                   CONST WCHAR * aValueName,
                   CONST WCHAR * aValue,
                   DWORD dwOption = REG_OPTION_NON_VOLATILE );



BOOL SetRegMultiString( HKEY aRegKey,
                        CONST WCHAR * aSection,
                        CONST WCHAR * aValueName,
                        vector<wstring> & aValues,
                        DWORD dwOption = REG_OPTION_NON_VOLATILE );

BOOL GetRegMultiString( HKEY aRegKey, CONST WCHAR * aSection, CONST WCHAR * aValueName, vector<wstring> & aValues );



BOOL SetRegFlag( HKEY aRegKey,
                 CONST WCHAR * aSection,
                 CONST WCHAR * pszFlagName,
                 DWORD dwFlag,
                 DWORD dwOption = REG_OPTION_NON_VOLATILE );
BOOL GetRegFlag( HKEY aRegKey, CONST WCHAR * aSection, CONST WCHAR * pszFlagName, DWORD & dwFlag );

BOOL RemoveRegPendingFileRename( CONST vector<wstring> & aFilenames );

#ifdef __cplusplus
}
#endif

}    //End of namespace CWUtils
