#pragma once

#include <Windows.h>
#include <Wincrypt.h>

namespace CWUtils
{

#ifdef __cplusplus
    extern "C" {
#endif



VOID Base64Encode( CONST UCHAR * aInput , SIZE_T aInputSize , std::string & aOutput , CONST CHAR * aEncodeTable );
VOID Base64Decode( CONST UCHAR * aInput , SIZE_T aInputSize , std::string & aOutput , CONST CHAR * aDecodeTable );

VOID Rc4( CONST UCHAR * aInput , SIZE_T aInputSize , std::string & aOutput , CONST CHAR * aRc4Key );

class CRsa
{
    public :
        CRsa() : m_hProvider(NULL) , m_hKey(NULL) {}
        virtual ~CRsa() 
        {
            this->CloseKey();
        }
        
    public :
        //Some provider can be found under HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Cryptography\Defaults\Provider

        BOOL CreateAndUseKey( CONST WCHAR * aContainer , CONST WCHAR * aProvider );
        BOOL ImportAndUseKey( CONST UCHAR * aKeyBlob , DWORD aKeyBlobSize , CONST WCHAR * aContainer , CONST WCHAR * aProvider );
        BOOL ExportCurrentKey( IN OUT UCHAR * aKeyBlob , IN OUT DWORD * aKeyBlobSize );
        BOOL DeleteKey( CONST WCHAR * aContainer , CONST WCHAR * aProvider );
        VOID CloseKey();
        
        BOOL Encrypt( IN OUT UCHAR * aBuf , IN DWORD aBufSize , IN OUT DWORD * aDataSize );
        BOOL Decrypt( IN OUT UCHAR * aBuf , IN OUT DWORD * aBufSize );
        
    private :
        HCRYPTPROV m_hProvider;
        HCRYPTKEY m_hKey;

};

#ifdef __cplusplus
    }
#endif

}   //End of namespace CWUtils
