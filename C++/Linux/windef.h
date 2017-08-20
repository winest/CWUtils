#ifndef __WINDEF_H__
#define __WINDEF_H__

#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif



#ifndef WINVER
    #define WINVER 0x0500
#endif

#ifdef __cplusplus
    #define EXTERN_C    extern "C"
#else
    #define EXTERN_C    extern
#endif

#define IN
#define OUT
#define OPTIONAL
#define CONST               const
#define CDECL               
#define CALLBACK            
#define WINAPI              
#define WINAPIV             
#define APIENTRY            WINAPI
#define APIPRIVATE          
#define PASCAL              
#define NTAPI              
#define UNREFERENCED_PARAMETER(P) (P) 

//Common types
typedef void                VOID;
typedef VOID                *PVOID, *LPVOID;
typedef CONST VOID          *LPCVOID;
typedef void                *HANDLE;
typedef HANDLE              *PHANDLE; 

typedef char CHAR;
typedef CHAR                *PCHAR, *LPCH, *PCH, *PNZCH;
typedef CONST CHAR          *LPCCH, *PCCH, *PCNZCH;
typedef unsigned char       UCHAR;
typedef UCHAR               *PUCHAR;
typedef wchar_t             WCHAR;
typedef WCHAR               *PWCHAR, *LPWCH, *PWCH, *PNZWCH;
typedef CONST WCHAR         *LPCWCH, *PCWCH, *PCNZWCH, *LPCWCHAR, *PCWCHAR;

typedef short               SHORT;
typedef unsigned short      USHORT;
typedef USHORT              *PUSHORT;
typedef long                LONG, LONG_PTR;
typedef LONG                *PLONG_PTR;
typedef LONG                *PLONG, *LPLONG;
typedef unsigned long       ULONG, ULONG_PTR;
typedef ULONG               *PULONG, *PULONG_PTR;
typedef int                 INT, INT_PTR;
typedef INT                 *PINT_PTR;
typedef INT                 *PINT, *LPINT;
typedef unsigned int        UINT, UINT_PTR;
typedef UINT                *PUINT, *PUINT_PTR;

typedef LONG_PTR            SSIZE_T, *PSSIZE_T;
typedef ULONG_PTR           SIZE_T, *PSIZE_T;

typedef float               FLOAT;
typedef FLOAT               *PFLOAT;
typedef double              DOUBLE;
typedef DOUBLE              *PDOUBLE;

//Need to check on different platforms
typedef signed char         INT8, *PINT8;
typedef unsigned char       UINT8, *PUINT8;
typedef signed short        INT16, *PINT16;
typedef unsigned short      UINT16, *PUINT16;
typedef int32_t             INT32, *PINT32;
typedef uint32_t            UINT32, *PUINT32;
typedef int64_t             INT64, *PINT64;
typedef uint64_t            UINT64, *PUINT64;
typedef int32_t             LONG32, *PLONG32;
typedef uint32_t            ULONG32, *PULONG32;
typedef uint32_t            DWORD32, *PDWORD32;
typedef int64_t             LONG64, *PLONG64;
typedef uint64_t            ULONG64, *PULONG64;
typedef uint64_t            DWORD64, *PDWORD64;
typedef int64_t             LONGLONG;
typedef uint64_t            ULONGLONG;

typedef unsigned char       BYTE;
typedef BYTE                *PBYTE, *LPBYTE;
typedef unsigned short      WORD;
typedef WORD                *PWORD, *LPWORD;
typedef unsigned long       DWORD;
typedef DWORD               *PDWORD, *LPDWORD;
typedef int                 BOOL;
typedef BOOL                *PBOOL, *LPBOOL;
typedef UINT_PTR            WPARAM;
typedef LONG_PTR            LPARAM;
typedef LONG_PTR            LRESULT;




//Constants
#define FALSE                           0
#define TRUE                            1
#ifndef NULL
    #define NULL                        0
#endif
#ifndef _TRUNCATE
    #define _TRUNCATE                   ((size_t)-1)
#endif

#ifndef MAX_PATH
    #define MAX_PATH                    260
