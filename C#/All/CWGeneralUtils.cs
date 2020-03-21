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

using System;
using System.Collections;
using System.IO;
using System.Text;

namespace CWUtils
{
    public static class CWGeneralUtils
    {
        public static void Randomize<T>( this System.Collections.Generic.IList<T> aList )
        {
            Random rng = new Random();
            int n = aList.Count;
            while ( n > 1 )
            {
                n--;
                int k = rng.Next( n + 1 );
                T value = aList[k];
                aList[k] = aList[n];
                aList[n] = value;
            }
        }
    }
}