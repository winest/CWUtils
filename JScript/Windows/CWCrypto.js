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
 * Provide stand-alone cryptography-related utilities for JScript developers.
 */

var CWUtils = CWUtils || {};

CWUtils.CaesarEncrypt = CWUtils.CaesarEncrypt || function( aSmallAlpha , aBigAlpha , aNumber , aDecryptedText )
{
    aSmallAlpha = aSmallAlpha || "abcdefghijklmnopqrstuvwxyz";
    aBigAlpha = aBigAlpha || "ABCDEFGHIJKLMNOPQRSTUVWXZY";
    aNumber = aNumber || "0123456789";
    var strEncrypted = "";

    do
    {
        if ( 26 != aSmallAlpha.length )
        {
            WScript.Echo( "Length of aSmallAlpha should be 26" );
            break;
        }
        if ( 26 != aBigAlpha.length )
        {
            WScript.Echo( "Length of aBigAlpha should be 26" );
            break;
        }
        if ( 10 != aNumber.length )
        {
            WScript.Echo( "Length of aNumber should be 10" );
            break;
        }

        for ( var i = 0 ; i < aDecryptedText.length ; i++ )
        {
            if ( 'a'.charCodeAt(0) <= aDecryptedText.charCodeAt(i) && aDecryptedText.charCodeAt(i) <= 'z'.charCodeAt(0) )
            {
                strEncrypted += aSmallAlpha.charAt( aDecryptedText.charCodeAt(i) - 'a'.charCodeAt(0) );
            }
            else if ( 'A'.charCodeAt(0) <= aDecryptedText.charCodeAt(i) && aDecryptedText.charCodeAt(i) <= 'Z'.charCodeAt(0) )
            {
                strEncrypted += aBigAlpha.charAt( aDecryptedText.charCodeAt(i) - 'A'.charCodeAt(0) );
            }
            else if ( '0'.charCodeAt(0) <= aDecryptedText.charCodeAt(i) && aDecryptedText.charCodeAt(i) <= '9'.charCodeAt(0) )
            {
                strEncrypted += aNumber.charAt( aDecryptedText.charCodeAt(i) - '0'.charCodeAt(0) );
            }
            else
            {
                WScript.Echo( "Unknown non-alphanum character. [" + i + "]=" + aDecryptedText.charAt( i ) );
                strEncrypted += aDecryptedText.charAt( i );
            }
        }
    } while ( 0 );

    return strEncrypted;
};

CWUtils.CaesarDecrypt = CWUtils.CaesarDecrypt || function( aSmallAlpha , aBigAlpha , aNumber , aEncryptedText )
{
    aSmallAlpha = aSmallAlpha || "abcdefghijklmnopqrstuvwxyz";
    aBigAlpha = aBigAlpha || "ABCDEFGHIJKLMNOPQRSTUVWXZY";
    aNumber = aNumber || "0123456789";
    var strDecrypted = "";

    do
    {
        if ( 26 != aSmallAlpha.length )
        {
            WScript.Echo( "Length of aSmallAlpha should be 26" );
            break;
        }
        if ( 26 != aBigAlpha.length )
        {
            WScript.Echo( "Length of aBigAlpha should be 26" );
            break;
        }
        if ( 10 != aNumber.length )
        {
            WScript.Echo( "Length of aNumber should be 10" );
            break;
        }

        for ( var i = 0 ; i < aEncryptedText.length ; i++ )
        {
            if ( 'a'.charCodeAt(0) <= aEncryptedText.charCodeAt(i) && aEncryptedText.charCodeAt(i) <= 'z'.charCodeAt(0) )
            {
                strDecrypted += String.fromCharCode( 'a'.charCodeAt(0) + aSmallAlpha.indexOf( aEncryptedText.charAt(i) ) );
            }
            else if ( 'A'.charCodeAt(0) <= aEncryptedText.charCodeAt(i) && aEncryptedText.charCodeAt(i) <= 'Z'.charCodeAt(0) )
            {
                strDecrypted += String.fromCharCode( 'A'.charCodeAt(0) + aBigAlpha.indexOf( aEncryptedText.charAt(i) ) );
            }
            else if ( '0'.charCodeAt(0) <= aEncryptedText.charCodeAt(i) && aEncryptedText.charCodeAt(i) <= '9'.charCodeAt(0) )
            {
                strDecrypted += String.fromCharCode( '0'.charCodeAt(0) + aNumber.indexOf( aEncryptedText.charAt(i) ) );
            }
            else
            {
                WScript.Echo( "Unknown non-alphanum character. [" + i + "]=" + aEncryptedText.charAt( i ) );
                strDecrypted += aEncryptedText.charAt( i );
            }
        }
    } while ( 0 );

    return strDecrypted;
};


CWUtils.Base64Encode = CWUtils.Base64Encode || function( aString , aEncodeTable , aEscape )
{
    aEncodeTable = aEncodeTable || "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
    aEscape = aEscape || false;
    if ( 65 != aEncodeTable.length )
    {
        WScript.Echo( "Table length should be 65" );
        return null;
    }

    var strOutput = "";
    var i = 0;
    do
    {
        var ch1 = aString.charCodeAt( i++ );
        var ch2 = aString.charCodeAt( i++ );
        var ch3 = aString.charCodeAt( i++ );

        var enc1 = ch1 >> 2;
        var enc2 = ( (ch1 & 3) << 4 ) | ( ch2 >> 4 );
        var enc3 = ( (ch2 & 15) << 2 ) | ( ch3 >> 6 );
        var enc4 = ch3 & 63;

        if ( isNaN(ch2) )
        {
            enc3 = enc4 = 64;
        }
        else if ( isNaN(ch3) )
        {
            enc4 = 64;
        }
        else {}

        strOutput = strOutput + aEncodeTable.charAt( enc1 ) + aEncodeTable.charAt( enc2 ) + aEncodeTable.charAt( enc3 ) + aEncodeTable.charAt( enc4 );
    } while( i < aString.length );

    return ( aEscape ) ? escape( strOutput ) : strOutput;
}


CWUtils.Base64Decode = CWUtils.Base64Decode || function( aString , aDecodeTable , aUnescape )
{
    aDecodeTable = aDecodeTable || "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
    aUnescape = aUnescape || false;
    if ( 65 != aDecodeTable.length )
    {
        WScript.Echo( "Table length should be 65" );
        return null;
    }
    
    if ( aUnescape )
    {
        aString = unescape( aString );
    }
    var reTestBase64 = /[^A-Za-z0-9\+\/\=]/g;
    if ( reTestBase64.exec(aString) )
    {
        WScript.Echo( "Invalid character found in string" );
        return null;
    }

    var strOutput = "";
    var i = 0;
    do
    {
        var enc1 = aDecodeTable.indexOf( aString.charAt(i++) );
        var enc2 = aDecodeTable.indexOf( aString.charAt(i++) );
        var enc3 = aDecodeTable.indexOf( aString.charAt(i++) );
        var enc4 = aDecodeTable.indexOf( aString.charAt(i++) );

        var ch1 = ( enc1 << 2 ) | ( enc2 >> 4 );
        var ch2 = ( (enc2 & 15) << 4 ) | ( enc3 >> 2 );
        var ch3 = ( (enc3 & 3) << 6 ) | enc4;

        strOutput += String.fromCharCode( ch1 );

        if ( enc3 != 64 )
        {
            strOutput += String.fromCharCode( ch2 );
        }
        if ( enc4 != 64 )
        {
            strOutput += String.fromCharCode( ch3 );
        }
    } while ( i < aString.length );

    return strOutput;
}
