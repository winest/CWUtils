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
 * Provide stand-alone file-related utilities for JScript developers.
 *
 * For complete documents, please refer to http://msdn.microsoft.com/en-us/library/hww8txat(v=vs.84).aspx
 * Some frequently used functions:
 * CWUtils.WshFileSystem.CreateFolder( foldername )
 * CWUtils.WshFileSystem.CopyFile/CopyFolder( source , destination [, overwrite=true] )
 * CWUtils.WshFileSystem.MoveFile/MoveFolder( source , destination )
 * CWUtils.WshFileSystem.DeleteFile/DeleteFolder( path [, force=false] )
 * CWUtils.WshFileSystem.DriveExists/FileExists/FolderExists( path )
 * CWUtils.WshFileSystem.GetAbsolutePathName/GetDriveName/GetParentFolderName/GetFileName/GetBaseName/GetExtensionName( path )
 * CWUtils.WshFileSystem.GetDriver/GetFolder/GetFile( path )
 * CWUtils.WshFileSystem.GetTempName()
 */

//Avoid multiple instances
var CWUtils = CWUtils || {};
CWUtils.WshFileSystem = CWUtils.WshFileSystem || new ActiveXObject( "Scripting.FileSystemObject" );

CWUtils.SaveToFile = CWUtils.SaveToFile || function( aFilePath , aContent , aUnicode , aAppend )
{
    //WScript.Echo( "aFilePath=" + aFilePath + " aContent=" + aContent + " aUnicode=" + aUnicode + " aAppend=" + aAppend );
    //CWUtils.WshFileSystem.OpenTextFile( filename [, iomode [, create=false [, format=0]]] )
    //iomode=1 for read, 2 for write, 8 for append
    //format=-2 for system default, -1 for unicode, 0 for ASCII
    fileSrc = CWUtils.WshFileSystem.OpenTextFile( aFilePath , (aAppend) ? 8 : 2 ,
    true , (aUnicode) ? -1 : -2 );
    if ( null == fileSrc )
    {
        WScript.Echo( "Failed to open " + aFilePath );
        return false;
    }
    
    fileSrc.Write( aContent );
    fileSrc.Close();
    return true;
};

CWUtils.DbgMsg = CWUtils.DbgMsg || function( aLevel , aTag , aMsg , aLogFolder )
{
    if ( ("undefined" === typeof(aLogFolder)) || (null == aLogFolder) )
    {
        aLogFolder = ".";
    }
    var strMsg = "[" + aLevel + "][" + aTag + "]: " + aMsg;
    WScript.Echo( strMsg );
    if ( CWUtils.WshFileSystem.FolderExists(aLogFolder) )
    {
        CWUtils.SaveToFile( aLogFolder + "\\" + aTag + ".txt" , strMsg + "\n" , true , true );
    }
};

CWUtils.CreateFolderIfNeed = CWUtils.CreateFolderIfNeed || function( aFolderPath )
{
    if ( CWUtils.WshFileSystem.FolderExists(aFolderPath) )
    {
        return true;
    }

    var strPaths = aFolderPath.split( "\\" );
    var strPath = "";
    for ( var uName in strPaths )
    {
        if ( 0 < strPath.length )
        {
            strPath += "\\";
        }
        strPath += strPaths[uName];

        if ( false == CWUtils.WshFileSystem.FolderExists( strPath ) &&
             null == CWUtils.WshFileSystem.CreateFolder( strPath ) )
        {
            return false;
        }
    }

    return true;
};

CWUtils.ForceDeleteFile = CWUtils.ForceDeleteFile || function( aFilePath )
{
    if ( CWUtils.WshFileSystem.FileExists(aFilePath) )
    {
        CWUtils.WshFileSystem.DeleteFile( aFilePath , true );
    }
};

CWUtils.SelectFile = CWUtils.SelectFile || function( aMsg , aCheckExist )
{
    var strFile = null;
    for ( ;; )
    {
        if ( null == strFile || ( aCheckExist && false == CWUtils.WshFileSystem.FileExists(strFile) ) )
        {
            WScript.Echo( aMsg );
            strFile = CWUtils.WshFileSystem.GetAbsolutePathName( WScript.StdIn.ReadLine() );
        }
        else
        {
            break;
        }
    }
    return strFile;
};

