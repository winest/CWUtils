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
 * Provide stand-alone math-related utilities for JScript developers.
 */
 
var CWUtils = CWUtils || {};

//Generate a random integer in [aMin , aMax]
CWUtils.RandInt = CWUtils.RandInt || function( aMin , aMax )
{
    return Math.floor( (aMax-aMin+1) * Math.random() + aMin );
};

//Generate a random character from the given sample set
CWUtils.RandChar = CWUtils.RandChar || function( aSample )
{
    if ( aSample == null )
    {
        aSample = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    }
    return aSample.charAt( Math.floor( aSample.length * Math.random() ) );
};

//Generate a random character from the given string length and sample set
CWUtils.RandStr = CWUtils.RandStr || function( aLength , aSample )
{
    var str = "";
    for ( var i = 0 ; i < aLength ; i++ )
    {
        str += CWUtils.RandChar( aSample );
    }
    return str;
};

//Get one of the array element randomlly
CWUtils.RandAryElement = CWUtils.RandAryElement || function( aAry )
{
    var element = null;
    var nCurr = 0;
    var nRandIndex = CWUtils.RandInt( 0 , aAry.length - 1 );

    for ( var tmp in aAry )
    {
        if ( nCurr == nRandIndex )
        {
            element = aAry[tmp];
            break;
        }
        else
        {
            nCurr++;
        }
    }
    return element;
};
