#include "stdafx.h"
#include "CWSharedMem.h"

#if defined( USE_WPP )
#    include "_GenerateTmh.h"
#    include "CWSharedMem.tmh"
#elif defined( USE_G3LOG )
#    include <g3log/g3log.hpp>
#    include <g3log/logworker.hpp>
#    include "G3LogLevel.h"
#endif

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

        m_hSm = CreateFileMappingA( INVALID_HANDLE_VALUE, &secAttr, dwCreatePerm, ( aMaxSize >> 32 ),
                                    ( aMaxSize & 0xFFFFFFFF ), aName );
        if ( NULL == m_hSm )
        {
            DbgOut( ERRO, DBG_DLL_INJECT_MGR, "CreateFileMapping() failed. GetLastError()=%!WINERROR!",
                    GetLastError() );
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
            DbgOut( ERRO, DBG_DLL_INJECT_MGR, "MapViewOfFile() failed. GetLastError()=%!WINERROR!", GetLastError() );
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



BOOL CSharedMemFileMapping::Open( CONST CHAR * aName, SIZE_T aMaxSize, UINT32 aPermission )
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
            DbgOut( ERRO, DBG_DLL_INJECT_MGR, "OpenFileMappingA() failed. GetLastError()=%!WINERROR!", GetLastError() );
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
            DbgOut( ERRO, DBG_DLL_INJECT_MGR, "MapViewOfFile() failed. GetLastError()=%!WINERROR!", GetLastError() );
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
    if ( m_pData != NULL )
    {
        UnmapViewOfFile( m_pData );
        m_pData = NULL;
    }

    m_uMaxSize = 0;

    if ( m_hSm >= 0 )
    {
        CloseHandle( m_hSm );
        m_strName.clear();
    }
}

VOID * CSharedMemFileMapping::GetData()
{
    return m_pData;
}

SIZE_T CSharedMemFileMapping::GetMaxSize()
{
    return m_uMaxSize;
}



#ifdef __cplusplus
}
#endif

}    //End of namespace CWUtils
