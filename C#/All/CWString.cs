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
    class CWString
    {
        private static readonly uint[] m_lookup32 = CreateLookup32();
        private static uint[] CreateLookup32()
        {
            var result = new uint[256];
            for ( int i = 0 ; i < 256 ; i++ )
            {
                string s = i.ToString( "X2" );
                result[i] = ( (uint)s[0] ) + ( (uint)s[1] << 16 );
            }
            return result;
        }

        public static string ByteArrayToHexString( byte[] aBytes , string aSplitter )
        {
            char [] result = new char[aBytes.Length * ( 2 + aSplitter.Length )];
            for ( int i = 0 ; i < aBytes.Length ; i++ )
            {
                int nCurr = i * (2 + aSplitter.Length);
                var val = m_lookup32[aBytes[i]];
                result[nCurr] = (char)val;
                result[nCurr + 1] = (char)(val >> 16);

                for ( int j = 0 ; j < aSplitter.Length ; j++ )
                {
                    result[nCurr + 2 + j] = Convert.ToChar(aSplitter[j]);
                }
            }
            return new string( result , 0 , result.Length -  aSplitter.Length );
        }

        public static byte[] HexStringToByteArray( string aHexStr )
        {
            if ( 0 < (aHexStr.Length % 2) )
            {
                throw new ArgumentException( "Length if not the multiple of 2" );
            }

            byte [] bytes = new byte[aHexStr.Length / 2];
            for ( int i = 0 ; i < bytes.Length ; i++ )
            {
                int hi = aHexStr[i*2] - 65;
                hi = hi + 10 + ((hi >> 31) & 7);

                int lo = aHexStr[i*2 + 1] - 65;
                lo = lo + 10 + ((lo >> 31) & 7) & 0x0f;

                bytes[i] = (byte)(lo | hi << 4);
            }
            return bytes;
        }

        public static string ByteArrayToHexDump( byte[] aBytes , int aBytesPerLine = 16 , string aSplitter = " | " )
        {
            //<8 bytes address><aSplitter><aBytesPerLine's hex with (aBytesPerLine - 1)'s space><aSplitter><aBytesPerLine's ascii>
            int nOneLineSize = 8 + aSplitter.Length * 2 + aBytesPerLine * 4 - 1 + Environment.NewLine.Length;
            StringBuilder strBuilder = new StringBuilder();
            strBuilder.EnsureCapacity( (int)Math.Ceiling((double)aBytes.Length/(double)aBytesPerLine) * nOneLineSize );
            for ( int i = 0 ; i < aBytes.Length ; i += aBytesPerLine )
            {
                strBuilder.AppendFormat( "{0:X8}{1}" , i , aSplitter );
                for ( int j = 0 ; j < aBytesPerLine ; j++ )
                {
                    int nCurr = i + j;
                    if ( nCurr >= aBytes.Length )
                    {
                        strBuilder.Append( ' ' , (aBytesPerLine - j) * 3 - 1 );
                        break;
                    }
                    var val = m_lookup32[aBytes[nCurr]];
                    strBuilder.Append( (char)val );
                    strBuilder.Append( (char)(val >> 16) );
                    if ( j + 1 < aBytesPerLine )
                    {
                        strBuilder.Append( ' ' );
                    }
                }

                strBuilder.Append( aSplitter );

                for ( int j = 0 ; j < aBytesPerLine ; j++ )
                {
                    int nCurr = i + j;
                    if ( nCurr >= aBytes.Length )
                    {
                        strBuilder.Append( ' ' , aBytesPerLine - j );
                        break;
                    }
                    strBuilder.Append( char.IsControl((char)aBytes[nCurr]) ? '.' : (char)aBytes[nCurr] );
                }
                strBuilder.AppendLine();
            }
            return strBuilder.ToString();
        }

        public static byte[] HexDumpToByteArray( string aHexDump , int aBytesPerLine = 16 , string aSplitter = " | " )
        {
            //<8 bytes address><aSplitter><aBytesPerLine's hex with (aBytesPerLine - 1)'s space><aSplitter><aBytesPerLine's ascii>
            int nOneLineSize = 8 + aSplitter.Length * 2 + aBytesPerLine * 4 - 1 + Environment.NewLine.Length;
            if ( 0 != aHexDump.Length % nOneLineSize )
            {
                throw new ArgumentException( string.Format("Length if not the multiple of {0}" , nOneLineSize) );
            }
            int nTotalLines = ( aHexDump.Length / nOneLineSize );
            byte [] bytes = new byte[nTotalLines *  aBytesPerLine];
            int nRealSize = 0;
            for ( int i = 0 ; i < nTotalLines ; i++ )
            {
                for ( int j = 0 ; j < aBytesPerLine ; j++ )
                {
                    if ( ' ' == aHexDump[i * nOneLineSize + 8 + aSplitter.Length + 3*j] )
                    {
                        break;
                    }
                    int hi = aHexDump[i * nOneLineSize + 8 + aSplitter.Length + 3*j] - 65;
                    hi = hi + 10 + ((hi >> 31) & 7);
                    int lo = aHexDump[i * nOneLineSize + 8 + aSplitter.Length + 3*j+1] - 65;
                    lo = lo + 10 + ((lo >> 31) & 7) & 0x0f;
                    bytes[nRealSize] = (byte)(lo | hi << 4);
                    nRealSize++;
                }
            }

            if ( nRealSize != nTotalLines *  aBytesPerLine )
            {
                Array.Resize<byte>( ref bytes , nRealSize );
            }
            return bytes;
        }

    }
}

