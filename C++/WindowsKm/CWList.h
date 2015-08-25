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

#include <ntifs.h>

#ifndef INFINITE
    #define INFINITE        0xFFFFFFFF
#endif

#ifndef CW_MEM_TAG_UTILS
    #define CW_MEM_TAG_UTILS        'tUWC'
#endif

namespace KmUtils
{

template <class TYPE>
class CListMRMW   //Multiple-Reader-Multiple-Writer
{
    typedef struct _ListMRMW
    {
        LIST_ENTRY lsNext;
        TYPE pEntry;
    } ListMRMW;

    public :
        CListMRMW() { ExInitializeResourceLite( &m_res ); InitializeListHead( &m_lsHead ); }
        ~CListMRMW() { RemoveAllEntries(); ExDeleteResourceLite( &m_res ); }

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
                ListMRMW * pMyListEntry = (ListMRMW *)CONTAINING_RECORD( pEntry , ListMRMW , lsNext );
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
            ListMRMW * pEntry = (ListMRMW *)ExAllocatePoolWithTag( NonPagedPool , sizeof(ListMRMW) , CW_MEM_TAG_UTILS );
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
            ListMRMW * pEntry = (ListMRMW *)ExAllocatePoolWithTag( NonPagedPool , sizeof(ListMRMW) , CW_MEM_TAG_UTILS );
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
            ListMRMW * pMyListEntry = (ListMRMW *)CONTAINING_RECORD( pEntry , ListMRMW , lsNext );
            ExFreePoolWithTag( pMyListEntry , CW_MEM_TAG_UTILS );
            this->ReleaseWriterLock();
        }

        VOID RemoveTailEntry()
        {
            this->AcquireWriterLock();
            PLIST_ENTRY pEntry = RemoveTailList( &m_lsHead );
            ListMRMW * pMyListEntry = (ListMRMW *)CONTAINING_RECORD( pEntry , ListMRMW , lsNext );
            ExFreePoolWithTag( pMyListEntry , CW_MEM_TAG_UTILS );
            this->ReleaseWriterLock();
        }

        VOID RemoveEntry( TYPE aEntry )
        {
            this->AcquireWriterLock();
            for ( PLIST_ENTRY pEntry = m_lsHead.Flink ; pEntry != &m_lsHead ; pEntry = pEntry->Flink )
            {
                ListMRMW * pMyListEntry = (ListMRMW *)CONTAINING_RECORD( pEntry , ListMRMW , lsNext );
                if ( aEntry == pMyListEntry->pEntry )
                {
                    RemoveEntryList( pEntry );
                    ExFreePoolWithTag( pMyListEntry , CW_MEM_TAG_UTILS );
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
                ListMRMW * pMyListEntry = CONTAINING_RECORD( pEntry , ListMRMW , lsNext );
                ExFreePoolWithTag( pMyListEntry , CW_MEM_TAG_UTILS );
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
