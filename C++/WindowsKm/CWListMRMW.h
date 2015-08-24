#pragma once

#include <ntifs.h>

#ifndef INFINITE
    #define INFINITE        0xFFFFFFFF
#endif

#ifndef MY_MEM_TAG_UTILS
    #define MY_MEM_TAG_UTILS        'litU'
#endif

namespace KmUtils
{

template <class TYPE>
class CMyListMRMW
{
    typedef struct _MyListEntryMRMW
    {
        LIST_ENTRY lsNext;
        TYPE pEntry;
    } MyListEntryMRMW;

    public :
        CMyListMRMW() { ExInitializeResourceLite( &m_res ); InitializeListHead( &m_lsHead ); }
        ~CMyListMRMW() { RemoveAllEntries(); ExDeleteResourceLite( &m_res ); }

    public :
        BOOLEAN IsEmpty()
        {
            this->AcquireReaderLock();
            BOOLEAN bRet = IsListEmpty( &m_lsHead );
            this->ReleaseReaderLock();
            return bRet;
        }

        BOOLEAN IsEntryExists( TYPE aEntry )
        {
            BOOLEAN bRet = FALSE;
            this->AcquireReaderLock();
            for ( PLIST_ENTRY pEntry = m_lsHead.Flink ; pEntry != &m_lsHead ; pEntry = pEntry->Flink )
            {
                MyListEntryMRMW * pMyListEntry = (MyListEntryMRMW *)CONTAINING_RECORD( pEntry , MyListEntryMRMW , lsNext );
                if ( aEntry == pMyListEntry->pEntry )
                {
                    bRet = TRUE;
                    break;
                }
            }
            this->ReleaseReaderLock();
            return bRet;            
        }

        NTSTATUS InsertToHead( TYPE aEntry )
        {
            MyListEntryMRMW * pEntry = (MyListEntryMRMW *)ExAllocatePoolWithTag( NonPagedPool , sizeof(MyListEntryMRMW) , MY_MEM_TAG_UTILS );
            if ( NULL == pEntry )
            {
                return STATUS_INSUFFICIENT_RESOURCES;
            }
            pEntry->pEntry = aEntry;

            this->AcquireWriterLock();
            InsertHeadList( &m_lsHead , &pEntry->lsNext );
            this->ReleaseWriterLock();
            return STATUS_SUCCESS;
        }

        NTSTATUS InsertToTail( TYPE aEntry )
        {
            MyListEntryMRMW * pEntry = (MyListEntryMRMW *)ExAllocatePoolWithTag( NonPagedPool , sizeof(MyListEntryMRMW) , MY_MEM_TAG_UTILS );
            if ( NULL == pEntry )
            {
                return STATUS_INSUFFICIENT_RESOURCES;
            }
            pEntry->pEntry = aEntry;

            this->AcquireWriterLock();
            InsertTailList( &m_lsHead , &pEntry->lsNext );
            this->ReleaseWriterLock();
            return STATUS_SUCCESS;
        }

        VOID RemoveHeadEntry()
        {
            this->AcquireWriterLock();
            PLIST_ENTRY pEntry = RemoveHeadList( &m_lsHead );
            MyListEntryMRMW * pMyListEntry = (MyListEntryMRMW *)CONTAINING_RECORD( pEntry , MyListEntryMRMW , lsNext );
            ExFreePoolWithTag( pMyListEntry , MY_MEM_TAG_UTILS );
            this->ReleaseWriterLock();
        }

        VOID RemoveTailEntry()
        {
            this->AcquireWriterLock();
            PLIST_ENTRY pEntry = RemoveTailList( &m_lsHead );
            MyListEntryMRMW * pMyListEntry = (MyListEntryMRMW *)CONTAINING_RECORD( pEntry , MyListEntryMRMW , lsNext );
            ExFreePoolWithTag( pMyListEntry , MY_MEM_TAG_UTILS );
            this->ReleaseWriterLock();
        }

        VOID RemoveEntry( TYPE aEntry )
        {
            this->AcquireWriterLock();
            for ( PLIST_ENTRY pEntry = m_lsHead.Flink ; pEntry != &m_lsHead ; pEntry = pEntry->Flink )
            {
                MyListEntryMRMW * pMyListEntry = (MyListEntryMRMW *)CONTAINING_RECORD( pEntry , MyListEntryMRMW , lsNext );
                if ( aEntry == pMyListEntry->pEntry )
                {
                    RemoveEntryList( pEntry );
                    ExFreePoolWithTag( pMyListEntry , MY_MEM_TAG_UTILS );
                    break;
                }
            }
            this->ReleaseWriterLock();
        }

        VOID RemoveAllEntries()
        {
            this->AcquireWriterLock();
            while ( ! IsListEmpty( &m_lsHead ) )
            {
                PLIST_ENTRY pEntry = RemoveTailList( &m_lsHead );
                MyListEntryMRMW * pMyListEntry = CONTAINING_RECORD( pEntry , MyListEntryMRMW , lsNext );
                ExFreePoolWithTag( pMyListEntry , MY_MEM_TAG_UTILS );
            }
            this->ReleaseWriterLock();
        }

    protected :
        BOOLEAN AcquireReaderLock()
        {
            KeEnterCriticalRegion();
            return ExAcquireResourceSharedLite( &m_res , TRUE );
        }
        VOID ReleaseReaderLock()
        {
            ExReleaseResourceLite( &m_res );
            KeLeaveCriticalRegion();
        }
        BOOLEAN AcquireWriterLock()
        {
            KeEnterCriticalRegion();
            return ExAcquireResourceExclusiveLite( &m_res , TRUE );
        }
        VOID ReleaseWriterLock()
        {
            ExReleaseResourceLite( &m_res );
            KeLeaveCriticalRegion();
        }

    private :
        ERESOURCE m_res;
        LIST_ENTRY m_lsHead;
};


}   //End of namespace KmUtils
