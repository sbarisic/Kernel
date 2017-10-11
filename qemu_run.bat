@echo off
del Kernel.iso

rmdir bin\boot /S /Q
mkdir bin\boot
xcopy boot bin\boot /E

bash -c "grub-mkrescue -o Kernel.iso bin"

"C:\Program Files\qemu\qemu-system-x86_64.exe" -smp 4 -m 32 -cdrom Kernel.iso

rem pause