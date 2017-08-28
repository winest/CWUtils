#include "stdafx.h"
#include "CWSharedMem.h"



namespace CWUtils
{

#ifdef __cplusplus
extern "C" {
#endif



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
