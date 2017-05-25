@ECHO OFF
CD /D "%~dp0"
REM Parameter     is                            #typev
REM %1            GUID Friendly Name            String
REM %2            GUID SubType Name             String
REM %3            Thread ID                     LONG
REM %4            System Time                   String
REM %5            Kernel Time or User Time      String
REM %6            User Time or NULL             String
REM %7            Sequence Number               LONG
REM %8            Process Id                    LONG
REM %9            CPU Number                    LONG
REM %10 and above are the user parameters
REM %254          Reserved
REM %255          Reserved
REM %!FUNC!       Function name                 String
REM %!FLAGS!      Trace flags                   String
REM %!LEVEL!      Trace level                   String
REM %!COMPNAME!   Component name                String
REM %!SUBCOMP!    Subcomponent name             String

SET "ToolDir=F:\Code\_Tools\WppLog\Windows"
SET "TmpDir=Temp"
REM SET "SymbolDir=..\Output\Debug\Win32"
SET "SymbolDir=..\Output\Release\x64"
SET "LogName=TestCWUtils"

RMDIR "%TmpDir%" /S /Q
MKDIR "%TmpDir%"

REM [Sequence Number][Time][Level][CPU-PID:TID][Source Line][Function Name]: 
SET TRACE_FORMAT_PREFIX=[%%7!-8u!][%%4!s!][%%!LEVEL!][%%9!X!-%%8!04X!:%%3!04X!][%%2!s!][%%!FUNC!]: 

"%ToolDir%\tracelog.exe" -start "%LogName%" -guid #84B0E5DA-6A9F-4124-B960-EBC6507AFC1B -flag 0x7fffffff -level 9 -b 512 -UsePerfCounter -rt -ft 1 -ls 
"%ToolDir%\tracepdb.exe" -f "%SymbolDir%\*.pdb" -s -p "%TmpDir%" -v
"%ToolDir%\tracefmt.exe" -rt "%LogName%" -p "%TmpDir%" -display -o "%LogName%.txt" -hires
DEL /F /Q "%LogName%.txt.sum"

RMDIR "%TmpDir%" /S /Q

PAUSE
