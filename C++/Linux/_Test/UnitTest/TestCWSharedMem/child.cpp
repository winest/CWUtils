#include "stdafx.h"
#include "CWSharedMem.h"

using namespace std;



int main( int aArgc , char * aArgv[] )
{
    CWUtils::CSharedMemFifo pipeP2C , pipeC2P;

    BOOL bLoop = TRUE;
    do
    {
        if ( FALSE == pipeP2C.Open( "FIFO_P2C" , CWUtils::SHARED_MEM_PERM_READ | CWUtils::SHARED_MEM_PERM_WRITE ) )
        {
            printf( "pipeP2C.Open() failed, errno=%d\n" , errno );
            break;
        }
        if ( FALSE == pipeC2P.Open( "FIFO_C2P" , CWUtils::SHARED_MEM_PERM_READ | CWUtils::SHARED_MEM_PERM_WRITE ) )
        {
            printf( "pipeC2P.Open() failed, errno=%d\n" , errno );
            break;
        }

        printf( "Connecting\n" );
        if ( FALSE == pipeP2C.Connect( CWUtils::SHARED_MEM_PERM_READ ) )
        {
            printf( "pipeP2C.Connect() failed, errno=%d\n" , errno );
            break;
        }
        if ( FALSE == pipeC2P.Connect( CWUtils::SHARED_MEM_PERM_WRITE ) )
        {
            printf( "pipeC2P.Connect() failed, errno=%d\n" , errno );
            break;
        }
        printf( "Connected\n" );



        while ( bLoop )
        {
            //First sizeof(SIZE_T) bytes is data size, then data
            printf( "Wait data from parent\n" );
            string strP2C;
            if ( FALSE == pipeP2C.SmartRead( strP2C ) )
            {
                printf( "pipeP2C.SmartRead() failed, errno=%d\n" , errno );
                bLoop = FALSE;
                break;
            }
            printf( "pipeP2C.SmartRead() succeed\n" );
            printf( "P->C: %s\n" , strP2C.c_str() );



            //First sizeof(SIZE_T) bytes is data size, then data
            printf( "Send data to parent\n" );
            string strC2P = "THIS_IS_THE_RESPONSE_FROM_CLIENT";
            if ( FALSE == pipeC2P.SmartWrite( (CONST UCHAR *)strC2P.data() , strC2P.size() ) )
            {
                printf( "pipeC2P.SmartWrite() failed, errno=%d\n" , errno );
                break;
            }
            printf( "pipeC2P.SmartWrite() succeed\n" );





            INT nNum;
            printf( "Enter a positive number or 0 to exit: " );
            scanf( "%d" , &nNum );
            bLoop = ( nNum == 0 ) ? FALSE : TRUE;
        }
    } while ( 0 );



    pipeP2C.Close();
    pipeC2P.Close();
    return 0;
}

