OLD=%PATH%
PATH=%PATH%;%CD%\bin
scons platform=windows
PATH=%OLD%
