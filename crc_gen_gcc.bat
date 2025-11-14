@echo off
ECHO Computing CRC
ECHO -------------------------------------

REM Path configuration
SET SREC_PATH=..\Tools\Srec
SET TARGET_NAME=%1
SET VERSION_NAME=%2
SET BYTE_SWAP=1
SET COMPARE_HEX=1
SET CRC_ADDR_FROM_MAP=1
REM Not used when CRC_ADDR_FROM_MAP=1
SET CRC_ADDR=0x08009000

REM Derived configuration
SET ELF_FILE=%TARGET_NAME%.elf
SET MAP_FILE=%TARGET_NAME%.map
SET RELEASE_BASE_NAME=Pure
SET INPUT_BIN_NO_FLAGS=%RELEASE_BASE_NAME%.%VERSION_NAME%.bin
SET INPUT_BIN=%INPUT_BIN_NO_FLAGS% -binary
SET INPUT_HEX=%RELEASE_BASE_NAME%.%VERSION_NAME%.hex -intel
SET OUTPUT_HEX_NO_FLAGS=%RELEASE_BASE_NAME%.%VERSION_NAME%_CRC.hex
SET OUTPUT_HEX=%OUTPUT_HEX_NO_FLAGS% -intel
SET TMP_FILE=crc_tmp_file.txt

IF NOT "%CRC_ADDR_FROM_MAP%"=="1" goto:end_of_map_extraction
REM Extract CRC address from MAP file
REM -----------------------------------------------------------
REM Load line with checksum location to crc_search variable
FINDSTR "_Check_Sum" %MAP_FILE% > %TMP_FILE%
SET /p crc_search=<%TMP_FILE%
DEL %TMP_FILE%

REM Extract the FIRST hexadecimal address in the line (tokens separated by space)
for /f "tokens=1-10" %%a in ("%crc_search%") do (
    REM iterate through tokens and pick the first one that starts with 0x
    for %%X in (%%a %%b %%c %%d %%e %%f %%g %%h %%i %%j) do (
        echo %%X | FINDSTR /B "0x" >NUL
        if NOT ERRORLEVEL 1 (
            set CRC_ADDR=%%X
            goto:found_crc_addr
        )
    )
)

:found_crc_addr
REM -----------------------------------------------------------
REM End of CRC address extraction
:end_of_map_extraction
REM Generate bin file with 0xff in gaps (instead of 00)
ECHO %SREC_PATH%\arm-none-eabi-objcopy.exe --gap-fill 0xff -O binary %ELF_FILE% %INPUT_BIN_NO_FLAGS%
%SREC_PATH%\arm-none-eabi-objcopy.exe --gap-fill 0xff -O binary %ELF_FILE% %INPUT_BIN_NO_FLAGS%
REM Convert BIN to HEX with offset
%SREC_PATH%\srec_cat.exe ^
	%INPUT_BIN% ^
	-offset 0x08000000 ^
	-o %INPUT_HEX%
REM Compute CRC and store it to new HEX file
ECHO CRC address: %CRC_ADDR%
if "%BYTE_SWAP%"=="1" (
REM ECHO to see what is going on
ECHO %SREC_PATH%\srec_cat.exe ^
	%INPUT_HEX% ^
	-crop 0x08000000 %CRC_ADDR% ^
	-stm32-l-e %CRC_ADDR% ^
	-o %TMP_FILE% -intel	
%SREC_PATH%\srec_cat.exe ^
	%INPUT_HEX% ^
	-crop 0x08000000 %CRC_ADDR% ^
	-stm32-l-e %CRC_ADDR% ^
	-o %TMP_FILE% -intel	
) else (
REM ECHO to see what is going on
ECHO %SREC_PATH%\srec_cat.exe ^
	%INPUT_HEX% ^
	-crop 0x08000000 %CRC_ADDR% ^
	-stm32-l-e %CRC_ADDR% ^
	-o %TMP_FILE% -intel
%SREC_PATH%\srec_cat.exe ^
	%INPUT_HEX% ^
	-crop 0x08000000 %CRC_ADDR% ^
	-stm32-l-e %CRC_ADDR% ^
	-o %TMP_FILE% -intel
)
ECHO %SREC_PATH%\srec_cat.exe ^
	%INPUT_HEX% -exclude -within %TMP_FILE% -intel ^
	%TMP_FILE% -intel ^
	-o %OUTPUT_HEX%
%SREC_PATH%\srec_cat.exe ^
	%INPUT_HEX% -exclude -within %TMP_FILE% -intel ^
	%TMP_FILE% -intel ^
	-o %OUTPUT_HEX%
REM Delete temporary file
DEL %TMP_FILE%
ECHO Modified HEX file with CRC stored at %OUTPUT_HEX%

REM Compare input HEX file with output HEX file
if "%COMPARE_HEX%"=="1" (
ECHO Comparing %INPUT_HEX% with %OUTPUT_HEX%
%SREC_PATH%\srec_cmp.exe ^
	%INPUT_HEX% %OUTPUT_HEX% -v

REM Check result of srec_cmp
if ERRORLEVEL 1 (
    echo XXX CRC ERROR - update LD to CRC at last line here  
) else (
    echo VVV CRC is OK !!
)
ECHO CRC bytes from %OUTPUT_HEX_NO_FLAGS% at %CRC_ADDR%:
%SREC_PATH%\srec_dump.exe ^
    %OUTPUT_HEX_NO_FLAGS% ^
    %CRC_ADDR%
)
ECHO -------------------------------------