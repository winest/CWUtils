@ECHO OFF
CD /D "%~dp0"

SET "ToolDir=F:\Code\_Tools\WppLog\Windows"
SET "LogName=TestCWUtils"

"%ToolDir%\tracelog.exe" -stop "%LogName%"
"%ToolDir%\tracelog.exe" -stop "84B0E5DA-6A9F-4124-B960-EBC6507AFC1B"

PAUSE
