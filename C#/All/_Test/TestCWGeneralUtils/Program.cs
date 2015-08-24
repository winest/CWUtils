using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace TestCWGeneralUtils
{
    class Program
    {
        static void Main( string [] aArgs )
        {
            List<int> lsNum = new List<int>();
            for ( int i = 0 ; i < 10 ; i++ )
            {
                lsNum.Add( i );
            }
            Console.WriteLine( "Before: {0}" , string.Join("," , lsNum) );

            CWUtils.CWGeneralUtils.Randomize<int>( lsNum );

            Console.WriteLine( "After: {0}" , string.Join("," , lsNum) );
        }
    }
}
