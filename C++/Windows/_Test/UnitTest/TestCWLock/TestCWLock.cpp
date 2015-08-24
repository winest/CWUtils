#include "stdafx.h"
#include <Windows.h>
#include <Process.h>
#include <string>
#include <list>
using namespace std;

#include "CWTime.h"
#include "CWLock.h"
#include "CWString.h"

#include "_GenerateTmh.h"
#include "TestCWLock.tmh"


#define PRODUCER_COUNT   2
#define PRODUCER_WAIT_TIME 1000
#define CONSUMER_COUNT   64
#define CONSUMER_WAIT_TIME 10


#define RAND_INT( aMin , aMax ) ( aMin + ( rand() % (aMax-aMin+1) ) )

ULONG g_ulProducerCount = 0;
ULONG g_ulConsumerCount = 0;

#define RWLOCK
#ifndef RWLOCK
    CWUtils::CCriticalSection g_lock;
#else
    CWUtils::CRWLockSlim g_lock;
#endif
typedef struct _DATA
{
    wstring wstrData;
} DATA;
list<DATA> g_lsData;
volatile ULARGE_INTEGER g_liCount;


typedef struct _PRODUCER_ARGS
{
    HANDLE hEventStop;
} PRODUCER_ARGS;

typedef struct _CONSUMER_ARGS
{
    HANDLE hEventStop;
} CONSUMER_ARGS;

UINT CALLBACK ProducerThread( VOID * aArgs );
UINT CALLBACK ConsumerThread( VOID * aArgs );

VOID TestRWLock()
{
    wprintf_s( L"\n========== TestRWLock() Enter ==========\n" );

    //Prepare thread arguments
    PRODUCER_ARGS * producerArgs = new PRODUCER_ARGS[PRODUCER_COUNT];
    CONSUMER_ARGS * consumerArgs = new CONSUMER_ARGS[CONSUMER_COUNT];
    HANDLE hEventStop = CreateEventW( NULL , TRUE , FALSE , NULL );
    if ( ! hEventStop )
    {
        wprintf_s( L"Failed to create stop event. GetLastError()=%I32u" , GetLastError() );
        goto exit;
    }
    for ( UINT uProducer = 0 ; uProducer < PRODUCER_COUNT ; uProducer++ )
    {
        producerArgs[uProducer].hEventStop = hEventStop;
    }
    for ( UINT uConsumer = 0 ; uConsumer < CONSUMER_COUNT ; uConsumer++ )
    {
        consumerArgs[uConsumer].hEventStop = hEventStop;
    }

    for ( UINT uProducer = 0 ; uProducer < PRODUCER_COUNT ; uProducer++ )
    {
        InterlockedIncrement( &g_ulProducerCount );        
        HANDLE hThread = (HANDLE)_beginthreadex( NULL , 0 , ProducerThread , &producerArgs[uProducer] , 0 , NULL );
    }

    for ( UINT uConsumer = 0 ; uConsumer < CONSUMER_COUNT ; uConsumer++ )
    {
        InterlockedIncrement( &g_ulConsumerCount );
        HANDLE hThread = (HANDLE)_beginthreadex( NULL , 0 , ConsumerThread , &consumerArgs[uConsumer] , 0 , NULL );
    }

    Sleep( 5000 );
    SetEvent( hEventStop );
    while ( g_ulProducerCount && g_ulConsumerCount )
    {
        Sleep( 100 );
    }

exit :
    if ( NULL != producerArgs )
    {
        delete [] producerArgs;
    }

    if ( NULL != consumerArgs )
    {
        delete [] consumerArgs;
    }

    CloseHandle( hEventStop );
    wprintf_s( L"\n========== TestRWLock() Leave ==========\n" );    
}


