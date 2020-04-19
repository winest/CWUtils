#pragma once

#include "stdafx.h"

#include "GenerateTmh.h"
#include <_Generated/Wpp/FromHdr/CWSharedMem.tmh>


namespace CWUtils
{
#ifdef __cplusplus
extern "C" {
#endif

CONST UINT32 SHARED_MEM_PERM_NONE = 0x00000000;
CONST UINT32 SHARED_MEM_PERM_READ = 0x00000001;
CONST UINT32 SHARED_MEM_PERM_WRITE = 0x00000002;
CONST UINT32 SHARED_MEM_PERM_EXECUTE = 0x00000004;
CONST UINT32 SHARED_MEM_PERM_ALL = 0xFFFFFFFF;

class CSharedMemFileMapping
{
    public:
    CSharedMemFileMapping() : m_uMaxSize( 0 ), m_hSm( NULL ), m_pData( NULL ) {}
    ~CSharedMemFileMapping() { this->Close(); }

    BOOL Create( CONST CHAR * aName,
                 SIZE_T aMaxSize,
                 UINT32 aPermission = SHARED_MEM_PERM_READ | SHARED_MEM_PERM_WRITE );
    BOOL Open( CONST CHAR * aName, UINT32 aPermission = SHARED_MEM_PERM_READ | SHARED_MEM_PERM_WRITE );
    VOID Close();
    const std::string & GetName() const;

    VOID * GetData() const;
    SIZE_T GetMaxSize() const;

    private:
    SIZE_T m_uMaxSize;
    HANDLE m_hSm;
    VOID * m_pData;
    std::string m_strName;
};



#ifdef __cplusplus
}
#endif


template<typename TShmType>
class CFixedSizeShmQueue
{
    public:
    CFixedSizeShmQueue();
    ~CFixedSizeShmQueue();

    bool InitWriter( std::string aShmName, size_t aDataSize, size_t aMaxDataCnt );
    bool InitReader( std::string aShmName, size_t aDataSize, bool aReadFromEnd );

    void PushBack( const uint8_t * aData );
    uint8_t * GetData() const;
    void PopFront();

    void Close();

    protected:
    //      StartPos             LastIdx                  EndIdx
    //         |                    |                        |
    //--------------------------------------------------------
    //| ShmHdr | Data | Data | Data |          Empty         |
    //--------------------------------------------------------
    //                       |
    //                    CurrPos
    struct ShmHdr
    {
        size_t LastIdx;    //No data since LastIdx
        size_t EndIdx;
    };

    private:
    TShmType m_Shm;
    ShmHdr * m_Hdr;          //Common header, only writer will modify it
    uint8_t * m_StartPos;    //Used by each process to remember start position
    uint8_t * m_CurrPos;     //Used by each process to remember current position

