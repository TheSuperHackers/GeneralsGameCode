@echo off

rem
rem This batch file generates DoxyGen info and compiles the separate
rem HTML files into CHMs. 
rem
rem Assumptions:
rem ------------
rem - doxygen installed and in path
rem - hhc (HTML Help Compiler) installed and in path
rem - dot (Graphviz package) installed and in path
rem
rem Both generated CHM files are then copied over to the
rem common DOC area.

doxygen profile.dox
doxygen profile_priv.dox
cd doc\html
hhc index.hhp
copy index.chm ..\..\profile.chm
cd ..\html_priv
hhc index.hhp
copy index.chm ..\..\profile_priv.chm
cd ..\..

copy profile.chm ..\..\docs
