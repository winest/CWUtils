@ECHO OFF
CD /D "%~dp0"

SET "STYLE_ARG=--style=file"
FOR /F %%a IN ('git config --get hooks.clangformat.style') DO (
    IF NOT "%%~a" == "" (
        SET "STYLE_ARG=--style=%%a"
    )
)
ECHO STYLE_ARG=%STYLE_ARG%

IF "%~1" == "--about" (
    ECHO Utility to run clang-format on source files
) ELSE IF "%~1" == "--all" (
    ECHO Run clang-format on all source files
    FOR /F %%f IN ('DIR Src\* /s /b ^| FINDSTR "\.h \.cpp"') DO (
        CALL :FormatFile "%%f"
    )
) ElSE (
    ECHO Run clang-format on diff source files
    FOR /F %%f IN ('git diff-index --cached --name-only HEAD C++/*.h C++/*.cpp') DO (
        CALL :FormatFile "%%f"
    )
)

PAUSE
GOTO :EOF

:FormatFile ( aFile )
(
    ECHO Formatting "%~1" with style "%STYLE_ARG%"
    clang-format.exe -i %STYLE_ARG% "%~1"
    git.exe add "%~1"
    EXIT /B
)