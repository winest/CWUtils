@set @_PackJsInBatByWinest=0 /*
@ECHO OFF
CD /D "%~dp0"
CSCRIPT "%~0" //D //Nologo //E:JScript %1 %2 %3 %4 %5 %6 %7 %8 %9
IF %ERRORLEVEL% LSS 0 ( ECHO Failed. Error code is %ERRORLEVEL% )
PAUSE
EXIT /B
*/

var WshFileSystem = new ActiveXObject( "Scripting.FileSystemObject" );

function LoadJs( aJsPath )
{
    var file = WshFileSystem.OpenTextFile( aJsPath , 1 );
    var strContent = file.ReadAll();
    file.Close();
    return strContent;
}
eval( LoadJs( "..\\CWGeneralUtils.js" ) );

//Test functions
WScript.Echo( "CWUtils.Padding( 123 , 8 , 0 )=" + CWUtils.Padding( 123 , 8 , 0 ) );
WScript.Echo( "CWUtils.Dump( [\"abc\",\"ABC\"] )=" + CWUtils.Dump( ["abc","ABC"] ) );
WScript.Echo( "CWUtils.Dump( [\"abc\",\"ABC\"] )=" + CWUtils.Dump( CWUtils , 1 , false ) );
WScript.Echo( "CWUtils.sprintf( \"Number=0x%08X\" , 0x123 )=" + CWUtils.sprintf( "Number=0x%08X" , 0x123 ) );

WScript.Echo( "\nSuccessfully End" );
WScript.Quit( 0 );
