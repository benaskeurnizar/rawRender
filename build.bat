@echo off

mkdir builds
pushd builds

cl -Zi  ../win32_main.c  gdi32.lib user32.lib 

popd
