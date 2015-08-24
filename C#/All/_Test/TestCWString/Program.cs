using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace TestCWString
{
    class Program
    {
        static void Main( string [] aArgs )
        {
            byte [] byTestString = new byte[] { 0x00 , 0x30 , 0x39 , 0x41 , 0x46 , 0x61 , 0x66 , 0xFF };
            Console.WriteLine( CWUtils.CWString.ByteArrayToHexString( byTestString , " , ") );

            byte [] byTest = CWUtils.CWString.HexStringToByteArray( "30394146616600112233445566778899AABBCCDDEEFF0D0AAABBCC" );

            string strHexDump = CWUtils.CWString.ByteArrayToHexDump( byTest , 16 , " | " );
            Console.WriteLine( strHexDump );

            byte [] byHexDump = CWUtils.CWString.HexDumpToByteArray( strHexDump , 16 , " | " );
            Console.WriteLine( CWUtils.CWString.ByteArrayToHexDump( byHexDump , 16 , " | " ) );
        }
    }
}
