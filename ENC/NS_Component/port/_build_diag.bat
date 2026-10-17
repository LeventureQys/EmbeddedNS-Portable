@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
cd /d "D:\WorkShop\EmbeddedNS-Portable\ENC\NS_Component\port"
cl /nologo /W3 /wd4068 /utf-8 /I ..\api /I ..\src /I . /I ..\..\Test\harness /Fe:_diag_nan.exe _diag_nan.c ..\src\*.c arm_math_compat.c ..\..\Test\harness\wav_io.c
