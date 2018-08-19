#include "stdafx.h"
#include "CWSearch.h"


namespace CWUtils
{


inline VOID _ComputeSuffix( CONST UCHAR * aPattern , size_t aPatternSize , INT * aSuffix )
{
    size_t j = aPatternSize - 2;
    size_t k = 0;

    aSuffix[aPatternSize - 1] = aPatternSize;
    for ( size_t i = aPatternSize - 2 ; i >= 0 ; i-- ) 
    {
        if ( i > j && aSuffix[aPatternSize - 1 + i - k] < i - j )
        {
            aSuffix[i] = aSuffix[aPatternSize - 1 + i - k];
        }
        else
        {
            if ( i < j )
            {
                j = i;
            }

            k = i;

            while ( j >= 0 && aPattern[j] == aPattern[aPatternSize - 1 + j - i] )
            {
                j--;
            }

            aSuffix[i] = i - j;
        }
    }
}
 
inline VOID _ComputeGoodSuffixShift( CONST UCHAR * aPattern , size_t aPatternSize , INT * aGoodSuffix ) 
{
    INT * suffix = new (std::nothrow) INT[aPatternSize];
    _ComputeSuffix( aPattern , aPatternSize , suffix );

    for ( size_t i = 0 ; i < aPatternSize ; i++ )
    {
        aGoodSuffix[i] = aPatternSize;
    }

    INT j = 0;
    for ( INT i = (INT)aPatternSize - 1 ; i >= 0 ; i-- )
    {
        if ( suffix[i] == i + 1 )
        {
            for ( ; j < (INT)aPatternSize - 1 - i ; j++ )
            {
                if ( aGoodSuffix[j] == aPatternSize )
                {
                    aGoodSuffix[j] = aPatternSize - 1 - i;
                }
            }
        }
    }

    for ( size_t i = 0 ; i <= aPatternSize - 2 ; i++ )
    {
        aGoodSuffix[aPatternSize - 1 - suffix[i]] = aPatternSize - 1 - i;
    }

    delete [] suffix;
}


inline VOID _ComputeBadCharacterShift( const UCHAR * aPattern , size_t aPatternSize , size_t * aBadChar ) 
{
    //Initialize the table to default value: when a character is encountered that does not occur
    //in the aPattern, we can safely skip ahead for the whole length of the aPattern
    size_t last = aPatternSize - 1;

    for ( size_t i = 0 ; i < 256 ; i++ )
        aBadChar[i] = aPatternSize;
    for ( size_t i = 0 ; i < aPatternSize - 1 ; i++ )
        aBadChar[ aPattern[i] ] = last - i;
}




BOOL CBloomFilter::Init( DOUBLE aFalsePositiveRate , size_t aPatternCnt , BloomFilterHashFunc * aHashFuncs , size_t aHashFuncCnt )
{
    //fFilterSize and fHashFuncCnt are counted according to http://en.wikipedia.org/wiki/Bloom_filter
    CONST DOUBLE fFilterSize = -(aPatternCnt * log( aFalsePositiveRate )) / pow( log(2.0) , 2.0 );
    CONST DOUBLE fAllHashFuncCnt = log( 2.0 ) * fFilterSize / aPatternCnt;
    CONST size_t uFilterSize = (size_t)( fFilterSize + 0.5 );
    CONST size_t uAllHashFuncCnt = (size_t)( fAllHashFuncCnt + 0.5 );
    CONST size_t uBitsAryLen = (uFilterSize + (CHAR_BIT * sizeof(BITS_ARY_UNIT) - 1)) / (CHAR_BIT * sizeof(BITS_ARY_UNIT));

    m_pBitsAry = (BITS_ARY_UNIT *)malloc( uBitsAryLen * sizeof(BITS_ARY_UNIT) );
    if ( NULL == m_pBitsAry )
    {
        goto exit;
    }

    m_pfHashFuncs = (BloomFilterHashFunc *)malloc( uAllHashFuncCnt * sizeof(BloomFilterHashFunc) );
    if ( NULL == m_pfHashFuncs )
    {
        goto exit;
    }    

    memset( m_pBitsAry , 0 , uBitsAryLen * sizeof(BITS_ARY_UNIT) );
    memset( m_pfHashFuncs , 0 , uAllHashFuncCnt * sizeof(BloomFilterHashFunc) );
    
    m_uFilterSize = uFilterSize;
    m_uPatternCnt = 0;
    m_uAllHashFuncCnt = uAllHashFuncCnt;

    m_uUserHashFuncCnt = min( aHashFuncCnt , uAllHashFuncCnt );
    for ( size_t i = 0 ; i < m_uUserHashFuncCnt ; i++ )
    {
        assert( NULL != aHashFuncs[i] );
        m_pfHashFuncs[i] = aHashFuncs[i];
    }
    for ( size_t i = m_uUserHashFuncCnt ; i < m_uAllHashFuncCnt ; i++ )
    {
        #ifndef __x86_64__
            m_pfHashFuncs[i] = (BloomFilterHashFunc)CBloomFilter::MurmurHash3_x86_32;
        #else
            m_pfHashFuncs[i] = (BloomFilterHashFunc)CBloomFilter::MurmurHash2_x64_64;
        #endif
    }
    
    m_bInited = TRUE;

exit :
    if ( FALSE == m_bInited )
    {
        if ( NULL != m_pfHashFuncs )
        {
            free( m_pfHashFuncs );
            m_pfHashFuncs = NULL;
        }
        if ( NULL != m_pBitsAry )
        {
            free( m_pBitsAry );
            m_pBitsAry = NULL;
        }
    }
    return m_bInited;
}


BOOL CBloomFilter::UnInit()
{
    if ( m_bInited )
    {
        if ( NULL != m_pBitsAry )
        {
            free( m_pBitsAry );
            m_pBitsAry = NULL;
        }
        if ( NULL != m_pfHashFuncs )
        {
            free( m_pfHashFuncs );
            m_pfHashFuncs = NULL;
        }
        m_bInited = FALSE;
    }
    return TRUE;
}



BOOL CBloomFilter::AddPattern( CONST UCHAR * aMem , size_t aMemSize )
{
    if ( FALSE == m_bInited )
    {
        return FALSE;
    }

    //Repeatedly hash the string, and set m_pBitsAry in the Bloom filter's bit array
    for ( size_t i = 0 ; i < m_uAllHashFuncCnt ; i++ )
    {
        size_t uHash;
        m_pfHashFuncs[i]( aMem , aMemSize , (UINT32)i , &uHash );
        CONST size_t pos = uHash % m_uFilterSize;
        CONST size_t slot = pos / (CHAR_BIT * sizeof(BITS_ARY_UNIT));
        CONST size_t bit = pos % (CHAR_BIT * sizeof(BITS_ARY_UNIT));

        m_pBitsAry[slot] |= (size_t)1 << bit;
    }
    m_uPatternCnt++;
    return TRUE;
}

BOOL CBloomFilter::Contains( CONST UCHAR * aMem , size_t aMemSize )
{
    for ( size_t i = 0 ; i < m_uAllHashFuncCnt ; i++ )
    {
        size_t uHash;
        m_pfHashFuncs[i]( aMem , aMemSize , (UINT32)i , &uHash );
        CONST size_t pos = uHash % m_uFilterSize;
        CONST size_t slot = pos / (CHAR_BIT * sizeof(BITS_ARY_UNIT));
        CONST size_t bit = pos % (CHAR_BIT * sizeof(BITS_ARY_UNIT));

        //If a bit is not set, the element is not contained
        if ( 0 == (m_pBitsAry[slot] & ((size_t)1 << bit)) )
        {
            return FALSE;
        }
    }
    //If a bit is set, the element is possibly contained
    return TRUE;
}



VOID CBloomFilter::MurmurHash3_x86_32( CONST VOID * aMem , size_t aMemSize , UINT32 aSeed , UINT32 * a32BitsOutput )
{
    CONST UCHAR * pData = (CONST UCHAR *)aMem;
    CONST size_t uBlocks = aMemSize / 4;

    UINT32 h1 = aSeed;

    CONST UINT32 c1 = 0xcc9e2d51;
    CONST UINT32 c2 = 0x1b873593;
    CONST UINT32 mask = (CHAR_BIT*sizeof(UINT32) - 1);

    //Body
    CONST UINT32 * blocks = (CONST UINT32 *)(pData + uBlocks * 4);
    for ( size_t i = uBlocks ; 0 < i ; i-- )
    {
        UINT32 k1 = *(blocks - i);

        k1 *= c1;    k1 = (k1<<15) | (k1>>( (-15)&mask ));    k1 *= c2;
        h1 ^= k1;    h1 = (h1<<13) | (h1>>( (-13)&mask ));    h1 = h1 * 5 + 0xe6546b64;
    }

    //Tail
    CONST UCHAR * tail = (CONST UCHAR *)(pData + uBlocks * 4);
    UINT32 k1 = 0;
    switch ( aMemSize & 3 )
    {
        case 3 :
            k1 ^= tail[2] << 16;
        case 2 :
            k1 ^= tail[1] << 8;
        case 1 :
            k1 ^= tail[0];
            k1 *= c1;
            k1 = (k1<<15) | (k1>>( (-15)&mask ));
            k1 *= c2;
            h1 ^= k1;
    };

    //Finalization
    h1 ^= aMemSize;
    h1 ^= h1 >> 16;    h1 *= 0x85ebca6b;    h1 ^= h1 >> 13;    h1 *= 0xc2b2ae35;    h1 ^= h1 >> 16;

    *a32BitsOutput = h1; 
}


VOID CBloomFilter::MurmurHash2_x64_64( CONST VOID * aMem , size_t aMemSize , UINT64 aSeed , UINT64 * a64BitsOutput )
{
    CONST UINT64 m = 0xc6a4a7935bd1e995;
    CONST INT r = 47;
    UINT64 h = aSeed ^ (aMemSize * m);

    CONST UINT64 * pData = (CONST UINT64 *)aMem;
    CONST UINT64 * pEnd = pData + (aMemSize / 8);
    while ( pData != pEnd )
    {
        UINT64 k = *pData++;

        k *= m;
        k ^= k >> r;
        k *= m;

        h ^= k;
        h *= m;
    }

    CONST UCHAR * pData2 = (CONST UCHAR *)pData;
    switch ( aMemSize & 7 )
    {
        case 7 :
            h ^= UINT64( pData2[6] ) << 48;
        case 6 :
            h ^= UINT64( pData2[5] ) << 40;
        case 5 :
            h ^= UINT64( pData2[4] ) << 32;
        case 4 :
            h ^= UINT64( pData2[3] ) << 24;
        case 3 :
            h ^= UINT64( pData2[2] ) << 16;
        case 2 :
            h ^= UINT64( pData2[1] ) << 8;
        case 1 :
            h ^= UINT64( pData2[0] );
            h *= m;
    };

    h ^= h >> r;
    h *= m;
    h ^= h >> r;

    *((UINT64 *)a64BitsOutput) = h;
}







inline INT SearchByte( CONST UCHAR * aMemory , size_t aMemorySize , UCHAR aPattern )
{
    size_t i;
    for ( i = 0 ; i < aMemorySize ; i++ )
    {
        if ( aMemory[i] == aPattern )
        {
            return (INT)i;
        }
    }
    return -1;
}


INT BinarySearch( INT * aArray , INT aArrayLen , INT aKey )
{
    INT nStart = 0 , nEnd = aArrayLen - 1;
    
    while ( aKey != aArray[( nEnd + nStart ) / 2] && nEnd >= nStart )
    {
         if ( aArray[( nEnd + nStart ) / 2] > aKey )
         {
              nEnd = ( nEnd + nStart ) / 2 - 1;
         }
         else
         {
              nStart = ( nEnd + nStart ) / 2 + 1;
         }
    }

    if ( aArray[ ( nEnd + nStart ) / 2 ] != aKey )
    {
       return -1;
    }
    else
    {
       return ( ( nEnd + nStart ) / 2 );
    }
}










INT BoyerMoore( CONST UCHAR * aMemory , size_t aMemorySize , CONST UCHAR * aPattern , size_t aPatternSize )
{
    //Check parameters
    if ( NULL == aMemory || 0 == aMemorySize || NULL == aPattern )
    {
        return -1;
    }
    else if ( 0 == aPatternSize )
    {
        return 0;
    }
    else if ( 1 == aPatternSize )
    {
        return SearchByte( aMemory , aMemorySize , aPattern[0] );
    }
    else{}



    size_t badChar[256];
    INT * goodSuffix = new (std::nothrow) INT[aPatternSize];

    //Initialize the table to default value: when a character is encountered that does not occur
    //in the aPattern, we can safely skip ahead for the whole length of the aPattern
    _ComputeBadCharacterShift( aPattern , aPatternSize , badChar );

    //Good suffix shift
    _ComputeGoodSuffixShift( aPattern , aPatternSize , goodSuffix );



    size_t scan;
    size_t last = aPatternSize - 1;

    //Boyer-Moore algorithm
    CONST UCHAR * pCurrMem = aMemory;    //We move pCurrMem instead of aMemory
    while ( aPatternSize <= aMemorySize )
    {
        for ( scan = last ; pCurrMem[scan] == aPattern[scan] ; scan-- )
        {
            if ( scan == 0 )
            {
                delete [] goodSuffix;
                return pCurrMem - aMemory;    //Calculate the index
            }
        }

        INT move = max( goodSuffix[scan] , (INT)badChar[ pCurrMem[scan] ] );
        aMemorySize -= move;
        pCurrMem += move;
    }

    delete [] goodSuffix;
    return -1;
}



INT BoyerMooreHorspool( CONST UCHAR * aMemory , size_t aMemorySize , CONST UCHAR * aPattern , size_t aPatternSize )
{
    //Check parameters
    if ( NULL == aMemory || 0 == aMemorySize || NULL == aPattern )
    {
        return -1;
    }
    else if ( 0 == aPatternSize )
    {
        return 0;
    }
    else if ( 1 == aPatternSize )
    {
        return SearchByte( aMemory , aMemorySize , aPattern[0] );
    }
    else{}


    
    size_t badChar[256];    //A byte could have 2^8 = 256 values
 
    //Initialize the table to default value: when a character is encountered that does not occur
    //in the aPattern, we can safely skip ahead for the whole length of the aPattern
    _ComputeBadCharacterShift( aPattern , aPatternSize , badChar );
 


    size_t scan;
    size_t last = aPatternSize - 1;
    //Boyer-Moore-Horspool algorithm
    CONST UCHAR * pCurrMem = aMemory;    //We move pCurrMem instead of aMemory
    while ( aPatternSize <= aMemorySize )
    {
        //Scan from the end of the aPattern
        for ( scan = last ; pCurrMem[scan] == aPattern[scan] ; scan-- )
        {
            if ( scan == 0 ) //If the first byte matches, we've found it
            {
                return ( pCurrMem - aMemory );    //Calculate the index
            }
        }
 
        //If a mismatch is found, skip an appropriate range
        //Here we are getting the skip value based on the last byte of aPattern, no matter where we didn't match.
        //So if aPattern is: "abcd" then we are skipping based on 'd' and that value will be 4, and for "abcdd" we 
        //again skip on 'd' but the value will be only 1.
        //The alternative of pretending that the mismatched character was the last character is slower 
        //in the normal case (Eg. finding "abcd" in "...azcd..." gives 4 by using 'd' but only 4-2==2 using 'z')
        //If you don't believe, try following
        //aMemorySize -= badCharSkip[ aMemory[scan] ];
        //pCurrMem += badCharSkip[ aMemory[scan] ];

        aMemorySize -= badChar[ pCurrMem[last] ];
        pCurrMem += badChar[ pCurrMem[last] ];
    }
 
    return -1;
}


INT BoyerMooreHorspoolRaita( CONST UCHAR * aMemory , size_t aMemorySize , CONST UCHAR * aPattern , size_t aPatternSize )
{
    //Check parameters
    if ( NULL == aMemory || 0 == aMemorySize || NULL == aPattern )
    {
        return -1;
    }
    else if ( 0 == aPatternSize )
    {
        return 0;
    }
    else if ( 1 == aPatternSize )
    {
        return SearchByte( aMemory , aMemorySize , aPattern[0] );
    }
    else{}

    

    size_t badChar[256];    //A byte could have 2^8 = 256 values    
 
    //Initialize the table to default value: when a character is encountered that does not occur
    //in the aPattern, we can safely skip ahead for the whole length of the aPattern
    _ComputeBadCharacterShift( aPattern , aPatternSize , badChar );



    size_t scan;
    size_t last = aPatternSize - 1;
    size_t secLast = aPatternSize - 2;
    //Boyer-Moore-Horspool-Raita algorithm
    CONST UCHAR * pCurrMem = aMemory;    //We move pCurrMem instead of aMemory
    while  ( aPatternSize <= aMemorySize )
    {
        if ( pCurrMem[last] == aPattern[last] )    //Scan the last character
        {
            if ( pCurrMem[0] == aPattern[0] )    //Scan the first character
            {
                //Scan from the second last character to the second character
                for ( scan = secLast ; pCurrMem[scan] == aPattern[scan] ; scan-- )
                {
                    if ( scan <= 1 )    //Since we have tested 0
                    {
                        return ( pCurrMem - aMemory );    //Calculate the index
                    }
                }
            }
        }

        aMemorySize -= badChar[ pCurrMem[last] ];
        pCurrMem += badChar[ pCurrMem[last] ];
    }

    return -1;
}





VOID InitPatternInfo( CONST UCHAR * aPattern , size_t aPatternSize , BoyerMooreInfo * aPatternInfo )
{
    aPatternInfo->Pattern = aPattern;
    aPatternInfo->PatternSize = aPatternSize;
    _ComputeBadCharacterShift( aPattern , aPatternSize , aPatternInfo->BadCharShift );
}

INT SearchPatternByBmOnce( CONST UCHAR * aMemory , size_t aMemorySize , CONST UCHAR * aPattern , size_t aPatternSize )
{
    BoyerMooreInfo patternInfo;
    InitPatternInfo( aPattern , aPatternSize , &patternInfo );
    return SearchPatternByBm( aMemory , aMemorySize , &patternInfo );
}

INT SearchPatternByBm( CONST UCHAR * aMemory , size_t aMemorySize , CONST BoyerMooreInfo * aBmInfo )
{
    //Check parameters
    if ( NULL == aMemory || NULL == aBmInfo || NULL == aBmInfo->Pattern )
    {
        return -1;
    }
    else if ( 0 == aBmInfo->PatternSize )
    {
        return 0;
    }
    else if ( 1 == aBmInfo->PatternSize )
    {
        return SearchByte( aMemory , aMemorySize , aBmInfo->Pattern[0] );
    }
    else{}

    size_t scan;
    size_t last = aBmInfo->PatternSize - 1;
    size_t secLast = aBmInfo->PatternSize - 2;
    //Boyer-Moore-Horspool-Raita algorithm
    CONST UCHAR * pCurrMem = aMemory;    //We move pCurrMem instead of aMemory
    while ( aBmInfo->PatternSize <= aMemorySize )
    {
        if ( pCurrMem[last] == aBmInfo->Pattern[last] )    //Scan the last character
        {
            if ( pCurrMem[0] == aBmInfo->Pattern[0] )    //Scan the first character
            {
                //Scan from the second last character to the second character
                for ( scan = secLast ; pCurrMem[scan] == aBmInfo->Pattern[scan] ; scan-- )
                {
                    if ( scan <= 1 )    //Since we have tested 0
                    {
                        return (int)( pCurrMem - aMemory );    //Calculate the index
                    }
                }
            }
        }

        aMemorySize -= aBmInfo->BadCharShift[ pCurrMem[last] ];
        pCurrMem += aBmInfo->BadCharShift[ pCurrMem[last] ];
    }

    return -1;
}








}   //End of namespace CWUtils
