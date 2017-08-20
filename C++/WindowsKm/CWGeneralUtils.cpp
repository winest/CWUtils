#include "CWGeneralUtils.h"


VOID * __cdecl operator new( size_t aSize )
{
    return ExAllocatePoolWithTag( NonPagedPool , aSize , CW_MEM_TAG_UTILS );
}
VOID __cdecl operator delete( VOID * aVoid )
{
    ExFreePoolWithTag( aVoid , CW_MEM_TAG_UTILS );
}

namespace KmUtils
{

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_M_AMD64) || defined(_M_IA64)
    #pragma section( ".CRT$XCA" , long , read )
    #pragma section( ".CRT$XCAA" , long , read )
    #pragma section( ".CRT$XCC" , long , read )
    #pragma section( ".CRT$XCZ" , long , read )
    #pragma section( ".CRT$XIA" , long , read )
    #pragma section( ".CRT$XIC" , long , read )
    #pragma section( ".CRT$XIY" , long , read )
    #pragma section( ".CRT$XIZ" , long , read )
    #pragma section( ".CRT$XLA" , long , read )
    #pragma section( ".CRT$XLZ" , long , read )
    #pragma section( ".CRT$XPA" , long , read )
    #pragma section( ".CRT$XPX" , long , read )
    #pragma section( ".CRT$XPZ" , long , read )
    #pragma section( ".CRT$XTA" , long , read )
    #pragma section( ".CRT$XTB" , long , read )
    #pragma section( ".CRT$XTX" , long , read )
    #pragma section( ".CRT$XTZ" , long , read )
    #pragma section( ".rdata$T" , long , read )
    #pragma section( ".rtc$IAA" , long , read )
    #pragma section( ".rtc$IZZ" , long , read )
    #pragma section( ".rtc$TAA" , long , read )
    #pragma section( ".rtc$TZZ" , long , read )

    #define _CRTALLOC(x) __declspec(allocate(x))
#else   //#if defined(_M_AMD64) || defined(_M_IA64)
    #define _CRTALLOC(x)
#endif  //#if defined(_M_AMD64) || defined(_M_IA64)

//
// Define function type used in startup source
//
typedef void (__cdecl *_PVFV)(void);
//
// Do initialization segment declarations
// These are copied from crt0init.c
//
#pragma data_seg(".CRT$XIA")
_CRTALLOC(".CRT$XIA") _PVFV __xi_a[] = { NULL };


#pragma data_seg(".CRT$XIZ")
_CRTALLOC(".CRT$XIZ") _PVFV __xi_z[] = { NULL };


#pragma data_seg(".CRT$XCA")
_CRTALLOC(".CRT$XCA") _PVFV __xc_a[] = { NULL };


#pragma data_seg(".CRT$XCZ")
_CRTALLOC(".CRT$XCZ") _PVFV __xc_z[] = { NULL };


#pragma data_seg(".CRT$XPA")
_CRTALLOC(".CRT$XPA") _PVFV __xp_a[] = { NULL };


#pragma data_seg(".CRT$XPZ")
_CRTALLOC(".CRT$XPZ") _PVFV __xp_z[] = { NULL };


#pragma data_seg(".CRT$XTA")
_CRTALLOC(".CRT$XTA") _PVFV __xt_a[] = { NULL };


#pragma data_seg(".CRT$XTZ")
_CRTALLOC(".CRT$XTZ") _PVFV __xt_z[] = { NULL };

#pragma data_seg()  //reset

#if _MSC_FULL_VER >= 140050214

#pragma comment(linker, "/merge:.CRT=.rdata")

#else  //_MSC_FULL_VER >= 140050214