#endif
#define INTERNET_SCHEME_HTTP            (1)
#define INTERNET_SCHEME_HTTPS           (2)
#define INTERNET_SCHEME_FTP             (3)
#define INTERNET_SCHEME_SOCKS           (4)
#define INTERNET_MAX_HOST_NAME_LENGTH   256
#define INTERNET_MAX_USER_NAME_LENGTH   128
#define INTERNET_MAX_PASSWORD_LENGTH    128
#define INTERNET_MAX_PORT_NUMBER_LENGTH 5           // INTERNET_PORT is unsigned short
#define INTERNET_MAX_PORT_NUMBER_VALUE  65535       // maximum unsigned short value
#define INTERNET_MAX_PATH_LENGTH        2048
#define INTERNET_MAX_SCHEME_LENGTH      32          // longest protocol name length
#define INTERNET_MAX_URL_LENGTH         (INTERNET_MAX_SCHEME_LENGTH + sizeof("://") + INTERNET_MAX_PATH_LENGTH)
#define MAXUINT8            ((UINT8)~((UINT8)0))
#define MAXINT8             ((INT8)(MAXUINT8 >> 1))
#define MININT8             ((INT8)~MAXINT8)
#define MAXUINT16           ((UINT16)~((UINT16)0))
#define MAXINT16            ((INT16)(MAXUINT16 >> 1))
#define MININT16            ((INT16)~MAXINT16)
#define MAXUINT32           ((UINT32)~((UINT32)0))
#define MAXINT32            ((INT32)(MAXUINT32 >> 1))
#define MININT32            ((INT32)~MAXINT32)
#define MAXUINT64           ((UINT64)~((UINT64)0))
#define MAXINT64            ((INT64)(MAXUINT64 >> 1))
#define MININT64            ((INT64)~MAXINT64)
#define MAXULONG32          ((ULONG32)~((ULONG32)0))
#define MAXLONG32           ((LONG32)(MAXULONG32 >> 1))
#define MINLONG32           ((LONG32)~MAXLONG32)
#define MAXULONG64          ((ULONG64)~((ULONG64)0))
#define MAXLONG64           ((LONG64)(MAXULONG64 >> 1))
#define MINLONG64           ((LONG64)~MAXLONG64)
#define MAXLONGLONG         (0x7fffffffffffffff)
#define MAXULONGLONG        ((ULONGLONG)~((ULONGLONG)0))
#define MINLONGLONG         ((LONGLONG)~MAXLONGLONG)
#define MAXSIZE_T           ((SIZE_T)~((SIZE_T)0))
#define MAXSSIZE_T          ((SSIZE_T)(MAXSIZE_T >> 1))
#define MINSSIZE_T          ((SSIZE_T)~MAXSSIZE_T)
#define MAXUINT             ((UINT)~((UINT)0))
#define MAXINT              ((INT)(MAXUINT >> 1))
#define MININT              ((INT)~MAXINT)
#define MAXDWORD32          ((DWORD32)~((DWORD32)0))
#define MAXDWORD64          ((DWORD64)~((DWORD64)0))
#define MINCHAR             0x80
#define MAXCHAR             0x7f
#define MINSHORT            0x8000
#define MAXSHORT            0x7fff
#define MINLONG             0x80000000
#define MAXLONG             0x7fffffff
#define MAXBYTE             0xff
#define MAXWORD             0xffff
#define MAXDWORD            0xffffffff
#define INFINITE            0xffffffff



#define DECLARE_HANDLE(name) struct name##__{int unused;}; typedef struct name##__ *name
DECLARE_HANDLE( HWND );
DECLARE_HANDLE( HHOOK );
DECLARE_HANDLE( HEVENT );
DECLARE_HANDLE( HGDIOBJ );
DECLARE_HANDLE( HACCEL );
DECLARE_HANDLE( HBITMAP );
DECLARE_HANDLE( HBRUSH );
DECLARE_HANDLE( HCOLORSPACE );
DECLARE_HANDLE( HDC );
DECLARE_HANDLE( HGLRC );          // OpenGL
DECLARE_HANDLE( HDESK );
DECLARE_HANDLE( HENHMETAFILE );
DECLARE_HANDLE( HFONT );
DECLARE_HANDLE( HICON );
DECLARE_HANDLE( HMENU );
DECLARE_HANDLE( HPALETTE );
DECLARE_HANDLE( HPEN );
DECLARE_HANDLE( HWINEVENTHOOK );
DECLARE_HANDLE( HMONITOR );
DECLARE_HANDLE( HUMPD );
DECLARE_HANDLE( HCURSOR );    /* HICONs & HCURSORs are not polymorphic */
DECLARE_HANDLE( HKEY );
typedef HKEY *PHKEY;
DECLARE_HANDLE( HMETAFILE );
DECLARE_HANDLE( HINSTANCE );
typedef HINSTANCE HMODULE;      /* HMODULEs can be used in place of HINSTANCEs */
DECLARE_HANDLE( HRGN );
DECLARE_HANDLE( HRSRC );
DECLARE_HANDLE( HSPRITE );
DECLARE_HANDLE( HLSURF );
DECLARE_HANDLE( HSTR );
DECLARE_HANDLE( HTASK );
DECLARE_HANDLE( HWINSTA );
DECLARE_HANDLE( HKL );