CWUtils.SelectFolder = CWUtils.SelectFolder || function( aMsg , aCheckExist )
{
    var strFolder = null;
    for ( ;; )
    {
        if ( null == strFolder || ( aCheckExist && false == CWUtils.WshFileSystem.FolderExists(strFolder) ) )
        {
            WScript.Echo( aMsg );
            strFolder = CWUtils.WshFileSystem.GetAbsolutePathName( WScript.StdIn.ReadLine() );
        }
        else
        {
            break;
        }
    }
    return strFolder;
};

CWUtils.ComputeRelativePath = CWUtils.ComputeRelativePath || function( aSrcPath , aDstPath )
{
    var strRelativePath = "";

    aSrcPath = CWUtils.WshFileSystem.GetAbsolutePathName( aSrcPath );
    aDstPath = CWUtils.WshFileSystem.GetAbsolutePathName( aDstPath );
    WScript.Echo( "aSrcPath=" + aSrcPath + "\taDstPath=" + aDstPath );
    //Considering the case like C:\\Window123 and C:\\Windows456
    var nCommonPrefix = -1;
    var nCommonFolder = -1;
    for ( var i = 0 ; i < Math.min(aSrcPath.length , aDstPath.length) ; i++ )
    {
        if ( aSrcPath.charAt(i) != aDstPath.charAt(i) )
        {
            nCommonPrefix = i - 1;
            break;
        }
        if ( '\\' == aSrcPath.charAt(i) )
        {
            nCommonFolder = i;
        }
    }
    
    var nDifferentFolderCnt = 0;
    for ( var i = nCommonFolder + 1 ; i < aSrcPath.length ; i++ )
    {
        if ( '\\' == aSrcPath.charAt(i) )
        {
            strRelativePath += "..\\";
            nDifferentFolderCnt++;
        }
    }
    
    if ( 0 == nDifferentFolderCnt )
    {
        strRelativePath = ".\\";
    }
    strRelativePath += aDstPath.substr( nCommonFolder + 1 );
    return strRelativePath;
};

//For regular expression, please refer to http://msdn.microsoft.com/en-us/library/1400241x(v=vs.84).aspx
CWUtils.ReplaceFile = CWUtils.ReplaceFile || function( aFilePath , aOldStr , aNewStr , aUnicode )
{
    //WScript.Echo( "aFilePath=" + aFilePath + " aOldStr=" + aOldStr + " aNewStr=" + aNewStr + " aUnicode=" + aUnicode );
    var tmpPath = CWUtils.WshFileSystem.GetTempName();

    fileSrc = CWUtils.WshFileSystem.OpenTextFile( aFilePath , 1 , false , (false == aUnicode) ? -2 : -1 );
    if ( null == fileSrc )
    {
        WScript.Echo( "Failed to open " + aFilePath );
        return false;
    }

    //CWUtils.WshFileSystem.CreateTextFile( filename [, overwrite=false [, unicode=false]] )
    fileTmp = CWUtils.WshFileSystem.CreateTextFile( tmpPath , true , aUnicode );
    if ( null == fileTmp )
    {
        WScript.Echo( "Failed to create " + tmpPath );
        return false;
    }
    
    while ( ! fileSrc.AtEndOfStream )
    {
        line = fileSrc.ReadLine();
        if ( aOldStr instanceof Array )
        {
            for ( var key in aOldStr )
            {
                if ( aNewStr instanceof Array )
                {
                    line = line.replace( aOldStr[key] , aNewStr[key] );
                }
                else
                {
                    line = line.replace( aOldStr[key] , aNewStr );
                }
            }
        }
        else
        {
            line = line.replace( aOldStr , aNewStr );
        }
        fileTmp.WriteLine( line );
    }
    
    fileSrc.Close();
    fileTmp.Close();

    CWUtils.WshFileSystem.CopyFile( tmpPath , aFilePath , true );
    CWUtils.WshFileSystem.DeleteFile( tmpPath , true );
    WScript.Echo( "Successfully replaced strings in " + aFilePath );
    return true;
};

