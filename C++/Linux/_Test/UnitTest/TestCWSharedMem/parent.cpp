#include "stdafx.h"
#include "CWSharedMem.h"

using namespace std;

VOID TestSharedMemFifo()
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

        printf( "Connecting\n" );
        if ( FALSE == pipeP2C.Connect( CWUtils::SHARED_MEM_PERM_WRITE ) )
        {
            printf( "pipeP2C.Connect() failed, errno=%d\n" , errno );
            break;
        }
        if ( FALSE == pipeC2P.Connect( CWUtils::SHARED_MEM_PERM_READ ) )
        {
            printf( "pipeC2P.Connect() failed, errno=%d\n" , errno );
            break;
        }
        printf( "Connected\n" );



        while ( bLoop )
        {
            //First sizeof(SIZE_T) bytes is data size, then data
            printf( "Send data to client\n" );
            string strP2C = "Message from parent";
            if ( FALSE == pipeP2C.SmartWrite( (CONST UCHAR *)strP2C.data() , strP2C.size() ) )
            {
                printf( "pipeP2C.SmartWrite() failed, errno=%d\n" , errno );
                bLoop = FALSE;
                break;
            }
            printf( "pipeP2C.SmartWrite() succeed\n" );



            //First sizeof(SIZE_T) bytes is data size, then data
            printf( "Wait data from child\n" );
            string strC2P;
            if ( FALSE == pipeC2P.SmartRead( strC2P ) )
            {
                printf( "pipeC2P.SmartRead() failed, errno=%d\n" , errno );
                bLoop = FALSE;
                break;
            }
            printf( "pipeC2P.SmartRead() succeed\n" );
            printf( "C->P: %s\n" , strC2P.c_str() );


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
        if ( FALSE == smP2C.Create( "SEGMENT_P2C" , 1 , CWUtils::SHARED_MEM_PERM_READ | CWUtils::SHARED_MEM_PERM_WRITE ) )
        {
            printf( "smP2C.Create() failed, errno=%d\n" , errno );
            break;
        }
        if ( FALSE == smC2P.Create( "SEGMENT_C2P" , 1 , CWUtils::SHARED_MEM_PERM_READ | CWUtils::SHARED_MEM_PERM_WRITE ) )
        {
            printf( "smC2P.Create() failed, errno=%d\n" , errno );
            break;
        }
        


        while ( bLoop )
        {
            printf( "Send data to client\n" );
            string strP2C = "Message from parent";
            if ( FALSE == smP2C.SmartWrite( (CONST UCHAR *)strP2C.data() , strP2C.size() ) )
            {
                printf( "smP2C.SmartWrite() failed, errno=%d\n" , errno );
                bLoop = FALSE;
                break;
            }
            printf( "smP2C.SmartWrite() succeed\n" );



            printf( "Wait data from child\n" );
            string strC2P;
            if ( FALSE == smC2P.SmartRead( strC2P ) )
            {
                printf( "smC2P.SmartRead() failed, errno=%d\n" , errno );
                bLoop = FALSE;
                break;
            }
            printf( "smC2P.SmartRead() succeed\n" );
            printf( "C->P: %s\n" , strC2P.c_str() );


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

