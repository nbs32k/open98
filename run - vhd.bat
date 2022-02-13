@echo off
echo select vdisk file="C:\Users\1337\Desktop\open98\hdd.vhd">C:\Users\1337\Desktop\open98\dprt
echo attach vdisk>>C:\Users\1337\Desktop\open98\dprt
diskpart /s C:\Users\1337\Desktop\open98\dprt
mkdir E:\SYSTEM
copy C:\Users\1337\Desktop\open98\ntoskrnl\ntoskrnl.elf E:\SYSTEM\NTOSKRNL.SYS
echo select vdisk file="C:\Users\1337\Desktop\open98\hdd.vhd">C:\Users\1337\Desktop\open98\dprt
echo detach vdisk>>C:\Users\1337\Desktop\open98\dprt
diskpart /s C:\Users\1337\Desktop\open98\dprt
del C:\Users\1337\Desktop\open98\dprt

:::PROMPT
::SET /P AREYOUSURE=Debug mode (Y/[N])?
::IF /I "%AREYOUSURE%" NEQ "Y" GOTO END

::"C:\Program Files\qemu\qemu-system-x86_64.exe" -M smm=off -no-reboot -d int -hdd C:\Users\1337\Desktop\open98\hdd.vhd
::pause

:":END
::"C:\Program Files\qemu\qemu-system-x86_64.exe" -debugcon stdio -accel whpx -rtc base=localtime C:\Users\1337\Desktop\open98\hdd.vhd
::pause

ping 127.0.0.1 -n 2 > nul
"C:\Program Files\qemu\qemu-system-x86_64.exe" -debugcon stdio -accel whpx -rtc base=localtime C:\Users\1337\Desktop\open98\hdd.vhd
pause