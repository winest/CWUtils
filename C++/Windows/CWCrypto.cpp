#include "stdafx.h"
#include "CWCrypto.h"
#include "CWString.h"

#include <string>
using std::string;
using std::wstring;

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






BOOL CRsa::CreateAndUseKey( CONST WCHAR * aContainer, CONST WCHAR * aProvider )
{
    BOOL bRet = FALSE;
    this->CloseKey();

    do
    {
        wstring wstrErr;
        if ( FALSE == CryptAcquireContextW( &m_hProvider, aContainer, aProvider, PROV_RSA_FULL, 0 ) )
        {
            if ( NTE_BAD_KEYSET != GetLastError() )
            {
                CWUtils::GetLastErrorStringW( wstrErr );
                wprintf_s( L"CryptAcquireContextW() failed. GetLastError()=0x%08X (%ws)\n", GetLastError(),
                           wstrErr.c_str() );
                break;
            }

            if ( FALSE == CryptAcquireContextW( &m_hProvider, aContainer, aProvider, PROV_RSA_FULL, CRYPT_NEWKEYSET ) )
            {
                CWUtils::GetLastErrorStringW( wstrErr );
                wprintf_s( L"CryptAcquireContextW() failed. GetLastError()=0x%08X (%ws)\n", GetLastError(),
                           wstrErr.c_str() );
                break;
            }
        }

        //The first 16 bits means the key length, here we use 2048 bits
        if ( FALSE ==
             CryptGenKey( m_hProvider, CALG_RSA_KEYX, /*( 2048 << 16 )*/ RSA1024BIT_KEY | CRYPT_EXPORTABLE, &m_hKey ) )
        {
            CWUtils::GetLastErrorStringW( wstrErr );
            wprintf_s( L"CryptGenKey() failed. GetLastError()=0x%08X (%ws)\n", GetLastError(), wstrErr.c_str() );
            break;
        }

        bRet = TRUE;
    } while ( 0 );

    return bRet;
}

BOOL CRsa::ImportAndUseKey( CONST UCHAR * aKeyBlob,
                            DWORD aKeyBlobSize,
                            CONST WCHAR * aContainer,
                            CONST WCHAR * aProvider )
{
    BOOL bRet = FALSE;
    this->CloseKey();

    do
    {
        wstring wstrErr;
        if ( FALSE == CryptAcquireContextW( &m_hProvider, aContainer, aProvider, PROV_RSA_FULL,
                                            CRYPT_VERIFYCONTEXT | CRYPT_SILENT ) )
        {
            if ( NTE_BAD_KEYSET != GetLastError() )
            {
                CWUtils::GetLastErrorStringW( wstrErr );
                wprintf_s( L"CryptAcquireContextW() failed. GetLastError()=0x%08X (%ws)\n", GetLastError(),
                           wstrErr.c_str() );
                break;
            }

            if ( FALSE == CryptAcquireContextW( &m_hProvider, aContainer, aProvider, PROV_RSA_FULL,
                                                CRYPT_NEWKEYSET | CRYPT_VERIFYCONTEXT | CRYPT_SILENT ) )
            {
                CWUtils::GetLastErrorStringW( wstrErr );
                wprintf_s( L"CryptAcquireContextW() failed. GetLastError()=0x%08X (%ws)\n", GetLastError(),
                           wstrErr.c_str() );
                break;
            }
        }

        if ( FALSE ==
             CryptImportKey( m_hProvider, (CONST BYTE *)aKeyBlob, aKeyBlobSize, NULL, CRYPT_NO_SALT, &m_hKey ) )
        {
            CWUtils::GetLastErrorStringW( wstrErr );
            wprintf_s( L"CryptImportKey() failed. GetLastError()=0x%08X (%ws)\n", GetLastError(), wstrErr.c_str() );
            break;
        }

        bRet = TRUE;
    } while ( 0 );

    return bRet;
}

BOOL CRsa::ExportCurrentKey( IN OUT UCHAR * aKeyBlob, IN OUT DWORD * aKeyBlobSize )
{
    return TRUE;
}


BOOL CRsa::DeleteKey( CONST WCHAR * aContainer, CONST WCHAR * aProvider )
{
    BOOL bRet = FALSE;

    this->CloseKey();
    bRet = CryptAcquireContextW( &m_hProvider, aContainer, aProvider, PROV_RSA_FULL, CRYPT_DELETEKEYSET );
    if ( FALSE == bRet )
    {
        wstring wstrErr;
        CWUtils::GetLastErrorStringW( wstrErr );
        wprintf_s( L"CryptAcquireContextW() failed. GetLastError()=0x%08X (%ws)\n", GetLastError(), wstrErr.c_str() );
    }
    return bRet;
}

VOID CRsa::CloseKey()
{
    if ( NULL != m_hKey )
    {
        CryptDestroyKey( m_hKey );
        m_hKey = NULL;
    }
    if ( NULL != m_hProvider )
    {
        CryptReleaseContext( m_hProvider, 0 );
        m_hProvider = NULL;
    }
}

BOOL CRsa::Encrypt( IN OUT UCHAR * aBuf, IN DWORD aBufSize, IN OUT DWORD * aDataSize )
{
    BOOL bRet = FALSE;

    do
    {
        if ( NULL == aBuf || NULL == aDataSize )
        {
            SetLastError( ERROR_INVALID_PARAMETER );
            break;
        }

        if ( NULL == m_hProvider || NULL == m_hKey )
        {
            SetLastError( ERROR_NOT_READY );
            break;
        }

        if ( FALSE == CryptEncrypt( m_hKey, NULL, TRUE, 0 /*CRYPT_OAEP*/, aBuf, aDataSize, aBufSize ) )
        {
            wstring wstrErr;
            CWUtils::GetLastErrorStringW( wstrErr );
            wprintf_s( L"CryptEncrypt() failed. GetLastError()=0x%08X (%ws)\n", GetLastError(), wstrErr.c_str() );
            break;
        }

        bRet = TRUE;
    } while ( 0 );

    return bRet;
}

BOOL CRsa::Decrypt( UCHAR * aBuf, DWORD * aBufSize )
{
    BOOL bRet = FALSE;

    do
    {
        if ( NULL == aBuf || NULL == aBufSize )
        {
            SetLastError( ERROR_INVALID_PARAMETER );
            break;
        }

        if ( NULL == m_hProvider || NULL == m_hKey )
        {
            SetLastError( ERROR_NOT_READY );
            break;
        }

        if ( FALSE == CryptDecrypt( m_hKey, NULL, TRUE, 0 /*CRYPT_OAEP*/, aBuf, aBufSize ) )
        {
            wstring wstrErr;
            CWUtils::GetLastErrorStringW( wstrErr );
            wprintf_s( L"CryptDecrypt() failed. GetLastError()=0x%08X (%ws)\n", GetLastError(), wstrErr.c_str() );
            break;
        }

        bRet = TRUE;
    } while ( 0 );

    return bRet;
}


#ifdef __cplusplus
}
#endif

}    //End of namespace CWUtils