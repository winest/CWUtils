#pragma once

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "CWEvent.h"



namespace CWUtils
{

#ifdef __cplusplus
extern "C" {
#endif

CONST UINT32 SHARED_MEM_PERM_NONE     = 0x00000000;
CONST UINT32 SHARED_MEM_PERM_READ     = 0x00000001;
CONST UINT32 SHARED_MEM_PERM_WRITE    = 0x00000002;
CONST UINT32 SHARED_MEM_PERM_EXECUTE  = 0x00000004;
CONST UINT32 SHARED_MEM_PERM_ALL      = 0xFFFFFFFF;

#pragma pack( push , 1 )
typedef struct _CWUtilsSmData
{
    UINT64    uTotalSize;            //Total data size we want to send in the pData field
    UINT64    uCurrSize;             //Current data size pointed by the pData field
    UINT64    uFlags;                //Reserved
    CHAR      pData[1];
} CWUtilsSmData;
#pragma pack( pop )



class CSharedMemFifo    //Named pipe on Linux
{
    public :
        CSharedMemFifo() : m_bOpenExisting(FALSE) , m_hSm(-1) {}
        ~CSharedMemFifo() { this->Close(); }

    public :
        BOOL Create( CONST CHAR * aName , UINT32 aPermission = SHARED_MEM_PERM_READ | SHARED_MEM_PERM_WRITE );
        BOOL Open( CONST CHAR * aName , UINT32 aPermission = SHARED_MEM_PERM_READ | SHARED_MEM_PERM_WRITE );
        BOOL Connect( UINT32 aPermission );
        VOID Close( BOOL aRemove = FALSE );

        BOOL Write( CONST UCHAR * aBuf , SIZE_T aBufSize );
        INT Read( UCHAR * aBuf , SIZE_T aBufSize );   //Doesn't handle the case aBuf is not big enough
        BOOL Read( std::string & aBuf , SIZE_T aBufSize );  //Read until reaches aBufSize

        //SmartWrite() and SmartRead() can send data without the size limitation
        //SmartWrite() will block execution until all data are written
        //SmartRead() will block execution until all data are read
        //SmartWrite() and SmartRead() should only be used pairly
        //The content in FIFO will be <SIZE_T bytes content size><content>
        BOOL SmartWrite( CONST UCHAR * aBuf , SIZE_T aBufSize );
        BOOL SmartRead( std::string & aBuf );

    private :
        BOOL m_bOpenExisting;
        
        INT m_hSm;
        std::string m_strName;
};



class CSharedMemSegment
{
    public :
        CSharedMemSegment() : m_hSm(-1) , m_uMaxDataSize(0) , m_uMaxSmSize(0) , m_pData(NULL) {}
        ~CSharedMemSegment() { this->Close(); }

        BOOL Create( CONST CHAR * aName , SIZE_T aMaxDataSize , UINT32 aPermission = SHARED_MEM_PERM_READ | SHARED_MEM_PERM_WRITE );
        BOOL Open( CONST CHAR * aName , SIZE_T aMaxDataSize , UINT32 aPermission = SHARED_MEM_PERM_READ | SHARED_MEM_PERM_WRITE );
        VOID Close();

        VOID * GetData();
        SIZE_T GetMaxDataSize();
        
        //SmartWrite() and SmartRead() can send data without the size limitation
        //SmartWrite() will block execution until all data are written
        //SmartRead() will block execution until all data are read
        //SmartWrite() and SmartRead() should only be used pairly
        BOOL SmartWrite( CONST UCHAR * aBuf , SIZE_T aBufSize );
        BOOL SmartRead( std::string & aBuf );

    private :
        CWUtils::CEvent m_evtData , m_evtDataOk;
        
        INT m_hSm;
        SIZE_T m_uMaxDataSize;
        SIZE_T m_uMaxSmSize;    //sizeof(CWUtilsSmData) - 1 + m_uMaxDataSize
        VOID * m_pData;
        std::string m_strName;
};



#ifdef __cplusplus
}
#endif

}   //End of namespace CWUtils
