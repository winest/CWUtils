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
        if ( -1 == mkfifo( aName , uCreatePerm ) )
        {
            printf( "mkfifo() failed, errno=%s\n" , strerror(errno) );
            break;
        }

        bRet = TRUE;
    } while ( 0 );

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



VOID CSharedMemFifo::Close()
{
    if ( -1 != m_hSm )
    {
        close( m_hSm );
        m_hSm = -1;
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





BOOL CSharedMemSegment::Create( CONST CHAR * aName , SIZE_T aMaxSize , UINT32 aPermission )
{
    BOOL bRet = FALSE;
    this->Close();

    do
    {
        if ( NULL != aName )
        {
            m_strName = aName;
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
        struct stat smStat;
        if ( -1 != fstat( m_hSm , &smStat) && smStat.st_size == 0 )
        {
            if ( ftruncate( m_hSm , aMaxSize ) == -1 )
            {
                printf( "ftruncate() failed. errno=%s\n" , strerror(errno) );
                break;
            }
        }
        m_uMaxSize = aMaxSize;



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

        m_pData = mmap( NULL , m_uMaxSize , nProtMap , nFlagMap , m_hSm , 0 );
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



BOOL CSharedMemSegment::Open( CONST CHAR * aName , SIZE_T aMaxSize , UINT32 aPermission )
{
    BOOL bRet = FALSE;
    this->Close();

    do
    {
        if ( NULL != aName )
        {
            m_strName = aName;
        }

        //Open shared memory
        m_hSm = shm_open( aName , O_RDWR , aPermission );
        if ( m_hSm == -1 )
        {
            printf( "shm_open() failed, errno=%s\n" , strerror(errno) );
            break;
        }



        //Set max size
        struct stat smStat;
        if ( -1 != fstat( m_hSm , &smStat) && smStat.st_size == 0 )
        {
            if ( ftruncate( m_hSm , aMaxSize ) == -1 )
            {
                printf( "ftruncate() failed. errno=%s\n" , strerror(errno) );
                break;
            }
        }
        m_uMaxSize = aMaxSize;



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

        m_pData = mmap( NULL , aMaxSize , nProtMap , nFlagMap , m_hSm , 0 );
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
    if ( m_pData != NULL )
    {
        munmap( m_pData , m_uMaxSize );
        m_pData = NULL;
    }

    m_uMaxSize = 0;

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

SIZE_T CSharedMemSegment::GetMaxSize()
{
    return m_uMaxSize;
}



#ifdef __cplusplus
}
#endif

}   //End of namespace CWUtils