CWUtils.MoveFilesToFolder = CWUtils.MoveFilesToFolder || function( aSrcFolder , aDstFolder )
{
    if ( 0 < aDstFolder.length && '\\' != aDstFolder.charAt(aDstFolder.length-1) )
    {
        aDstFolder += "\\";
    }
    var folder = CWUtils.WshFileSystem.GetFolder( aSrcFolder );
    var enumFolder = new Enumerator( folder.SubFolders );
    var uSubFolderCnt = 0;
    for ( ; ! enumFolder.atEnd() ; enumFolder.moveNext() )
    {
        uSubFolderCnt++;
        if ( true == MoveFilesToFolder( enumFolder.item().Path , aDstFolder ) )
        {
            uSubFolderCnt--;
        }
    }
    
    var bRet = true;
    var uFileCnt = 0;
    var enumFile = new Enumerator( folder.Files );
    for ( ; ! enumFile.atEnd() ; enumFile.moveNext() )
    {
        uFileCnt++;
        var strSrc = enumFile.item().Path;
        //WScript.Echo( "\"" + strSrc + "\" => \"" + aDstFolder + "\"" );
        try
        {
            if ( "MoveFilesToFolder.bat" == enumFile.item().Name )
            {
                WScript.Echo( "Skip. Path=\"" + strSrc + "\"" );
            }
            else if ( true == CWUtils.WshFileSystem.MoveFile( strSrc , aDstFolder ) )
            {
                WScript.Echo( "OK. Path=\"" + strSrc + "\"" );
                uFileCnt--;
            }
            else
            {
                WScript.Echo( "Failed. Path=\"" + strSrc + "\"" );
                bRet = false;
            }
        }
        catch ( err ) 
        {
            WScript.Echo( "CWUtils.WshFileSystem.MoveFile() failed. err=" + err.message + ". Path=" + strSrc );
            bRet = false;
        }
    }
    
    if ( 0 == uSubFolderCnt && 0 == uFileCnt )
    {
        CWUtils.WshFileSystem.DeleteFolder( aSrcFolder , true );
    }
    return bRet;
};

CWUtils.CWshTextFile = CWUtils.CWshTextFile || function()
{
    var m_WshFileSystem = null;
    var m_file = null;
    var m_mode = null;
    
    CWUtils.CWshTextFile.Mode = { ForReading:1 , ForWriting:2 , ForAppending:8 };
    
    this.Open = function( aFilePath , aMode , aUnicode , aCreate )
    {
        //WScript.Echo( "aFilePath=" + aFilePath + ", aMode=" + aMode );
        var bRet = false;
        try
        {
            if ( null == m_WshFileSystem )
            {
                m_WshFileSystem = new ActiveXObject( "Scripting.FileSystemObject" );
            }
            if ( null != m_file )
            {
                m_file.Close();
            }
            
            m_file = m_WshFileSystem.OpenTextFile( aFilePath , aMode , aCreate , (aUnicode) ? -1 : -2 );
            if ( null != m_file )
            {
                m_mode = aMode;
                bRet = true;
            }
        }
        catch ( err ) {}
        return bRet;
    }
    this.Close = function() { if (m_file) { m_file.Close(); } }
    
    this.Read = function( aCharCnt ) { return (m_file) ? m_file.Read(aCharCnt) : ""; }
    this.ReadLine = function() { return (m_file) ? m_file.ReadLine() : ""; }
    this.ReadAll = function() { return (m_file) ? m_file.ReadAll() : ""; }
    this.Skip = function( aCharCnt ) { if (m_file) { m_file.Skip(aCharCnt); } }
    this.SkipLine = function() { if (m_file) { m_file.SkipLine(); } }
    this.Write = function( aStr ) { if (m_file) { m_file.Write(aStr); } }
    this.WriteLine = function( aStr ) { if (m_file) { m_file.WriteLine(aStr); } }
    this.WriteBlankLines = function( aLineCnt ) { if (m_file) { m_file.WriteBlankLines(aLineCnt); } }
    
    this.AtEndOfStream = function() { return (m_file && CWUtils.CWshTextFile.Mode.ForReading == m_mode) ? m_file.AtEndOfStream : true; }
};

