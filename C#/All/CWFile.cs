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
using System.Threading;

namespace CWUtils
{
    class CLogFile
    {
        private static CLogFile m_self = new CLogFile();
        public static CLogFile GetInstance()
        {
            return m_self;
        }



        public enum LogLevel
        {
            LOG_LV_NONE ,
            LOG_LV_MUST ,
            LOG_LV_ERRO ,
            LOG_LV_WARN ,
            LOG_LV_INFO ,
            LOG_LV_VERB ,
            LOG_LV_FUNC ,
            LOG_LV_NOSY
        };
        private FileStream m_fsFile = null;
        private TextWriter m_twFile = null;
        private String m_strLogPath = String.Empty;
        private LogLevel   m_nLogLv = LogLevel.LOG_LV_ERRO;
        private EventWaitHandle m_hEvtStop = null;
        private Thread m_hFlushThread = null;
        private int m_nFlushTime = -1;  //-1 means auto, 0 means immediately; otherwise in milli-second

        CLogFile() {}
        ~CLogFile()
        {
            this.StopLog();
        }

        public bool StartLog( String aLogPath , LogLevel aLogLevel , int aFlushTime )
        {
            bool bRet = false;
            if ( null == this.m_twFile && 0 < aLogPath.Length )
            {
                try
                {
                    String strDirPath = Path.GetDirectoryName( aLogPath );
                    if ( 0 < strDirPath.Length && false == Directory.Exists(strDirPath) )
                    {
                        Directory.CreateDirectory( strDirPath );
                    }

                    if ( File.Exists(aLogPath) )
                    {
                        File.Delete( aLogPath );
                    }
                    this.m_fsFile = new FileStream( aLogPath , FileMode.Create , FileAccess.Write );
                    this.m_twFile = new StreamWriter( m_fsFile );
                    this.m_strLogPath = aLogPath;
                    this.m_nLogLv = aLogLevel;

                    this.m_nFlushTime = aFlushTime;
                    if ( 0 < aFlushTime && null == this.m_hEvtStop )
                    {
                        this.m_hEvtStop = new EventWaitHandle( false , EventResetMode.AutoReset );
                        this.m_hFlushThread = new Thread( new ParameterizedThreadStart(FlushThread) );
                        this.m_hFlushThread.Start( this );
                    }
                    bRet = true;
                }
                catch ( System.Exception ex )
                {
                    throw ex;
                }
                finally
                {
                    if ( null == this.m_twFile )
                    {
                        this.m_fsFile.Dispose();
                        this.m_fsFile = null;
                    }
                }
            }
            else if ( null != this.m_twFile && this.m_strLogPath == aLogPath )
            {
                bRet = true;
            }
            else {}

            return bRet;
        }

        public void StopLog()
        {
            if ( null != this.m_hEvtStop )
            {
                this.m_hEvtStop.Set();
                if ( null != m_hFlushThread )
                {
                    this.m_hFlushThread.Join();
                }
                this.m_hEvtStop.Close();
                this.m_hEvtStop = null;
            }

            if ( null != this.m_twFile )
            {
                this.m_twFile.Dispose();
                this.m_twFile = null;
                this.m_fsFile = null;
            }
        }

        public static void FlushThread( object aThis )
        {
            CLogFile pThis = (CLogFile)aThis;

            pThis.LogMust( "FlushThread" , "Enter" );
            while ( false == pThis.m_hEvtStop.WaitOne( pThis.m_nFlushTime ) )
            {
                pThis.Flush();
            }
            pThis.LogMust( "FlushThread" , "Leave" );
            pThis.Flush();
        }

        public bool IsLogging()
        {
            return ( null != this.m_twFile );
        }

        public String GetLogPath()
        {
            return m_strLogPath;
        }

        public void SetLogLevel( LogLevel aLogLevel )
        {
            if ( aLogLevel != this.m_nLogLv )
            {
                aLogLevel = (LogLevel)Math.Min( Math.Max( (int)LogLevel.LOG_LV_NONE , (int)aLogLevel ) , (int)LogLevel.LOG_LV_NOSY );
                this.LogMust( "SetLogLevel" , String.Format("Log level changed from {0} to {1}" , this.m_nLogLv , aLogLevel) );
                this.m_nLogLv = aLogLevel;
            }
        }

        public LogLevel GetLogLevel()
        {
            return this.m_nLogLv;
        }

        public TextWriter GetWriter()
        {
            return this.m_twFile;
        }

        public void Flush()
        {
            if ( null != this.m_twFile )
            {
                lock( this.m_twFile )
                {
                    this.m_twFile.Flush();
                }
            }
        }



