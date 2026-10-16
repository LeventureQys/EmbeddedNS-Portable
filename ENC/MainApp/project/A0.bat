@echo off
::项目名称，可根据实际项目更改
::set Name=JFK2406040_1821M
set Name=JFK2406040_1821M_V104
::启动配置文件名称，可根据实际项目更改
set BootCfgFile=..\project\A0
::set Data0=%date:~2,2%%date:~5,2%%date:~8,2%_%time:~0,2%%time:~3,2%
set Data0=%date:~2,2%%date:~5,2%%date:~8,2%
set Data=%Data0: =0%

::原方式进行bin的拼接
::cd .\Bin
::copy /b /y FF_64K.bin Code0.bin >nul
::dd if=RO_CODE_RAM of=Code0.bin >nul 2>nul
::dd if=RO_CODE_ROM bs=1 of=Code0.bin seek=32k >nul 2>nul
::copy /b /y Code0.bin+RO_CONST_ROM %Name%_All_%Data%.bin >nul
::copy %BootCfgFile%.xml %Name%_All_%Data%.xml >nul
::set NoteInfo=current ram code is max 32KByte, rom code is max 32KByte, if not currect, please modify "A0.bat" and "A0.sct"
::当前sct，bat： 32K-RAMCode+32K-ROMCode+ConstData
::echo %NoteInfo%

::新方式直接通过Hex转换，更方便适配SCT的变更
cd ..\bin\
copy %BootCfgFile%.xml %Name%_All_%Data%.xml >nul
copy ..\project\Objects\A0.hex .\%Name%_All_%Data%.hex >nul
hex2bin.exe %Name%_All_%Data%.hex
del %Name%_All_%Data%.hex
