#include "stdafx.h"
#include "CWEvent.h"

using namespace std;



int main( int aArgc, char * aArgv[] )
{
    CWUtils::CEvent evtManualP2C, evtManualC2P;
    CWUtils::CEvent evtAutoP2C, evtAutoC2P;

    BOOL bLoop = TRUE;
    do
    {
        if ( FALSE == evtManualP2C.Create( "Manual_P2C", TRUE, FALSE ) )
        {
            printf( "evtManualP2C.Create() failed, errno=%s\n", strerror( errno ) );
            break;
        }
        if ( FALSE == evtManualC2P.Create( "Manual_C2P", TRUE, FALSE ) )
        {
            printf( "evtManualC2P.Create() failed, errno=%s\n", strerror( errno ) );
            break;
        }

        if ( FALSE == evtAutoP2C.Create( "Auto_P2C", FALSE, FALSE ) )
        {
            printf( "evtAutoP2C.Create() failed, errno=%s\n", strerror( errno ) );
            break;
        }
        if ( FALSE == evtAutoC2P.Create( "Auto_C2P", FALSE, FALSE ) )
        {
            printf( "evtAutoC2P.Create() failed, errno=%s\n", strerror( errno ) );
            break;
        }


        while ( bLoop )
        {
            INT nNum;
            printf(
                "Please enter option:\n"
                "    0: exit\n"
                "    1: test manual event\n"
                "    2: test auto event\n" );
            scanf( "%d", &nNum );
            if ( nNum == 0 )
            {
                bLoop = FALSE;
                break;
            }
            else if ( nNum == 1 )
            {
                printf( "Set event to child_manual\n" );
                if ( FALSE == evtManualP2C.Set() )
                {
                    printf( "evtManualP2C.Set() failed, errno=%d\n", errno );
                    bLoop = FALSE;
                    break;
                }
                sleep( 5 );

                if ( FALSE == evtManualP2C.Reset() )
                {
                    printf( "evtManualP2C.Reset() failed, errno=%d\n", errno );
                    bLoop = FALSE;
                    break;
                }
            }
            else if ( nNum == 2 )
            {
                printf( "Wait event from child_auto\n" );
                if ( FALSE == evtAutoC2P.Wait() )
                {
                    printf( "evtAutoC2P.Wait() failed, errno=%d\n", errno );
                    bLoop = FALSE;
                    break;
                }
                printf( "evtAutoC2P.Wait() succeed\n" );
                sleep( 3 );



                printf( "Set event to child_auto\n" );
                if ( FALSE == evtAutoP2C.Set() )
                {
                    printf( "evtAutoP2C.Set() failed, errno=%d\n", errno );
                    bLoop = FALSE;
                    break;
                }
                printf( "evtAutoP2C.Set() succeed\n" );



                printf( "Wait event from child_auto\n" );
                if ( FALSE == evtAutoC2P.Wait() )
                {
                    printf( "evtAutoC2P.Wait() failed, errno=%d\n", errno );
                    bLoop = FALSE;
                    break;
                }
                printf( "evtAutoC2P.Wait() succeed\n" );
            }
            else
            {
                printf( "Invaild option\n" );
            }
        }
    } while ( 0 );


    evtManualP2C.Close();
    evtManualC2P.Close();
    evtAutoP2C.Close();
    evtAutoC2P.Close();
    return 0;
}
