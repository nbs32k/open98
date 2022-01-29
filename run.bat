@echo off
:PROMPT
SET /P AREYOUSURE=Debug mode (Y/[N])?
IF /I "%AREYOUSURE%" NEQ "Y" GOTO END

"C:\Program Files\qemu\qemu-system-x86_64.exe" -M smm=off -no-reboot -d int disk.iso
pause

:END
"C:\Program Files\qemu\qemu-system-x86_64.exe" -debugcon stdio -accel whpx -M q35 -rtc base=localtime disk.iso
pause