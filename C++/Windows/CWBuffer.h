#pragma once

/*
 * Copyright (c) 2009-2015, ChienWei Hung <winestwinest@gmail.com>
 * CWUtils is published under the BSD-3-Clause license.
 *
 * CWUtils is a set of standalone APIs for developers to speed up their 
 * programming. It should be very easy to port them to other projects or 
 * learn how to implement things on different languages and platforms. 
 *
 * The latest version can be found at https://github.com/winest/CWUtils
 */

#pragma warning( push, 0 )
#include <Windows.h>
#include <WTypesbase.h>
#include <new>
#include <string>
#pragma warning( pop )

namespace CWUtils
{
//CAutoBuffer is a general container for a buffer. It will free the memory itself when it's not needed
template<class TYPE>
class CAutoBuffer
{
    public:
    CAutoBuffer( SIZE_T aBufSize = 1 ) : m_pBuf( NULL ), m_ulBufSize( aBufSize )
    {
        m_pBuf = new ( std::nothrow ) TYPE[aBufSize];
    }
    virtual ~CAutoBuffer()
    {
        if ( NULL != m_pBuf )
        {
            delete[] m_pBuf;
            m_pBuf = NULL;
        }
    }
    TYPE * GetBuf() { return m_pBuf; }
    SIZE_T GetBufSize() { return m_ulBufSize; }
    TYPE & operator[]( SIZE_T aIndex ) { return m_pBuf[aIndex]; }

    private:
    TYPE * m_pBuf;
    SIZE_T m_ulBufSize;
};



//CDynamicBuffer is designed to keep a buffer with the least memory-reallocation times
//CDynamicBuffer can extend or shrink the memory usage according to buffer size it holds
class CDynamicBuffer
{
    public:
    CDynamicBuffer( SIZE_T aInitSize = 0, DOUBLE aGrowRate = 1.618, DOUBLE aShrinkRate = (DOUBLE)1.0 / (DOUBLE)3.0 ) :
        m_fGrowRate( aGrowRate ),
        m_fShrinkRate( aShrinkRate ),
        m_pBuf( NULL ),
        m_ulCurSize( 0 ),
        m_ulAllocSize( 0 )
    {
        if ( 0 < aInitSize )
        {
            m_pBuf = new ( std::nothrow ) UCHAR[aInitSize];
            if ( NULL != m_pBuf )
            {
                m_pBuf[m_ulCurSize] = (UCHAR)NULL;
                m_ulAllocSize = aInitSize;
            }
        }
    }

    CDynamicBuffer( CONST CDynamicBuffer & aDynamicBuffer ) :
        m_fGrowRate( aDynamicBuffer.m_fGrowRate ),
        m_fShrinkRate( aDynamicBuffer.m_fShrinkRate ),
        m_pBuf( NULL ),
        m_ulCurSize( 0 ),
        m_ulAllocSize( 0 )
    {
        this->Append( aDynamicBuffer.m_pBuf, aDynamicBuffer.m_ulCurSize );
    }

    virtual ~CDynamicBuffer() { Free(); }

    public:
    VOID Free();

    UCHAR * GetBuf() { return m_pBuf; }
    SIZE_T GetBufSize() { return m_ulCurSize; }
    SIZE_T GetAllocSize() { return m_ulAllocSize; }
    SIZE_T GetFreeSize() { return max( m_ulAllocSize - m_ulCurSize, 1 ) - 1; }
    UCHAR & operator[]( SIZE_T aIndex ) { return m_pBuf[aIndex]; }


    BOOL Append( CONST UCHAR * aBuf, SIZE_T aBufSize );
    BOOL Insert( SIZE_T aIndexFrom, CONST UCHAR * aBuf, SIZE_T aBufSize );
    BOOL Erase( SIZE_T aIndexFrom, SIZE_T aCount );

    CDynamicBuffer operator+( CONST CDynamicBuffer & aMyDynamicBuffer );
    CDynamicBuffer & operator=( CONST CDynamicBuffer & aMyDynamicBuffer );
    CDynamicBuffer & operator+=( CONST CDynamicBuffer & aMyDynamicBuffer );

    protected:
    BOOL ExtendBufferIfNeed( SIZE_T aExtendSize, UCHAR ** aOldBuf );
    BOOL ShrinkBufferIfNeed( SIZE_T aShrinkSize, UCHAR ** aOldBuf );

    protected:
    CONST DOUBLE m_fGrowRate, m_fShrinkRate;
    UCHAR * m_pBuf;
    SIZE_T m_ulCurSize, m_ulAllocSize;
};





//CMemFileBuffer is designed to contain a unknown size buffer which could be very short or very long
//If the buffer size is smaller than aMaxMemSize, it will contain the buffer in memory
//If the buffer size exceed aMaxMemSize, the remaining (aBufSize-aMaxMemSize) bytes will be written to a temporary file
class CMemFileBuffer
{
    public:
    CMemFileBuffer( SIZE_T aMaxMemSize = 1024 * 1024, WCHAR * aFileFolder = NULL ) :
        m_pMem( NULL ),
        m_ulCurMemSize( 0 ),
        m_ulAllocMemSize( 0 ),
        m_ulMaxMemSize( aMaxMemSize ),
        m_hFile( INVALID_HANDLE_VALUE ),
        m_ulFileSize( 0 )
    {
        CreateTempFile( m_wstrFilePath, aFileFolder );
    }
    virtual ~CMemFileBuffer() { Free(); }

    public:
    VOID Free();

    SIZE_T GetSize() { return m_ulCurMemSize + m_ulFileSize; }

    SIZE_T Read( SIZE_T aIndexFrom, UCHAR * aBuf, SIZE_T aBufSize, BOOL & aEnd );
    BOOL Write( CONST UCHAR * aBuf, SIZE_T aBufSize );    //Append aBuf to internal buffer

    UCHAR operator[]( SIZE_T aIndex );                          //Read only since not all buffer are saved in memory
    CMemFileBuffer & operator>>( std::string & aBuf );          //Append internal buffer to aBuf
    CMemFileBuffer & operator<<( CONST CHAR * aBuf );           //Append aBuf to internal buffer
    CMemFileBuffer & operator<<( CONST std::string & aBuf );    //Append aBuf to internal buffer



    protected:
    VOID CreateTempFile( OUT std::wstring & aFilePath, IN CONST WCHAR * aPreferFolderPath = NULL );

    BOOL WriteToMem( CONST UCHAR * aBuf, SIZE_T aBufSize );
    BOOL WriteToFile( CONST UCHAR * aBuf, SIZE_T aBufSize );

    private:
    UCHAR * m_pMem;
    SIZE_T m_ulCurMemSize;
    SIZE_T m_ulAllocMemSize;
    CONST SIZE_T m_ulMaxMemSize;

    HANDLE m_hFile;
    SIZE_T m_ulFileSize;
    std::wstring m_wstrFilePath;
};


}    //End of namespace CWUtils