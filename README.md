# open98
An NT-Styled Kernel Clone | x86_64 architecture. It is able to read & write ATA Disks. Supports FAT32 Filesystem.
#### Made out of Clean Room Reverse Engineering

# Features
- Global Descriptor Table
- Interrupt Descriptor Table
- Programmable Interval Timer
- Exceptions Handler
- Keyboard & Mouse Driver
- Physical Memory Manager
- Virtual Memory Manager
- Heap ( [heap.c](https://github.com/n00bes/open98/blob/52ae7de0c150f7432103f8f936a1de52e2d3bb07/ntoskrnl/mm/heap.c) )
- Real Time Clock
- Mutex
- Some "UNC" VFS
- ATA Read / Write ( [NtCreateFile](https://github.com/n00bes/open98/blob/52ae7de0c150f7432103f8f936a1de52e2d3bb07/ntoskrnl/fsrtl/drive.c#L4), [NtWriteFile](https://github.com/n00bes/open98/blob/52ae7de0c150f7432103f8f936a1de52e2d3bb07/ntoskrnl/fsrtl/drive.c#L49), [NtReadFile](https://github.com/n00bes/open98/blob/52ae7de0c150f7432103f8f936a1de52e2d3bb07/ntoskrnl/fsrtl/drive.c#L88), [NtClose](https://github.com/n00bes/open98/blob/52ae7de0c150f7432103f8f936a1de52e2d3bb07/ntoskrnl/fsrtl/drive.c#L132) )
- ACPI Support ( [NtShutdownSystem](https://github.com/n00bes/open98/blob/52ae7de0c150f7432103f8f936a1de52e2d3bb07/ntoskrnl/hal/acpi/shutdown.c#L31) )
- Preemptive Multitasking
- Syscalls
- Panic Screen
- Basic GUI

![qemu-system-x86_64_ZvbQLWIKe1](https://user-images.githubusercontent.com/68382500/156880300-ca718f2f-33f9-4493-8f93-91b8ff5ab487.png)
![qemu-system-x86_64_0kYM6K1KMf](https://user-images.githubusercontent.com/68382500/156881165-d2cdcf8d-f91b-4336-a475-8dcc1ed2b1b5.png)


![vlc_rv3SOV5q4O](https://user-images.githubusercontent.com/68382500/156881195-6a093b2e-fc0e-4f81-8fc2-da61509d779e.png)
![vlc_yBORa2eSn8](https://user-images.githubusercontent.com/68382500/156881224-6049b673-ca59-463a-aa17-d6054fedddad.png)
