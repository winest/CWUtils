#include "CWRegistry.h"
#include <ntstrsafe.h>


#ifndef MAX_PATH
    #define MAX_PATH 260
#endif

#ifndef _countof
    #define _countof( aArray ) ( sizeof(aArray) / sizeof(aArray[0]) )
#endif

#ifndef CW_MEM_TAG_UTILS
    #define CW_MEM_TAG_UTILS        'tUWC'
#endif

#define HKEY_LOCAL_MACHINE                  ((VOID *) (ULONG_PTR)((LONG)0x80000002) )
#define HKEY_USERS                          ((VOID *) (ULONG_PTR)((LONG)0x80000002) )

#include "_GenerateTmhKm.h"
#include "CWRegistry.t.h"

namespace KmUtils
{

#ifdef __cplusplus
extern "C" {
#endif

NTSTATUS _GetRegRaw( IN CONST WCHAR * aKeyPath , IN CONST WCHAR * aValueName , IN ULONG aType , OUT VOID * aBuf , IN OUT ULONG * aBufSize )
{
    NT_ASSERT( aBufSize );
    DbgOut( VERB , DBG_KM_UTILS , "Enter. aKeyPath=%ws, aValueName=%ws, aType=%lu, aBuf=0x%p, aBufSize=%lu" , 
            aKeyPath , aValueName , aType , aBuf , *aBufSize );
    
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    HANDLE hKey = NULL;
    UNICODE_STRING usKeyPath , usValueName;
    RtlUnicodeStringInit( &usKeyPath , aKeyPath );
    RtlUnicodeStringInit( &usValueName , aValueName );

    OBJECT_ATTRIBUTES objRegKey;
    InitializeObjectAttributes( &objRegKey , &usKeyPath , OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE , NULL , NULL );

    status = ZwOpenKey( &hKey , KEY_QUERY_VALUE , &objRegKey );
    if ( NT_SUCCESS(status) )
    {
        ULONG ulRetSize = 0;
        CHAR szTmpBuf[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + MAX_PATH];
        KEY_VALUE_PARTIAL_INFORMATION * pResult = (KEY_VALUE_PARTIAL_INFORMATION *)szTmpBuf;        

        status = ZwQueryValueKey( hKey , &usValueName , KeyValuePartialInformation , pResult , sizeof(szTmpBuf) , &ulRetSize );
        if ( NT_SUCCESS(status) )
        {
            DbgOut( INFO , DBG_KM_UTILS , "ZwQueryValueKey() succeed. TitleIndex=%lu, Type=%lu, Data=0x%p, DataLength=%lu" , 
                    pResult->TitleIndex , pResult->Type , pResult->Data , pResult->DataLength );

            if ( aType != pResult->Type )
            {
                DbgOut( ERRO , DBG_KM_UTILS , "Type mismatched. aType=%lu, pResult->Type=%lu" , aType , pResult->Type );
                *aBufSize = 0;
                status = STATUS_OBJECT_TYPE_MISMATCH;
            }
            else if ( aBuf && *aBufSize >= pResult->DataLength )
            {
                *aBufSize = pResult->DataLength;
                RtlCopyMemory( aBuf , pResult->Data , pResult->DataLength );
                status = STATUS_SUCCESS;
            }
            else
            {
                *aBufSize = pResult->DataLength;
                status = STATUS_BUFFER_TOO_SMALL;
            }
        }
        else
        {
            DbgOut( ERRO , DBG_KM_UTILS , "ZwQueryValueKey() failed. usValueName=%wZ, status=%!STATUS!" , &usValueName , status );
        }
        ZwClose( hKey );
    }

    DbgOut( VERB , DBG_KM_UTILS , "Leave. status=%!STATUS!" , status );
    return status;
}

NTSTATUS GetRegFlag( IN CONST VOID * aRoot , IN CONST WCHAR * aKeyPath , IN CONST WCHAR * aFlagName , OUT ULONG32 * aFlagVal )
{
    NTSTATUS status = STATUS_INVALID_PARAMETER;
    ULONG32 u32Flag = 0;
    ULONG ulFlagSize = sizeof(u32Flag);
    
    if ( NULL == aRoot )
    {
        status = _GetRegRaw( aKeyPath , aFlagName , REG_DWORD , &u32Flag , &ulFlagSize );
    }
    else
    {
        struct _Root_Map
        {
            CONST VOID * hRoot;
            CONST WCHAR * wzRootReg;
        } rootMap[] = { {HKEY_LOCAL_MACHINE , L"\\Registry\\Machine\\"} , 
                        {HKEY_USERS , L"\\Registry\\User\\"} };

        for ( size_t i = 0 ; i < _countof(rootMap) ; i++ )
        {
            if ( rootMap[i].hRoot == aRoot )
            {
                UNICODE_STRING usFullKeyPath = {};
                usFullKeyPath.MaximumLength = (USHORT)(( wcslen(rootMap[i].wzRootReg) + wcslen(aKeyPath) + 1 ) * sizeof(WCHAR));
                usFullKeyPath.Buffer = (WCHAR *)ExAllocatePoolWithTag( NonPagedPool , usFullKeyPath.MaximumLength , CW_MEM_TAG_UTILS );
                if ( NULL == usFullKeyPath.Buffer )
                {
                    DbgOut( ERRO , DBG_KM_UTILS , "ExAllocatePoolWithTag() failed. usFullKeyPath.MaximumLength=%hu" , usFullKeyPath.MaximumLength );
                    break;
                }
                RtlZeroMemory( usFullKeyPath.Buffer , usFullKeyPath.MaximumLength );
                RtlUnicodeStringCopyString( &usFullKeyPath , rootMap[i].wzRootReg );
                RtlUnicodeStringCatString( &usFullKeyPath , aKeyPath );
                DbgOut( VERB , DBG_KM_UTILS , "usFullKeyPath.MaximumLength=%hu, usFullKeyPath=%wZ, aKeyPath=%ws" , 
                        usFullKeyPath.MaximumLength , &usFullKeyPath , aKeyPath );

                status = _GetRegRaw( usFullKeyPath.Buffer , aFlagName , REG_DWORD , &u32Flag , &ulFlagSize );
                ExFreePoolWithTag( usFullKeyPath.Buffer , CW_MEM_TAG_UTILS );
                break;
            }
        }        
    }

    *aFlagVal = u32Flag;
    return status;

}




#ifdef __cplusplus
}
#endif

}   //End of namespace KmUtils
