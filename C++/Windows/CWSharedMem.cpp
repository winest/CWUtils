#include "stdafx.h"
#include "CWSharedMem.h"

#include "_GenerateTmh.h"
#include <body/CWSharedMem.tmh>


namespace CWUtils
{
#ifdef __cplusplus
extern "C" {
#endif

BOOL CSharedMemFileMapping::Create( CONST CHAR * aName, SIZE_T aMaxSize, UINT32 aPermission )
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
        SECURITY_ATTRIBUTES secAttr;
        SECURITY_DESCRIPTOR secDesc;
        ZeroMemory( &secAttr, sizeof( secAttr ) );
        ZeroMemory( &secDesc, sizeof( secDesc ) );
        InitializeSecurityDescriptor( &secDesc, SECURITY_DESCRIPTOR_REVISION );
        SetSecurityDescriptorDacl( &secDesc, TRUE, NULL, FALSE );
        secAttr.nLength = sizeof( SECURITY_ATTRIBUTES );
        secAttr.lpSecurityDescriptor = &secDesc;
        secAttr.bInheritHandle = FALSE;

        DWORD dwCreatePerm = 0;
        if ( aPermission & SHARED_MEM_PERM_READ )
        {
            if ( aPermission & SHARED_MEM_PERM_EXECUTE )
            {
                dwCreatePerm = PAGE_EXECUTE_READ;
            }
            else
            {
                dwCreatePerm = PAGE_READONLY;
            }
        }
        if ( aPermission & SHARED_MEM_PERM_WRITE )
        {
            if ( aPermission & SHARED_MEM_PERM_EXECUTE )
            {
                dwCreatePerm = PAGE_EXECUTE_READWRITE;
            }
            else
            {
                dwCreatePerm = PAGE_READWRITE;
            }
        }

#if INTPTR_MAX == INT32_MAX
        m_hSm = CreateFileMappingA( INVALID_HANDLE_VALUE, &secAttr, dwCreatePerm, 0, aMaxSize, aName );
#else
        m_hSm = CreateFileMappingA( INVALID_HANDLE_VALUE, &secAttr, dwCreatePerm, ( aMaxSize >> 32 ),
                                    ( aMaxSize & 0xFFFFFFFF ), aName );
#endif
        if ( NULL == m_hSm )
        {
            DbgOut( ERRO, DBG_UTILS, "CreateFileMapping() failed. GetLastError()=%!WINERROR!", GetLastError() );
            break;
        }
        m_uMaxSize = aMaxSize;



        //Map shared memory
        DWORD dwMapPerm = 0;
        if ( aPermission & SHARED_MEM_PERM_READ )
        {
            dwMapPerm = FILE_MAP_READ;
        }
        if ( aPermission & SHARED_MEM_PERM_WRITE )
        {
            dwMapPerm = FILE_MAP_WRITE;
        }
        if ( aPermission & SHARED_MEM_PERM_EXECUTE )
        {
            dwMapPerm |= FILE_MAP_EXECUTE;
        }

        m_pData = (VOID *)MapViewOfFile( m_hSm, dwMapPerm, 0, 0, 0 );
        if ( NULL == m_pData )
        {
            DbgOut( ERRO, DBG_UTILS, "MapViewOfFile() failed. GetLastError()=%!WINERROR!", GetLastError() );
            break;
        }



        //Zero-fill memory for initialization
        ZeroMemory( m_pData, aMaxSize );

        bRet = TRUE;
    } while ( 0 );

    if ( FALSE == bRet )
    {
        this->Close();
    }

    return bRet;
}



BOOL CSharedMemFileMapping::Open( CONST CHAR * aName, UINT32 aPermission )
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
        DWORD dwOpenPerm = 0;
        if ( aPermission & SHARED_MEM_PERM_READ )
        {
            dwOpenPerm = FILE_MAP_READ;
        }
        if ( aPermission & SHARED_MEM_PERM_WRITE )
        {
            dwOpenPerm = FILE_MAP_WRITE;
        }
        if ( aPermission & SHARED_MEM_PERM_EXECUTE )
        {
            dwOpenPerm |= FILE_MAP_EXECUTE;
        }

        m_hSm = OpenFileMappingA( dwOpenPerm, FALSE, aName );
        if ( NULL == m_hSm )
        {
            DbgOut( ERRO, DBG_UTILS, "OpenFileMappingA() failed. GetLastError()=%!WINERROR!", GetLastError() );
            break;
        }



        //Map shared memory
        DWORD dwMapPerm = 0;
        if ( aPermission & SHARED_MEM_PERM_READ )
        {
            dwMapPerm = FILE_MAP_READ;
        }
        if ( aPermission & SHARED_MEM_PERM_WRITE )
        {
            dwMapPerm = FILE_MAP_WRITE;
        }
        if ( aPermission & SHARED_MEM_PERM_EXECUTE )
        {
            dwMapPerm |= FILE_MAP_EXECUTE;
        }

        m_pData = (VOID *)MapViewOfFile( m_hSm, dwMapPerm, 0, 0, 0 );
        if ( NULL == m_pData )
        {
            DbgOut( ERRO, DBG_UTILS, "MapViewOfFile() failed. GetLastError()=%!WINERROR!", GetLastError() );
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



VOID CSharedMemFileMapping::Close()
{
    if ( m_pData != nullptr )
    {
        UnmapViewOfFile( m_pData );
        m_pData = nullptr;
    }

    m_uMaxSize = 0;

    if ( m_hSm != nullptr )
    {
        CloseHandle( m_hSm );
        m_hSm = nullptr;
        m_strName.clear();
    }
}

const std::string & CSharedMemFileMapping::GetName() const
{
    return m_strName;
}


VOID * CSharedMemFileMapping::GetData() const
{
    return m_pData;
}

SIZE_T CSharedMemFileMapping::GetMaxSize() const
{
    return m_uMaxSize;
}



#ifdef __cplusplus
}
#endif


}    //End of namespace CWUtils
