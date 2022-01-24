@echo off
wsl make
pause
cls
"C:\Program Files\qemu\qemu-system-x86_64.exe" -debugcon stdio -accel whpx -M q35 -rtc base=localtime disk.iso
pause