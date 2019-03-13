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



BOOL GetCrc32( CONST UCHAR * aBuf, SIZE_T aBufSize, UINT32 & aCrc32 )
{
    aCrc32 = 0 ^ ( -1 );
    static UINT32 hexTable[] = {
        0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3, 0x0EDB8832,
        0x79DCB8A4, 0xE0D5E91E, 0x97D2D988, 0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91, 0x1DB71064, 0x6AB020F2,
        0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7, 0x136C9856, 0x646BA8C0, 0xFD62F97A,
        0x8A65C9EC, 0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5, 0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
        0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B, 0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940, 0x32D86CE3,
        0x45DF5C75, 0xDCD60DCF, 0xABD13D59, 0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423,
        0xCFBA9599, 0xB8BDA50F, 0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924, 0x2F6F7C87, 0x58684C11, 0xC1611DAB,
        0xB6662D3D, 0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
        0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01, 0x6B6B51F4,
        0x1C6C6162, 0x856530D8, 0xF262004E, 0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457, 0x65B0D9C6, 0x12B7E950,
        0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65, 0x4DB26158, 0x3AB551CE, 0xA3BC0074,
        0xD4BB30E2, 0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB, 0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
        0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9, 0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086, 0x5768B525,
        0x206F85B3, 0xB966D409, 0xCE61E49F, 0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81,
        0xB7BD5C3B, 0xC0BA6CAD, 0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A, 0xEAD54739, 0x9DD277AF, 0x04DB2615,
        0x73DC1683, 0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
        0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7, 0xFED41B76,
        0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC, 0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5, 0xD6D6A3E8, 0xA1D1937E,
        0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B, 0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6,
        0x41047A60, 0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79, 0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
        0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F, 0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7,
        0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D, 0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F,
        0x72076785, 0x05005713, 0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38, 0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7,
        0x0BDBDF21, 0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
        0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45, 0xA00AE278,
        0xD70DD2EE, 0x4E048354, 0x3903B3C2, 0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB, 0xAED16A4A, 0xD9D65ADC,
        0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9, 0xBDBDF21C, 0xCABAC28A, 0x53B39330,
        0x24B4A3A6, 0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF, 0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
        0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
    };

    for ( SIZE_T i = 0; i < aBufSize; i++ )
    {
        aCrc32 = ( aCrc32 >> 8 ) ^ hexTable[( aCrc32 ^ aBuf[i] ) & 0xFF];
    }

    aCrc32 = ( aCrc32 ^ ( -1 ) );
    return TRUE;
}



BOOL GetFileMd5( CONST WCHAR * aFilePath, std::string & aMd5 )
{
    aMd5.clear();
    BOOL bRet = FALSE;
    HCRYPTPROV hProv = NULL;
    HCRYPTHASH hHash = NULL;
    HANDLE hFile =
        CreateFileW( aFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL );
    if ( INVALID_HANDLE_VALUE == hFile )
    {
        goto exit;
    }

    // Get handle to the crypto provider
    if ( FALSE == CryptAcquireContext( &hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT ) ||
         FALSE == CryptCreateHash( hProv, CALG_MD5, 0, 0, &hHash ) )
    {
        goto exit;
    }

    BYTE byBuf[8192];
    DWORD dwRead = 0;
    while ( FALSE != ReadFile( hFile, byBuf, sizeof( byBuf ), &dwRead, NULL ) && 0 < dwRead )
    {
        if ( FALSE == CryptHashData( hHash, byBuf, dwRead, 0 ) )
        {
            goto exit;
        }
    }

    BYTE byHash[16] = { 0 };
    DWORD dwHash = _countof( byHash );
    CHAR szDigits[] = "0123456789abcdef";
    if ( FALSE == CryptGetHashParam( hHash, HP_HASHVAL, byHash, &dwHash, 0 ) )
    {
        goto exit;
    }
    else
    {
        for ( DWORD i = 0; i < dwHash; i++ )
        {
            aMd5.append( 1, szDigits[byHash[i] >> 4] );
            aMd5.append( 1, szDigits[byHash[i] & 0xf] );
        }
        bRet = TRUE;
    }

exit:
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
        CryptReleaseContext( hProv, 0 );
        hProv = NULL;
    }
    return bRet;
}




