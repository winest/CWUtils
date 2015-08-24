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
eval( LoadJs( "..\\CWXmlHttp.js" ) );


//Test CXml
WScript.Echo( "\n========== CXml ==========\n" );
var xml = new CWUtils.CXml();
if ( false == xml.LoadFromFile(WshShell.CurrentDirectory + "\\Samples\\Local_HookOnSkype.vcxproj") )
{
    WScript.Echo( "xml.LoadFromFile() failed" );
    WScript.Quit( -1 );
}
else
{
    //If the XML has xmlns, you must assign a prefix to access the namespace with XML 1.0
    var strSetNamespace = "xmlns:mynamespace=\"" + xml.GetNamespacesByName("Project")[0] + "\"";
    xml.document.setProperty( "SelectionNamespaces" , strSetNamespace );
}

var aryXmlTagNameResults = xml.GetElementsByTagName( "AdditionalIncludeDirectories" );
WScript.Echo( "aryXmlTagNameResults.length=" + aryXmlTagNameResults.length );
for ( var i = 0 ; i < aryXmlTagNameResults.length ; i++ )
{
    WScript.Echo( i + "-" + aryXmlTagNameResults[i].nodeName + "-" + aryXmlTagNameResults[i].text );
}

var aryXmlSelectNodesResult4 = xml.document.selectNodes( "/" );
WScript.Echo( "aryXmlSelectNodesResult4.length=" + aryXmlSelectNodesResult4.length );
for ( var i = 0 ; i < aryXmlSelectNodesResult4.length ; i++ )
{
    WScript.Echo( i + "-" + aryXmlSelectNodesResult4[i].nodeName + "-" + aryXmlSelectNodesResult4[i].text );
}

var aryXmlSelectNodesResult = xml.document.selectNodes( "//mynamespace:ItemGroup" );
WScript.Echo( "aryXmlSelectNodesResult.length=" + aryXmlSelectNodesResult.length );
for ( var i = 0 ; i < aryXmlSelectNodesResult.length ; i++ )
{
    WScript.Echo( i + "-" + aryXmlSelectNodesResult[i].nodeName + "-" + aryXmlSelectNodesResult[i].text );
}
var aryXmlSelectNodesResult2 = xml.document.selectNodes( "/mynamespace:Project/ItemGroup/ClCompile" );
WScript.Echo( "aryXmlSelectNodesResult2.length=" + aryXmlSelectNodesResult2.length );
for ( var i = 0 ; i < aryXmlSelectNodesResult2.length ; i++ )
{
    WScript.Echo( i + "-" + aryXmlSelectNodesResult2[i].nodeName + "-" + aryXmlSelectNodesResult2[i].text );
}
var aryXmlSelectNodesResult3 = xml.document.selectNodes("//mynamespace:ClCompile[@Include] | //mynamespace:ClInclude[@Include]");
WScript.Echo( "aryXmlSelectNodesResult3.length=" + aryXmlSelectNodesResult3.length );
for ( var i = 0 ; i < aryXmlSelectNodesResult3.length ; i++ )
{
    WScript.Echo( i + "-" + aryXmlSelectNodesResult3[i].nodeName + "-" + aryXmlSelectNodesResult3[i].getAttribute("Include") );
}
//Can save to any file including the current using XML
xml.document.selectNodes("//mynamespace:Project")[0].setAttribute( "test" , "QQ" );
WScript.Echo( "xml.SaveToFile=" + xml.SaveToFile( WshShell.CurrentDirectory + "\\Samples\\Local_HookOnSkype_Modified.vcxproj" ) );



//Test CHttp
WScript.Echo( "\n========== CHttp ==========\n" );
var http = new CWUtils.CHttp();
if ( false == http.LoadFromFile(WshShell.CurrentDirectory + "\\Samples\\Local_HookOnSkype.vcxproj" , "UTF-8" ) )
{
    WScript.Echo( "http.LoadFromFile() failed" );
    WScript.Quit( -1 );
}
var aryHttpTagNameResults = http.document.getElementsByTagName( "AdditionalIncludeDirectories" );
WScript.Echo( "aryHttpTagNameResults.length=" + aryHttpTagNameResults.length );
for ( var i = 0 ; i < aryHttpTagNameResults.length ; i++ )
{
    WScript.Echo( i + "-" + aryHttpTagNameResults[i].nodeName + "-" + aryHttpTagNameResults[i].innerText );
}
var aryHttpClassNameResults = http.document.getElementsByClassName( "AdditionalIncludeDirectories" );
WScript.Echo( "aryHttpClassNameResults.length=" + aryHttpClassNameResults.length );
for ( var i = 0 ; i < aryHttpClassNameResults.length ; i++ )
{
    WScript.Echo( i + "-" + aryHttpClassNameResults[i].nodeName + "-" + aryHttpClassNameResults[i].innerText );
}

WScript.Echo( "Press any key to leave" );
WScript.StdIn.ReadLine();
WScript.Quit( 0 );