#include "CWFile.h"
#include <ntstrsafe.h>



#ifndef MAX_PATH
    #define MAX_PATH 260
#endif

#ifndef CW_MEM_TAG_UTILS
    #define CW_MEM_TAG_UTILS        'tUWC'
#endif

#include "_GenerateTmhKm.h"
#include "CWFile.t.h"

namespace KmUtils
{

#ifdef __cplusplus
extern "C" {
#endif

NTSTATUS DevicePathToFilePath( IN CONST PUNICODE_STRING aDevicePath , OUT PUNICODE_STRING aFilePath )
{
    NTSTATUS status = STATUS_SUCCESS;    
    WCHAR wzSymbolicName[] = L"\\??\\A:";
    WCHAR wzDriveLetter[] = L"A:";
    UNICODE_STRING  usSymbolicName;
    UNICODE_STRING usDriveLetter;
    UNICODE_STRING  usDeviceName;
    RtlUnicodeStringInit( &usSymbolicName , wzSymbolicName );
    RtlUnicodeStringInit( &usDriveLetter , wzDriveLetter );
    usDeviceName.Length = 0;
    usDeviceName.MaximumLength = MAX_PATH * sizeof(WCHAR);
    usDeviceName.Buffer = (WCHAR *)ExAllocatePoolWithTag( NonPagedPool , usDeviceName.MaximumLength , CW_MEM_TAG_UTILS );    

    for ( WCHAR wLetter = L'A'; wLetter <= L'Z'; wLetter++ ) 
    {
        usSymbolicName.Buffer[4] = wLetter;
        usDriveLetter.Buffer[0] = wLetter;

        HANDLE hSymbolic;
        OBJECT_ATTRIBUTES objDevicePath;
        ULONG ulRetSize;
        InitializeObjectAttributes( &objDevicePath , &usSymbolicName , OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE , NULL , NULL );
        status = ZwOpenSymbolicLinkObject( &hSymbolic , GENERIC_READ , &objDevicePath );
        if( ! NT_SUCCESS( status ) )
        {
            DbgOut( INFO , DBG_KM_UTILS , "ZwOpenSymbolicLinkObject() failed. status=%!STATUS!" , status );
            continue;
        }

        status = ZwQuerySymbolicLinkObject( hSymbolic , &usDeviceName , &ulRetSize );
        DbgOut( INFO , DBG_KM_UTILS , "usDeviceName: ulRetSize=%lu, Length=%hu, Buffer=%wZ" , ulRetSize , usDeviceName.Length , &usDeviceName );

        if ( STATUS_BUFFER_TOO_SMALL == status )
        {
            if ( NULL != usDeviceName.Buffer )
            {
                ExFreePoolWithTag( usDeviceName.Buffer , CW_MEM_TAG_UTILS );
            }
            usDeviceName.Length = 0;
            usDeviceName.MaximumLength = (USHORT)ulRetSize;
            usDeviceName.Buffer = (WCHAR *)ExAllocatePoolWithTag( NonPagedPool , usDeviceName.MaximumLength , CW_MEM_TAG_UTILS );            
            status = ZwQuerySymbolicLinkObject( hSymbolic , &usDeviceName , &ulRetSize );
            DbgOut( INFO , DBG_KM_UTILS , "usDevicePath: Length=%hu, Buffer=%wZ" , usDeviceName.Length , &usDeviceName );
        }

        if ( ! NT_SUCCESS(status) )
        {
            DbgOut( INFO , DBG_KM_UTILS , "ZwQuerySymbolicLinkObject() failed. status=%!STATUS!" , status );
            ZwClose( hSymbolic );
            continue;
        }
        else if ( 0 != _wcsnicmp(usDeviceName.Buffer , aDevicePath->Buffer , min(usDeviceName.Length , aDevicePath->Length)/sizeof(WCHAR) ) )
        {
            ZwClose( hSymbolic );
            continue;
        }
        else
        {
            USHORT uFilePathSize = usDriveLetter.Length + (aDevicePath->Length - usDeviceName.Length);
            if ( aFilePath->MaximumLength < uFilePathSize )
            {
                DbgOut( WARN , DBG_KM_UTILS , "Buffer too small. aFilePath->MaximumLength=%hu, uFilePathSize=%hu" , aFilePath->MaximumLength , uFilePathSize );
                aFilePath->Length = 0;
                RtlZeroMemory( aFilePath->Buffer , aFilePath->MaximumLength );
                aFilePath->MaximumLength = uFilePathSize;
                status = STATUS_BUFFER_TOO_SMALL;
            }            
            
            if ( NT_SUCCESS(status) )
            {
                RtlCopyUnicodeString( aFilePath , &usDriveLetter );
                RtlUnicodeStringCbCatStringN( aFilePath , &aDevicePath->Buffer[usDeviceName.Length/sizeof(WCHAR)] , aDevicePath->Length - usDeviceName.Length );
                DbgOut( INFO , DBG_KM_UTILS , "aFilePath: Length=%hu, MaximumLength=%hu, Buffer=%wZ" , aFilePath->Length , aFilePath->MaximumLength , aFilePath );
            }

            ZwClose( hSymbolic );
            break;
        }
    }

    if ( NULL != usDeviceName.Buffer )
    {
        ExFreePoolWithTag( usDeviceName.Buffer , CW_MEM_TAG_UTILS );
    }

    return status;
}





NTSTATUS FilePathToDevicePath( IN CONST PUNICODE_STRING aFilePath , OUT PUNICODE_STRING aDevicePath )
{
    NT_ASSERT( 4 < aFilePath->Length );

    NTSTATUS status = STATUS_SUCCESS;
    HANDLE hSymbolic = NULL;
    UNICODE_STRING usSymbolicName = {} , usDeviceName = {};
    usSymbolicName.Length = 0;
    usSymbolicName.MaximumLength = aFilePath->MaximumLength + sizeof(L"\\??\\");
    usSymbolicName.Buffer = (WCHAR *)ExAllocatePoolWithTag( NonPagedPool , usSymbolicName.MaximumLength , CW_MEM_TAG_UTILS );
    if ( NULL == usSymbolicName.Buffer )
    {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto exit;
    }
    usDeviceName.Length = 0;
    usDeviceName.MaximumLength = MAX_PATH * sizeof(WCHAR);
    usDeviceName.Buffer = (WCHAR *)ExAllocatePoolWithTag( NonPagedPool , usDeviceName.MaximumLength , CW_MEM_TAG_UTILS );
    if ( NULL == usDeviceName.Buffer )
    {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto exit;
    }

    //Path started with back-slash must be absolute path or UNC path
    if ( L'\\' != aFilePath->Buffer[0] )
    {
        RtlUnicodeStringCopyString( &usSymbolicName , L"\\??\\" );
    }
    USHORT uSymbolicLen = 0;
    for ( uSymbolicLen = 2 ; uSymbolicLen < aFilePath->Length ; uSymbolicLen++ )
    {
        if ( L'\\' == aFilePath->Buffer[uSymbolicLen] )
        {
            break;
        }
    }
    RtlUnicodeStringCbCatN( &usSymbolicName , aFilePath , uSymbolicLen * sizeof(WCHAR) );    
    DbgOut( INFO , DBG_KM_UTILS , "usSymbolicName=%wZ" , &usSymbolicName );

    OBJECT_ATTRIBUTES objDevicePath;
    InitializeObjectAttributes( &objDevicePath , &usSymbolicName , OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE , NULL , NULL );
    status = ZwOpenSymbolicLinkObject( &hSymbolic , GENERIC_READ , &objDevicePath );
    if( ! NT_SUCCESS( status ) )
    {
        DbgOut( INFO , DBG_KM_UTILS , "ZwOpenSymbolicLinkObject() failed. status=%!STATUS!" , status );
        goto exit;
    }

    ULONG ulRetSize;
    status = ZwQuerySymbolicLinkObject( hSymbolic , &usDeviceName , &ulRetSize );
    DbgOut( INFO , DBG_KM_UTILS , "usDeviceName: ulRetSize=%lu, Length=%hu, Buffer=%wZ" , ulRetSize , usDeviceName.Length , &usDeviceName );

    if ( STATUS_BUFFER_TOO_SMALL == status )
    {
        if ( NULL != usDeviceName.Buffer )
        {
            ExFreePoolWithTag( usDeviceName.Buffer , CW_MEM_TAG_UTILS );
        }
        usDeviceName.Length = 0;
        usDeviceName.MaximumLength = (USHORT)ulRetSize;
        usDeviceName.Buffer = (WCHAR *)ExAllocatePoolWithTag( NonPagedPool , usDeviceName.MaximumLength , CW_MEM_TAG_UTILS );            
        status = ZwQuerySymbolicLinkObject( hSymbolic , &usDeviceName , &ulRetSize );
        DbgOut( INFO , DBG_KM_UTILS , "usDevicePath: Length=%hu, Buffer=%wZ" , usDeviceName.Length , &usDeviceName );
    }

    if ( ! NT_SUCCESS(status) )
    {
        DbgOut( INFO , DBG_KM_UTILS , "ZwQuerySymbolicLinkObject() failed. status=%!STATUS!" , status );
        goto exit;
    }

    USHORT uDevicePathSize = usDeviceName.Length + ( aFilePath->Length - (uSymbolicLen*sizeof(WCHAR)) );
    if ( aDevicePath->MaximumLength < uDevicePathSize )
    {
        DbgOut( WARN , DBG_KM_UTILS , "Buffer too small. aDevicePath->MaximumLength=%hu, uDevicePathSize=%hu" , aDevicePath->MaximumLength , uDevicePathSize );
        aDevicePath->Length = 0;
        RtlZeroMemory( aDevicePath->Buffer , aDevicePath->MaximumLength );
        aDevicePath->MaximumLength = uDevicePathSize;
        status = STATUS_BUFFER_TOO_SMALL;
    }

    if ( NT_SUCCESS(status) )
    {
        RtlCopyUnicodeString( aDevicePath , &usDeviceName );
        RtlUnicodeStringCbCatStringN( aDevicePath , &aFilePath->Buffer[uSymbolicLen] , aFilePath->Length - (uSymbolicLen*sizeof(WCHAR)) );
        DbgOut( INFO , DBG_KM_UTILS , "aDevicePath: Length=%hu, Buffer=%wZ" , aDevicePath->Length , aDevicePath );
    }



exit :
    if ( NULL != usSymbolicName.Buffer )
    {
        ExFreePoolWithTag( usSymbolicName.Buffer , CW_MEM_TAG_UTILS );
    }
    if ( NULL != usDeviceName.Buffer )
    {
        ExFreePoolWithTag( usDeviceName.Buffer , CW_MEM_TAG_UTILS );
    }
    if ( NULL != hSymbolic )
    {
        ZwClose( hSymbolic );
    }
    return status;
}


#ifdef __cplusplus
}
#endif

}   //End of namespace KmUtils
