#pragma once

#ifdef __cplusplus
extern "C" {
#endif


//These defines are missing in evntrace.h in some DDK build environments (XP)
#if !defined(TRACE_LEVEL_NONE)
#define TRACE_LEVEL_NONE        0   // Tracing is not on
#define TRACE_LEVEL_CRITICAL    1   // Abnormal exit or termination
#define TRACE_LEVEL_FATAL       1   // Deprecated name for Abnormal exit or termination
#define TRACE_LEVEL_ERROR       2   // Severe errors that need logging
#define TRACE_LEVEL_WARNING     3   // Warnings such as allocation failure
#define TRACE_LEVEL_INFORMATION 4   // Includes non-error cases(e.g.,Entry-Exit)
#define TRACE_LEVEL_VERBOSE     5   // Detailed traces from intermediate steps
#define TRACE_LEVEL_RESERVED6   6
#define TRACE_LEVEL_RESERVED7   7
#define TRACE_LEVEL_RESERVED8   8
#define TRACE_LEVEL_RESERVED9   9
#endif

//Just a shorter version
#define FATL      TRACE_LEVEL_FATAL
#define ERRO      TRACE_LEVEL_ERROR
#define WARN      TRACE_LEVEL_WARNING
#define INFO      TRACE_LEVEL_INFORMATION
#define VERB      TRACE_LEVEL_VERBOSE
#define NOSY      TRACE_LEVEL_RESERVED6
#define LOUD      TRACE_LEVEL_RESERVED7



//DbgOut is a custom macro that adds support for levels to the
//default DoTraceMessage, which supports only flags. In this version, both
//flags and level are conditions for generating the trace message.
//The preprocessor is told to recognize the function by using the -func argument
//in the RUN_WPP line on the source file. In the source file you will find
//-func:DbgOut(LEVEL,FLAGS,MSG,...)
//
//The conditions for triggering this event in the macro are the Levels defined in evntrace.h
//The flags defined above and are evaluated by the macro WPP_LEVEL_FLAGS_ENABLED below.
#define WPP_LEVEL_FLAGS_LOGGER(level , flags)    WPP_LEVEL_LOGGER(flags)
#define WPP_LEVEL_FLAGS_ENABLED(level , flags)   (WPP_LEVEL_ENABLED(flags) && WPP_CONTROL(WPP_BIT_ ## flags).Level >= level)




//Define the 'WppHexDump' structure for logging buffer and length pairs
//Usage example: DbgOut( VERB , DBG_UTILS , "Data=%!HEXDUMP!" , WppHexDump(pBuf , ulBufSize) );
#pragma warning( push )
#pragma warning( disable : 4204 )  //C4204 non-standard extension used: non-constant aggregate initializer

typedef struct _WppHexDump {
    _WppHexDump( const unsigned char * aBuf , unsigned long aBufSize ) : pBuf(aBuf) , ulBufSize(aBufSize) {}
    const unsigned char * pBuf;
    unsigned long ulBufSize;
} WppHexDump;

#pragma warning (pop)

//#define WPP_LOGPAIR(_Size , _Addr)  (_Addr) , ((SIZE_T)_Size)
//Variable-length arguments require two length/address pairs. As a result, the WPP_LOGHEXDUMP
//macro defines two calls to WPP_LOGPAIR in the following way:
//1. The 1st call to WPP_LOGPAIR passes the size of the variable-length buffer
//2. The 2nd call to WPP_LOGPAIR passes the address of the buffer itself
#define WPP_LOGIPV6(x)      WPP_LOGPAIR(16 , x)
#define WPP_LOGHEXDUMP(x)   WPP_LOGPAIR(2 , &((x).ulBufSize)) WPP_LOGPAIR((x).ulBufSize , (x).pBuf)



//Name of the logger is the WppLogGuid and the GUID is {84B0E5DA-6A9F-4124-B960-EBC6507AFC1B}
#define WPP_CONTROL_GUIDS \
    WPP_DEFINE_CONTROL_GUID( WppLogGuid , (84B0E5DA , 6A9F , 4124 , B960 , EBC6507AFC1B) , \
    WPP_DEFINE_BIT(DBG_TEST)                            \
    WPP_DEFINE_BIT(DBG_DLL_INJECT_MGR)                  \
    WPP_DEFINE_BIT(DBG_DLL_INJECT_MGR_HELPER)           \
    WPP_DEFINE_BIT(DBG_NOTEPAD_HANDLER)                 \
    WPP_DEFINE_BIT(DBG_SKYPE_HANDLER)                   \
    WPP_DEFINE_BIT(DBG_UTILS)                           \
    )

#ifdef __cplusplus
}
#endif