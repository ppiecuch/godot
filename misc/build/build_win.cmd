@ECHO OFF

REM Prerequsits:
REM ------------
REM c:\Python\27\Scripts\pip.exe install pywin32

CLS
SETLOCAL

PATH=%PATH%;%CD%\bin;%CD%\misc\tools
SET GODOT_DIR=%~dp0
ECHO *** Building 64-bit default editor for Windows (core: %NUMBER_OF_PROCESSORS%) ..
CALL scons -j%NUMBER_OF_PROCESSORS% platform=windows tools=yes use_lto=yes || GOTO END

IF NOT "%1"=="templates" GOTO END
    ECHO *** Building 64-bit debug export template for Windows ..
    CALL scons -j%NUMBER_OF_PROCESSORS% platform=windows target=release_debug bits=64 tools=no use_lto=yes || GOTO END

    ECHO *** Building 64-bit release export template for Windows ..
    CALL scons -j%NUMBER_OF_PROCESSORS% platform=windows target=release bits=64 tools=no use_lto=yes || GOTO END

    CALL strip "%GODOT_DIR%\bin\godot.windows.opt.debug.64.exe" ^
        "%GODOT_DIR%\bin/\odot.windows.opt.64.exe"

    SHIFT

:END
ENDLOCAL
ECHO *** Done
