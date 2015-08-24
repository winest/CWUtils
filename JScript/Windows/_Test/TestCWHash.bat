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
eval( LoadJs( "..\\CWHash.js" ) );

//Test functions
var strTest = "Just a testing";

WScript.Echo( "Hashes for \"" + strTest + "\"" );
WScript.Echo( "CRC32: " + CWUtils.Crc32( strTest ) );
WScript.Echo( "MD5: " + CWUtils.Md5( strTest ) );
WScript.Echo( "SHA1: " + CWUtils.Sha1( strTest ) );
WScript.Echo( "SHA224: " + CWUtils.Sha224( strTest ) );
WScript.Echo( "SHA256: " + CWUtils.Sha256( strTest ) );
WScript.Echo( "SHA384: " + CWUtils.Sha384( strTest ) );
WScript.Echo( "SHA512: " + CWUtils.Sha512( strTest ) );
WScript.Echo( "\nStreaming hashes for \"" + strTest + "\"" );
var crc32 = new CWUtils.CCrc32();
crc32.Update( strTest.substr( 0 , 2 ) );
crc32.Update( strTest.substr( 2 , strTest.length - 2 ) );
WScript.Echo( "Streaming CRC32: " + crc32.Finalize() );
var md5 = new CWUtils.CMd5();
md5.Update( strTest.substr( 0 , 2 ) );
md5.Update( strTest.substr( 2 , strTest.length - 2 ) );
WScript.Echo( "Streaming MD5: " + md5.Finalize() );
md5.Init();
md5.Update( strTest.substr( 0 , 2 ) );
md5.Update( strTest.substr( 2 , strTest.length - 2 ) );
WScript.Echo( "Streaming MD5 again: " + md5.Finalize() );
var sha1 = new CWUtils.CSha1();
sha1.Update( strTest.substr( 0 , 2 ) );
sha1.Update( strTest.substr( 2 , strTest.length - 2 ) );
WScript.Echo( "Streaming SHA1: " + sha1.Finalize() );
var sha224 = new CWUtils.CSha224();
sha224.Update( strTest.substr( 0 , 2 ) );
sha224.Update( strTest.substr( 2 , strTest.length - 2 ) );
WScript.Echo( "Streaming SHA224: " + sha224.Finalize() );
var sha256 = new CWUtils.CSha256();
sha256.Update( strTest.substr( 0 , 2 ) );
sha256.Update( strTest.substr( 2 , strTest.length - 2 ) );
WScript.Echo( "Streaming SHA256: " + sha256.Finalize() );
var sha384 = new CWUtils.CSha384();
sha384.Update( strTest.substr( 0 , 2 ) );
sha384.Update( strTest.substr( 2 , strTest.length - 2 ) );
WScript.Echo( "Streaming SHA384: " + sha384.Finalize() );
var sha512 = new CWUtils.CSha512();
sha512.Update( strTest.substr( 0 , 2 ) );
sha512.Update( strTest.substr( 2 , strTest.length - 2 ) );
WScript.Echo( "Streaming SHA512: " + sha512.Finalize() );


WScript.Echo( "\nSuccessfully End" );
WScript.Quit( 0 );
