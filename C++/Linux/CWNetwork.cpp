#include "stdafx.h"

#include "CWNetwork.h"
using namespace std;



namespace CWUtils
{
#ifdef __cplusplus
extern "C" {
#endif

#define DEFAULT_ADAPTER_BUF_SIZE ( 16 * 1024 )
#define DEFAULT_SOCKET_BUF_SIZE ( 4 * 1024 )
#define MAX_RETRY 3

#ifndef STATUS_SUCCESS
#    define STATUS_SUCCESS ( 0x00000000L )
#endif


#ifdef __cplusplus
}
#endif

}    //End of namespace CWUtils