#if defined (_M_IA64) || defined (_M_AMD64)
#pragma comment(linker, "/merge:.CRT=.rdata")
#else
#pragma comment(linker, "/merge:.CRT=.data")
#endif  //_M_IA64
#endif  //_MSC_FULL_VER >= 140050214
//
// Define memory tag for global objects
//
#define GOBJECT_MEM_TAG         'ITRC' // CRTI, TMComm global object
//
// Structure for PVFV terminator function
//
typedef struct _CALL_ON_EXIT_INFO
{
    SINGLE_LIST_ENTRY   SListEntry;
    _PVFV               pfCallOnExit;
} CALL_ON_EXIT_INFO, *PCALL_ON_EXIT_INFO;
//
// Global LIFO list of terminator functions
//
static SINGLE_LIST_ENTRY gCallOnExitListHead;
// =============================================================================
// Routine:    initterm()
// Description:
//     This routine walks a table of function pointers, calling each entry. It
//     is copied from crt0dat.c
//     
// Arguments:
//     [IN]pfbegin:
//         Pointer to the beginning of the table (first valid entry).
//     [IN]pfend:
//         Pointer to the end of the table (after last valid entry).
// Return Value:
//     None.
// =============================================================================
static void __cdecl initterm (
    _PVFV *pfbegin,
    _PVFV *pfend
    )
{
    
//     walk the table of function pointers from the bottom up, until
//     the end is encountered.  Do not skip the first entry.  The initial
//     value of pfbegin points to the first valid entry.  Do not try to
//     execute what pfend points to.  Only entries before pfend are valid.
    
    while(pfbegin < pfend)
    {
        // if current table entry is non-NULL, call thru it.
        
        if(*pfbegin != NULL)
            (**pfbegin)();
        ++pfbegin;
    }
}
// =============================================================================
// Routine:    CppInit()
// Description:
//     This routine enables C++ global objects support for the NT DDK.
// 
// Arguments:
//     None.
// Return Value:
//     None.
// =============================================================================
VOID CppInit()
{
    initterm(__xc_a, __xc_z);
}
// =============================================================================
// Routine:    atexit()
// Description:
//     This routine allocates storage for the pointer and adds the pointer to 
//     a LIFO list of terminator functions. When the linkage unit in question 
//     terminates, the LIFO list is run, calling through each stored function 
//     pointer in turn.
//     
// Arguments:
//     [IN]pfCallOnExit:
//         Pointer to a PVFV terminator function
// Return Value:
//     If this routine succeeds, it returns zero. Otherwise it returns -1.
// =============================================================================
int __cdecl atexit (
    _PVFV pfCallOnExit
    )
{
    int iRet = -1;
    if(pfCallOnExit)
    {
        //
        // Allocate storage for the PVFV terminator pointer.
        //
        PCALL_ON_EXIT_INFO pCallOnExitInfo = (PCALL_ON_EXIT_INFO) ExAllocatePoolWithTag(NonPagedPool, sizeof(CALL_ON_EXIT_INFO), GOBJECT_MEM_TAG);
        if(pCallOnExitInfo)
        {
            RtlZeroMemory(pCallOnExitInfo, sizeof(CALL_ON_EXIT_INFO));
            pCallOnExitInfo->pfCallOnExit = pfCallOnExit;
            //
            // Add this entry to LIFO list
            //
            PushEntryList(&gCallOnExitListHead, &pCallOnExitInfo->SListEntry);
            iRet = 0;
        }
    }
    return iRet;
}
// =============================================================================
// Routine:    CppDeInit()
// Description:
//     This routine walks the LIFO list, calling through each stored 
//     function pointer in turn, and frees allocated storage for PVFV 
//     terminator function entry.
// 
// Arguments:
//     None.
// Return Value:
//     None.
// =============================================================================
VOID CppUnInit()
{
    PSINGLE_LIST_ENTRY pSListEntry = NULL;
    do 
    {
        //
        // These terminators MUST be executed in reverse order (LIFO)
        //
        pSListEntry = PopEntryList(&gCallOnExitListHead);
        if(pSListEntry)
        {
            //
            // If this entry is non-NULL, call it.
            //
            PCALL_ON_EXIT_INFO pCallOnExitInfo = CONTAINING_RECORD(pSListEntry, CALL_ON_EXIT_INFO, SListEntry);
            if(pCallOnExitInfo->pfCallOnExit)
                pCallOnExitInfo->pfCallOnExit();
            //
            // Free the allocated storage for this entry
            //
            ExFreePool(pCallOnExitInfo);
        }
    }
    while(pSListEntry);
}

#ifdef __cplusplus
}
#endif

}   //End of namespace KmUtils

