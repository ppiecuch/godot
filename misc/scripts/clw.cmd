@echo off

setlocal DisableDelayedExpansion

set "errorlevel="
set "_errorlevel=errorlevel"
set "__errorlevel=_errorlevel"
set "line="

set cmd=$1
set source=$2

for /F "delims=" %%A in ('
    ^( %cmd% ^& call set /p "=%%%%%%__errorlevel%%%%%%"^ <NUL ^) ^| findstr /V /X "%source%"
    ') do (
    setlocal EnableDelayedExpansion
    echo(!line!
    endlocal
    set "line=%%A"
)
set "cl_errorlevel=%line%"

echo,
echo cl_errorlevel:%cl_errorlevel%
