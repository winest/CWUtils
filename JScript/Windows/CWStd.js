/*
 * Copyright (c) 2009-2015, ChienWei Hung <winestwinest@gmail.com>
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * 
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
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
