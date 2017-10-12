@echo off
del Kernel.iso

rmdir bin\boot /S /Q
mkdir bin\boot
xcopy boot bin\boot /E

bash -c "grub-mkrescue -o Kernel.iso bin"

Kernel.bxrc

