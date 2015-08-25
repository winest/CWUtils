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
 * Provide stand-alone shell-related utilities for JScript developers.
 *
 * For complete documents, please refer to http://msdn.microsoft.com/en-us/library/98591fh7(v=vs.84).aspx
 */

//Avoid multiple instances
var CWUtils = CWUtils || {};
CWUtils.WshShell = CWUtils.WshShell || WScript.CreateObject( "WScript.Shell" );

CWUtils.IsX86 = CWUtils.IsX86 || function()
{
    var strArch = CWUtils.WshShell.Environment( "System" )("PROCESSOR_ARCHITECTURE").toLowerCase();
    if ( strArch.match(/x86|i386/) )
    {
        return true;
    }
    else
    {
        return false;
    }
};

CWUtils.Exec = CWUtils.Exec || function( aCmd , aWaitEnd , aWaitEndOutput )
{
    aWaitEnd = aWaitEnd || true;
    
    //Return a WshScriptExec object, refer to http://msdn.microsoft.com/en-us/library/2f38xsxe(v=vs.84).aspx
    var execObj = CWUtils.WshShell.Exec( aCmd );
    if ( true == aWaitEnd )
    {
        var strOutput = "";
        while ( 0 == execObj.Status )   //0 for running, 1 for completed
        {
            WScript.Sleep( 100 );
            //Sometimes it will loop infinitely here, it means that you need to clean StdOut at first
            strOutput += execObj.StdOut.ReadAll();
        }
        strOutput += execObj.StdOut.ReadAll();
        if ( aWaitEndOutput 
             //&& Object.prototype.toString.call( aWaitEndOutput ) === "[object Array]"         //Don't check it so programmer can find bug when developing
           )
        {
            aWaitEndOutput.push( strOutput );
        }
    }
    return execObj;
};

CWUtils.RunCmd = CWUtils.RunCmd || function( aCmd , aHideWindow , aWaitEnd )
{
    // 0: Hides the window and activates another window.
    // 1: Activates and displays a window. If the window is minimized or maximized, the system restores it to its original size and position. An application should specify this flag when displaying the window for the first time.
    // 2: Activates the window and displays it as a minimized window.
    // 3: Activates the window and displays it as a maximized window.
    // 4: Displays a window in its most recent size and position. The active window remains active.
    // 5: Activates the window and displays it in its current size and position.
    // 6: Minimizes the specified window and activates the next top-level window in the Z order.
    // 7: Displays the window as a minimized window. The active window remains active.
    // 8: Displays the window in its current state. The active window remains active.
    // 9: Activates and displays the window. If the window is minimized or maximized, the system restores it to its original size and position. An application should specify this flag when restoring a minimized window.
    // 10: Sets the show-state based on the state of the program that started the application.
    var nWindowStyle = ( aHideWindow ) ? 0 : 8;
    aWaitEnd = ( aWaitEnd ) ? true : false;    
    return CWUtils.WshShell.Run( aCmd , nWindowStyle , aWaitEnd );
};

CWUtils.SendKeys = CWUtils.SendKeys || function( aPid , aKeys )
{
    CWUtils.WshShell.AppActivate( aPid );
    CWUtils.WshShell.SendKeys( aKeys );
};

CWUtils.ReadReg = CWUtils.ReadReg || function( aValName )
{
    try
    {
        return CWUtils.WshShell.RegRead( aValName );
    }
    catch( err )
    {
        return null;
    }
};

CWUtils.WriteReg = CWUtils.WriteReg || function( aValName , aValContent , aValType )
{
    try
    {
        //REG_SZ, REG_EXPAND_SZ, REG_DWORD, REG_BINARY
        //RegWrite will write at most one DWORD to a REG_BINARY value. Larger values are not supported with this method.
        CWUtils.WshShell.RegWrite( aValName , aValContent , aValType );
        return true;
    }
    catch( err )
    {
        return false;
    }
};

CWUtils.DeleteReg = CWUtils.DeleteReg || function( aValName , aIsKey )
{
    //If you are deleting a key, you must end the path with back-slash
    if ( aIsKey && 0 < aValName.length && '\\' != aValName[aValName.length - 1] )
    {
        aValName += "\\";
    }

    try
    {
        
        CWUtils.WshShell.RegDelete( aValName );
        return true;
    }
    catch( err )
    {
        return false;
    }
};
