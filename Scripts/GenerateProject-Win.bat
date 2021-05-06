@echo off
::push dir to ../ (Seagull-Engine-v0.2)
pushd ..\
call Third-party\premake\premake5win.exe vs2019
::go back to the dir (scripts/)
popd

pause