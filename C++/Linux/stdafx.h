#pragma once

#if defined( USE_WPP )
//Do nothing
#elif defined( USE_G3LOG )
#    define WPP_INIT_TRACING( ... )                                  \
        auto g3Worker = g3::LogWorker::createLogWorker();            \
        auto hG3Worker = worker->addDefaultLogger( __FILE__, "./" ); \
        g3::initializeLogging( g3Worker.get() );
#    define DbgOut( aLogLevel, aComponent, ... ) LOGF( aLogLevel, __VA_ARGS__ )
#    define WPP_CLEANUP( ... )
#else
#    define WPP_INIT_TRACING( ... )
#    define DbgOut( ... )
#    define WPP_CLEANUP( ... )
#endif

#define LIKELY( x ) __builtin_expect( !!( x ), 1 )
#define UNLIKELY( x ) __builtin_expect( !!( x ), 0 )

#define _CONSOLE

#include "WinDef.h"
#include <algorithm>
#include <assert.h>
#include <iostream>
#include <list>
#include <map>
#include <queue>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
using namespace std;
