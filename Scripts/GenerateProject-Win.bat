@echo off
::push dir to ../ (Seagull-Engine-v2)
pushd ..\
call Vendor\premake\premake5win.exe vs2019
::go back to the dir (scripts/)
popd

pause