    size_t m_DataSize;
};

template<typename TShmType>
CFixedSizeShmQueue<TShmType>::CFixedSizeShmQueue() :
    m_Hdr( nullptr ), m_StartPos( nullptr ), m_CurrPos( nullptr ), m_DataSize( 0 )
{
}

template<typename TShmType>
CFixedSizeShmQueue<TShmType>::~CFixedSizeShmQueue()
{
    this->Close();
}

template<typename TShmType>
bool CFixedSizeShmQueue<TShmType>::InitWriter( std::string aShmName, size_t aDataSize, size_t aMaxDataCnt )
{
    assert( aDataSize > 0 );
    bool bRet = false;

    do
    {
        size_t uAllDataSize = 0;
        if ( aMaxDataCnt == SIZE_MAX )
        {
            aMaxDataCnt = ( SIZE_MAX - sizeof( ShmHdr ) ) / aDataSize;
            uAllDataSize = aMaxDataCnt * aDataSize;
        }
        else
        {
            uAllDataSize = aMaxDataCnt * aDataSize;
            while ( uAllDataSize < aMaxDataCnt || uAllDataSize < aDataSize ||
                    sizeof( ShmHdr ) + uAllDataSize < uAllDataSize )    //Overflow
            {
                aMaxDataCnt--;
                if ( aMaxDataCnt == 0 )
                {
                    DbgOut( ERRO, DBG_UTILS, "Not enough memory to hold data" );
                    break;
                }
                uAllDataSize = aMaxDataCnt * aDataSize;
            }
            if ( aMaxDataCnt == 0 )
            {
                break;
            }
        }

        DbgOut( INFO, DBG_UTILS, "Try to create shm. aShmName=%s, aDataSize=%Iu, aMaxDataCnt=%Iu", aShmName.c_str(),
                aDataSize, aMaxDataCnt );
        if ( ! m_Shm.Create( aShmName.c_str(), sizeof( ShmHdr ) + uAllDataSize,
                             CWUtils::SHARED_MEM_PERM_READ | CWUtils::SHARED_MEM_PERM_WRITE ) )
        {
            DbgOut( ERRO, DBG_UTILS, "Failed to create shm. aShmName=%s, Err=%!WINERROR!", aShmName.c_str(),
                    GetLastError() );
            break;
        }

        uint8_t * pMem = static_cast<uint8_t *>( m_Shm.GetData() );
        m_Hdr = reinterpret_cast<ShmHdr *>( pMem );
        m_Hdr->LastIdx = 0;
        m_Hdr->EndIdx = aMaxDataCnt;
        m_StartPos = pMem + sizeof( ShmHdr );
        m_CurrPos = m_StartPos;
        m_DataSize = aDataSize;

        DbgOut( VERB, DBG_UTILS, "aShmName=%s, pMem=0x%p, m_StartPos=0x%p, m_CurrPos=0x%p", aShmName.c_str(), pMem,
                m_StartPos, m_CurrPos );
        bRet = true;
    } while ( 0 );

    return bRet;
}

template<typename TShmType>
bool CFixedSizeShmQueue<TShmType>::InitReader( std::string aShmName, size_t aDataSize, bool aReadFromEnd )
{
    bool bRet = false;

    do
    {
        DbgOut( INFO, DBG_UTILS, "Try to open shm. aShmName=%s", aShmName.c_str() );
        if ( ! m_Shm.Open( aShmName.c_str(), CWUtils::SHARED_MEM_PERM_READ ) )
        {
            DbgOut( ERRO, DBG_UTILS, "Failed to create shm. aShmName=%s, Err=%!WINERROR!", aShmName.c_str(),
                    GetLastError() );
            break;
        }

        uint8_t * pMem = static_cast<uint8_t *>( m_Shm.GetData() );
        m_Hdr = reinterpret_cast<ShmHdr *>( pMem );
        m_StartPos = pMem + sizeof( ShmHdr );
        m_CurrPos = ( aReadFromEnd ) ? m_StartPos + ( m_DataSize * m_Hdr->LastIdx ) : m_StartPos;
        m_DataSize = aDataSize;

        DbgOut( VERB, DBG_UTILS, "aShmName=%s, pMem=0x%p, m_StartPos=0x%p, m_CurrPos=0x%p, m_DataSize=%Iu",
                aShmName.c_str(), pMem, m_StartPos, m_CurrPos, m_DataSize );
        bRet = true;
    } while ( 0 );

    return bRet;
}

template<typename TShmType>
void CFixedSizeShmQueue<TShmType>::PushBack( const uint8_t * aData )
{
    memcpy( m_StartPos + ( m_DataSize * m_Hdr->LastIdx ), aData, m_DataSize );

    size_t uNextIdx = m_Hdr->LastIdx + 1;
    if ( uNextIdx == m_Hdr->EndIdx )
    {
        DbgOut( WARN, DBG_UTILS, "Shm is full. Overwrite from beginning. ShmName=%s", m_Shm.GetName().c_str() );
        uNextIdx = 0;
    }

    m_Hdr->LastIdx = uNextIdx;
}

template<typename TShmType>
uint8_t * CFixedSizeShmQueue<TShmType>::GetData() const
{
    //wprintf_s( L"%hs: Idx=%Iu / %Iu\n", m_Shm.GetName().c_str(), ( m_CurrPos - m_StartPos ) / m_DataSize , m_Hdr->LastIdx );
    if ( m_CurrPos != m_StartPos + ( m_DataSize * m_Hdr->LastIdx ) )
    {
        return m_CurrPos;
    }
    else
    {
        //DbgOut( VERB, DBG_UTILS, "Shm is empty. ShmName=%s", m_Shm.GetName().c_str() );
        return nullptr;
    }
}

template<typename TShmType>
void CFixedSizeShmQueue<TShmType>::PopFront()
{
    if ( m_CurrPos == m_StartPos + ( m_DataSize * m_Hdr->LastIdx ) )
    {
        //DbgOut( VERB, DBG_UTILS, "Shm is empty. ShmName=%s", m_Shm.GetName().c_str() );
        return;
    }

    uint8_t * pNextPos = m_CurrPos + m_DataSize;
    if ( pNextPos == m_StartPos + ( m_DataSize * m_Hdr->EndIdx ) )
    {
        DbgOut( WARN, DBG_UTILS, "Shm is full. Get from beginning. ShmName=%s", m_Shm.GetName().c_str() );
        pNextPos = m_StartPos;
    }
    m_CurrPos = pNextPos;
}

template<typename TShmType>
void CFixedSizeShmQueue<TShmType>::Close()
{
    m_Shm.Close();
}






template<typename TShmType, typename TDataType>
class CFixedTypeShmQueue
{
    public:
    CFixedTypeShmQueue();
    ~CFixedTypeShmQueue();

