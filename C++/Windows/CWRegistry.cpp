#include "stdafx.h"
#include "CWRegistry.h"
#include <algorithm>

namespace CWUtils
{

#ifdef __cplusplus
extern "C" {
#endif

BOOL IsRegKeyExists( HKEY aRegKey, CONST WCHAR * aSection )
{
    HKEY hKey;
    if ( ERROR_SUCCESS == RegOpenKeyExW( aRegKey , aSection , 0 , KEY_READ , &hKey ) )
    {
        RegCloseKey( hKey );
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

BOOL IsRegValueExists( HKEY aRegKey , CONST WCHAR * aSection , CONST WCHAR * aValueName )
{
    BOOL bReturn = FALSE;
    HKEY hKey = NULL;
    if ( ERROR_SUCCESS != RegOpenKeyExW( aRegKey , aSection, 0, KEY_READ, &hKey) )
        return bReturn;

    if ( ERROR_SUCCESS != RegQueryValueExW( hKey, aValueName, NULL, NULL, NULL, NULL) )
        bReturn = TRUE;
    else
        bReturn = FALSE;

    RegCloseKey( hKey );
    return bReturn;
}

BOOL DeleteRegValue( HKEY aRegKey , CONST WCHAR * aSection , CONST WCHAR * aValueName )
{
    HKEY hKey;

    if ( ERROR_SUCCESS == RegOpenKeyExW( aRegKey , aSection , 0 , KEY_ALL_ACCESS , &hKey ) )
    {
        LONG lResult = RegDeleteValueW( hKey , aValueName );
        RegCloseKey( hKey );
        return ( ERROR_SUCCESS == lResult );
    }
    return FALSE;
}


BOOL SetRegString( HKEY aRegKey , CONST WCHAR * aSection , CONST WCHAR * aValueName , CONST WCHAR * aValue , DWORD dwOption )
{
    HKEY hKey;
    DWORD dw;
    if ( ERROR_SUCCESS == RegCreateKeyExW( aRegKey , aSection , 0 , REG_NONE , dwOption , KEY_WRITE | KEY_READ , NULL , &hKey , &dw ) ) 
    {
        LONG lResult = RegSetValueExW( hKey , aValueName , NULL , REG_SZ ,(LPBYTE)aValue , static_cast<DWORD>(wcslen(aValue)*sizeof(WCHAR) + sizeof(WCHAR)) );
        RegCloseKey( hKey );
        return ( ERROR_SUCCESS == lResult );
    }
    return FALSE;
}



BOOL SetRegMultiString( HKEY aRegKey , CONST WCHAR * aSection , CONST WCHAR * aValueName , vector<wstring> & aValues , DWORD dwOption )
{
    HKEY hKey;
    DWORD dw;
    WCHAR * string_param = NULL;
    const wchar_t * string_buffer = NULL;

    if ( ERROR_SUCCESS == RegCreateKeyExW( aRegKey , aSection , 0 , REG_NONE , dwOption , KEY_WRITE | KEY_READ , NULL , &hKey , &dw ) )
    {
        size_t pos_current = 0 , string_length = 0 , total_length = 0;

        for ( size_t i = 0 ; i < aValues.size() ; i++ )
        {
            total_length += ( aValues[i].length() + 1 );
        }
        total_length++;
        string_param = new (std::nothrow) WCHAR[total_length];

        for (size_t i = 0;i < aValues.size();i++)
        {
            string_buffer = aValues[i].c_str();
            string_length = aValues[i].length() + 1;
            wcsncpy_s( string_param + pos_current , total_length - pos_current , string_buffer , string_length );
            pos_current += string_length;
        }
        // Add the final null termination
        *(string_param + pos_current) = 0;
        LONG result = RegSetValueExW( hKey , aValueName , NULL , REG_MULTI_SZ , (LPBYTE)string_param , static_cast<DWORD>(total_length * sizeof(WCHAR)) );
        delete [] string_param;
        RegCloseKey( hKey );
        return ( ERROR_SUCCESS == result );
    }
    return false;
}

BOOL GetRegMultiString( HKEY aRegKey , CONST WCHAR * aSection , CONST WCHAR * aValueName , vector<wstring> & aValues )
{
    HKEY hKey;
    DWORD reg_value_type = 0, buffer_count = 0;
    WCHAR * string_param = NULL;

    if ( ERROR_SUCCESS == RegOpenKeyExW( aRegKey , aSection , 0 , KEY_READ , &hKey ) )
    {
        LONG result = RegQueryValueExW( hKey , aValueName , NULL , &reg_value_type , NULL , &buffer_count );
        if (ERROR_SUCCESS == result)
        {
            string_param = new (std::nothrow) WCHAR[(buffer_count / sizeof(WCHAR))];
            result = RegQueryValueExW( hKey , aValueName , NULL , &reg_value_type , (LPBYTE)string_param , &buffer_count );

            size_t pos_current = 0 , pos_previous = 0 , total_string_length = (buffer_count / sizeof(WCHAR));
            aValues.clear();

            if ( '\0' == *(string_param + total_string_length - 1) )
            {
                total_string_length--;
            }

            while ( (pos_current = (size_t)(wcschr(string_param + pos_previous, '\0') - string_param)) < total_string_length )
            {
                aValues.push_back( string_param + pos_previous );
                pos_previous = pos_current + 1;
            }
            delete [] string_param;
        }
        RegCloseKey( hKey );

        if ( ERROR_SUCCESS == result )
        {
            return TRUE;
        }
    }
    return FALSE;
}


BOOL SetRegFlag( HKEY aRegKey , CONST WCHAR * aSection , CONST WCHAR * pszFlagName , DWORD dwFlag, DWORD dwOption )
{
    HKEY hKey;
    DWORD dw;
    if ( ERROR_SUCCESS == RegCreateKeyExW( aRegKey , aSection , 0 , REG_NONE , dwOption , KEY_WRITE | KEY_READ , NULL , &hKey , &dw ) ) 
    {
        LONG lResult = RegSetValueExW( hKey , pszFlagName , NULL , REG_DWORD ,(LPBYTE)&dwFlag , sizeof(DWORD) );
        RegCloseKey( hKey );
        return ( ERROR_SUCCESS == lResult );
    }
    return FALSE;
}

BOOL GetRegFlag( HKEY aRegKey , CONST WCHAR * aSection , CONST WCHAR * pszFlagName , DWORD & dwFlag )
{
    HKEY hKey;
    DWORD cbFlag = sizeof( DWORD );

    if ( ERROR_SUCCESS == RegOpenKeyExW( aRegKey , aSection , 0 , KEY_READ , &hKey ) )
    {
        LONG result = RegQueryValueExW( hKey , pszFlagName , NULL , NULL , (LPBYTE)&dwFlag , &cbFlag );
        RegCloseKey( hKey );

        if ( ERROR_SUCCESS == result )
            return TRUE;
    }
    return FALSE;
}




#define REG_KEY_SESSION_MANAGER              L"SYSTEM\\CurrentControlSet\\Control\\Session Manager"
#define REG_VAL_PENDING_FILE_RENAME_OP      L"PendingFileRenameOperations"

BOOL RemoveRegPendingFileRename( CONST vector<wstring> & aFilenames )
{
    BOOL bSuccess = TRUE;
    std::vector<wstring> multi_string;

    if ( TRUE == IsRegKeyExists(HKEY_LOCAL_MACHINE , REG_KEY_SESSION_MANAGER) )
    {
        if ( TRUE == IsRegValueExists(HKEY_LOCAL_MACHINE , REG_KEY_SESSION_MANAGER , REG_VAL_PENDING_FILE_RENAME_OP) )
        {
            if ( TRUE == GetRegMultiString(HKEY_LOCAL_MACHINE , REG_KEY_SESSION_MANAGER , REG_VAL_PENDING_FILE_RENAME_OP, multi_string) )
            {
                BOOL bModified = FALSE;
                size_t found;
                wstring temp_string;

                std::vector<wstring> vecLowerFilenames(aFilenames);
                for ( size_t i = 0 ; i < aFilenames.size() ; i++ )
                {
                    std::transform( aFilenames[i].begin() , aFilenames[i].end() , vecLowerFilenames[i].begin() , ::towlower );
                }
                
                for ( size_t i = 0 ; i < multi_string.size() ; i++ )
                {
                    //wprintf(L"multi string %d: %s\n", i, multi_string[i].c_str());
                    temp_string = multi_string[i];
                    std::transform( temp_string.begin() , temp_string.end() , temp_string.begin() , ::towlower );

                    for ( size_t j = 0 ; j < vecLowerFilenames.size() ; j++ )
                    {
                        found = temp_string.find( vecLowerFilenames[j] );
                        if ( wstring::npos != found )   //Found
                        {
                            bModified = TRUE;
                            //DbgOut(INFO,DBG_SENTRY,"Find pending remove \"%ws\" in \"%ws\"\n",vecLowerFilenames[j].c_str(),multi_string[i].c_str());
                            multi_string.erase(multi_string.begin() + i);
                            break;
                        }
                    }
                }

                if ( bModified )
                {
                    if ( SetRegMultiString(HKEY_LOCAL_MACHINE, REG_KEY_SESSION_MANAGER, REG_VAL_PENDING_FILE_RENAME_OP, multi_string) )
                    {
                        //DbgOut(INFO,DBG_SENTRY,"Success to modify PendingFileRenameOperations to Session Manager registry key");
                    }
                    else
                    {
                        //DbgOut(ERRO,DBG_SENTRY,"Fail to modify PendingFileRenameOperations to Session Manager registry key");
                        bSuccess = FALSE;
                    }
                }
            }
            else
            {
                //DbgOut(ERRO,DBG_SENTRY,"Fail to get PendingFileRenameOperations from Session Manager registry key");
                bSuccess = FALSE;
            }
        }
    }

    return bSuccess;
}

#ifdef __cplusplus
}
#endif

}   //End of namespace CWUtils
