/*
 * Copyright (c) 2009-2015, ChienWei Hung <winestwinest@gmail.com>
 * CWUtils is published under the BSD-3-Clause license.
 *
 * CWUtils is a set of standalone APIs for developers to speed up their 
 * programming. It should be very easy to port them to other projects or 
 * learn how to implement things on different languages and platforms. 
 *
 * The latest version can be found at https://github.com/winest/CWUtils
 */

/*
 * Provide stand-alone stdio-related utilities for JScript developers.
 *
 * For complete documents, please refer to http://msdn.microsoft.com/en-us/library/98591fh7(v=vs.84).aspx
 */

//Avoid multiple instances
var CWUtils = CWUtils || {};
CWUtils.WshFileSystem = CWUtils.WshFileSystem || new ActiveXObject( "Scripting.FileSystemObject" );

CWUtils.SelectYesNo = CWUtils.SelectYesNo || function( aMessage )
{
    var strInput;
    for ( ;; )
    {
        WScript.Echo( aMessage );
        strInput = WScript.StdIn.ReadLine().toLowerCase();
        if ( "y" == strInput || "yes" == strInput )
        {
            return true;
        }
        else if ( "n" == strInput || "no" == strInput )
        {
            return false;
        }
        else {}
    }
};

CWUtils.SelectFolder = CWUtils.SelectFolder || function( aMessage )
{
    var strInput;
    for ( ;; )
    {
        WScript.Echo( aMessage );
        strInput = WScript.StdIn.ReadLine();
        if ( true == CWUtils.WshFileSystem.FolderExists( strInput ) )
        {
            return strInput;
        }
        else
        {
            WScript.Echo( "\"" + strInput + "\" not found" );
        }
    }
};

CWUtils.SelectFile = CWUtils.SelectFile || function( aMessage )
{
    var strInput;
    for ( ;; )
    {
        WScript.Echo( aMessage );
        strInput = WScript.StdIn.ReadLine();
        if ( true == CWUtils.WshFileSystem.FileExists( strInput ) )
        {
            return strInput;
        }
        else
        {
            WScript.Echo( "\"" + strInput + "\" not found" );
        }
    }
};

CWUtils.Pause = CWUtils.Pause || function( aPauseMsg )
{
    if ( null == aPauseMsg )
    {
        aPauseMsg = "Please press any key to continue";
    }
    WScript.StdOut.WriteLine( aPauseMsg );
    while ( ! WScript.StdIn.AtEndOfLine )
    {
        WScript.StdIn.Read( 1 );
    }
};
