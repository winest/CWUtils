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
eval( LoadJs( "..\\CWCrypto.js" ) );

//Test functions
WScript.Echo( CWUtils.CaesarDecrypt("abcdefghijklmnopqrstuvwxyz","ABCDEFGHIJKLMNOPQRSTUVWXZY","0123456789","abcdefg") );
WScript.Echo( CWUtils.CaesarDecrypt("tzgnahuboyflsipwcjqdkxermv","BIPVCJWDQXKRELYSFMZTGNAHUO","0123456789","ryelsyfmszgmtagnuahoubag") );
WScript.Echo( CWUtils.CaesarDecrypt("bipvcjwdqxkrelysfmztgnahuo","TZGNAHUBOYFLSIPWCJQDKXERMV","0123456789","ryelsyfmszgmtagnuahoubag") );
WScript.Echo( CWUtils.CaesarDecrypt("tzgnahuboyflsipwcjqdkxermv","BIPVCJWDQXKRELYSFMZTGNAHUO","0123456789","cnllbHN5Zm1zemdtdGFnbnVhaG91YmFnZgtetWRHVvrgdUZiUFRht2ZBPT0K") );
WScript.Echo( CWUtils.CaesarDecrypt("bipvcjwdqxkrelysfmztgnahuo","TZGNAHUBOYFLSIPWCJQDKXERMV","0123456789","cnllbHN5Zm1zemdtdGFnbnVhaG91YmFnZgtetWRHVvrgdUZiUFRht2ZBPT0K") );

WScript.Echo( "\n\n" );

var strTest = "Just a testing";
var strTestEncode = CWUtils.Base64Encode( strTest );
var strTestDecode = CWUtils.Base64Decode( strTestEncode );
WScript.Echo( strTest + " => " + strTestEncode + " => " + strTestDecode );

strTest = "http://abc.com/?a=1&b=2";
strTestEncode = CWUtils.Base64Encode( strTest );
strTestDecode = CWUtils.Base64Decode( strTestEncode );
WScript.Echo( strTest + " => " + strTestEncode + " => " + strTestDecode );

WScript.Echo( "\nSuccessfully End" );
WScript.Quit( 0 );
