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
eval( LoadJs( "..\\CWStd.js" ) );

//Test functions
WScript.Echo( "CWUtils.SelectYesNo( \"Input 'y' or 'yes' or 'n' or 'no'\" ) return " + CWUtils.SelectYesNo( "Input 'y' or 'yes' or 'n' or 'no'" ) );

WScript.Echo( "CWUtils.SelectFolder( \"Please enter a folder path: \" ) return " + CWUtils.SelectFolder( "Please enter a folder path: " ) );
WScript.Echo( "CWUtils.SelectFile( \"Please enter a file path: \" ) return " + CWUtils.SelectFile( "Please enter a file path: " ) );

WScript.Echo( "CWUtils.Pause( \"Please press any key to continue\" ) return " + CWUtils.Pause( "Please press any key to continue" ) );



WScript.Echo( "\nSuccessfully End" );
WScript.Quit( 0 );