INT wmain( INT aArgc , WCHAR * aArgv[] )
{
    WPP_INIT_TRACING( L"TestCWLock" );
    DbgOut( INFO , DBG_TEST , "Enter" );
    for ( int i = 0 ; i < aArgc ; i++ )
    {
        wprintf_s( L"aArgv[%d]=%ws\n" , i , aArgv[i] );
    }
    wprintf_s( L"Start\n" );

    CWUtils::CStopWatch stopWatch;
    stopWatch.Start();

    do 
    {
        TestRWLock();
    } while ( 0 );

    stopWatch.Stop();
    wprintf_s( L"%I64u micro-sec\n" , stopWatch.GetIntervalInMicro() );
    wprintf_s( L"End of the program\n" );
    DbgOut( INFO , DBG_TEST , "Leave" );
    WPP_CLEANUP();
    system( "pause" );
    return 0;
}


UINT CALLBACK ProducerThread( VOID * aArgs )
{
    PRODUCER_ARGS * args = (PRODUCER_ARGS *)aArgs;
    HANDLE hEvents[] = { args->hEventStop };
    DWORD dwRet = 0;
    for ( ;; )
    {
        dwRet = WaitForMultipleObjects( _countof(hEvents) , hEvents , FALSE , PRODUCER_WAIT_TIME );
        switch ( dwRet )
        {
            case WAIT_OBJECT_0 :
            {
                wprintf_s( L"Producer: Quitting\n" );
                g_lsData.clear();
                goto exit;
            }                
            case WAIT_TIMEOUT :
            {                
                #ifndef RWLOCK
                    CWUtils::CAutoLock lock( &g_lock );
                #else
                    CWUtils::CAutoWriteLock lock( &g_lock );
                #endif
                if ( 100000 == g_lsData.size() )
                {
                    g_lsData.clear();
                }
                else
                {
                    DATA data;
                    CWUtils::FormatStringW( data.wstrData , L"%d" , RAND_INT( 0 , 1000 ) );
                    g_lsData.push_back( data );
                }
                break;
            }
            default :
            {
                wprintf_s( L"Producer: Some error happen cause thread close. dwRet=%u, GetLastError()=%u\n" , dwRet , GetLastError() );
                goto exit;
            }
        }
    }

exit :
    InterlockedDecrement( &g_ulProducerCount );
    _endthreadex( (UINT)dwRet );
    return (UINT)dwRet;
}



UINT CALLBACK ConsumerThread( VOID * aArgs )
{
    CONSUMER_ARGS * args = (CONSUMER_ARGS *)aArgs;
    HANDLE hEvents[] = { args->hEventStop };
    DWORD dwRet = 0;
    for ( ;; )
    {
        dwRet = WaitForMultipleObjects( _countof(hEvents) , hEvents , FALSE , CONSUMER_WAIT_TIME );
        switch ( dwRet )
        {
            case WAIT_OBJECT_0 :
            {
                wprintf_s( L"Consumer: Quitting\n" );
                goto exit;
            }
            case WAIT_TIMEOUT :
            {
                #ifndef RWLOCK
                    CWUtils::CAutoLock lock( &g_lock );
                #else
                    CWUtils::CAutoReadLock lock( &g_lock );
                #endif
                ULONG ulSum = 0;
                for ( list<DATA>::iterator it = g_lsData.begin() ; it != g_lsData.end() ; it++ )
                {
                    ulSum += wcstoul( it->wstrData.c_str() , NULL , 10 );
                }
                //wprintf_s( L"[%04X] %lu\n" , GetCurrentThreadId() , ulSum );
                InterlockedIncrement64( (volatile LONG64 *)&g_liCount.QuadPart );
                break;
            }
            default :
            {
                wprintf_s( L"Consumer: Some error happen cause thread close. dwRet=%u, GetLastError()=%u\n" , dwRet , GetLastError() );
                goto exit;
            }
        }
    }

exit :
    InterlockedDecrement( &g_ulConsumerCount );
    _endthreadex( (UINT)dwRet );
    return (UINT)dwRet;
}