@echo off
rem run the examples ...

echo off

set path=%cd%\dll;%PATH%

rem
rem Example x14 requires a second name
rem
echo dummy > dummy

rem
rem Build the dedicated comparison program (see comments in the program text)
rem
cl %~dp0\compare.c

rem
rem Run the C examples - they are the references
rem

cd examples\c
for %%i in (x*.exe) do (
    %%i -dev psc -o %%~ni.ps < ..\..\dummy
)

rem
rem Run the C++ examples and compare the PostScript files against the C reference files
rem
if not exist ..\c++ goto fortran
cd ..\c++
echo --------------------------------------------
echo C++ examples
echo --------------------------------------------
for %%i in (x*.exe) do (
    %%i -dev psc -o %%~ni.ps < ..\..\dummy
)
echo Comparing the PostScript files:
for %%i in (*.ps) do (
    ..\..\compare.exe %%i
)
echo ----
echo Done
echo ----

rem
rem Run the Fortran examples and compare the PostScript files against the C reference files
rem
:fortran
if not exist ..\fortran goto tcl
cd ..\fortran
echo --------------------------------------------
echo Fortran examples
echo --------------------------------------------
for %%i in (x*.exe) do (
    %%i -dev psc -o %%~ni.ps < ..\..\dummy
)
echo Comparing the PostScript files:
for %%i in (*.ps) do (
    ..\..\compare.exe %%i
)
echo ----
echo Done
echo ----

rem
rem Run the Tcl examples and compare the PostScript files against the C reference files
rem
:tcl
if not exist ..\tcl goto end
cd ..\tcl
echo --------------------------------------------
echo Tcl examples
echo --------------------------------------------
for %%i in (x*.tcl) do (
    ..\..\bindings\tcl\pltcl %%~ni -dev psc -o %%~ni.ps < ..\..\dummy
)
echo Comparing the PostScript files:
for %%i in (*.ps) do (
    ..\..\compare.exe %%i
)
echo ----
echo Done
echo ----

:end
echo Testing completed
cd ..\..
del dummy
del compare.obj