//Structures
typedef union _LARGE_INTEGER {
    struct {
        DWORD LowPart;
        LONG HighPart;
    } DUMMYSTRUCTNAME;
    struct {
        DWORD LowPart;
        LONG HighPart;
    } u;
    LONGLONG QuadPart;
} LARGE_INTEGER;
typedef LARGE_INTEGER *PLARGE_INTEGER;

typedef union _ULARGE_INTEGER {
    struct {
        DWORD LowPart;
        DWORD HighPart;
    } DUMMYSTRUCTNAME;
    struct {
        DWORD LowPart;
        DWORD HighPart;
    } u;
    ULONGLONG QuadPart;
} ULARGE_INTEGER;
typedef ULARGE_INTEGER *PULARGE_INTEGER;



typedef struct _FILETIME {
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
} FILETIME, *PFILETIME, *LPFILETIME;



typedef struct tagRECT
{
        LONG    left;
        LONG    top;
        LONG    right;
        LONG    bottom;
} RECT, * PRECT, * NPRECT, * LPRECT;

typedef const RECT * LPCRECT;

typedef struct _RECTL       /* rcl */
{
        LONG    left;
        LONG    top;
        LONG    right;
        LONG    bottom;
} RECTL, * PRECTL, * LPRECTL;

typedef const RECTL * LPCRECTL;

typedef struct tagPOINT
{
        LONG  x;
        LONG  y;
} POINT, * PPOINT, * NPPOINT, * LPPOINT;

typedef struct _POINTL      /* ptl  */
{
        LONG  x;
        LONG  y;
} POINTL, * PPOINTL;

typedef struct tagSIZE
{
        LONG        cx;
        LONG        cy;
} SIZE, * PSIZE, * LPSIZE;

typedef SIZE               SIZEL;
typedef SIZE               * PSIZEL, * LPSIZEL;

typedef struct tagPOINTS
{
        SHORT   x;
        SHORT   y;
} POINTS, * PPOINTS, * LPPOINTS;



//Macro functions
#ifndef max
    #define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
    #define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#define MAKEWORD(a, b)                  ((WORD)(((BYTE)(((DWORD_PTR)(a)) & 0xff)) | ((WORD)((BYTE)(((DWORD_PTR)(b)) & 0xff))) << 8))
#define MAKELONG(a, b)                  ((LONG)(((WORD)(((DWORD_PTR)(a)) & 0xffff)) | ((DWORD)((WORD)(((DWORD_PTR)(b)) & 0xffff))) << 16))
#define LOWORD(l)                       ((WORD)(((DWORD_PTR)(l)) & 0xffff))
#define HIWORD(l)                       ((WORD)((((DWORD_PTR)(l)) >> 16) & 0xffff))
#define LOBYTE(w)                       ((BYTE)(((DWORD_PTR)(w)) & 0xff))
#define HIBYTE(w)                       ((BYTE)((((DWORD_PTR)(w)) >> 8) & 0xff))
#define FIELD_OFFSET(type, field)       ((LONG)(LONG_PTR)&(((type *)0)->field))
#define RTL_FIELD_SIZE(type, field)     (sizeof(((type *)0)->field))
#define RTL_SIZEOF_THROUGH_FIELD(type, field) \
    (FIELD_OFFSET(type, field) + RTL_FIELD_SIZE(type, field))
#define RTL_CONTAINS_FIELD(Struct, Size, Field) \
    ( (((PCHAR)(&(Struct)->Field)) + sizeof((Struct)->Field)) <= (((PCHAR)(Struct))+(Size)) )
#define RTL_NUMBER_OF_V1(A)             (sizeof(A)/sizeof((A)[0]))
#define RTL_NUMBER_OF_V2(A)             (sizeof(*RtlpNumberOf(A)))
#define RTL_NUMBER_OF(A)                RTL_NUMBER_OF_V1(A)
#define ARRAYSIZE(A)                    RTL_NUMBER_OF_V2(A)
#define _ARRAYSIZE(A)                   RTL_NUMBER_OF_V1(A)
#define _countof(A)                     RTL_NUMBER_OF_V1(A)
#define RTL_FIELD_TYPE(type, field) (((type*)0)->field)
#define RTL_NUMBER_OF_FIELD(type, field) (RTL_NUMBER_OF(RTL_FIELD_TYPE(type, field)))
#define CONTAINING_RECORD(address, type, field) ((type *)( \
                                                  (PCHAR)(address) - \
                                                  (ULONG_PTR)(&((type *)0)->field)))



#ifdef __cplusplus
}
#endif

#endif
