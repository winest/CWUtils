#pragma once

#include "Windows.h"
#include <string>
#include <vector>
using std::wstring;
using std::vector;

namespace CWUtils
{

#ifdef __cplusplus
extern "C" {
#endif

BOOL IsRegKeyExists( HKEY aRegKey, CONST WCHAR * aSection );

BOOL IsRegValueExists( HKEY aRegKey , CONST WCHAR * aSection , CONST WCHAR * aValueName );

BOOL DeleteRegValue( HKEY aRegKey , CONST WCHAR * aSection , CONST WCHAR * aValueName );

BOOL SetRegString( HKEY aRegKey , CONST WCHAR * aSection , CONST WCHAR * aValueName , CONST WCHAR * aValue , DWORD dwOption = REG_OPTION_NON_VOLATILE );



BOOL SetRegMultiString( HKEY aRegKey , CONST WCHAR * aSection , CONST WCHAR * aValueName , vector<wstring> & aValues , DWORD dwOption = REG_OPTION_NON_VOLATILE );

BOOL GetRegMultiString( HKEY aRegKey , CONST WCHAR * aSection , CONST WCHAR * aValueName , vector<wstring> & aValues );



BOOL SetRegFlag( HKEY aRegKey , CONST WCHAR * aSection , CONST WCHAR * pszFlagName , DWORD dwFlag, DWORD dwOption = REG_OPTION_NON_VOLATILE );
BOOL GetRegFlag( HKEY aRegKey , CONST WCHAR * aSection , CONST WCHAR * pszFlagName , DWORD& dwFlag );

BOOL RemoveRegPendingFileRename( CONST vector<wstring> & aFilenames );

#ifdef __cplusplus
}
#endif

}   //End of namespace CWUtils