VOID MurmurHash2_x64_64( CONST VOID * aMem, size_t aMemSize, UINT64 aSeed, UINT64 * a64BitsOutput )
{
    CONST UINT64 m = 0xc6a4a7935bd1e995;
    CONST INT r = 47;
    UINT64 h = aSeed ^ ( aMemSize * m );

    CONST UINT64 * pData = (CONST UINT64 *)aMem;
    CONST UINT64 * pEnd = pData + ( aMemSize / 8 );
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
        case 7:
            h ^= UINT64( pData2[6] ) << 48;
        case 6:
            h ^= UINT64( pData2[5] ) << 40;
        case 5:
            h ^= UINT64( pData2[4] ) << 32;
        case 4:
            h ^= UINT64( pData2[3] ) << 24;
        case 3:
            h ^= UINT64( pData2[2] ) << 16;
        case 2:
            h ^= UINT64( pData2[1] ) << 8;
        case 1:
            h ^= UINT64( pData2[0] );
            h *= m;
    };

    h ^= h >> r;
    h *= m;
    h ^= h >> r;

    *( (UINT64 *)a64BitsOutput ) = h;
}


VOID MurmurHash3_x86_32( CONST VOID * aMem, size_t aMemSize, UINT32 aSeed, UINT32 * a32BitsOutput )
{
    CONST UCHAR * pData = (CONST UCHAR *)aMem;
    CONST size_t uBlocks = aMemSize / 4;

    UINT32 h1 = aSeed;

    CONST UINT32 c1 = 0xcc9e2d51;
    CONST UINT32 c2 = 0x1b873593;

    //Body
    CONST UINT32 * blocks = (CONST UINT32 *)( pData + uBlocks * 4 );
    for ( size_t i = uBlocks; 0 < i; i-- )
    {
        UINT32 k1 = *( blocks - i );

        k1 *= c1;
        k1 = _rotl( k1, 15 );
        k1 *= c2;
        h1 ^= k1;
        h1 = _rotl( h1, 13 );
        h1 = h1 * 5 + 0xe6546b64;
    }

    //Tail
    CONST UCHAR * tail = (CONST UCHAR *)( pData + uBlocks * 4 );
    UINT32 k1 = 0;
    switch ( aMemSize & 3 )
    {
        case 3:
            k1 ^= tail[2] << 16;
        case 2:
            k1 ^= tail[1] << 8;
        case 1:
            k1 ^= tail[0];
            k1 *= c1;
            k1 = _rotl( k1, 15 );
            k1 *= c2;
            h1 ^= k1;
    };

    //Finalization
    h1 ^= aMemSize;
    h1 = _FinalizationMix32( h1 );

    *a32BitsOutput = h1;
}

VOID MurmurHash3_x86_128( CONST VOID * aMem, size_t aMemSize, UINT32 aSeed, VOID * a128BitsOutput )
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
    CONST UINT32 * blocks = (CONST UINT32 *)( pData + uBlocks * 16 );
    for ( size_t i = uBlocks; 0 < i; i-- )
    {
        UINT32 k1 = *( blocks - ( i * 4 ) + 0 );
        UINT32 k2 = *( blocks - ( i * 4 ) + 1 );
        UINT32 k3 = *( blocks - ( i * 4 ) + 2 );
        UINT32 k4 = *( blocks - ( i * 4 ) + 3 );

        k1 *= c1;
        k1 = _rotl( k1, 15 );
        k1 *= c2;
        h1 ^= k1;
        h1 = _rotl( h1, 19 );
        h1 += h2;
        h1 = h1 * 5 + 0x561ccd1b;

        k2 *= c2;
        k2 = _rotl( k2, 16 );
        k2 *= c3;
        h2 ^= k2;
        h2 = _rotl( h2, 17 );
        h2 += h3;
        h2 = h2 * 5 + 0x0bcaa747;

        k3 *= c3;
        k3 = _rotl( k3, 17 );
        k3 *= c4;
        h3 ^= k3;
        h3 = _rotl( h3, 15 );
        h3 += h4;
        h3 = h3 * 5 + 0x96cd1c35;

        k4 *= c4;
        k4 = _rotl( k4, 18 );
        k4 *= c1;
        h4 ^= k4;
        h4 = _rotl( h4, 13 );
        h4 += h1;
        h4 = h4 * 5 + 0x32ac3b17;
    }

    //Tail
    CONST UCHAR * tail = (CONST UCHAR *)( pData + uBlocks * 16 );
    UINT32 k1 = 0;
    UINT32 k2 = 0;
    UINT32 k3 = 0;
    UINT32 k4 = 0;
    switch ( aMemSize & 15 )
    {
        case 15:
            k4 ^= tail[14] << 16;
        case 14:
            k4 ^= tail[13] << 8;
        case 13:
            k4 ^= tail[12] << 0;
            k4 *= c4;
            k4 = _rotl( k4, 18 );
            k4 *= c1;
            h4 ^= k4;

        case 12:
            k3 ^= tail[11] << 24;
        case 11:
            k3 ^= tail[10] << 16;
        case 10:
            k3 ^= tail[9] << 8;
        case 9:
            k3 ^= tail[8] << 0;
            k3 *= c3;
            k3 = _rotl( k3, 17 );
            k3 *= c4;
            h3 ^= k3;

        case 8:
            k2 ^= tail[7] << 24;
        case 7:
            k2 ^= tail[6] << 16;
        case 6:
            k2 ^= tail[5] << 8;
        case 5:
            k2 ^= tail[4] << 0;
            k2 *= c2;
            k2 = _rotl( k2, 16 );
            k2 *= c3;
            h2 ^= k2;

        case 4:
            k1 ^= tail[3] << 24;
        case 3:
            k1 ^= tail[2] << 16;
        case 2:
            k1 ^= tail[1] << 8;
        case 1:
            k1 ^= tail[0] << 0;
            k1 *= c1;
            k1 = _rotl( k1, 15 );
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

    ( (UINT32 *)a128BitsOutput )[0] = h1;
    ( (UINT32 *)a128BitsOutput )[1] = h2;
    ( (UINT32 *)a128BitsOutput )[2] = h3;
    ( (UINT32 *)a128BitsOutput )[3] = h4;
}


