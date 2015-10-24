#include "stdafx.h"
#include "CWHash.h"
using std::string;

namespace CWUtils
{

#ifdef __cplusplus
    extern "C" {
#endif

//Finalization mix - force all bits of a hash block to avalanche
__forceinline UINT32 _FinalizationMix32( UINT32 aNum )
{
    aNum ^= aNum >> 16;
    aNum *= 0x85ebca6b;
    aNum ^= aNum >> 13;
    aNum *= 0xc2b2ae35;
    aNum ^= aNum >> 16;
    return aNum;
}

__forceinline UINT64 _FinalizationMix64( UINT64 aNum )
{
    aNum ^= aNum >> 33;
    aNum *= 0xff51afd7ed558ccd;
    aNum ^= aNum >> 33;
    aNum *= 0xc4ceb9fe1a85ec53;
    aNum ^= aNum >> 33;
    return aNum;
}



BOOL GetFileMd5( CONST WCHAR * aFilePath , std::string & aMd5 )
{
    aMd5.clear();
    BOOL bRet = FALSE;
    HCRYPTPROV hProv = NULL;
    HCRYPTHASH hHash = NULL;
    HANDLE hFile = CreateFileW( aFilePath , GENERIC_READ , FILE_SHARE_READ , NULL , OPEN_EXISTING , FILE_FLAG_SEQUENTIAL_SCAN , NULL );
    if ( INVALID_HANDLE_VALUE == hFile )
    {
        goto exit;
    }
    
    // Get handle to the crypto provider
    if ( FALSE == CryptAcquireContext( &hProv , NULL , NULL , PROV_RSA_FULL , CRYPT_VERIFYCONTEXT ) ||
         FALSE == CryptCreateHash( hProv , CALG_MD5 , 0 , 0 , &hHash ) )
    {
        goto exit;
    }

    BYTE byBuf[8192];
    DWORD dwRead = 0;        
    while ( FALSE != ReadFile( hFile , byBuf , sizeof(byBuf) , &dwRead , NULL ) && 0 < dwRead )
    {
        if ( FALSE == CryptHashData( hHash , byBuf , dwRead , 0 ) )
        {
            goto exit;
        }
    }

    BYTE byHash[16] = { 0 };
    DWORD dwHash = _countof( byHash );
    CHAR szDigits[] = "0123456789abcdef";
    if ( FALSE == CryptGetHashParam( hHash , HP_HASHVAL , byHash , &dwHash , 0 ) )
    {
        goto exit;
    }
    else
    {
        for ( DWORD i = 0 ; i < dwHash ; i++ )
        {
            aMd5.append( 1 , szDigits[byHash[i] >> 4] );
            aMd5.append( 1 , szDigits[byHash[i] & 0xf] );
        }
        bRet = TRUE;
    }

exit :
    if ( INVALID_HANDLE_VALUE != hFile )
    {
        CloseHandle( hFile );
        hFile = NULL;
    }
    if ( NULL != hHash )
    {
        CryptDestroyHash( hHash );
        hHash = NULL;
    }
    if ( NULL != hProv )
    {
        CryptReleaseContext( hProv , 0 );
        hProv = NULL;
    }    
    return bRet; 
}




VOID MurmurHash2_x64_64( CONST VOID * aMem , size_t aMemSize , UINT64 aSeed , UINT64 * a64BitsOutput )
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


VOID MurmurHash3_x86_32( CONST VOID * aMem , size_t aMemSize , UINT32 aSeed , UINT32 * a32BitsOutput )
{
    CONST UCHAR * pData = (CONST UCHAR *)aMem;
    CONST size_t uBlocks = aMemSize / 4;

    UINT32 h1 = aSeed;

    CONST UINT32 c1 = 0xcc9e2d51;
    CONST UINT32 c2 = 0x1b873593;

    //Body
    CONST UINT32 * blocks = (CONST UINT32 *)(pData + uBlocks * 4);
    for ( size_t i = uBlocks ; 0 < i ; i-- )
    {
        UINT32 k1 = *(blocks - i);

        k1 *= c1;    k1 = _rotl( k1 , 15 );    k1 *= c2;
        h1 ^= k1;    h1 = _rotl( h1 , 13 );    h1 = h1 * 5 + 0xe6546b64;
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
            k1 = _rotl( k1 , 15 );
            k1 *= c2;
            h1 ^= k1;
    };

    //Finalization
    h1 ^= aMemSize;
    h1 = _FinalizationMix32( h1 );

    *a32BitsOutput = h1; 
}

VOID MurmurHash3_x86_128( CONST VOID * aMem , size_t aMemSize , UINT32 aSeed , VOID * a128BitsOutput )
{
    CONST UCHAR * pData = (CONST UCHAR *)aMem;
    CONST size_t uBlocks = aMemSize / 16;

    UINT32 h1 = aSeed;
    UINT32 h2 = aSeed;
    UINT32 h3 = aSeed;
    UINT32 h4 = aSeed;

    CONST UINT32 c1 = 0x239b961b;
    CONST UINT32 c2 = 0xab0e9789;
    CONST UINT32 c3 = 0x38b34ae5;
    CONST UINT32 c4 = 0xa1e38b93;

    //Body
    CONST UINT32 * blocks = (CONST UINT32 *)(pData + uBlocks * 16);
    for ( size_t i = uBlocks ; 0 < i ; i-- )
    {
        UINT32 k1 = *(blocks - (i * 4) + 0);
        UINT32 k2 = *(blocks - (i * 4) + 1);
        UINT32 k3 = *(blocks - (i * 4) + 2);
        UINT32 k4 = *(blocks - (i * 4) + 3);

        k1 *= c1;    k1 = _rotl( k1 , 15 );    k1 *= c2;    h1 ^= k1;
        h1 = _rotl( h1 , 19 );    h1 += h2;    h1 = h1 * 5 + 0x561ccd1b;

        k2 *= c2;    k2 = _rotl( k2 , 16 );    k2 *= c3;    h2 ^= k2;
        h2 = _rotl( h2 , 17 );    h2 += h3;    h2 = h2 * 5 + 0x0bcaa747;

        k3 *= c3;    k3 = _rotl( k3 , 17 );    k3 *= c4;    h3 ^= k3;
        h3 = _rotl( h3 , 15 );    h3 += h4;    h3 = h3 * 5 + 0x96cd1c35;

        k4 *= c4;    k4 = _rotl( k4 , 18 );    k4 *= c1;    h4 ^= k4;
        h4 = _rotl( h4 , 13 );    h4 += h1;    h4 = h4 * 5 + 0x32ac3b17;
    }

    //Tail
    CONST UCHAR * tail = (CONST UCHAR *)(pData + uBlocks * 16);
    UINT32 k1 = 0;
    UINT32 k2 = 0;
    UINT32 k3 = 0;
    UINT32 k4 = 0;
    switch ( aMemSize & 15 )
    {
        case 15 :
            k4 ^= tail[14] << 16;
        case 14 :
            k4 ^= tail[13] << 8;
        case 13 :
            k4 ^= tail[12] << 0;
            k4 *= c4;
            k4  = _rotl( k4 , 18 );
            k4 *= c1;
            h4 ^= k4;

        case 12 :
            k3 ^= tail[11] << 24;
        case 11 :
            k3 ^= tail[10] << 16;
        case 10 :
            k3 ^= tail[9] << 8;
        case  9 :
            k3 ^= tail[8] << 0;
            k3 *= c3;
            k3  = _rotl( k3 , 17 );
            k3 *= c4;
            h3 ^= k3;

        case  8 :
            k2 ^= tail[7] << 24;
        case  7 :
            k2 ^= tail[6] << 16;
        case  6 :
            k2 ^= tail[5] << 8;
        case  5 :
            k2 ^= tail[4] << 0;
            k2 *= c2;
            k2  = _rotl( k2 , 16 );
            k2 *= c3;
            h2 ^= k2;

        case  4 :
            k1 ^= tail[3] << 24;
        case  3 :
            k1 ^= tail[2] << 16;
        case  2 :
            k1 ^= tail[1] << 8;
        case  1 :
            k1 ^= tail[0] << 0;
            k1 *= c1;
            k1  = _rotl( k1 , 15 );
            k1 *= c2;
            h1 ^= k1;
    };

    //Finalization
    h1 ^= aMemSize;
    h2 ^= aMemSize;
    h3 ^= aMemSize;
    h4 ^= aMemSize;

    h1 += h2;
    h1 += h3;
    h1 += h4;
    h2 += h1;
    h3 += h1;
    h4 += h1;

    h1 = _FinalizationMix32( h1 );
    h2 = _FinalizationMix32( h2 );
    h3 = _FinalizationMix32( h3 );
    h4 = _FinalizationMix32( h4 );

    h1 += h2;
    h1 += h3;
    h1 += h4;
    h2 += h1;
    h3 += h1;
    h4 += h1;

    ((UINT32*)a128BitsOutput)[0] = h1;
    ((UINT32*)a128BitsOutput)[1] = h2;
    ((UINT32*)a128BitsOutput)[2] = h3;
    ((UINT32*)a128BitsOutput)[3] = h4; 
}


VOID MurmurHash3_x64_128( CONST VOID * aMem , size_t aMemSize , UINT64 aSeed , VOID * a128BitsOutput )
{
    CONST UCHAR * pData = (CONST UCHAR *)aMem;
    CONST size_t uBlocks = aMemSize / 16;

    UINT64 h1 = aSeed;
    UINT64 h2 = aSeed;

    CONST UINT64 c1 = 0x87c37b91114253d5;
    CONST UINT64 c2 = 0x4cf5ad432745937f;

    //Body
    CONST UINT64 * blocks = (CONST UINT64 *)pData;
    for ( size_t i = 0 ; i < uBlocks ; i++ )
    {
        UINT64 k1 = blocks[i * 2 + 0];
        UINT64 k2 = blocks[i * 2 + 1];

        k1 *= c1;    k1 = _rotl64( k1 , 31 );    k1 *= c2;    h1 ^= k1;
        h1 = _rotl64( h1 , 27 );    h1 += h2;    h1 = h1 * 5 + 0x52dce729;

        k2 *= c2;    k2 = _rotl64( k2 , 33 ); k2 *= c1;    h2 ^= k2;
        h2 = _rotl64( h2 , 31 );    h2 += h1;    h2 = h2 * 5 + 0x38495ab5;
    }

    //Tail
    CONST UCHAR * tail = (CONST UCHAR *)(pData + uBlocks * 16);
    UINT64 k1 = 0;
    UINT64 k2 = 0;
    switch ( aMemSize & 15 )
    {
        case 15 :
            k2 ^= ((UINT64)tail[14]) << 48;
        case 14 :
            k2 ^= ((UINT64)tail[13]) << 40;
        case 13 :
            k2 ^= ((UINT64)tail[12]) << 32;
        case 12 :
            k2 ^= ((UINT64)tail[11]) << 24;
        case 11 :
            k2 ^= ((UINT64)tail[10]) << 16;
        case 10 :
            k2 ^= ((UINT64)tail[9]) << 8;
        case  9 :
            k2 ^= ((UINT64)tail[8]) << 0;
            k2 *= c2;
            k2  = _rotl64( k2 , 33 );
            k2 *= c1;
            h2 ^= k2;

        case  8 :
            k1 ^= ((UINT64)tail[7]) << 56;
        case  7 :
            k1 ^= ((UINT64)tail[6]) << 48;
        case  6 :
            k1 ^= ((UINT64)tail[5]) << 40;
        case  5 :
            k1 ^= ((UINT64)tail[4]) << 32;
        case  4 :
            k1 ^= ((UINT64)tail[3]) << 24;
        case  3 :
            k1 ^= ((UINT64)tail[2]) << 16;
        case  2 :
            k1 ^= ((UINT64)tail[1]) << 8;
        case  1 :
            k1 ^= ((UINT64)tail[0]) << 0;
            k1 *= c1;
            k1  = _rotl64( k1 , 31 );
            k1 *= c2;
            h1 ^= k1;
    };

    //Finalization
    h1 ^= aMemSize;
    h2 ^= aMemSize;

    h1 += h2;
    h2 += h1;

    h1 = _FinalizationMix64( h1 );
    h2 = _FinalizationMix64( h2 );

    h1 += h2;
    h2 += h1;

    ((UINT64*)a128BitsOutput)[0] = h1;
    ((UINT64*)a128BitsOutput)[1] = h2; 
}


#ifdef __cplusplus
    }
#endif

}   //End of namespace CWUtils