        public void LogMust( String aFuncName , String aBuf )
        {
            if ( LogLevel.LOG_LV_MUST <= this.m_nLogLv && null != this.m_twFile )
            {
                if ( null == aBuf )
                {
                    aBuf = "<NULL>";
                }
                lock( this.m_twFile )
                {
                    this.m_twFile.WriteLine( "[MUST][" + aFuncName + "] " + aBuf );
                    if ( 0 == this.m_nFlushTime )
                    {
                        this.m_twFile.Flush();
                    }
                }
            }
        }

        public void LogErro( String aFuncName , String aBuf )
        {
            if ( LogLevel.LOG_LV_ERRO <= this.m_nLogLv && null != this.m_twFile )
            {
                if ( null == aBuf )
                {
                    aBuf = "<NULL>";
                }
                lock( this.m_twFile )
                {
                    this.m_twFile.WriteLine( "[ERRO][" + aFuncName + "] " + aBuf );
                    if ( 0 == this.m_nFlushTime )
                    {
                        this.m_twFile.Flush();
                    }
                }
            }
        }

        public void LogWarn( String aFuncName , String aBuf )
        {
            if ( LogLevel.LOG_LV_WARN <= this.m_nLogLv && null != this.m_twFile )
            {
                if ( null == aBuf )
                {
                    aBuf = "<NULL>";
                }
                lock( this.m_twFile )
                {
                    this.m_twFile.WriteLine( "[WARN][" + aFuncName + "] " + aBuf );
                    if ( 0 == this.m_nFlushTime )
                    {
                        this.m_twFile.Flush();
                    }
                }
            }
        }

        public void LogInfo( String aFuncName , String aBuf )
        {
            if ( LogLevel.LOG_LV_INFO <= this.m_nLogLv && null != this.m_twFile )
            {
                if ( null == aBuf )
                {
                    aBuf = "<NULL>";
                }
                lock( this.m_twFile )
                {
                    this.m_twFile.WriteLine( "[INFO][" + aFuncName + "] " + aBuf );
                    if ( 0 == this.m_nFlushTime )
                    {
                        this.m_twFile.Flush();
                    }
                }
            }
        }

        public void LogVerb( String aFuncName , String aBuf )
        {
            if ( LogLevel.LOG_LV_VERB <= this.m_nLogLv && null != this.m_twFile )
            {
                if ( null == aBuf )
                {
                    aBuf = "<NULL>";
                }
                lock( this.m_twFile )
                {
                    this.m_twFile.WriteLine( "[VERB][" + aFuncName + "] " + aBuf );
                    if ( 0 == this.m_nFlushTime )
                    {
                        this.m_twFile.Flush();
                    }
                }
            }
        }

        public void LogNosy( String aFuncName , String aBuf )
        {
            if ( LogLevel.LOG_LV_NOSY <= this.m_nLogLv && null != this.m_twFile )
            {
                if ( null == aBuf )
                {
                    aBuf = "<NULL>";
                }
                lock( this.m_twFile )
                {
                    this.m_twFile.WriteLine( "[NOSY][" + aFuncName + "] " + aBuf );
                    if ( 0 == this.m_nFlushTime )
                    {
                        this.m_twFile.Flush();
                    }
                }
            }
        }

        public void LogPrologue( String aFuncName )
        {
            if ( LogLevel.LOG_LV_FUNC <= this.m_nLogLv && null != this.m_twFile )
            {
                lock( this.m_twFile )
                {
                    this.m_twFile.WriteLine( "[FUNC][" + aFuncName + "] Enter" );
                    if ( 0 == this.m_nFlushTime )
                    {
                        this.m_twFile.Flush();
                    }
                }
            }
        }
        public void LogEpilogue( String aFuncName )
        {
            if ( LogLevel.LOG_LV_FUNC <= this.m_nLogLv && null != this.m_twFile )
            {
                lock( this.m_twFile )
                {
                    this.m_twFile.WriteLine( "[FUNC][" + aFuncName + "] Leave" );
                    if ( 0 == this.m_nFlushTime )
                    {
                        this.m_twFile.Flush();
                    }
                }
            }
        }
    }












    public class CIniParser
    {
        private Hashtable m_mapKeyVals = new Hashtable();
        private String m_strIniPath = string.Empty;

        private struct SectionPair
        {
            public String strSection;
            public String strKey;
        }

        public CIniParser() {}
        ~CIniParser() {}

        public String GetIniPath()
        {
            return m_strIniPath;
        }

