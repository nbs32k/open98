@echo off
:lol
move ntoskrnl/ntoskrnl.elf ntoskrnl.elf
cls
set /P id=Address to disassembly ntoskrnl.elf: 
echo.
echo Disassembled address:
echo ===============================================
wsl objdump -d ntoskrnl.elf | wsl grep %id%
echo.
echo Disassembled function:
echo ===============================================
wsl objdump -d ntoskrnl.elf | wsl grep -B 15 %id%
pause
cls
goto lol
