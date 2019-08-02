#pragma once

#pragma warning( push, 0 )
#include <Windows.h>
#pragma warning( pop )


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
    BOOL Open( CONST CHAR * aName, SIZE_T aMaxSize, UINT32 aPermission = SHARED_MEM_PERM_READ | SHARED_MEM_PERM_WRITE );
    VOID Close();

    VOID * GetData();
    SIZE_T GetMaxSize();

    private:
    SIZE_T m_uMaxSize;
    HANDLE m_hSm;
    VOID * m_pData;
    std::string m_strName;
};



#ifdef __cplusplus
}
#endif

}    //End of namespace CWUtils
