#include "stdafx.h"
#include "CWEvent.h"

using namespace std;



int main( int aArgc , char * aArgv[] )
{
    CWUtils::CEvent evtP2C , evtC2P;
    
    BOOL bLoop = TRUE;
    do
    {
        if ( FALSE == evtP2C.Open( "Auto_P2C" , FALSE ) )
        {
            printf( "evtP2C.Open() failed, errno=%d\n" , errno );
            break;
        }
        if ( FALSE == evtC2P.Open( "Auto_C2P" , FALSE ) )
        {
            printf( "evtC2P.Open() failed, errno=%d\n" , errno );
            break;
        }
        
        
        while ( bLoop )
        {
            printf( "Set event to parent\n" );
            if ( FALSE == evtC2P.Set() )
            {
                printf( "evtC2P.Set() failed, errno=%d\n" , errno );
                bLoop = FALSE;
                break;
            }
            printf( "evtC2P.Set() succeed\n" );
            
            
            
            printf( "Wait event from parent\n" );
            if ( FALSE == evtP2C.Wait() )
            {
                printf( "evtP2C.Wait() failed, errno=%d\n" , errno );
                bLoop = FALSE;
                break;
            }
            printf( "evtP2C.Wait() succeed\n" );
            sleep( 3 );
            
            
            
            printf( "Set event to parent\n" );
            if ( FALSE == evtC2P.Set() )
            {
                printf( "evtC2P.Set() failed, errno=%d\n" , errno );
                bLoop = FALSE;
                break;
            }
            printf( "evtC2P.Set() succeed\n" );
            
            INT nNum;
            printf( "Enter a positive number or 0 to exit: " );
            scanf( "%d" , &nNum );
            bLoop = ( nNum == 0 ) ? FALSE : TRUE;
        }
    } while ( 0 );
    
    
    evtC2P.Close();
    return 0;
}

