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
