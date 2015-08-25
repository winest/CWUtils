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
 * Provide stand-alone xmlhttp-related utilities for JScript developers.
 */

//Avoid multiple instances
var CWUtils = CWUtils || {};

CWUtils.CXml = CWUtils.CXml || function()
{
    this.objDom = null;
    this.document = null;
    
    this.UnInit = function()
    {
        objHttp = null;
        objHtml = null;
        this.window = null;
        this.document = null;
    }
    
    this.LoadFromFile = function( aFilePath )
    {
        var bRet = false;
        var types = ["Msxml2.DOMDocument.6.0" , "MMsxml2.DOMDocument.3.0" , "Microsoft.XMLDOM"];
        for ( var i = 0 ; i < types.length ; i++ )
        {
            try
            {
                this.objDom = new ActiveXObject( types[i] );
                this.objDom.async = false;
                this.objDom.load( aFilePath );
                
                bRet = true;
                break;
            }
            catch( e ) {}
        }

        if ( true == bRet )
        {
            this.document = this.objDom;
            this.document.setProperty( "SelectionLanguage" , "XPath" );
        }
    }
    
    this.LoadFromString = function( aXmlContent )
    {
        var bRet = false;
        var types = ["Msxml2.DOMDocument.6.0" , "MMsxml2.DOMDocument.3.0" , "Microsoft.XMLDOM"];
        for ( var i = 0 ; i < types.length ; i++ )
        {
            try
            {
                this.objDom = new ActiveXObject( types[i] );
                this.objDom.async = false;
                this.objDom.loadXML( aXmlContent );
                
                bRet = true;
                break;
            }
            catch( e ) {}
        }

        if ( true == bRet )
        {
            this.document = this.objDom;
            this.document.setProperty( "SelectionLanguage" , "XPath" );
        }
    }

    this.SaveToFile = function( aFilePath )
    {
        if ( this.document )
        {
            this.document.save( aFilePath );
        }
    }
    
    //We write a version of ourselves because Microsoft's version doesn't support namespace in XML (xmlns)
    //For accessing elements in namespace, use selectNodes()
    this.GetElementsByTagName = function( aTagName , aParent )
    {
        var elements = (aParent || this.document).getElementsByTagName('*');
        var pattern = new RegExp('(^|\\s)' + aTagName + '(\\s|$)');
        var results = [];
        for ( var i = 0 ; i < elements.length ; i++ )
        {
            if ( ! pattern.test(elements[i].tagName) )
            {
                continue;
            }
            results.push( elements[i] );
        }
        return results;
    }

    this.GetNamespaceByIndex = function( aElementIndex )
    {
        if ( this.document )
        {
            if ( 0 < nodes.length && aElementIndex < nodes[0].childNodes.length )
            {
                return nodes[0].childNodes[aElementIndex].namespaceURI;
            }
        }
        return "";
    }

    this.GetNamespacesByName = function( aElementName )
    {
        var aryRet = new Array();
        if ( this.document )
        {
            var aryElements = this.GetElementsByTagName( aElementName );
            for ( var i = 0 ; i < aryElements.length ; i++ )
            {
                aryRet.push( aryElements[i].namespaceURI );
            }
        }
        return aryRet;
    }
};



CWUtils.CHttp = CWUtils.CHttp || function()
{
    var objHttp = null;
    var objHtml = null;
    this.window = null;
    this.document = null;

    this.UnInit = function()
    {
        objHttp = null;
        objHtml = null;
        this.window = null;
        this.document = null;
    }

    this.Connect = function( aMethod , aUrl , aParameter )
    {
        this.UnInit();

        //Refer to WinHttpRequest object on http://msdn.microsoft.com/en-us/library/windows/desktop/aa384106(v=vs.85).aspx
        objHttp = new ActiveXObject( "WinHttp.WinHttpRequest.5.1" );

        objHttp.Open( aMethod , aUrl , false );
        if ( null != aParameter )
        {
            objHttp.SetRequestHeader( "Content-Type" , "application/x-www-form-urlencoded" );
        }

        objHttp.Send( aParameter );
        if ( 200 != objHttp.Status )
        {
            WScript.Echo( "Failed to connect with status code " + objHttp.Status + "(" + objHttp.StatusText + ")" );
            return false;
        }
        
        return this.LoadFromString( objHttp.ResponseText );
    }
    
    this.LoadFromFile = function( aFilePath , aEncoding )
    {
        var bRet = false;
        try
        {
            var file = WScript.CreateObject( "ADODB.Stream" );
            file.CharSet = aEncoding;
            file.Type = 2;
            file.Open();
            file.LoadFromFile( aFilePath );
            bRet = this.LoadFromString( file.ReadText( -1 ) );
            file.Close();
        }
        catch ( err ) {}
    }
    
    this.LoadFromString = function( aString )
    {
        //Refer to IHTMLDocument object on http://msdn.microsoft.com/en-us/library/hh801967(v=vs.85).aspx
        objHtml = new ActiveXObject( "htmlfile" );
        objHtml.open();
        objHtml.write( aString );
        objHtml.close();
        
        //Refer to IHTMLElement object on http://msdn.microsoft.com/en-us/library/aa752279(v=vs.85).aspx
        //and html element object on http://msdn.microsoft.com/en-us/library/ms535255(v=vs.85).aspx
        if ( null == objHtml.parentWindow || null == objHtml.parentWindow.document )
        {
            WScript.Echo( "window or document is null" );
            return false;
        }
        
        this.window = objHtml.parentWindow;
        this.document = objHtml.parentWindow.document;
        this.document.getElementsByClassName = function( aClassName , aParent )
        {
            var elements = (aParent || this).getElementsByTagName('*');
            var pattern = new RegExp('(^|\\s)' + aClassName + '(\\s|$)');
            var results = [];
            for ( var i = 0 ; i < elements.length ; i++ )
            {
                if ( ! pattern.test(elements[i].className) )
                {
                    continue;
                }
                results.push( elements[i] );
            }
            return results;
        }

        return true;
    }

    this.GetStatusCode = function() { return ( objHttp ) ? objHttp.Status : -1; }
    this.GetStatusText = function() { return ( objHttp ) ? objHttp.StatusText : ""; }
    this.GetReponse = function() { return ( objHttp ) ? objHttp.Response : null; }
    this.GetReponseText = function() { return ( objHttp ) ? objHttp.ResponseText : ""; }
};
