using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using CWUtils;

namespace TestCWFile
{
    class Program
    {
        static void Main( string [] aArgs )
        {
            TestLog();
            TestIni();

            Console.WriteLine( "End of the program" );
            Console.ReadKey();
        }

        static void TestLog()
        {
            CLogFile.GetInstance().StartLog( "Log.txt" , CLogFile.LogLevel.LOG_LV_NOSY , 5000 );
            TestWriteLog();
            
            Console.WriteLine( "Press any key to change log level" );
            Console.ReadKey();

            CLogFile.GetInstance().SetLogLevel( CLogFile.LogLevel.LOG_LV_ERRO );
            TestWriteLog();
            CLogFile.GetInstance().StopLog();
        }
        static void TestWriteLog()
        {
            CLogFile.GetInstance().LogPrologue( "TestLog" );
            CLogFile.GetInstance().LogMust( "TestLog" , "LogMust" );
            CLogFile.GetInstance().LogErro( "TestLog" , "LogErro" );
            CLogFile.GetInstance().LogWarn( "TestLog" , "LogWarn" );
            CLogFile.GetInstance().LogInfo( "TestLog" , "LogInfo" );
            CLogFile.GetInstance().LogVerb( "TestLog" , "LogVerb" );
            CLogFile.GetInstance().LogNosy( "TestLog" , "LogNosy" );
            CLogFile.GetInstance().LogEpilogue( "TestLog" );
        }



        static void TestIni()
        {
            do
            {
                CIniParser ini = new CIniParser();
                if( false == ini.ParseIni( "..\\..\\..\\Samples\\rasphone.pbk" , Encoding.Default ) )
                {
                    Console.WriteLine( "ParseIni() failed" );
                    break;
                }

                string [] strSections = ini.GetSections();
                foreach ( string strSection in strSections )
                {
                    Console.WriteLine( "[{0}]" , strSection );
                    string [] strSectionKeys = ini.GetSectionKeys( strSection );
                    foreach ( string strSectionKey in strSectionKeys )
                    {
                        string strValue = ini.GetValue( strSection , strSectionKey );
                        Console.WriteLine( "    {0}={1}" , strSectionKey , strValue );
                    }
                    Console.WriteLine();
                }
            } while( false );
        }
    }
}
