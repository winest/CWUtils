#include "stdafx.h"
#include "CWCrypto.h"

using namespace std;

namespace CWUtils
{
#ifdef __cplusplus
extern "C" {
#endif



VOID Base64Encode( CONST UCHAR * aInput, SIZE_T aInputSize, std::string & aOutput, CONST CHAR * aEncodeTable )
{
    aOutput.clear();
    aOutput.reserve( ( ( aInputSize + 2 ) / 3 ) * 4 );
    CONST CHAR * pEncTable =
        ( aEncodeTable ) ? aEncodeTable : "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
    SIZE_T uLeft = aInputSize;
    SIZE_T uCurr = 0;
    for ( ; uCurr < aInputSize; uCurr += 3 )
    {
        uLeft = aInputSize - uCurr;
        if ( 1 == uLeft || 2 == uLeft )
        {
            break;
        }
        int nEnc1 = aInput[uCurr] >> 2;
        int nEnc2 = ( ( aInput[uCurr] & 3 ) << 4 | aInput[uCurr + 1] >> 4 );
        int nEnc3 = ( ( aInput[uCurr + 1] & 15 ) << 2 | aInput[uCurr + 2] >> 6 );
        int nEnc4 = aInput[uCurr + 2] & 63;

        aOutput.push_back( pEncTable[nEnc1] );
        aOutput.push_back( pEncTable[nEnc2] );
        aOutput.push_back( pEncTable[nEnc3] );
        aOutput.push_back( pEncTable[nEnc4] );
    }
    if ( 2 == uLeft )
    {
        int nEnc1 = aInput[uCurr] >> 2;
        int nEnc2 = ( ( aInput[uCurr] & 3 ) << 4 | aInput[uCurr + 1] >> 4 );
        int nEnc3 = ( ( aInput[uCurr + 1] & 15 ) << 2 | aInput[uCurr + 2] >> 6 );
        int nEnc4 = 64;
        aOutput.push_back( pEncTable[nEnc1] );
        aOutput.push_back( pEncTable[nEnc2] );
        aOutput.push_back( pEncTable[nEnc3] );
        aOutput.push_back( pEncTable[nEnc4] );
    }
    else if ( 1 == uLeft )
    {
        int nEnc1 = aInput[uCurr] >> 2;
        int nEnc2 = ( ( aInput[uCurr] & 3 ) << 4 | aInput[uCurr + 1] >> 4 );
        int nEnc3 = 64;
        int nEnc4 = 64;
        aOutput.push_back( pEncTable[nEnc1] );
        aOutput.push_back( pEncTable[nEnc2] );
        aOutput.push_back( pEncTable[nEnc3] );
        aOutput.push_back( pEncTable[nEnc4] );
    }
    else
    {
    }
}


VOID Base64Decode( CONST UCHAR * aInput, SIZE_T aInputSize, std::string & aOutput, CONST CHAR * aDecodeTable )
{
    aOutput.clear();
    if ( 0 != aInputSize % 4 )
    {
        return;
    }
    aOutput.reserve( ( ( aInputSize + 3 ) / 4 ) * 3 );
    CONST CHAR * pDecTable =
        ( aDecodeTable ) ? aDecodeTable : "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
    for ( SIZE_T uCurr = 0; uCurr < aInputSize; uCurr += 4 )
    {
        int nEnc1 = strchr( pDecTable, aInput[uCurr] ) - pDecTable;
        int nEnc2 = strchr( pDecTable, aInput[uCurr + 1] ) - pDecTable;
        int nEnc3 = strchr( pDecTable, aInput[uCurr + 2] ) - pDecTable;
        int nEnc4 = strchr( pDecTable, aInput[uCurr + 3] ) - pDecTable;

        aOutput.push_back( nEnc1 << 2 | nEnc2 >> 4 );
        if ( nEnc3 != 64 )
        {
            aOutput.push_back( ( nEnc2 & 15 ) << 4 | nEnc3 >> 2 );
        }
        if ( nEnc4 != 64 )
        {
            aOutput.push_back( ( nEnc3 & 3 ) << 6 | nEnc4 );
        }
    }
}



VOID Rc4( CONST UCHAR * aInput, SIZE_T aInputSize, std::string & aOutput, CONST CHAR * aRc4Key )
{
    aOutput.clear();
    size_t uKeyLen = strlen( aRc4Key );
    UCHAR S[256];

    for ( int i = 0; i < 256; i++ )
    {
        S[i] = (UCHAR)i;
    }

    for ( int i = 0, j = 0; i < 256; i++ )
    {
        j = ( j + S[i] + aRc4Key[i % uKeyLen] ) & 0xFF;
        UCHAR uTmp = S[i];
        S[i] = S[j];
        S[j] = uTmp;
    }

    int i = 0;
    int j = 0;
    for ( SIZE_T uCurr = 0; uCurr < aInputSize; uCurr++ )
    {
        i = ( i + 1 ) & 0xFF;
        j = ( j + S[i] ) & 0xFF;
        UCHAR uTmp = S[i];
        S[i] = S[j];
        S[j] = uTmp;
        aOutput.push_back( aInput[uCurr] ^ S[( S[i] + S[j] ) & 0xFF] );
    }
}



#ifdef __cplusplus
}
#endif

}    //End of namespace CWUtils