CWUtils.CAdoTextFile = CWUtils.CAdoTextFile || function()
{
    var m_AdoStream = null;
    var m_path = null;
    
    CWUtils.CAdoTextFile.ConnectModeEnum = { adModeUnknown:0 , adModeRead:1 , adModeWrite:2 , adModeReadWrite:3 , adModeShareDenyRead:4 , adModeShareDenyWrite:8 , adModeShareExclusive:12 , adModeShareDenyNone:16 , adModeRecursive:4194304 };
    CWUtils.CAdoTextFile.StreamTypeEnum = { adTypeBinary:1 , adTypeText:2 };
    CWUtils.CAdoTextFile.StreamReadEnum = { adReadAll:-1 , adReadLine:-2 };
    CWUtils.CAdoTextFile.StreamWriteEnum = { adWriteChar:0 , adWriteLine:1 };
    CWUtils.CAdoTextFile.SaveOptionsEnum = { adSaveCreateNotExist:1 , adSaveCreateOverWrite:2 };
    CWUtils.CAdoTextFile.LineSeparatorsEnum = { adCRLF:-1 , adLF:10 , adCR:13 };


    //aEncoding could be any codepage name such as _autodetect, ASCII, ISO-8859-1, BIG5, UNICODE, UTF-8
    //List of encoding: HKEY_CLASSES_ROOT\MIME\Database\Charset
    this.Open = function( aFilePath , aEncoding )
    {
        var bRet = false;
        try
        {
            if ( null != m_AdoStream )
            {
                m_AdoStream.Close();
            }
            if ( null == m_AdoStream )
            {
                m_AdoStream = WScript.CreateObject( "ADODB.Stream" );
            }

            m_path = aFilePath;

            m_AdoStream.CharSet = aEncoding;
            m_AdoStream.Type = CWUtils.CAdoTextFile.StreamTypeEnum.adTypeText;
            m_AdoStream.Open();
            m_AdoStream.LoadFromFile( aFilePath );

            bRet = true;
        }
        catch ( err ) {}
        return bRet;
    }
    this.Close = function() { if ( m_AdoStream ) { m_AdoStream.Close(); } }
   
    this.Write = function( aBuf )
    {
        if ( m_AdoStream )
        {
            m_AdoStream.WriteText( aBuf , CWUtils.CAdoTextFile.StreamWriteEnum.adWriteChar );
            //m_AdoStream.SaveToFile( m_path , CWUtils.CAdoTextFile.SaveOptionsEnum.adSaveCreateOverWrite );
            m_AdoStream.Flush();
        }
    }
    this.WriteLine = function( aBuf )
    {
        if ( m_AdoStream )
        {
            //m_AdoStream.LineSeparator = CWUtils.CAdoTextFile.LineSeparatorsEnum.adCRLF
            m_AdoStream.WriteText( aBuf , CWUtils.CAdoTextFile.StreamWriteEnum.adWriteLine );
            //m_AdoStream.SaveToFile( m_path , CWUtils.CAdoTextFile.SaveOptionsEnum.adSaveCreateOverWrite );
            m_AdoStream.Flush();
        }
    }
    
    this.Read = function( aCharCnt ) { return (m_AdoStream) ? m_AdoStream.ReadText(aCharCnt) : ""; }
    this.ReadLine = function() { return (m_AdoStream) ? m_AdoStream.ReadText(CWUtils.CAdoTextFile.StreamReadEnum.adReadLine) : ""; }
    this.ReadAll = function() { return (m_AdoStream) ? m_AdoStream.ReadText(CWUtils.CAdoTextFile.StreamReadEnum.adReadAll) : ""; }

    this.AtEndOfStream = function() { return (m_AdoStream) ? m_AdoStream.EOS : true; }
};


