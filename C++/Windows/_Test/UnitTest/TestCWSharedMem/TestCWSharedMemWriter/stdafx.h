#pragma once

//Including SDKDDKVer.h defines the highest available Windows platform
//If you wish to build your application for a previous Windows platform, include WinSDKVer.h and
//set the _WIN32_WINNT macro to the platform you wish to support before including SDKDDKVer.h

//#define _WIN32_WINNT   0x0501

#pragma warning( push, 0 )
#include <SDKDDKVer.h>

#define WIN32_LEAN_AND_MEAN    // Exclude rarely-used stuff from Windows headers
#define NOMINMAX
#include <Windows.h>
#undef NOMINMAX
#include <WTypesbase.h>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream>
#include <atomic>
#include <algorithm>
#include <vector>
#include <list>
#include <deque>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>

#pragma warning( pop )