        public bool ParseIni( String aIniPath , Encoding aEncoding )
        {
            if ( false == File.Exists(aIniPath) )
            {
                return false;
            }

            bool bRet = false;
            FileStream fsFile = null;
            StreamReader srFile = null;
            String strLine = null;
            String strCurrentSection = String.Empty;
            String [] pairKeyVal = null;

            m_strIniPath = aIniPath;
            m_mapKeyVals.Clear();

            try
            {
                fsFile = new FileStream( aIniPath , FileMode.Open , FileAccess.Read );
                srFile = new StreamReader( fsFile , aEncoding );

                do
                {
                    strLine = srFile.ReadLine();
                    if ( null == strLine )
                    {
                        break;
                    }

                    strLine = strLine.Trim().ToUpper();
                    if ( 0 < strLine.Length && false == strLine.StartsWith(";") )
                    {
                        if ( strLine.StartsWith("[") && strLine.EndsWith("]") )
                        {
                            strCurrentSection = strLine.Substring( 1 , strLine.Length - 2 );
                        }
                        else
                        {
                            pairKeyVal = strLine.Split( new char[] { '=' } , 2 );

                            SectionPair sectionPair;
                            sectionPair.strSection = strCurrentSection;
                            sectionPair.strKey = pairKeyVal[0];

                            if ( false == m_mapKeyVals.Contains( sectionPair ) )
                            {
                                if ( 1 < pairKeyVal.Length )
                                {
                                    m_mapKeyVals.Add( sectionPair , pairKeyVal[1] );
                                }
                                else
                                {
                                    m_mapKeyVals.Add( sectionPair , String.Empty );
                                }
                            }
                            else
                            {
                                //throw new ArgumentException( string.Format("Key {0} already exists in section {1}",pairKeyVal[0],strCurrentSection) );
                            }
                        }
                    }
                } while ( -1 < srFile.Peek() ); //Use Peek() instead of EndOfStream since Mono doesn't have this method
                bRet = true;
            }
            catch ( Exception ex )
            {
                throw ex;
            }
            finally
            {
                if ( null != srFile )
                {
                    srFile.Dispose();
                }
            }
            return bRet;
        }



        public void SetValue( String aSection , String aKey , String aVal )
        {
            SectionPair sectionPair;
            sectionPair.strSection = aSection.ToUpper();
            sectionPair.strKey = aKey.ToUpper();

            if ( m_mapKeyVals.ContainsKey(sectionPair) )
            {
                m_mapKeyVals.Remove( sectionPair );
            }
            m_mapKeyVals.Add( sectionPair , aVal );
        }

        public String GetValue( String aSection , String aKey )
        {
            SectionPair sectionPair;
            sectionPair.strSection = aSection.ToUpper();
            sectionPair.strKey = aKey.ToUpper();

            if ( m_mapKeyVals.ContainsKey(sectionPair) )
            {
                return (String)m_mapKeyVals[sectionPair];
            }
            else
            {
                return String.Empty;
            }
        }



        public void DeleteSectionKey( String aSection , String aKey )
        {
            SectionPair sectionPair;
            sectionPair.strSection = aSection.ToUpper();
            sectionPair.strKey = aKey.ToUpper();

            m_mapKeyVals.Remove( sectionPair );
        }

        public String[] GetSectionKeys( String aSection )
        {
            ArrayList keys = new ArrayList();

            foreach ( SectionPair pair in m_mapKeyVals.Keys )
            {
                if ( pair.strSection == aSection.ToUpper() )
                {
                    keys.Add( pair.strKey );
                }
            }

            return (String[])keys.ToArray( typeof(String) );
        }



        public String[] GetSections()
        {
            ArrayList sections = new ArrayList();

            foreach ( SectionPair sectionPair in m_mapKeyVals.Keys )
            {
                if ( ! sections.Contains(sectionPair.strSection) )
                {
                    sections.Add( sectionPair.strSection );
                }
            }
            return (String[])sections.ToArray( typeof(String) );
        }



        public bool SaveIni( String aIniPath )
        {
            bool bRet = false;
            try
            {
                String [] sections = this.GetSections();
                TextWriter twFile = new StreamWriter( aIniPath );
                foreach ( String section in sections )
                {
                    twFile.WriteLine( "[" + section + "]" );

                    foreach ( SectionPair sectionPair in m_mapKeyVals.Keys )
                    {
                        if ( sectionPair.strSection == section )
                        {
                            twFile.WriteLine( sectionPair.strKey + "=" + (String)m_mapKeyVals[sectionPair] );
                        }
                    }

                    twFile.WriteLine();
                }
                twFile.Dispose();
                bRet = true;
            }
            catch (Exception ex)
            {
                throw ex;
            }

            return bRet;
        }
    }
}
