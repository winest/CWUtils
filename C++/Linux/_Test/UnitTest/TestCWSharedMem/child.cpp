#include "stdafx.h"
#include "CWSharedMem.h"

using namespace std;

VOID TestSharedMemFifo()
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
}








VOID TestSharedMemSegment()
{
    CWUtils::CSharedMemSegment smP2C , smC2P;

    BOOL bLoop = TRUE;
    do
    {
        if ( FALSE == smP2C.Open( "SEGMENT_P2C" , 1 , CWUtils::SHARED_MEM_PERM_READ | CWUtils::SHARED_MEM_PERM_WRITE ) )
        {
            printf( "smP2C.Open() failed, errno=%d\n" , errno );
            break;
        }
        if ( FALSE == smC2P.Open( "SEGMENT_C2P" , 1 , CWUtils::SHARED_MEM_PERM_READ | CWUtils::SHARED_MEM_PERM_WRITE ) )
        {
            printf( "smC2P.Open() failed, errno=%d\n" , errno );
            break;
        }



        while ( bLoop )
        {
            printf( "Wait data from parent\n" );
            string strP2C;
            if ( FALSE == smP2C.SmartRead( strP2C ) )
            {
                printf( "smP2C.SmartRead() failed, errno=%d\n" , errno );
                bLoop = FALSE;
                break;
            }
            printf( "smP2C.SmartRead() succeed\n" );
            printf( "P->C: %s\n" , strP2C.c_str() );



            printf( "Send data to parent\n" );
            string strC2P = "THIS_IS_THE_RESPONSE_FROM_CLIENT";
            if ( FALSE == smC2P.SmartWrite( (CONST UCHAR *)strC2P.data() , strC2P.size() ) )
            {
                printf( "smC2P.SmartWrite() failed, errno=%d\n" , errno );
                break;
            }
            printf( "smC2P.SmartWrite() succeed\n" );





            INT nNum;
            printf( "Enter a positive number or 0 to exit: " );
            scanf( "%d" , &nNum );
            bLoop = ( nNum == 0 ) ? FALSE : TRUE;
        }
    } while ( 0 );



    smP2C.Close();
    smC2P.Close();
}














int main( int aArgc , char * aArgv[] )
{
    BOOL bLoop = TRUE;
    
    do
    {
        INT nNum;
        printf( "Enter command:\n"
                "0: Exit\n"
                "1: Test SharedMemFifo\n"
                "2: Test SharedMemSegment\n" );
        scanf( "%d" , &nNum );
        switch ( nNum )
        {
            case 0 :
            {
                bLoop = FALSE;
                break;
            }
            case 1 : 
            {
                TestSharedMemFifo();
                break;
            }
            case 2 :
            {
                TestSharedMemSegment();
                break;
            }
            default :
            {
                printf( "Unknown command: %d\n" , nNum );
                break;
            }
        }
    } while ( bLoop );
    return 0;
}

