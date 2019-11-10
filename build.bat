@echo off
set "common_compiler_flags= /diagnostics:column /MT /Gm- /FC /Wall /wd4710 /wd4820 /wd4100 /wd4201 /Od /Oi /std:c++17 /nologo /GR- /MP /Zo /Zf /Z7 /DEBUG"
set "libs= user32.lib kernel32.lib"

pushd D:\Gnom\build

cl %common_compiler_flags% -ID:\WindowsSDK\ -ID:\msvc_build_tools\VC\Tools\MSVC\14.22.27905\include .\..\gnom.cpp /link /INCREMENTAL:NO /LIBPATH:D:\WindowsSDK %libs% /opt:ref

popd