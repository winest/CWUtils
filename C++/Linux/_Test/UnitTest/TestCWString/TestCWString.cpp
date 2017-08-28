#include "stdafx.h"
#include <string>
#include "CWString.h"

using namespace std;

int main()
{
    string strData = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    string strOutput;
    wstring wstrOutput;
    CWUtils::ToHexString( (const UCHAR *)strData.c_str() , strData.length() , strOutput , " " );
    printf( "%s\n" , strOutput.c_str() );

    CWUtils::ToHexDump( (const UCHAR *)strData.c_str() , strData.length() , strOutput , " | " , 16 );
    printf( "%s\n" , strOutput.c_str() );


    CWUtils::FormatStringA( strOutput , "%s\n" , "123" );
    printf( "%s\n" , strOutput.c_str() );

    CWUtils::FormatStringW( wstrOutput , L"%ls\n" , L"123" );
    printf( "%ls\n" , wstrOutput.c_str() );
    return 0;

}
