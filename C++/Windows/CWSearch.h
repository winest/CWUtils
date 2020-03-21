#pragma once

/*
 * Copyright (c) 2009-2020, ChienWei Hung <winestwinest@gmail.com>
 * CWUtils is published under the BSD-3-Clause license.
 *
 * CWUtils is a set of standalone APIs for developers to speed up their
 * programming. It should be very easy to port them to other projects or
 * learn how to implement things on different languages and platforms.
 *
 * The latest version can be found at https://github.com/winest/CWUtils
 */

/*
 * Bloom filter is the technique to speed up matching by telling whether a given pattern is found.
 * If it say it's found, then it really means the pattern has possibility to be found.
 * However, it it say it's not found, then it means the pattern doesn't exist.
 * This can help to reduce the impossible data before performing the full search
 *
 * Boyer-Moore is the string search algorithm. This source includes primitive Boyer-Moore and it's variants
 * Primitive Boyer-Moore create bad-character-shift and good-suffix-shift tables by preprocessing pattern
 * Boyer-Moore-Horspool and Boyer-Moore-Horspool-Raita use only bad-character-shift table, where the latter check the first
 * character soon after checking the last character
 *
 * If the full string has length n, and pattern has length m
 * Use amortize analysis to analyze time complexity depends on comparison times without preprocessing
 * Primitive Boyer-Moore: Best: n/m , Average: theta(n) with higher coefficient , Worst: 4n
 * Primitive Boyer-Moore-Horspool: Best: n/m , Average: theta(n) with lower coefficient , Worst: m*n
 * Primitive Boyer-Moore-Horspool-Raita: Best: n/m , Average: theta(n) with lower coefficient , Worst: m*n
 */

#pragma warning( push, 0 )
#include <Windows.h>
#pragma warning( pop )


namespace CWUtils
{
#ifdef __cplusplus
extern "C" {
#endif


#ifndef _WIN64
typedef VOID ( *BloomFilterHashFunc )( CONST UCHAR * aMem, size_t aMemSize, UINT32 aSeed, UINT32 * a32BitsOutput );
#else
typedef VOID ( *BloomFilterHashFunc )( CONST UCHAR * aMem, size_t aMemSize, UINT64 aSeed, UINT64 * a64BitsOutput );
#endif

class CBloomFilter
{
    public:
    CBloomFilter() : m_bInited( FALSE ), m_pfHashFuncs( NULL ), m_pBitsAry( NULL ) {}
    virtual ~CBloomFilter()
    {
        if ( m_bInited )
        {
            UnInit();
        }
    }

    //If aHashFuncCnt is not optimal, default hash function will be used as well
    //Optimal aHashFuncCnt is -( log(2) * aPatternCnt * log(aFalsePositiveRate) ) / ( pow( log(2) , 2 ) * aPatternCnt )
    BOOL Init( DOUBLE aFalsePositiveRate,
               size_t aPatternCnt,
               BloomFilterHashFunc * aHashFuncs = NULL,
               size_t aHashFuncCnt = 0 );
    BOOL UnInit();
    BOOL AddPattern( CONST UCHAR * aMem, size_t aMemSize );
    BOOL Contains( CONST UCHAR * aMem, size_t aMemSize );

    private:
    //Default hash function if user doesn't specify there own hash function
    //Get detail about MurmurHash on https://code.google.com/p/smhasher/w/list
    static VOID MurmurHash3_x86_32( CONST VOID * aMem, size_t aMemSize, UINT32 aSeed, UINT32 * a32BitsOutput );
    static VOID MurmurHash2_x64_64( CONST VOID * aMem, size_t aMemSize, UINT64 aSeed, UINT64 * a64BitsOutput );

    private:
#ifndef _WIN64
    typedef UINT32 BITS_ARY_UNIT;
#else
    typedef UINT64 BITS_ARY_UNIT;
#endif
    BOOL m_bInited;

    size_t m_uFilterSize;
    size_t m_uPatternCnt;

    BloomFilterHashFunc * m_pfHashFuncs;
    size_t m_uUserHashFuncCnt;
    size_t m_uAllHashFuncCnt;

    BITS_ARY_UNIT * m_pBitsAry;
};


inline INT SearchByte( CONST UCHAR * aMemory, size_t aMemorySize, UCHAR aPattern );
INT BinarySearch( INT * aArray, INT aArrayLen, INT aKey );

//Return the index to the first occurrence of "aPattern" within "aMemory", or -1 if not found
INT BoyerMoore( CONST UCHAR * aMemory, size_t aMemorySize, CONST UCHAR * aPattern, size_t aPatternSize );
INT BoyerMooreHorspool( CONST UCHAR * aMemory, size_t aMemorySize, CONST UCHAR * aPattern, size_t aPatternSize );
INT BoyerMooreHorspoolRaita( CONST UCHAR * aMemory, size_t aMemorySize, CONST UCHAR * aPattern, size_t aPatternSize );



//Here is the utility function used in real case
typedef struct _BoyerMooreInfo
{
    CONST UCHAR * Pattern;
    size_t PatternSize;
    size_t BadCharShift[256];    //One byte has at most 2^8=256 values
} BoyerMooreInfo;

VOID InitPatternInfo( CONST UCHAR * aPattern, size_t aPatternSize, BoyerMooreInfo * aPatternInfo );

//Return the index to the first occurrence of "aPattern" within "aMemory", or -1 if not found
INT SearchPatternByBmOnce( CONST UCHAR * aMemory, size_t aMemorySize, CONST UCHAR * aPattern, size_t aPatternSize );
INT SearchPatternByBm( CONST UCHAR * aMemory, size_t aMemorySize, CONST BoyerMooreInfo * aBmInfo );




#ifdef __cplusplus
}
#endif

}    //End of namespace CWUtils