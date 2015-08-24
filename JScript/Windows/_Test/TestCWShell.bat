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
eval( LoadJs( "..\\CWShell.js" ) );

//Test functions
WScript.Echo( "CWUtils.RunCmd( \"calc.exe\" ) return " + CWUtils.RunCmd( "calc.exe" ) );



var execObj = CWUtils.Exec( "notepad.exe" );
WScript.Echo( "CWUtils.Exec( \"notepad.exe\" ) return " + execObj );
WScript.Sleep( 1000 );
WScript.Echo( "CWUtils.SendKeys() return " + CWUtils.SendKeys( execObj.ProcessID , "123" ) );
WScript.Sleep( 2000 );
execObj.Terminate();
WScript.Sleep( 1000 );
WScript.Echo( "CWUtils.SendKeys() return " + CWUtils.SendKeys( execObj.ProcessID , "N" ) );



WScript.Echo( "CWUtils.IsX86() return " + CWUtils.IsX86() );

WScript.Echo( "CWUtils.WriteReg() return " + CWUtils.WriteReg( "HKEY_LOCAL_MACHINE\\SOFTWARE\\TestKey\\TestValueName" , "TestValueValue" , "REG_SZ" ) );
WScript.Echo( "CWUtils.ReadReg() return " + CWUtils.ReadReg( "HKEY_LOCAL_MACHINE\\SOFTWARE\\TestKey\\TestValueName" ) );
WScript.Echo( "CWUtils.DeleteReg() return " + CWUtils.DeleteReg( "HKEY_LOCAL_MACHINE\\SOFTWARE\\TestKey" , true ) );



WScript.Echo( "\nSuccessfully End" );
WScript.Quit( 0 );
