#include "stdafx.h"
#include "CWBuffer.h"
#include <new>

namespace CWUtils
{

VOID CDynamicBuffer::Free()
{
    if ( NULL != m_pBuf )
    {
        delete [] m_pBuf;
        m_pBuf = NULL;
        m_ulCurSize = m_ulAllocSize = 0;
    }
}

BOOL CDynamicBuffer::Append( CONST UCHAR * aBuf , SIZE_T aBufSize )
{
    UCHAR * pOldBuf = NULL;
    if ( FALSE == ExtendBufferIfNeed(aBufSize , &pOldBuf) )
    {
        return FALSE;
    }
    else
    {
        if ( NULL != pOldBuf )
        {
            CopyMemory( m_pBuf , pOldBuf , m_ulCurSize );
            delete [] pOldBuf;
        }

        CopyMemory( &m_pBuf[m_ulCurSize] , aBuf , aBufSize );
        m_ulCurSize += aBufSize;
        m_pBuf[m_ulCurSize] = (UCHAR)NULL;
        return TRUE;
    }
}

BOOL CDynamicBuffer::Insert( SIZE_T aIndexFrom , CONST UCHAR * aBuf , SIZE_T aBufSize )
{
    if ( NULL == m_pBuf || m_ulCurSize <= aIndexFrom )
    {
        return Append( aBuf , aBufSize );
    }

    UCHAR * pOldBuf = NULL;
    if ( FALSE == ExtendBufferIfNeed(aBufSize , &pOldBuf) )
    {
        return FALSE;
    }
    else
    {
        if ( NULL == pOldBuf )
        {            
            SIZE_T ulTail = ( m_ulCurSize - aIndexFrom ) % sizeof(DWORD);

            for ( SIZE_T i = m_ulCurSize ; i > (m_ulCurSize-ulTail) ; i-- )
            {
                m_pBuf[i+aBufSize-1] = m_pBuf[i-1];
            }
            for ( SIZE_T i = m_ulCurSize-ulTail ; i > (aIndexFrom) ; i-=sizeof(DWORD) )
            {
                *((DWORD *)m_pBuf[i+aBufSize-sizeof(DWORD)]) = *((DWORD *)&m_pBuf[i-sizeof(DWORD)]);
            }
            CopyMemory( &m_pBuf[aIndexFrom] , aBuf , aBufSize );
        }
        else
        {
            CopyMemory( m_pBuf , pOldBuf , aIndexFrom );
            CopyMemory( &m_pBuf[aIndexFrom] , aBuf , aBufSize );
            CopyMemory( &m_pBuf[aIndexFrom+aBufSize] , &pOldBuf[aIndexFrom] , m_ulCurSize - aIndexFrom );
            delete [] pOldBuf;
        }

        m_ulCurSize += aBufSize;
        m_pBuf[m_ulCurSize] = (UCHAR)NULL;
        return TRUE;
    }
}

BOOL CDynamicBuffer::Erase( SIZE_T aIndexFrom , SIZE_T aCount )
{
    aCount = min( aCount , m_ulCurSize - aIndexFrom );
    if ( NULL == m_pBuf || 0 == aCount || m_ulCurSize < aIndexFrom )
    {
        return TRUE;
    }

    UCHAR * pOldBuf = NULL;
    if ( FALSE == ShrinkBufferIfNeed( aCount , &pOldBuf ) || NULL == pOldBuf )
    {
        CopyMemory( &m_pBuf[aIndexFrom] , &m_pBuf[aIndexFrom+aCount] , m_ulCurSize - (aIndexFrom+aCount) );
    }
    else
    {
        CopyMemory( m_pBuf , pOldBuf , aIndexFrom );
        CopyMemory( &m_pBuf[aIndexFrom] , &pOldBuf[aIndexFrom+aCount] , m_ulCurSize - (aIndexFrom+aCount) );
        delete [] pOldBuf;
    }
    
    m_ulCurSize -= aCount;
    m_pBuf[m_ulCurSize] = (UCHAR)NULL;   
    return TRUE;
}

CDynamicBuffer CDynamicBuffer::operator+( CONST CDynamicBuffer & aMyDynamicBuffer )
{
    CDynamicBuffer buf(*this);
    buf.Append( aMyDynamicBuffer.m_pBuf , aMyDynamicBuffer.m_ulCurSize );
    return buf;
}

CDynamicBuffer & CDynamicBuffer::operator=( CONST CDynamicBuffer & aMyDynamicBuffer )
{
    if ( this != &aMyDynamicBuffer )
    {
        this->Free();
        this->Append( aMyDynamicBuffer.m_pBuf , aMyDynamicBuffer.m_ulCurSize );
    }
    return *this;
}

CDynamicBuffer & CDynamicBuffer::operator+=( CONST CDynamicBuffer & aMyDynamicBuffer )
{
    this->Append( aMyDynamicBuffer.m_pBuf , aMyDynamicBuffer.m_ulCurSize );
    return *this;
    
}

BOOL CDynamicBuffer::ExtendBufferIfNeed( SIZE_T aExtendSize , UCHAR ** aOldBuf )
{
    *aOldBuf = NULL;
    if ( NULL == m_pBuf || m_ulAllocSize < (m_ulCurSize+aExtendSize+1) )    //+1 for terminating NULL
    {
        SIZE_T ulNewAllocSize = max( (SIZE_T)((DOUBLE)m_ulAllocSize * m_fGrowRate) , m_ulAllocSize + aExtendSize ) + 1; //+1 for terminating NULL
        UCHAR * pNewBuf = new (std::nothrow) UCHAR[ulNewAllocSize];
        if ( NULL == pNewBuf )
        {
            return FALSE;
        }

        *aOldBuf = m_pBuf;
        m_pBuf = pNewBuf;
        m_pBuf[m_ulCurSize] = (UCHAR)NULL;
        m_ulAllocSize = ulNewAllocSize;
    }
    return TRUE;
}


BOOL CDynamicBuffer::ShrinkBufferIfNeed( SIZE_T aShrinkSize , UCHAR ** aOldBuf )
{
    *aOldBuf = NULL;
    if ( m_fShrinkRate > ((DOUBLE)(m_ulCurSize-aShrinkSize)/(DOUBLE)m_ulAllocSize) )
    {
        SIZE_T ulNewAllocSize = max( SIZE_T((DOUBLE)(m_ulCurSize-aShrinkSize) / m_fShrinkRate) , (m_ulCurSize-aShrinkSize) ) + 1; //+1 for terminating NULL
        if ( ulNewAllocSize < m_ulAllocSize  )
        {
            UCHAR * pNewBuf = new (std::nothrow) UCHAR[ulNewAllocSize];
            if ( NULL == pNewBuf )
            {
                return FALSE;
            }

            *aOldBuf = m_pBuf;
            m_pBuf = pNewBuf;
            m_pBuf[m_ulCurSize] = (UCHAR)NULL;
            m_ulAllocSize = ulNewAllocSize;
        }
    }
    return TRUE;
}






VOID CMemFileBuffer::Free()
{
    if ( NULL != m_pMem )
    {
        delete [] m_pMem;
        m_pMem = NULL;
        m_ulCurMemSize = m_ulAllocMemSize = 0;
    }
    if ( INVALID_HANDLE_VALUE != m_hFile )
    {
        CloseHandle( m_hFile );
        m_ulFileSize = 0;
    }
    DeleteFileW( m_wstrFilePath.c_str() );  //Since GetTempPathW() will always create the file
    m_wstrFilePath.clear();
}

BOOL CMemFileBuffer::Write( CONST UCHAR * aBuf , SIZE_T aBufSize )
{
    SIZE_T ulRemainMem = m_ulMaxMemSize - m_ulCurMemSize;
    if ( ulRemainMem >= aBufSize )
    {
        WriteToMem( aBuf , aBufSize );
    }
    else
    {
        WriteToMem( aBuf , ulRemainMem );
        WriteToFile( &aBuf[ulRemainMem] , aBufSize - ulRemainMem );
    }
    return TRUE;
}

SIZE_T CMemFileBuffer::Read( SIZE_T aIndexFrom , UCHAR * aBuf , SIZE_T aBufSize , BOOL & aEnd )
{
    SIZE_T ulRet = 0;
    if ( aIndexFrom < m_ulCurMemSize )
    {
        SIZE_T ulCopyFromMemSize = min( (m_ulCurMemSize-aIndexFrom) , aBufSize );
        CopyMemory( aBuf , m_pMem , min( (m_ulCurMemSize-aIndexFrom) , aBufSize) );
        aIndexFrom += ulCopyFromMemSize;
        aBuf += ulCopyFromMemSize;
        aBufSize -= ulCopyFromMemSize;
        ulRet += ulCopyFromMemSize;
    }

    
    if ( INVALID_HANDLE_VALUE == m_hFile || (m_ulCurMemSize+m_ulFileSize) <= aIndexFrom )
    {
        aEnd = TRUE;
        return ulRet;
    }
    else if ( 0 == aBufSize )
    {
        return ulRet;
    }
    else
    {
        aIndexFrom -= m_ulCurMemSize;
    }

    
    LARGE_INTEGER liIndex;
    liIndex.QuadPart = aIndexFrom;
    if ( 0 == SetFilePointerEx( m_hFile , liIndex , NULL , FILE_BEGIN ) )
    {
        return ulRet;
    }
    else
    {
        DWORD dwRead = 0;
        if ( ReadFile( m_hFile , aBuf , aBufSize , &dwRead , NULL ) && dwRead < aBufSize )
        {
            aEnd = TRUE;
        }
        ulRet += dwRead;
        return ulRet;
    }
}


VOID CMemFileBuffer::CreateTempFile( OUT std::wstring & aFilePath , IN CONST WCHAR * aPreferFolderPath )
{
    aFilePath.clear();
    WCHAR wzFileName[MAX_PATH];
    do 
    {
        if ( NULL != aPreferFolderPath && 0 != GetTempFileNameW( aPreferFolderPath , NULL , 0 , wzFileName ) )
        {
            aFilePath = aPreferFolderPath;
            break;
        }            
                
        WCHAR wzFolderPath[MAX_PATH+1];
        DWORD dwFolderLen = GetTempPathW( _countof(wzFolderPath) , wzFolderPath );
        if ( 0 != dwFolderLen && MAX_PATH >= dwFolderLen && 0 < GetTempFileNameW( wzFolderPath , NULL , 0 , wzFileName ) )
        {
            aFilePath = wzFolderPath;
            break;
        }

        GetTempFileNameW( L"." , NULL , 0 , wzFileName );
    } while ( 0 );

    if ( aFilePath.length() && L'\\' != aFilePath[aFilePath.length()-1] )
    {
        aFilePath.push_back( L'\\' );
    }
    aFilePath.append( wzFileName );
    if ( MAX_PATH <= aFilePath.length() )
    {
        aFilePath.insert( 0 , L"\\\\?\\" );
    }
}

BOOL CMemFileBuffer::WriteToMem( CONST UCHAR * aBuf , SIZE_T aBufSize )
{
    if ( 0 == aBufSize )
    {
        return TRUE;
    }

    if ( NULL == m_pMem || m_ulAllocMemSize < (m_ulCurMemSize+aBufSize) )
    {
        SIZE_T ulNewAllocMemSize = min( m_ulMaxMemSize , max( m_ulAllocMemSize * 2 , m_ulAllocMemSize + aBufSize ) );
        UCHAR * pNewBuf = new (std::nothrow) UCHAR[ulNewAllocMemSize];
        if ( NULL == pNewBuf )
        {
            return FALSE;
        }

        CopyMemory( pNewBuf , m_pMem , m_ulCurMemSize );
        delete [] m_pMem;
        m_pMem = pNewBuf;
        m_ulAllocMemSize = ulNewAllocMemSize;
    }

    CopyMemory( &m_pMem[m_ulCurMemSize] , aBuf , aBufSize );
    m_ulCurMemSize += aBufSize;
    return TRUE;
}

BOOL CMemFileBuffer::WriteToFile( CONST UCHAR * aBuf , SIZE_T aBufSize )
{
    if ( 0 == aBufSize )
    {
        return TRUE;
    }

    if ( m_hFile == INVALID_HANDLE_VALUE )
    {
        m_hFile = CreateFileW( m_wstrFilePath.c_str() , GENERIC_READ | GENERIC_WRITE , FILE_SHARE_READ , NULL , OPEN_EXISTING , FILE_ATTRIBUTE_NORMAL , NULL );
        if ( INVALID_HANDLE_VALUE == m_hFile )
        {
            return FALSE;
        }
    }

    if ( INVALID_SET_FILE_POINTER == SetFilePointer( m_hFile , 0 , NULL , FILE_END ) )
    {
        return FALSE;
    }

    do
    {
        DWORD dwWritten = 0;
        if ( FALSE == WriteFile( m_hFile ,  aBuf , aBufSize , &dwWritten , NULL ) )
        {
            return FALSE;
        }
        aBuf += dwWritten;
        aBufSize -= dwWritten;
        m_ulFileSize += dwWritten;
    } while ( aBufSize );
    
    return TRUE;
}


UCHAR CMemFileBuffer::operator[]( SIZE_T aIndex )
{
    BOOL bEnd;
    UCHAR uByte;
    Read( aIndex , &uByte , 1 , bEnd );
    return uByte;
}

CMemFileBuffer & CMemFileBuffer::operator>>( std::string & aBuf )
{
    BOOL bEnd = FALSE;
    UCHAR buf[4096];
    SIZE_T ulCopied = 0;

    aBuf.clear();
    for ( SIZE_T i = 0 ; i < m_ulCurMemSize + m_ulFileSize ; i+= sizeof(buf) )
    {
        ulCopied = Read( i , buf , sizeof(buf) , bEnd );
        if ( 0 < ulCopied )
        {
            aBuf.append( (CONST CHAR *)buf , ulCopied );
        }
    }
    return *this;    
}

CMemFileBuffer & CMemFileBuffer::operator<<( CONST CHAR * aBuf )
{
    Write( (CONST UCHAR *)aBuf , strlen(aBuf) );
    return *this;
}

CMemFileBuffer & CMemFileBuffer::operator<<( CONST std::string & aBuf )
{
    Write( (CONST UCHAR *)aBuf.c_str() , aBuf.size() );
    return *this;
}


}   //End of namespace CWUtils