CWUtils.CBinaryFile = CWUtils.CBinaryFile || function()
{
    var adTypeBinary = 1;
    var adTypeText   = 2;
    var adSaveCreateOverWrite = 2;
    //The trick - this is the 'old fassioned' not translation page. It let javascript use strings to act like raw octets
    var codePage = '437';
   
    var forward  = new Array();
    var backward = new Array();
   
    //Note - for better performance I should preconvert these hex
    //definitions to decimal - at some point :-) - AJT
    forward['80'] = '00C7';    forward['81'] = '00FC';    forward['82'] = '00E9';
    forward['83'] = '00E2';    forward['84'] = '00E4';    forward['85'] = '00E0';
    forward['86'] = '00E5';    forward['87'] = '00E7';    forward['88'] = '00EA';
    forward['89'] = '00EB';    forward['8A'] = '00E8';    forward['8B'] = '00EF';
    forward['8C'] = '00EE';    forward['8D'] = '00EC';    forward['8E'] = '00C4';
    forward['8F'] = '00C5';    forward['90'] = '00C9';    forward['91'] = '00E6';
    forward['92'] = '00C6';    forward['93'] = '00F4';    forward['94'] = '00F6';
    forward['95'] = '00F2';    forward['96'] = '00FB';    forward['97'] = '00F9';
    forward['98'] = '00FF';    forward['99'] = '00D6';    forward['9A'] = '00DC';
    forward['9B'] = '00A2';    forward['9C'] = '00A3';    forward['9D'] = '00A5';
    forward['9E'] = '20A7';    forward['9F'] = '0192';    forward['A0'] = '00E1';
    forward['A1'] = '00ED';    forward['A2'] = '00F3';    forward['A3'] = '00FA';
    forward['A4'] = '00F1';    forward['A5'] = '00D1';    forward['A6'] = '00AA';
    forward['A7'] = '00BA';    forward['A8'] = '00BF';    forward['A9'] = '2310';
    forward['AA'] = '00AC';    forward['AB'] = '00BD';    forward['AC'] = '00BC';
    forward['AD'] = '00A1';    forward['AE'] = '00AB';    forward['AF'] = '00BB';
    forward['B0'] = '2591';    forward['B1'] = '2592';    forward['B2'] = '2593';
    forward['B3'] = '2502';    forward['B4'] = '2524';    forward['B5'] = '2561';
    forward['B6'] = '2562';    forward['B7'] = '2556';    forward['B8'] = '2555';
    forward['B9'] = '2563';    forward['BA'] = '2551';    forward['BB'] = '2557';
    forward['BC'] = '255D';    forward['BD'] = '255C';    forward['BE'] = '255B';
    forward['BF'] = '2510';    forward['C0'] = '2514';    forward['C1'] = '2534';
    forward['C2'] = '252C';    forward['C3'] = '251C';    forward['C4'] = '2500';
    forward['C5'] = '253C';    forward['C6'] = '255E';    forward['C7'] = '255F';
    forward['C8'] = '255A';    forward['C9'] = '2554';    forward['CA'] = '2569';
    forward['CB'] = '2566';    forward['CC'] = '2560';    forward['CD'] = '2550';
    forward['CE'] = '256C';    forward['CF'] = '2567';    forward['D0'] = '2568';
    forward['D1'] = '2564';    forward['D2'] = '2565';    forward['D3'] = '2559';
    forward['D4'] = '2558';    forward['D5'] = '2552';    forward['D6'] = '2553';
    forward['D7'] = '256B';    forward['D8'] = '256A';    forward['D9'] = '2518';
    forward['DA'] = '250C';    forward['DB'] = '2588';    forward['DC'] = '2584';
    forward['DD'] = '258C';    forward['DE'] = '2590';    forward['DF'] = '2580';
    forward['E0'] = '03B1';    forward['E1'] = '00DF';    forward['E2'] = '0393';
    forward['E3'] = '03C0';    forward['E4'] = '03A3';    forward['E5'] = '03C3';
    forward['E6'] = '00B5';    forward['E7'] = '03C4';    forward['E8'] = '03A6';
    forward['E9'] = '0398';    forward['EA'] = '03A9';    forward['EB'] = '03B4';
    forward['EC'] = '221E';    forward['ED'] = '03C6';    forward['EE'] = '03B5';
    forward['EF'] = '2229';    forward['F0'] = '2261';    forward['F1'] = '00B1';
    forward['F2'] = '2265';    forward['F3'] = '2264';    forward['F4'] = '2320';
    forward['F5'] = '2321';    forward['F6'] = '00F7';    forward['F7'] = '2248';
    forward['F8'] = '00B0';    forward['F9'] = '2219';    forward['FA'] = '00B7';
    forward['FB'] = '221A';    forward['FC'] = '207F';    forward['FD'] = '00B2';
    forward['FE'] = '25A0';    forward['FF'] = '00A0';
    backward['C7']   = '80';    backward['FC']   = '81';    backward['E9']   = '82';
    backward['E2']   = '83';    backward['E4']   = '84';    backward['E0']   = '85';
    backward['E5']   = '86';    backward['E7']   = '87';    backward['EA']   = '88';
    backward['EB']   = '89';    backward['E8']   = '8A';    backward['EF']   = '8B';
    backward['EE']   = '8C';    backward['EC']   = '8D';    backward['C4']   = '8E';
    backward['C5']   = '8F';    backward['C9']   = '90';    backward['E6']   = '91';
    backward['C6']   = '92';    backward['F4']   = '93';    backward['F6']   = '94';
    backward['F2']   = '95';    backward['FB']   = '96';    backward['F9']   = '97';
    backward['FF']   = '98';    backward['D6']   = '99';    backward['DC']   = '9A';
    backward['A2']   = '9B';    backward['A3']   = '9C';    backward['A5']   = '9D';
    backward['20A7'] = '9E';    backward['192']  = '9F';    backward['E1']   = 'A0';
    backward['ED']   = 'A1';    backward['F3']   = 'A2';    backward['FA']   = 'A3';
    backward['F1']   = 'A4';    backward['D1']   = 'A5';    backward['AA']   = 'A6';
    backward['BA']   = 'A7';    backward['BF']   = 'A8';    backward['2310'] = 'A9';
    backward['AC']   = 'AA';    backward['BD']   = 'AB';    backward['BC']   = 'AC';
    backward['A1']   = 'AD';    backward['AB']   = 'AE';    backward['BB']   = 'AF';
    backward['2591'] = 'B0';    backward['2592'] = 'B1';    backward['2593'] = 'B2';
    backward['2502'] = 'B3';    backward['2524'] = 'B4';    backward['2561'] = 'B5';
    backward['2562'] = 'B6';    backward['2556'] = 'B7';    backward['2555'] = 'B8';
    backward['2563'] = 'B9';    backward['2551'] = 'BA';    backward['2557'] = 'BB';
    backward['255D'] = 'BC';    backward['255C'] = 'BD';    backward['255B'] = 'BE';
    backward['2510'] = 'BF';    backward['2514'] = 'C0';    backward['2534'] = 'C1';
    backward['252C'] = 'C2';    backward['251C'] = 'C3';    backward['2500'] = 'C4';
    backward['253C'] = 'C5';    backward['255E'] = 'C6';    backward['255F'] = 'C7';
    backward['255A'] = 'C8';    backward['2554'] = 'C9';    backward['2569'] = 'CA';
    backward['2566'] = 'CB';    backward['2560'] = 'CC';    backward['2550'] = 'CD';
    backward['256C'] = 'CE';    backward['2567'] = 'CF';    backward['2568'] = 'D0';
    backward['2564'] = 'D1';    backward['2565'] = 'D2';    backward['2559'] = 'D3';
    backward['2558'] = 'D4';    backward['2552'] = 'D5';    backward['2553'] = 'D6';
    backward['256B'] = 'D7';    backward['256A'] = 'D8';    backward['2518'] = 'D9';
    backward['250C'] = 'DA';    backward['2588'] = 'DB';    backward['2584'] = 'DC';
    backward['258C'] = 'DD';    backward['2590'] = 'DE';    backward['2580'] = 'DF';
    backward['3B1']  = 'E0';    backward['DF']   = 'E1';    backward['393']  = 'E2';
    backward['3C0']  = 'E3';    backward['3A3']  = 'E4';    backward['3C3']  = 'E5';
    backward['B5']   = 'E6';    backward['3C4']  = 'E7';    backward['3A6']  = 'E8';
    backward['398']  = 'E9';    backward['3A9']  = 'EA';    backward['3B4']  = 'EB';
    backward['221E'] = 'EC';    backward['3C6']  = 'ED';    backward['3B5']  = 'EE';
    backward['2229'] = 'EF';    backward['2261'] = 'F0';    backward['B1']   = 'F1';
    backward['2265'] = 'F2';    backward['2264'] = 'F3';    backward['2320'] = 'F4';
    backward['2321'] = 'F5';    backward['F7']   = 'F6';    backward['2248'] = 'F7';
    backward['B0']   = 'F8';    backward['2219'] = 'F9';    backward['B7']   = 'FA';
    backward['221A'] = 'FB';    backward['207F'] = 'FC';    backward['B2']   = 'FD';
    backward['25A0'] = 'FE';    backward['A0']   = 'FF';

    var strDigitToStr = "0123456789ABCDEF";
    this.d2h = function( d )
    {
        var h = strDigitToStr.substr( d & 15 , 1 );
        while ( d > 15 )
        {
            d >>= 4;
            h = strDigitToStr.substr( d & 15 , 1 ) + h;
        }
        return h;
    }

    this.h2d = function( h )
    {
        return parseInt(h,16);
    }
   
    this.WriteAll = function( aFilePath , aText )
    {
        var AdoStream = null;
        try
        {
            WScript.CreateObject( "ADODB.Stream" );
            AdoStream.Type = adTypeText;    //Specify stream type - we cheat and get string but 'like' binary
            AdoStream.CharSet = codePage;
            AdoStream.Open();
            AdoStream.WriteText( this.Handle437(aText , true) );
            AdoStream.SaveToFile( aFilePath , adSaveCreateOverWrite );
        }
        catch ( err ) {}
        if ( null != AdoStream )
        {
            AdoStream.Close();
        }
    }

    this.ReadAll  = function( aFilePath )
    {
        var AdoStream = null;
        try
        {
            WScript.CreateObject( "ADODB.Stream" );
            AdoStream.Type = adTypeText;     //Specify stream type - we cheat and get string but 'like' binary
            AdoStream.CharSet = codePage;
            AdoStream.Open();
            AdoStream.LoadFromFile( aFilePath );
            var strWhat = AdoStream.ReadText;
            AdoStream.Close();
            return this.Handle437( strWhat , false );
        }
        catch ( err )
        {
            return "";
        }
    }
   
    //bEncodeToCodePage437 = true: Convert a octet number to a code page 437 char code
    //bEncodeToCodePage437 = false: Convert a code page 437 char code to a octet number
    this.Handle437 = function( inString , bEncodeToCodePage437 )
    {
        var encArray = new Array();
        var tmp = '';
        var c = 0;
        var cc;
        var h;
        var table = ( true == bEncodeToCodePage437 ) ? forward : backward;
        for ( var i = 0 ; i < inString.length ; ++i )
        {
            c++;
            if ( c == 128 )
            {
                encArray.push( tmp );
                tmp = '';
                c = 0;
            }
            cc = inString.charCodeAt( i );
            if ( cc < 128 )
            {
                tmp += String.fromCharCode( cc );
            }
            else
            {
                h = table['' + this.d2h(cc)];
                tmp += String.fromCharCode( this.h2d(h) );
            }
        }
        if ( tmp != '' )
        {
            encArray.push( tmp );
        }

        //This loop progressive concatonates the array elements until there is only one
        var ar2 = new Array();
        for ( ; encArray.length > 1 ; )
        {
            for ( var c = 0 ; c < encArray.length ; c += 2 )
            {
                if ( c + 1 == encArray.length )
                {
                    ar2.push( encArray[c] );
                }
                else
                {
                    ar2.push( '' + encArray[c] + encArray[c+1] );
                }
            }
            encArray = ar2;
            ar2 = new Array();
        }
        return encArray[0];
    }
};
