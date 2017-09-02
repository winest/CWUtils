#include "stdafx.h"
#include "CWSharedMem.h"



namespace CWUtils
{

#ifdef __cplusplus
extern "C" {
#endif



BOOL CSharedMemFifo::Create( CONST CHAR * aName , UINT32 aPermission )
{
    BOOL bRet = FALSE;
    this->Close();

    do
    {
        m_bOpenExisting = FALSE;
        if ( NULL != aName )
        {
            m_strName = aName;
        }

        //Create FIFO
        mode_t uCreatePerm = 0;
        if ( aPermission & SHARED_MEM_PERM_READ )
        {
            uCreatePerm |= ( S_IRUSR | S_IRGRP | S_IROTH );
        }
        if ( aPermission & SHARED_MEM_PERM_WRITE )
        {
            uCreatePerm |= ( S_IWUSR | S_IWGRP | S_IWOTH );
        }
        if ( aPermission & SHARED_MEM_PERM_EXECUTE )
        {
            uCreatePerm |= ( S_IXUSR | S_IXGRP | S_IXOTH );
        }
        if ( -1 == mkfifo( aName , uCreatePerm ) && EEXIST != errno )
        {
            printf( "mkfifo() failed, errno=%s\n" , strerror(errno) );
            break;
        }

        bRet = TRUE;
    } while ( 0 );

    if ( FALSE == bRet )
    {
        this->Close();
    }

    return bRet;
}


BOOL CSharedMemFifo::Open( CONST CHAR * aName , UINT32 aPermission )
{
    BOOL bRet = FALSE;
    this->Close();

    do
    {
        m_bOpenExisting = TRUE;
        if ( NULL != aName )
        {
            m_strName = aName;
        }

        //Open existing FIFO
        mode_t uCreatePerm = 0;
        if ( aPermission & SHARED_MEM_PERM_READ )
        {
            uCreatePerm |= ( S_IRUSR | S_IRGRP | S_IROTH );
        }
        if ( aPermission & SHARED_MEM_PERM_WRITE )
        {
            uCreatePerm |= ( S_IWUSR | S_IWGRP | S_IWOTH );
        }
        if ( aPermission & SHARED_MEM_PERM_EXECUTE )
        {
            uCreatePerm |= ( S_IXUSR | S_IXGRP | S_IXOTH );
        }
        if ( -1 != mkfifo( aName , uCreatePerm ) || EEXIST != errno )
        {
            printf( "Not created yet, errno=%s\n" , strerror(errno) );
            break;
        }

        bRet = TRUE;
    } while ( 0 );

    if ( FALSE == bRet )
    {
        this->Close();
    }

    return bRet;
}




BOOL CSharedMemFifo::Connect( UINT32 aPermission )
{
    BOOL bRet = FALSE;

    do
    {
        //Skip if it's already connected
        if ( -1 != m_hSm )
        {
            bRet = TRUE;
            break;
        }

        INT nOpenPerm = 0;
        if ( aPermission & SHARED_MEM_PERM_READ )
        {
            nOpenPerm = O_RDONLY;
            if ( aPermission & SHARED_MEM_PERM_WRITE )
            {
                nOpenPerm = O_RDWR;
            }
        }
        else if ( aPermission & SHARED_MEM_PERM_WRITE )
        {
            nOpenPerm = O_WRONLY;
            if ( aPermission & SHARED_MEM_PERM_WRITE )
            {
                nOpenPerm = O_RDWR;
            }
        }
        else {}

        m_hSm = open( m_strName.c_str() , nOpenPerm );
        if ( -1 == m_hSm )
        {
            printf( "open() failed, errno=%s\n" , strerror(errno) );
            break;
        }

        bRet = TRUE;
    } while ( 0 );

    return bRet;
}



VOID CSharedMemFifo::Close( BOOL aRemove )
{
    if ( -1 != m_hSm )
    {
        close( m_hSm );
        m_hSm = -1;

        if ( !m_bOpenExisting || aRemove )
        {
            if ( m_strName.length() )
            {
                unlink( m_strName.c_str() );
            }
        }
        m_strName.clear();
    }
}

BOOL CSharedMemFifo::Write( CONST UCHAR * aBuf , SIZE_T aBufSize )
{
    BOOL bRet = FALSE;

    do
    {
        SIZE_T uWritten = 0;
        do
        {
            SIZE_T uNowWritten = write( m_hSm , aBuf+uWritten , aBufSize-uWritten );
            if ( -1 == uNowWritten )
            {
                printf( "write() failed, errno=%s\n" , strerror(errno) );
                break;
            }
            uWritten += uNowWritten;
        } while ( uWritten < aBufSize );

        bRet = TRUE;
    } while ( 0 );

    return bRet;
}

INT CSharedMemFifo::Read( UCHAR * aBuf , SIZE_T aBufSize )
{
    return read( m_hSm , aBuf , aBufSize );
}

BOOL CSharedMemFifo::Read( std::string & aBuf , SIZE_T aBufSize )
{
    BOOL bRet = FALSE;
    aBuf.clear();
    aBuf.reserve( aBufSize );

    CHAR szBuf[4096];
    do
    {
        SIZE_T uRead = 0;
        do
        {
            SIZE_T uRemainSize = min( sizeof(szBuf) , aBufSize-uRead );
            SIZE_T uNowRead = read( m_hSm , szBuf , uRemainSize );
            if ( -1 == uNowRead )
            {
                printf( "read() failed, errno=%s\n" , strerror(errno) );
                break;
            }
            uRead += uNowRead;
            aBuf.append( szBuf , uNowRead );
        } while ( uRead < aBufSize );

        bRet = TRUE;
    } while ( 0 );

    return bRet;
}


BOOL CSharedMemFifo::SmartWrite( CONST UCHAR * aBuf , SIZE_T aBufSize )
{
    BOOL bRet = FALSE;

    do
    {
        if ( FALSE == this->Write( (CONST UCHAR *)&aBufSize , sizeof(SIZE_T) ) )
        {
            printf( "Write() failed for size, errno=%d\n" , errno );
            break;
        }
        if ( FALSE == this->Write( aBuf , aBufSize ) )
        {
            printf( "Write() failed for data, errno=%d\n" , errno );
            break;
        }

        bRet = TRUE;
    } while ( 0 );

    return bRet;
}

BOOL CSharedMemFifo::SmartRead( std::string & aBuf )
{
    BOOL bRet = FALSE;

    do
    {
        SIZE_T uDataSize = 0;
        if ( -1 == this->Read( (UCHAR *)&uDataSize , sizeof(SIZE_T) ) )
        {
            printf( "Read() failed for size, errno=%d\n" , errno );
            break;
        }

        if ( FALSE == this->Read( aBuf , uDataSize ) )
        {
            printf( "Read() failed for data, errno=%d\n" , errno );
            break;
        }

        bRet = TRUE;
    } while ( 0 );

    return bRet;
}



















BOOL CSharedMemSegment::Create( CONST CHAR * aName , SIZE_T aMaxDataSize , UINT32 aPermission )
{
    BOOL bRet = FALSE;
    this->Close();

    do
    {
        if ( NULL != aName )
        {
            m_strName = aName;
        }
        
        
        
        //Create IPC event
        if ( FALSE == m_evtData.Create( (string("EvtData_")+m_strName).c_str() , FALSE , FALSE ) )
        {
            printf( "m_evtData.Create() failed, errno=%d\n" , errno );
            break;
        }
        if ( FALSE == m_evtDataOk.Create( (string("EvtDataOk_")+m_strName).c_str() , FALSE , FALSE ) )
        {
            printf( "m_evtDataOk.Create() failed, errno=%d\n" , errno );
            break;
        }
        
        

        //Create shared memory
        UINT32 uCreatePerm = 0;
        if ( aPermission & SHARED_MEM_PERM_READ )
        {
            uCreatePerm |= ( S_IRUSR | S_IRGRP | S_IROTH );
        }
        if ( aPermission & SHARED_MEM_PERM_WRITE )
        {
            uCreatePerm |= ( S_IWUSR | S_IWGRP | S_IWOTH );
        }
        if ( aPermission & SHARED_MEM_PERM_EXECUTE )
        {
            uCreatePerm |= ( S_IXUSR | S_IXGRP | S_IXOTH );
        }

        m_hSm = shm_open( aName , O_CREAT | O_EXCL | O_RDWR , uCreatePerm );    //O_RDWR is needed for ftruncate()
        if ( m_hSm == -1 )
        {
            if ( errno == EEXIST )
            {
                //Try to open an existing one
                m_hSm = shm_open( aName , O_RDWR , uCreatePerm );
                if ( m_hSm == -1 )
                {
                    printf( "shm_open() failed, errno=%s\n" , strerror(errno) );
                    break;
                }
            }
            else
            {
                printf( "shm_open() failed, errno=%s\n" , strerror(errno) );
                break;
            }
        }



        //Set max size
        m_uMaxDataSize = aMaxDataSize;
        m_uMaxSmSize = sizeof(CWUtilsSmData) - 1 + m_uMaxDataSize;
        struct stat smStat;
        if ( -1 != fstat( m_hSm , &smStat) && smStat.st_size == 0 )
        {
            if ( ftruncate( m_hSm , m_uMaxSmSize ) == -1 )
            {
                printf( "ftruncate() failed. errno=%s\n" , strerror(errno) );
                break;
            }
        }



        //Map shared memory
        INT nProtMap = PROT_NONE;
        INT nFlagMap = ( aName ) ? MAP_SHARED : MAP_PRIVATE | MAP_ANONYMOUS;
        if ( aPermission & SHARED_MEM_PERM_READ )
        {
            nProtMap |= PROT_READ;
        }
        if ( aPermission & SHARED_MEM_PERM_WRITE )
        {
            nProtMap |= PROT_WRITE;
        }
        if ( aPermission & SHARED_MEM_PERM_EXECUTE )
        {
            nProtMap |= PROT_EXEC;
        }

        m_pData = mmap( NULL , m_uMaxSmSize , nProtMap , nFlagMap , m_hSm , 0 );
        if ( m_pData == MAP_FAILED )
        {
            printf( "mmap() failed. errno=%s\n" , strerror(errno) );
            break;
        }



        bRet = TRUE;
    } while ( 0 );

    if ( FALSE == bRet )
    {
        this->Close();
    }

    return bRet;
}



BOOL CSharedMemSegment::Open( CONST CHAR * aName , SIZE_T aMaxDataSize , UINT32 aPermission )
{
    BOOL bRet = FALSE;
    this->Close();

    do
    {
        if ( NULL != aName )
        {
            m_strName = aName;
        }
        
        
        
        //Open IPC event
        if ( FALSE == m_evtData.Open( (string("EvtData_")+m_strName).c_str() , FALSE ) )
        {
            printf( "m_evtData.Open() failed, errno=%d\n" , errno );
            break;
        }
        if ( FALSE == m_evtDataOk.Open( (string("EvtDataOk_")+m_strName).c_str() , FALSE ) )
        {
            printf( "m_evtDataOk.Open() failed, errno=%d\n" , errno );
            break;
        }
        
        

        //Open shared memory
        m_hSm = shm_open( aName , O_RDWR , aPermission );
        if ( m_hSm == -1 )
        {
            printf( "shm_open() failed, errno=%s\n" , strerror(errno) );
            break;
        }



        //Set max size
        m_uMaxDataSize = aMaxDataSize;
        m_uMaxSmSize = sizeof(CWUtilsSmData) - 1 + m_uMaxDataSize;
        struct stat smStat;
        if ( -1 != fstat( m_hSm , &smStat) && smStat.st_size == 0 )
        {
            if ( -1 == ftruncate( m_hSm , m_uMaxSmSize ) )
            {
                printf( "ftruncate() failed. errno=%s\n" , strerror(errno) );
                break;
            }
        }


        //Map shared memory
        INT nProtMap = PROT_NONE;
        INT nFlagMap = ( aName ) ? MAP_SHARED : MAP_PRIVATE | MAP_ANONYMOUS;
        if ( aPermission & SHARED_MEM_PERM_READ )
        {
            nProtMap |= PROT_READ;
        }
        if ( aPermission & SHARED_MEM_PERM_WRITE )
        {
            nProtMap |= PROT_WRITE;
        }
        if ( aPermission & SHARED_MEM_PERM_EXECUTE )
        {
            nProtMap |= PROT_EXEC;
        }

        m_pData = mmap( NULL , m_uMaxSmSize , nProtMap , nFlagMap , m_hSm , 0 );
        if ( m_pData == MAP_FAILED )
        {
            printf( "mmap() failed. errno=%s\n" , strerror(errno) );
            break;
        }



        bRet = TRUE;
    } while ( 0 );

    if ( FALSE == bRet )
    {
        this->Close();
    }

    return bRet;
}



VOID CSharedMemSegment::Close()
{
    m_evtData.Close();
    m_evtDataOk.Close();
    
    if ( m_pData != NULL )
    {
        munmap( m_pData , m_uMaxSmSize );
        m_pData = NULL;
    }

    m_uMaxDataSize = 0;
    m_uMaxSmSize = 0;

    if ( m_hSm >= 0 )
    {
        if ( m_strName.length() )
        {
            shm_unlink( m_strName.c_str() );
        }
        else
        {
            shm_unlink( NULL );
        }
        m_hSm = -1;
        m_strName.clear();
    }
}

VOID * CSharedMemSegment::GetData()
{
    return m_pData;
}

SIZE_T CSharedMemSegment::GetMaxDataSize()
{
    return m_uMaxDataSize;
}


BOOL CSharedMemSegment::SmartWrite( CONST UCHAR * aBuf , SIZE_T aBufSize )
{
    BOOL bRet = FALSE;
    
    do
    {
        CWUtilsSmData * pData = (CWUtilsSmData *)m_pData;
        if ( NULL == pData )
        {
            printf( "pData is NULL" );
            break;
        }
        
        pData->uTotalSize = aBufSize;
        pData->uFlags = 0;
        SIZE_T uWritten = 0;
        do
        {
            pData->uCurrSize = min( m_uMaxDataSize , aBufSize-uWritten );
            memcpy( pData->pData , aBuf+uWritten , pData->uCurrSize );
            uWritten += pData->uCurrSize;
            
            //Signal and wait response
            if ( FALSE == m_evtData.Set() )
            {
                if ( EIDRM != errno )
                {
                    printf( "m_evtData.Set() failed, errno=%s\n" , strerror(errno) );
                }
                bRet = FALSE;
                break;
            }
            if ( FALSE == m_evtDataOk.Wait() )
            {
                if ( EIDRM != errno )
                {
                    printf( "m_evtData.Wait() failed, errno=%s\n" , strerror(errno) );
                }
                bRet = FALSE;
                break;
            }
        } while ( uWritten < aBufSize );
        if ( uWritten < aBufSize )
        {
            break;
        }
        
        bRet = TRUE;
    } while ( 0 );
    
    return bRet;
}

BOOL CSharedMemSegment::SmartRead( std::string & aBuf )
{
    BOOL bRet = FALSE;
    aBuf.clear();

    do
    {
        CWUtilsSmData * pData = (CWUtilsSmData *)m_pData;
        if ( NULL == pData )
        {
            printf( "pData is NULL" );
            break;
        }
        
        SIZE_T uRead = 0;
        SIZE_T uTotalSize = 0;
        do
        {
            //Wait until someone fill shared memory and set event by SmartWrite()
            if ( FALSE == m_evtData.Wait() )
            {
                if ( EIDRM != errno )
                {
                    printf( "m_evtData.Wait() failed, errno=%s\n" , strerror(errno) );
                }
                bRet = FALSE;
                break;
            }
            aBuf.reserve( pData->uTotalSize );
            uTotalSize = pData->uTotalSize;

            aBuf.append( pData->pData , pData->uCurrSize );
            uRead += pData->uCurrSize;

            //Signal and wait response
            if ( FALSE == m_evtDataOk.Set() )
            {
                if ( EIDRM != errno )
                {
                    printf( "m_evtDataOk.Set() failed, errno=%s\n" , strerror(errno) );
                }
                bRet = FALSE;
                break;
            }
        } while ( uRead < uTotalSize );
        if ( uRead < uTotalSize )
        {
            break;
        }

        bRet = TRUE;
    } while ( 0 );

    return bRet;
}


#ifdef __cplusplus
}
#endif

}   //End of namespace CWUtils
