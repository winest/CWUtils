#include "stdafx.h"
#include "CWCmdArgsParser.h"

using namespace std;
using namespace CWUtils;

int main( int aArgc , CHAR * aArgv[] )
{
    CCmdArgsParser::GetInstance()->SetUsage( NULL , FALSE , "\nTestCWCmdArgsParser.exe testing\n" );
    CCmdArgsParser::GetInstance()->SetUsage( "opt" , FALSE , "Any optional value" );
    CCmdArgsParser::GetInstance()->SetUsage( "must" , TRUE , "Any must-exist value" );

    if ( FALSE == CCmdArgsParser::GetInstance()->ParseArgs( aArgc , aArgv , TRUE ) )
    {
        printf( "ParseArgs() failed\n" );
        for ( int i = 0 ; i < aArgc ; i++ )
        {
            printf( "aArgv[%d]=%s\n" , i , aArgv[i] );
        }
    }
    CCmdArgsParser::GetInstance()->DumpArgs();

    return 0;
}
