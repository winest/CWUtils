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
eval( LoadJs( "..\\CWString.js" ) );

//Test functions
WScript.Echo( "CWUtils.StringToHexDump( \"12345\" + String.fromCharCode(0) + \"789\\n\\n\\n\\n123456\" ) return:\n" +
              CWUtils.StringToHexDump( "12345" + String.fromCharCode(0) + "789\n\n\n\n123456" ) );




WScript.Echo( "\nSuccessfully End" );
WScript.Quit( 0 );
