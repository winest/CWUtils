#include "stdafx.h"
#include "CWSharedMem.h"

using namespace std;



int main( int aArgc , char * aArgv[] )
{
    CWUtils::CSharedMemFifo pipeP2C , pipeC2P;

    BOOL bLoop = TRUE;
    do
    {
        if ( FALSE == pipeP2C.Create( "FIFO_P2C" , CWUtils::SHARED_MEM_PERM_READ | CWUtils::SHARED_MEM_PERM_WRITE ) )
        {
            printf( "pipeP2C.Create() failed, errno=%d\n" , errno );
            break;
        }
        if ( FALSE == pipeC2P.Create( "FIFO_C2P" , CWUtils::SHARED_MEM_PERM_READ | CWUtils::SHARED_MEM_PERM_WRITE ) )
        {
            printf( "pipeC2P.Create() failed, errno=%d\n" , errno );
            break;
        }

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



        while ( bLoop )
        {
            //First sizeof(SIZE_T) bytes is data size, then data
            printf( "Wait data from parent\n" );
            string strP2C;
            if ( FALSE == pipeP2C.Read( strP2C , sizeof(SIZE_T) ) )
            {
                printf( "pipeP2C.Read() failed, errno=%d\n" , errno );
                bLoop = FALSE;
                break;
            }

            SIZE_T uP2CSize = *((SIZE_T *)strP2C.data());
            if ( FALSE == pipeP2C.Read( strP2C , uP2CSize ) )
            {
                printf( "pipeP2C.Read() failed, errno=%d\n" , errno );
                bLoop = FALSE;
                break;
            }
            printf( "pipeP2C.Read() succeed\n" );
            printf( "P->C: %s\n" , strP2C.c_str() );



            //First sizeof(SIZE_T) bytes is data size, then data
            printf( "Send data to parent\n" );
            string strC2P = "THIS_IS_THE_RESPONSE_FROM_CLIENT";
            SIZE_T uC2PSize = strC2P.size();
            if ( FALSE == pipeC2P.Write( (CONST UCHAR *)&uC2PSize , sizeof(SIZE_T) ) )
            {
                printf( "pipeC2P.Write() failed, errno=%d\n" , errno );
                bLoop = FALSE;
                break;
            }
            if ( FALSE == pipeC2P.Write( (CONST UCHAR *)strC2P.data() , uC2PSize ) )
            {
                printf( "pipeC2P.Write() failed, errno=%d\n" , errno );
                bLoop = FALSE;
                break;
            }
            printf( "pipeC2P.Read() succeed\n" );
            printf( "C->P: %s\n" , strC2P.c_str() );





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