    bool InitWriter( std::string aShmName, size_t aMaxDataCnt );
    bool InitReader( std::string aShmName, bool aReadFromEnd );

    void PushBack( const TDataType & aData );
    TDataType * GetData() const;
    void PopFront();

    void Close();

    protected:
    //      StartPos             LastIdx                  EndIdx
    //         |                    |                        |
    //--------------------------------------------------------
    //| ShmHdr | Data | Data | Data |          Empty         |
    //--------------------------------------------------------
    //                       |
    //                    CurrPos
    struct ShmHdr
    {
        size_t LastIdx;    //No data since LastIdx
        size_t EndIdx;
    };

    private:
    TShmType m_Shm;
    ShmHdr * m_Hdr;            //Common header, only writer will modify it
    TDataType * m_StartPos;    //Used by each process to remember start position
    TDataType * m_CurrPos;     //Used by each process to remember current position
};

template<typename TShmType, typename TDataType>
CFixedTypeShmQueue<TShmType, TDataType>::CFixedTypeShmQueue() :
    m_Hdr( nullptr ), m_StartPos( nullptr ), m_CurrPos( nullptr )
{
}

template<typename TShmType, typename TDataType>
CFixedTypeShmQueue<TShmType, TDataType>::~CFixedTypeShmQueue()
{
    this->Close();
}

template<typename TShmType, typename TDataType>
bool CFixedTypeShmQueue<TShmType, TDataType>::InitWriter( std::string aShmName, size_t aMaxDataCnt )
{
    bool bRet = false;

    do
    {
        if ( aMaxDataCnt == 0 )
        {
            aMaxDataCnt = ( SIZE_MAX - sizeof( ShmHdr ) ) / sizeof( TDataType );
        }
        size_t uAllDataSize = aMaxDataCnt * sizeof( TDataType );
        while ( sizeof( ShmHdr ) + uAllDataSize < uAllDataSize )    //Overflow
        {
            aMaxDataCnt--;
            if ( aMaxDataCnt == 0 )
            {
                DbgOut( ERRO, DBG_UTILS, "Not enough memory to hold data" );
                break;
            }
            uAllDataSize = aMaxDataCnt * sizeof( TDataType );
        }
        if ( aMaxDataCnt == 0 )
        {
            break;
        }

        if ( ! m_Shm.Create( aShmName.c_str(), sizeof( ShmHdr ) + uAllDataSize,
                             CWUtils::SHARED_MEM_PERM_READ | CWUtils::SHARED_MEM_PERM_WRITE ) )
        {
            DbgOut( ERRO, DBG_UTILS, "Failed to create shm. aShmName=%s, Err=%!WINERROR!", aShmName.c_str(),
                    GetLastError() );
            break;
        }

        uint8_t * pMem = static_cast<uint8_t *>( m_Shm.GetData() );
        m_Hdr = reinterpret_cast<ShmHdr *>( pMem );
        m_Hdr->LastIdx = 0;
        m_Hdr->EndIdx = aMaxDataCnt;
        m_StartPos = reinterpret_cast<TDataType *>( pMem + sizeof( ShmHdr ) );
        m_CurrPos = m_StartPos;

        DbgOut( VERB, DBG_UTILS, "aShmName=%s, pMem=0x%p, m_StartPos=0x%p, m_CurrPos=0x%p", aShmName.c_str(), pMem,
                m_StartPos, m_CurrPos );
        bRet = true;
    } while ( 0 );

    return bRet;
}

template<typename TShmType, typename TDataType>
bool CFixedTypeShmQueue<TShmType, TDataType>::InitReader( std::string aShmName, bool aReadFromEnd )
{
    bool bRet = false;

    do
    {
        if ( ! m_Shm.Open( aShmName.c_str(), CWUtils::SHARED_MEM_PERM_READ ) )
        {
            DbgOut( ERRO, DBG_UTILS, "Failed to create shm. aShmName=%s, Err=%!WINERROR!", aShmName.c_str(),
                    GetLastError() );
            break;
        }

        uint8_t * pMem = static_cast<uint8_t *>( m_Shm.GetData() );
        m_Hdr = reinterpret_cast<ShmHdr *>( pMem );
        m_StartPos = reinterpret_cast<TDataType *>( pMem + sizeof( ShmHdr ) );
        m_CurrPos = ( aReadFromEnd ) ? &m_StartPos[m_Hdr->LastIdx] : m_StartPos;

        DbgOut( VERB, DBG_UTILS, "aShmName=%s, pMem=0x%p, m_StartPos=0x%p, m_CurrPos=0x%p", aShmName.c_str(), pMem,
                m_StartPos, m_CurrPos );
        bRet = true;
    } while ( 0 );

    return bRet;
}

template<typename TShmType, typename TDataType>
void CFixedTypeShmQueue<TShmType, TDataType>::PushBack( const TDataType & aData )
{
    memcpy( &m_StartPos[m_Hdr->LastIdx], &aData, sizeof( TDataType ) );

    uint64_t uNextIdx = m_Hdr->LastIdx + 1;
    if ( uNextIdx == m_Hdr->EndIdx )
    {
        DbgOut( WARN, DBG_UTILS, "Shm is full. Overwrite from beginning. ShmName=%s", m_Shm.GetName().c_str() );
        uNextIdx = 0;
    }

    m_Hdr->LastIdx = uNextIdx;
}

template<typename TShmType, typename TDataType>
TDataType * CFixedTypeShmQueue<TShmType, TDataType>::GetData() const
{
    //wprintf_s( L"%hs: Idx=%Iu / %Iu\n", m_Shm.GetName().c_str(), ( m_CurrPos - m_StartPos ) , m_Hdr->LastIdx );
    if ( m_CurrPos != &m_StartPos[m_Hdr->LastIdx] )
    {
        return m_CurrPos;
    }
    else
    {
        //DbgOut( VERB, DBG_UTILS, "Shm is empty. ShmName=%s", m_Shm.GetName().c_str() );
        return nullptr;
    }
}

template<typename TShmType, typename TDataType>
void CFixedTypeShmQueue<TShmType, TDataType>::PopFront()
{
    if ( m_CurrPos == &m_StartPos[m_Hdr->LastIdx] )
    {
        //DbgOut( VERB, DBG_UTILS, "Shm is empty. ShmName=%s", m_Shm.GetName().c_str() );
        return;
    }

    TDataType * pNextPos = m_CurrPos + 1;
    if ( pNextPos == &m_StartPos[m_Hdr->EndIdx] )
    {
        DbgOut( WARN, DBG_UTILS, "Shm is full. Get from beginning. ShmName=%s", m_Shm.GetName().c_str() );
        pNextPos = m_StartPos;
    }
    m_CurrPos = pNextPos;
}

template<typename TShmType, typename TDataType>
void CFixedTypeShmQueue<TShmType, TDataType>::Close()
{
    m_Shm.Close();
}



}    //End of namespace CWUtils
