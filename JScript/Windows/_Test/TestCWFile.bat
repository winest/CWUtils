@set @_PackJsInBatByWinest=0 /*
@ECHO OFF
CD /D "%~dp0"
CSCRIPT "%~0" //D //Nologo //E:JScript %1 %2 %3 %4 %5 %6 %7 %8 %9
IF %ERRORLEVEL% LSS 0 ( ECHO Failed. Error code is %ERRORLEVEL% )
PAUSE
EXIT /B
*/

var WshShell = WScript.CreateObject( "WScript.Shell" );
var WshFileSystem = new ActiveXObject( "Scripting.FileSystemObject" );

function LoadJs( aJsPath )
{
    var file = WshFileSystem.OpenTextFile( aJsPath , 1 );
    var strContent = file.ReadAll();
    file.Close();
    return strContent;
}
eval( LoadJs( "..\\CWGeneralUtils.js" ) );
eval( LoadJs( "..\\CWString.js" ) );
eval( LoadJs( "..\\CWFile.js" ) );

//Test functions
CWUtils.SaveToFile( "123.txt" , "aContent" , true , true );

function stComputeRelativePath( aSrcPath , aDstPath )
{
    this.strSrcPath = aSrcPath;
    this.strDstPath = aDstPath;
}
var aryComputeRelativePath = [ new stComputeRelativePath( "C:\\Windows\\System32" , "C:\\Windows" ) ,
                               new stComputeRelativePath( "C:\\Windows" , "C:\\Windows\\System32" ) ,
                               new stComputeRelativePath( "C:\\Windows\\System32\\notepad.exe" , "C:\\Windows\\SysWOW64\\notepad.exe" ) ,
                               new stComputeRelativePath( "C:\\Windows\\notepad.exe" , "D:\\Windows\\notepad.exe" ) ,
                               new stComputeRelativePath( "C:\\Windows\\\\\\notepad.exe" , "C:\\Windows\\notepad++.exe" ) ,
                               new stComputeRelativePath( "C:\\Windows\\.\\notepad.exe" , "C:\\Windows\\notepad++.exe" ) ,
                               new stComputeRelativePath( "C:\\Windows\\..\\notepad.exe" , "C:\\Windows\\notepad++.exe" ) ];
for ( var i = 0 ; i < aryComputeRelativePath.length ; i++ )
{
    WScript.Echo( "Src=" + aryComputeRelativePath[i].strSrcPath + "\tDst=" + aryComputeRelativePath[i].strDstPath +
                  "\tResult=" + CWUtils.ComputeRelativePath( aryComputeRelativePath[i].strSrcPath , aryComputeRelativePath[i].strDstPath ) );
}





//Test CWshTextFile
var fileWshWrite = new CWUtils.CWshTextFile();   //Remember to new CWshTextFile so that all constants are initialized
if ( false == fileWshWrite.Open(WshShell.CurrentDirectory + "\\123.txt" , CWUtils.CWshTextFile.Mode.ForWriting , false , true) )
{
    WScript.Echo( "fileWshWrite.Open() failed" );
    WScript.Quit( -1 );
}
fileWshWrite.Write( "Content\r\nSecond Line" );
fileWshWrite.Close();

var fileWshRead = new CWUtils.CWshTextFile();
if ( false == fileWshRead.Open(WshShell.CurrentDirectory + "\\123.txt" , CWUtils.CWshTextFile.Mode.ForReading , false , false) )
{
    WScript.Echo( "fileWshRead.Open() failed" );
    WScript.Quit( -1 );
}
var strContent = "";
while ( ! fileWshRead.AtEndOfStream() )
{
    strContent += fileWshRead.ReadLine();
}
WScript.Echo( "strContent=" + strContent );
fileWshRead.Close();



//Test CAdoTextFile
var fileAdo = new CWUtils.CAdoTextFile();   //Remember to new CAdoTextFile so that all constants are initialized
if ( false == fileAdo.Open( WshShell.CurrentDirectory + "\\123.txt" , CWUtils.CAdoTextFile.ConnectModeEnum.adModeRead , "_autodetect" ) )
{
    WScript.Echo( "fileAdo.Open() failed" );
    WScript.Quit( -1 );
}
var strContent = fileAdo.ReadAll();
fileAdo.Close();
WScript.Echo( "Content=" + strContent );






//Test CBinaryFile
var fileBin = new CWUtils.CBinaryFile();   //Remember to new CBinaryFile so that all constants are initialized
var strBin = fileBin.ReadAll( WshShell.CurrentDirectory + "\\Samples\\Icon.ico" );
WScript.Echo( "strBin.length=" + strBin.length + " (0x" + strBin.length.toString(16) + ")" );
WScript.Echo( "typeof(strBin)=" + typeof(strBin) );
WScript.Echo( CWUtils.StringToHexDump( strBin ) );






WScript.Echo( "\nSuccessfully End" );
WScript.Quit( 0 );
