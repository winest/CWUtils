#include "stdafx.h"
#include "CWString.h"
#include "CWCrypto.h"



VOID TestBase64()
{
    printf( "\n========== TestBase64() Enter ==========\n" );

    string strOrg = "Just a testing";
    string strBase64Encoded;
    string strBase64Decoded;
    CWUtils::Base64Encode( (CONST UCHAR *)strOrg.c_str() , strOrg.length() , strBase64Encoded , NULL );
    CWUtils::Base64Decode( (CONST UCHAR *)strBase64Encoded.c_str() , strBase64Encoded.length() , strBase64Decoded , NULL );
    printf( "%s => %s => %s\n" , strOrg.c_str() , strBase64Encoded.c_str() , strBase64Decoded.c_str() );

    printf( "\n========== TestBase64() Leave ==========\n" );
}

VOID TestRc4()
{
    printf( "\n========== TestRc4() Enter ==========\n" );
    string strRc4Key = "abcdefg";
    string strOrg = "Just a testing";
    string strRc4Encoded , strRc4HexDump;
    string strRc4Decoded;
    CWUtils::Rc4( (CONST UCHAR *)strOrg.c_str() , strOrg.length() , strRc4Encoded , strRc4Key.c_str() );
    CWUtils::Rc4( (CONST UCHAR *)strRc4Encoded.c_str() , strRc4Encoded.length() , strRc4Decoded , strRc4Key.c_str() );
    CWUtils::ToHexDump( (CONST UCHAR *)strRc4Encoded.c_str() , strRc4Encoded.length() , strRc4HexDump , " | " , 16 );
    printf( "%s =>\n%s\n=> %s\n" , strOrg.c_str() , strRc4HexDump.c_str() , strRc4Decoded.c_str() );
    printf( "\n========== TestRc4() Leave ==========\n" );
}

INT main( INT aArgc , CHAR * aArgv[] )
{
    TestBase64();
    TestRc4();
    printf( "End of the program\n" );
    return 0;
}

