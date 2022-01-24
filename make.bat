@echo off
wsl make
pause
"C:\Program Files\qemu\qemu-system-x86_64.exe" -debugcon stdio -rtc base=localtime disk.iso
pause