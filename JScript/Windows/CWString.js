/*
 * Copyright (c) 2009-2020, ChienWei Hung <winestwinest@gmail.com>
 * CWUtils is published under the BSD-3-Clause license.
 *
 * CWUtils is a set of standalone APIs for developers to speed up their
 * programming. It should be very easy to port them to other projects or
 * learn how to implement things on different languages and platforms.
 *
 * The latest version can be found at https://github.com/winest/CWUtils
 */

/*
 * Provide stand-alone string-related utilities for JScript developers.
 */

//Avoid multiple instances
var CWUtils = CWUtils || {};

CWUtils.CreateLookup32 = CWUtils.CreateLookup32 || function()
{
    var result = new Array();
    for ( var i = 0 ; i < 256 ; i++ )
    {
        var s = ("00" + i.toString(16)).slice(-2);
        result[i] = ( s.charCodeAt(0) ) + ( ( s.charCodeAt(1) << 16 ) );
    }
    return result;
};
CWUtils.m_lookup32 = CWUtils.m_lookup32 || CWUtils.CreateLookup32();


CWUtils.StringToHexDump = CWUtils.StringToHexDump || function( aString , aBytesPerLine , aSplitter )
{
    aBytesPerLine = aBytesPerLine || 16;
    aSplitter = aSplitter || " | ";

    //<8 bytes address><aSplitter><aBytesPerLine's hex with (aBytesPerLine - 1)'s space><aSplitter><aBytesPerLine's ascii>
    var nOneLineSize = 8 + aSplitter.length * 2 + aBytesPerLine * 4 - 1 + "\n".length;
    var strTmp = "";
    var strBuilder = "";
    for ( var i = 0 ; i < aString.length ; i += aBytesPerLine )
    {
        strBuilder += ("00000000" + i.toString(16)).slice(-8);
        strBuilder += aSplitter;
        for ( var j = 0 ; j < aBytesPerLine ; j++ )
        {
            var nCurr = i + j;
            if ( nCurr >= aString.length )
            {
                for ( var k = 0 ; k < (aBytesPerLine - j) * 3 - 1 ; k++ )
                {
                    strBuilder += " ";
                }
                break;
            }

            //if ( CWUtils.m_lookup32[aString.charCodeAt(nCurr)] )
            //{
            //    WScript.Echo( nCurr + " => " + aString.charCodeAt(nCurr) + " => " + (aString.charCodeAt(nCurr) & 0xFF) + " => " + CWUtils.m_lookup32[aString.charCodeAt(nCurr) & 0xFF].toString(16) + " => " + CWUtils.m_lookup32[50].toString(16) );
            //}
            //else
            //{
            //    WScript.Echo( nCurr + " => " + aString.charCodeAt(nCurr) + " => " + (aString.charCodeAt(nCurr) & 0xFF) + " => " + CWUtils.m_lookup32[aString.charCodeAt(nCurr) & 0xFF] + " => " + CWUtils.m_lookup32[50].toString(16));
            //}
            var val = CWUtils.m_lookup32[aString.charCodeAt(nCurr) & 0xFF];
            strBuilder += String.fromCharCode( val & 0xFFFF );
            strBuilder += String.fromCharCode( (val >> 16) & 0xFFFF );
            if ( j + 1 < aBytesPerLine )
            {
                strBuilder += " ";
            }
        }

        strBuilder += aSplitter;

        for ( var j = 0 ; j < aBytesPerLine ; j++ )
        {
            var nCurr = i + j;
            if ( nCurr >= aString.length )
            {
                for ( var k = 0 ; k < aBytesPerLine - j ; k++ )
                {
                    strBuilder += " ";
                }
                break;
            }

            var code = aString.charCodeAt( nCurr );
            strBuilder += ((code >= 0x0000 && code <= 0x001f) || (code >= 0x007f && code <= 0x009f)) ? "." : aString.charAt(nCurr);
        }
        strBuilder += "\n";
    }
    return strBuilder;
};


