#pragma once

#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>



namespace CWUtils
{

#ifdef __cplusplus
extern "C" {
#endif



class CEvent
{
    public :
        CEvent() : m_uMaxCnt(1) , m_nSemId(-1) , m_bOpenExisting(TRUE) {}
        ~CEvent() { this->Close(); }
        
        BOOL Create( CONST CHAR * aName , BOOL aManualReset , BOOL aInitialState );
        BOOL Open( CONST CHAR * aName , BOOL aManualReset );
        VOID Close( BOOL aRemove = FALSE );
        
        BOOL Set();
        BOOL Reset();
        BOOL Wait();
        
        INT GetHandle();
        
    protected :
        CONST UINT m_uMaxCnt;
        INT m_nSemId;
        BOOL m_bOpenExisting;
        BOOL m_bManualReset;
};



#ifdef __cplusplus
}
#endif

}   //End of namespace CWUtils