VOID MurmurHash3_x64_128( CONST VOID * aMem, size_t aMemSize, UINT64 aSeed, VOID * a128BitsOutput )
{
    CONST UCHAR * pData = (CONST UCHAR *)aMem;
    CONST size_t uBlocks = aMemSize / 16;

    UINT64 h1 = aSeed;
    UINT64 h2 = aSeed;

    CONST UINT64 c1 = 0x87c37b91114253d5;
    CONST UINT64 c2 = 0x4cf5ad432745937f;

    //Body
    CONST UINT64 * blocks = (CONST UINT64 *)pData;
    for ( size_t i = 0; i < uBlocks; i++ )
    {
        UINT64 k1 = blocks[i * 2 + 0];
        UINT64 k2 = blocks[i * 2 + 1];

        k1 *= c1;
        k1 = _rotl64( k1, 31 );
        k1 *= c2;
        h1 ^= k1;
        h1 = _rotl64( h1, 27 );
        h1 += h2;
        h1 = h1 * 5 + 0x52dce729;

        k2 *= c2;
        k2 = _rotl64( k2, 33 );
        k2 *= c1;
        h2 ^= k2;
        h2 = _rotl64( h2, 31 );
        h2 += h1;
        h2 = h2 * 5 + 0x38495ab5;
    }

    //Tail
    CONST UCHAR * tail = (CONST UCHAR *)( pData + uBlocks * 16 );
    UINT64 k1 = 0;
    UINT64 k2 = 0;
    switch ( aMemSize & 15 )
    {
        case 15:
            k2 ^= ( (UINT64)tail[14] ) << 48;
        case 14:
            k2 ^= ( (UINT64)tail[13] ) << 40;
        case 13:
            k2 ^= ( (UINT64)tail[12] ) << 32;
        case 12:
            k2 ^= ( (UINT64)tail[11] ) << 24;
        case 11:
            k2 ^= ( (UINT64)tail[10] ) << 16;
        case 10:
            k2 ^= ( (UINT64)tail[9] ) << 8;
        case 9:
            k2 ^= ( (UINT64)tail[8] ) << 0;
            k2 *= c2;
            k2 = _rotl64( k2, 33 );
            k2 *= c1;
            h2 ^= k2;

        case 8:
            k1 ^= ( (UINT64)tail[7] ) << 56;
        case 7:
            k1 ^= ( (UINT64)tail[6] ) << 48;
        case 6:
            k1 ^= ( (UINT64)tail[5] ) << 40;
        case 5:
            k1 ^= ( (UINT64)tail[4] ) << 32;
        case 4:
            k1 ^= ( (UINT64)tail[3] ) << 24;
        case 3:
            k1 ^= ( (UINT64)tail[2] ) << 16;
        case 2:
            k1 ^= ( (UINT64)tail[1] ) << 8;
        case 1:
            k1 ^= ( (UINT64)tail[0] ) << 0;
            k1 *= c1;
            k1 = _rotl64( k1, 31 );
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

    ( (UINT64 *)a128BitsOutput )[0] = h1;
    ( (UINT64 *)a128BitsOutput )[1] = h2;
}


#ifdef __cplusplus
}
#endif

}    //End of namespace CWUtils