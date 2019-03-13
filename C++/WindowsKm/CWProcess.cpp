#include "CWProcess.h"
#include <ntstrsafe.h>




#include "_GenerateTmhKm.h"
#include "CWProcess.t.h"

#ifndef CW_MEM_TAG_UTILS
#    define CW_MEM_TAG_UTILS 'tUWC'
#endif

namespace KmUtils
{
#ifdef __cplusplus
extern "C" {
#endif

#define SystemProcessInformation 5
typedef struct _SYSTEM_PROCESS_INFORMATION    // Size=184
{
    ULONG NextEntryOffset;                   // Size=4 Offset=0
    ULONG NumberOfThreads;                   // Size=4 Offset=4
    LARGE_INTEGER WorkingSetPrivateSize;     // Size=8 Offset=8
    ULONG HardFaultCount;                    // Size=4 Offset=16
    ULONG NumberOfThreadsHighWatermark;      // Size=4 Offset=20
    ULONGLONG CycleTime;                     // Size=8 Offset=24
    LARGE_INTEGER CreateTime;                // Size=8 Offset=32
    LARGE_INTEGER UserTime;                  // Size=8 Offset=40
    LARGE_INTEGER KernelTime;                // Size=8 Offset=48
    UNICODE_STRING ImageName;                // Size=8 Offset=56
    LONG BasePriority;                       // Size=4 Offset=64
    PHANDLE UniqueProcessId;                 // Size=4 Offset=68
    PHANDLE InheritedFromUniqueProcessId;    // Size=4 Offset=72
    ULONG HandleCount;                       // Size=4 Offset=76
    ULONG SessionId;                         // Size=4 Offset=80
    ULONG UniqueProcessKey;                  // Size=4 Offset=84
    ULONG PeakVirtualSize;                   // Size=4 Offset=88
    ULONG VirtualSize;                       // Size=4 Offset=92
    ULONG PageFaultCount;                    // Size=4 Offset=96
    ULONG PeakWorkingSetSize;                // Size=4 Offset=100
    ULONG WorkingSetSize;                    // Size=4 Offset=104
    ULONG QuotaPeakPagedPoolUsage;           // Size=4 Offset=108
    ULONG QuotaPagedPoolUsage;               // Size=4 Offset=112
    ULONG QuotaPeakNonPagedPoolUsage;        // Size=4 Offset=116
    ULONG QuotaNonPagedPoolUsage;            // Size=4 Offset=120
    ULONG PagefileUsage;                     // Size=4 Offset=124
    ULONG PeakPagefileUsage;                 // Size=4 Offset=128
    ULONG PrivatePageCount;                  // Size=4 Offset=132
    LARGE_INTEGER ReadOperationCount;        // Size=8 Offset=136
    LARGE_INTEGER WriteOperationCount;       // Size=8 Offset=144
    LARGE_INTEGER OtherOperationCount;       // Size=8 Offset=152
    LARGE_INTEGER ReadTransferCount;         // Size=8 Offset=160
    LARGE_INTEGER WriteTransferCount;        // Size=8 Offset=168
    LARGE_INTEGER OtherTransferCount;        // Size=8 Offset=176
} SYSTEM_PROCESS_INFORMATION, *PSYSTEM_PROCESS_INFORMATION;


typedef NTSTATUS( __stdcall * PFN_ZW_QUERY_SYSTEM_INFORMATION )( IN ULONG aSystemInformationClass,
                                                                 IN OUT PVOID aSystemInformation,
                                                                 IN ULONG aSystemInformationLength,
                                                                 OPTIONAL OUT PULONG aReturnLength );

typedef NTSTATUS( __stdcall * PFN_ZW_QUERY_INFORMATION_PROCESS )( IN HANDLE aProcessHandle,
                                                                  IN PROCESSINFOCLASS aProcessInformationClass,
                                                                  OUT PVOID aProcessInformation,
                                                                  IN ULONG aProcessInformationLength,
                                                                  OPTIONAL OUT PULONG aReturnLength );


NTSTATUS Sleep( ULONG aMilliSeconds )
{
    LARGE_INTEGER liTime;
    liTime.QuadPart = ( -10000 ) * aMilliSeconds;
    return KeDelayExecutionThread( KernelMode, 0, &liTime );
}

NTSTATUS GetProcessPathByPid( HANDLE aPid, PUNICODE_STRING aProcessPath )
{
    NTSTATUS status = STATUS_ACCESS_DENIED;
    HANDLE hProcess = NULL;
    PEPROCESS eProcess = NULL;

    //Get process handle for a given PID
    status = PsLookupProcessByProcessId( aPid, &eProcess );
    if ( !NT_SUCCESS( status ) || NULL == eProcess )
    {
        goto exit;
    }

    status = ObOpenObjectByPointer( eProcess, 0, NULL, 0, 0, KernelMode, &hProcess );
    if ( !NT_SUCCESS( status ) || NULL == hProcess )
    {
        goto exit;
    }

    status = GetProcessPathByHandle( hProcess, aProcessPath );
    if ( !NT_SUCCESS( status ) )
    {
        goto exit;
    }

exit:
    if ( NULL != hProcess )
    {
        ZwClose( hProcess );
    }
    if ( NULL != eProcess )
    {
        ObDereferenceObject( eProcess );
    }
    return status;
}

NTSTATUS GetProcessPathByHandle( HANDLE aProcessHandle, PUNICODE_STRING aProcessPath )
{
    NTSTATUS status = STATUS_ACCESS_DENIED;
    ULONG ulSizeNeeded = 0;
    ULONG ulPathSize = 0;
    PVOID pusPath = NULL;

    UNICODE_STRING funcName;
    RtlUnicodeStringInit( &funcName, L"ZwQueryInformationProcess" );
    PFN_ZW_QUERY_INFORMATION_PROCESS ZwQueryInformationProcess =
        (PFN_ZW_QUERY_INFORMATION_PROCESS)MmGetSystemRoutineAddress( &funcName );
    if ( NULL == ZwQueryInformationProcess )
    {
        status = STATUS_NOT_FOUND;
        goto exit;
    }

    //Get the size we need
    status = ZwQueryInformationProcess( aProcessHandle, ProcessImageFileName, NULL, 0, &ulSizeNeeded );
    if ( STATUS_INFO_LENGTH_MISMATCH != status )
    {
        goto exit;
    }

    //Check the buffer size
    ulPathSize = ulSizeNeeded - sizeof( UNICODE_STRING );
    if ( aProcessPath->MaximumLength < ulPathSize )
    {
        aProcessPath->MaximumLength = (USHORT)ulPathSize;
        status = STATUS_BUFFER_TOO_SMALL;
        goto exit;
    }

    pusPath = ExAllocatePoolWithTag( PagedPool, ulSizeNeeded, CW_MEM_TAG_UTILS );
    if ( NULL == pusPath )
    {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto exit;
    }

    status = ZwQueryInformationProcess( aProcessHandle, ProcessImageFileName, pusPath, ulSizeNeeded, &ulSizeNeeded );
    if ( NT_SUCCESS( status ) )
    {
        RtlUnicodeStringCopy( aProcessPath, (PUNICODE_STRING)pusPath );
    }

exit:
    if ( NULL != pusPath )
    {
        ExFreePoolWithTag( pusPath, CW_MEM_TAG_UTILS );
    }
    return status;
}



NTSTATUS GetPidByProcessName( IN CONST PUNICODE_STRING aImagePath, IN OUT PHANDLE aPid )
{
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    PSYSTEM_PROCESS_INFORMATION pInfo = NULL;
    UNICODE_STRING funcName;
    RtlUnicodeStringInit( &funcName, L"ZwQuerySystemInformation" );
    PFN_ZW_QUERY_SYSTEM_INFORMATION ZwQuerySystemInformation =
        (PFN_ZW_QUERY_SYSTEM_INFORMATION)MmGetSystemRoutineAddress( &funcName );
    if ( NULL == ZwQuerySystemInformation )
    {
        status = STATUS_NOT_FOUND;
        goto exit;
    }

    ULONG ulByteNeed = 0;
    status = ZwQuerySystemInformation( SystemProcessInformation, NULL, 0, &ulByteNeed );
    if ( STATUS_INFO_LENGTH_MISMATCH != status )
    {
        DbgOut( ERRO, DBG_KM_UTILS, "ZwQuerySystemInformation() failed. status=%!STATUS!, ulByteNeed=%lu", status,
                ulByteNeed );
        goto exit;
    }

    for ( ULONG ulRetry = 0; ulRetry < 10; ulRetry++ )
    {
        DbgOut( INFO, DBG_KM_UTILS, "ulByteNeed=%lu", ulByteNeed );
        pInfo = (SYSTEM_PROCESS_INFORMATION *)ExAllocatePoolWithTag( NonPagedPool, ulByteNeed, CW_MEM_TAG_UTILS );

        status = ZwQuerySystemInformation( SystemProcessInformation, pInfo, ulByteNeed, &ulByteNeed );
        if ( NT_SUCCESS( status ) )
        {
            break;
        }
        else
        {
            ExFreePoolWithTag( pInfo, CW_MEM_TAG_UTILS );
            pInfo = (SYSTEM_PROCESS_INFORMATION *)ExAllocatePoolWithTag( NonPagedPool, ulByteNeed, CW_MEM_TAG_UTILS );
        }
    }
    if ( !NT_SUCCESS( status ) )
    {
        DbgOut( ERRO, DBG_KM_UTILS,
                "Failed to allocate buffer for ZwQuerySystemInformation(). status=%!STATUS!, ulByteNeed=%lu", status,
                ulByteNeed );
        goto exit;
    }

    status = STATUS_NOT_FOUND;
    for ( PSYSTEM_PROCESS_INFORMATION pProcInfo = pInfo; 0 < pProcInfo->NextEntryOffset;
          pProcInfo = ( PSYSTEM_PROCESS_INFORMATION )( ( (PUCHAR)pProcInfo ) + pProcInfo->NextEntryOffset ) )
    {
        DbgOut( VERB, DBG_KM_UTILS, "Enum [%04X][%wZ]", (ULONG)pProcInfo->UniqueProcessId, &pProcInfo->ImageName );
        if ( 0 == RtlCompareUnicodeString( aImagePath, &pProcInfo->ImageName, TRUE ) )
        {
            *aPid = pProcInfo->UniqueProcessId;
            status = STATUS_SUCCESS;
            break;
        }
    }

exit:
    if ( NULL != pInfo )
    {
        ExFreePoolWithTag( pInfo, CW_MEM_TAG_UTILS );
    }
    return status;
}


NTSTATUS GetPidAryByProcessName( IN CONST PUNICODE_STRING aImagePath, IN OUT PHANDLE aPidAry, IN OUT PULONG aPidAryCnt )
{
    NT_ASSERT( aPidAryCnt );
    ULONG ulPidAryMaxCnt = *aPidAryCnt;
    *aPidAryCnt = 0;

    NTSTATUS status = STATUS_UNSUCCESSFUL;
    PSYSTEM_PROCESS_INFORMATION pInfo = NULL;
    UNICODE_STRING funcName;
    RtlUnicodeStringInit( &funcName, L"ZwQuerySystemInformation" );
    PFN_ZW_QUERY_SYSTEM_INFORMATION ZwQuerySystemInformation =
        (PFN_ZW_QUERY_SYSTEM_INFORMATION)MmGetSystemRoutineAddress( &funcName );
    if ( NULL == ZwQuerySystemInformation )
    {
        status = STATUS_NOT_FOUND;
        goto exit;
    }

    ULONG ulByteNeed = 0;
    status = ZwQuerySystemInformation( SystemProcessInformation, NULL, 0, &ulByteNeed );
    if ( STATUS_INFO_LENGTH_MISMATCH != status )
    {
        DbgOut( ERRO, DBG_KM_UTILS, "ZwQuerySystemInformation() failed. status=%!STATUS!, ulByteNeed=%lu", status,
                ulByteNeed );
        goto exit;
    }

    for ( ULONG ulRetry = 0; ulRetry < 10; ulRetry++ )
    {
        DbgOut( INFO, DBG_KM_UTILS, "Allocate buffer for SystemProcessInformation. ulByteNeed=%lu", ulByteNeed );
        pInfo = (SYSTEM_PROCESS_INFORMATION *)ExAllocatePoolWithTag( NonPagedPool, ulByteNeed, CW_MEM_TAG_UTILS );

        status = ZwQuerySystemInformation( SystemProcessInformation, pInfo, ulByteNeed, &ulByteNeed );
        if ( NT_SUCCESS( status ) )
        {
            break;
        }
        else
        {
            ExFreePoolWithTag( pInfo, CW_MEM_TAG_UTILS );
            pInfo = (SYSTEM_PROCESS_INFORMATION *)ExAllocatePoolWithTag( NonPagedPool, ulByteNeed, CW_MEM_TAG_UTILS );
        }
    }
    if ( !NT_SUCCESS( status ) )
    {
        DbgOut( ERRO, DBG_KM_UTILS,
                "Failed to allocate buffer for ZwQuerySystemInformation(). status=%!STATUS!, ulByteNeed=%lu", status,
                ulByteNeed );
        goto exit;
    }


    //Traverse the link and compare process base name
    for ( PSYSTEM_PROCESS_INFORMATION pProcInfo = pInfo; 0 < pProcInfo->NextEntryOffset;
          pProcInfo = ( PSYSTEM_PROCESS_INFORMATION )( ( (PUCHAR)pProcInfo ) + pProcInfo->NextEntryOffset ) )
    {
        DbgOut( VERB, DBG_KM_UTILS, "Enum [%04X][%wZ]", (ULONG)pProcInfo->UniqueProcessId, &pProcInfo->ImageName );
        if ( 0 == RtlCompareUnicodeString( aImagePath, &pProcInfo->ImageName, TRUE ) )
        {
            if ( *aPidAryCnt < ulPidAryMaxCnt )
            {
                *aPidAry = pProcInfo->UniqueProcessId;
                aPidAry++;
            }
            ( *aPidAryCnt )++;
        }
    }

    if ( ( *aPidAryCnt ) > ulPidAryMaxCnt )
    {
        DbgOut( WARN, DBG_KM_UTILS, "Too many PIDs. Need %lu but only %lu are available", *aPidAryCnt, ulPidAryMaxCnt );
        status = STATUS_BUFFER_TOO_SMALL;
    }

exit:
    if ( NULL != pInfo )
    {
        ExFreePoolWithTag( pInfo, CW_MEM_TAG_UTILS );
    }
    return status;
}

#ifdef __cplusplus
}
#endif

}    //End of namespace KmUtils