CWUtils.sprintf = CWUtils.sprintf || function()
{
    // http://kevin.vanzonneveld.net
    // *     example 1: sprintf("%01.2f", 123.1);
    // *     returns 1: 123.10
    // *     example 2: sprintf("[%10s]", 'monkey');
    // *     returns 2: '[    monkey]'
    // *     example 3: sprintf("[%'#10s]", 'monkey');
    // *     returns 3: '[####monkey]'
    var regex = /%%|%(\d+\$)?([-+\'#0 ]*)(\*\d+\$|\*|\d+)?(\.(\*\d+\$|\*|\d+))?([scboxXuideEfFgG])/g;
    var a = arguments, i = 0, format = a[i++];

    // pad()
    var pad = function(str, len, chr, leftJustify)
    {
        if( !chr )
        {
            chr = ' ';
        }
        var padding = (str.length >= len) ? '' : Array(1 + len - str.length >>> 0).join(chr);
        return leftJustify ? str + padding : padding + str;
    };

    // justify()
    var justify = function(value, prefix, leftJustify, minWidth, zeroPad, customPadChar)
    {
        var diff = minWidth - value.length;
        if( diff > 0 )
        {
            if( leftJustify || !zeroPad )
            {
                value = pad(value, minWidth, customPadChar, leftJustify);
            }
            else
            {
                value = value.slice(0, prefix.length) + pad('', diff, '0', true) + value.slice(prefix.length);
            }
        }
        return value;
    };

    // formatBaseX()
    var formatBaseX = function(value, base, prefix, leftJustify, minWidth, precision, zeroPad)
    {
        // Note: casts negative numbers to positive ones
        var number = value >>> 0;
        prefix = prefix && number && {
            '2': '0b',
            '8': '0',
            '16': '0x'
        }[base] || '';
        value = prefix + pad(number.toString(base), precision || 0, '0', false);
        return justify(value, prefix, leftJustify, minWidth, zeroPad);
    };

    // formatString()
    var formatString = function(value, leftJustify, minWidth, precision, zeroPad, customPadChar)
    {
        if( precision != null )
        {
            value = value.slice(0, precision);
        }
        return justify(value, '', leftJustify, minWidth, zeroPad, customPadChar);
    };

    // doFormat()
    var doFormat = function(substring, valueIndex, flags, minWidth, _, precision, type)
    {
        var number;
        var prefix;
        var method;
        var textTransform;
        var value;

        if( substring == '%%' )
        {
            return '%';
        }

        // parse flags
        var leftJustify = false,
        positivePrefix = '',
        zeroPad = false,
        prefixBaseX = false,
        customPadChar = ' ';
        var flagsl = flags.length;
        for( var j = 0; flags && j < flagsl; j++ )
        {
            switch( flags.charAt(j) )
            {
                case ' ':
                    positivePrefix = ' ';
                    break;
                case '+':
                    positivePrefix = '+';
                    break;
                case '-':
                    leftJustify = true;
                    break;
                case "'":
                    customPadChar = flags.charAt(j + 1);
                    break;
                case '0':
                    zeroPad = true;
                    break;
                case '#':
                    prefixBaseX = true;
                    break;
            }
        }

        // parameters may be null, undefined, empty-string or real valued
        // we want to ignore null, undefined and empty-string values
        if( !minWidth )
        {
            minWidth = 0;
        }
        else if( minWidth == '*' )
        {
            minWidth = +a[i++];
        }
        else if( minWidth.charAt(0) == '*' )
        {
            minWidth = +a[minWidth.slice(1, -1)];
        }
        else
        {
            minWidth = +minWidth;
        }

        // Note: undocumented perl feature:
        if( minWidth < 0 )
        {
            minWidth = -minWidth;
            leftJustify = true;
        }

        if( !isFinite(minWidth) )
        {
            throw new Error('sprintf: (minimum-)width must be finite');
        }

        if( !precision )
        {
            precision = 'fFeE'.indexOf(type) > -1 ? 6 : (type == 'd') ? 0 : undefined;
        }
        else if( precision == '*' )
        {
            precision = +a[i++];
        }
        else if( precision.charAt(0) == '*' )
        {
            precision = +a[precision.slice(1, -1)];
        }
        else
        {
            precision = +precision;
        }

        // grab value using valueIndex if required?
        value = valueIndex ? a[valueIndex.slice(0, -1)] : a[i++];

        switch( type )
        {
            case 's':
                return formatString(String(value), leftJustify, minWidth, precision, zeroPad, customPadChar);
            case 'c':
                return formatString(String.fromCharCode(+value), leftJustify, minWidth, precision, zeroPad);
            case 'b':
                return formatBaseX(value, 2, prefixBaseX, leftJustify, minWidth, precision, zeroPad);
            case 'o':
                return formatBaseX(value, 8, prefixBaseX, leftJustify, minWidth, precision, zeroPad);
            case 'x':
                return formatBaseX(value, 16, prefixBaseX, leftJustify, minWidth, precision, zeroPad);
            case 'X':
                return formatBaseX(value, 16, prefixBaseX, leftJustify, minWidth, precision, zeroPad).toUpperCase();
            case 'u':
                return formatBaseX(value, 10, prefixBaseX, leftJustify, minWidth, precision, zeroPad);
            case 'i':
            case 'd':
                number = (+value) | 0;
                prefix = number < 0 ? '-' : positivePrefix;
                value = prefix + pad(String(Math.abs(number)), precision, '0', false);
                return justify(value, prefix, leftJustify, minWidth, zeroPad);
            case 'e':
            case 'E':
            case 'f': // Should handle locales (as per setlocale)
            case 'F':
            case 'g':
            case 'G':
                number = +value;
                prefix = number < 0 ? '-' : positivePrefix;
                method = ['toExponential', 'toFixed', 'toPrecision']['efg'.indexOf(type.toLowerCase())];
                textTransform = ['toString', 'toUpperCase']['eEfFgG'.indexOf(type) % 2];
                value = prefix + Math.abs(number)[method](precision);
                return justify(value, prefix, leftJustify, minWidth, zeroPad)[textTransform]();
            default:
                return substring;
        }
    };

    return format.replace(regex, doFormat);
};