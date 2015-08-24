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
eval( LoadJs( "..\\CWMath.js" ) );

//Test functions
WScript.Echo( "CWUtils.RandInt( 10 , 100 )=" + CWUtils.RandInt( 10 , 100 ) );
WScript.Echo( "CWUtils.RandChar( \"aAzZ\" )=" + CWUtils.RandChar( "aAzZ" ) );
WScript.Echo( "CWUtils.RandStr( 10 )=" + CWUtils.RandStr( 10 ) );
WScript.Echo( "CWUtils.RandAryElement( [1,2,3,4,5] )=" + CWUtils.RandAryElement( [1,2,3,4,5] ) );

WScript.Echo( "\nSuccessfully End" );
WScript.Quit( 0 );
