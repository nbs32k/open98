@echo off
:PROMPT
SET /P AREYOUSURE=Debug mode (Y/[N])?
IF /I "%AREYOUSURE%" NEQ "Y" GOTO END

"C:\Program Files\qemu\qemu-system-x86_64.exe" -M smm=off -no-reboot -m 1G -d int -hdd disk.iso
pause

:END
"C:\Program Files\qemu\qemu-system-x86_64.exe" -debugcon stdio -m 8G -accel whpx -rtc base=localtime disk.iso
pause