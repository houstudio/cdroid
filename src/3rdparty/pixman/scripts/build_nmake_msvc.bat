@echo off

if defined ENVIRONMENTREADY goto begin
if defined VS90COMNTOOLS set MSVCDIR=%VS90COMNTOOLS%
if defined VS100COMNTOOLS set MSVCDIR=%VS100COMNTOOLS%
if defined VS110COMNTOOLS set MSVCDIR=%VS110COMNTOOLS%
if defined VS120COMNTOOLS set MSVCDIR=%VS120COMNTOOLS%
if defined VS140COMNTOOLS set MSVCDIR=%VS140COMNTOOLS%
set ENVIRONMENTREADY=1

call "%MSVCDIR%\vsvars32.bat"

:begin

rmdir /S /Q ..\\build\\nmake_msvc
mkdir ..\\build\\nmake_msvc
cd ..\\build\\nmake_msvc
cmake -G "NMake Makefiles" -DCMAKE_INSTALL_PREFIX="${CMAKE_BIN_DIR}/install/" -DCMAKE_BUILD_TYPE=Release ../..
nmake install
cd ..\\..\\scripts\\