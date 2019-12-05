@ECHO OFF

SETLOCAL
SET "WppTools=F:\Code\_Tools\WppLog\Windows"
SET "WppExtraConfig=%~dp0"
IF "%WppExtraConfig:~-1%" EQU "\" ( SET "WppExtraConfig=%WppExtraConfig:~0,-1%" )



IF "%~1" EQU "" (
    ECHO Please input project folder path as the first parameter
    GOTO :EXIT
)

REM Get ProjectName for ProjectDir
SET "ProjectDir=%~1"
SET "ProjectName=%~1"
IF "%ProjectDir:~-1%" EQU "\" ( SET "ProjectDir=%ProjectDir:~0,-1%" )
FOR /f %%f IN ( "%ProjectDir%" ) DO ( SET "ProjectName=%%~nxf" )
ECHO ProjectDir=%ProjectDir%
ECHO ProjectName=%ProjectName%



:FIND_EXTRA_HEADER
SET "WppExtraConfigHeader=( "%ProjectDir%\%ProjectName%_wpp.h" "%WppExtraConfig%\%ProjectName%_wpp.h" "%WppExtraConfig%\_GenerateTmh.h"  )"
FOR %%i IN %WppExtraConfigHeader% DO (
    IF EXIST %%i (
        SET "WppExtraConfigHeader=%%~i"
        GOTO :FIND_EXTRA_INI
    )
)
ECHO Cannot find WPP header definition
GOTO :EXIT

:FIND_EXTRA_INI
SET "WppExtraConfigIni=( "%ProjectDir%\%ProjectName%_wpp.ini" "%WppExtraConfig%\%ProjectName%_wpp.ini" "%WppExtraConfig%\_GenerateTmh.ini" )"
FOR %%i IN %WppExtraConfigIni% DO (
    IF EXIST %%i (
        SET "WppExtraConfigIni=%%~i"
        GOTO :GENERATE_WPP
    )
)
ECHO Cannot find WPP ini definition
GOTO :EXIT



:GENERATE_WPP
ECHO WppExtraConfigHeader=%WppExtraConfigHeader%
ECHO WppExtraConfigIni=%WppExtraConfigIni%
IF NOT EXIST "%ProjectDir%\_tmh" ( MKDIR "%ProjectDir%\_tmh" )
FOR %%d IN ( %* ) DO (
    FOR /F %%f IN ('DIR /B "%%~d\*.c" OR "%%~d\*.cpp" OR "%%~d\*.h"') DO (
        ECHO Handling %%~d\%%f
        "%WppTools%\tracewpp.exe" -dll -func:"DbgOut(LEVEL,FLAGS,MSG,...)" -scan:"%WppExtraConfigHeader%" -ini:"%WppExtraConfigIni%" -cfgdir:"%WppTools%\WppConfig-WDK10" -odir:"%ProjectDir%\_tmh" -ext:".c.cpp.h" "%%~d\%%f"
    )
)

ECHO Generate tmh files completed

:EXIT

ENDLOCAL