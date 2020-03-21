#include "stdafx.h"
#include "CWEvent.h"

using namespace std;



int main( int aArgc, char * aArgv[] )
{
    CWUtils::CEvent evtP2C, evtC2P;

    BOOL bLoop = TRUE;
    do
    {
        if ( FALSE == evtP2C.Open( "Manual_P2C", TRUE ) )
        {
            printf( "evtP2C.Open() failed, errno=%d\n", errno );
            break;
        }
        if ( FALSE == evtC2P.Open( "Manual_C2P", TRUE ) )
        {
            printf( "evtC2P.Open() failed, errno=%d\n", errno );
            break;
        }


        while ( bLoop )
        {
            printf( "Wait event from parent\n" );
            if ( FALSE == evtP2C.Wait() )
            {
                printf( "evtP2C.Wait() failed, errno=%d\n", errno );
                bLoop = FALSE;
                break;
            }
            printf( "evtP2C.Wait() succeed\n" );
        }
    } while ( 0 );

    evtP2C.Close();
    evtC2P.Close();
    return 0;
}
