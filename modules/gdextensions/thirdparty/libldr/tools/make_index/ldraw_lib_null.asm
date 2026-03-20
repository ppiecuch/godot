bits 64

section .rodata

global _ldrawlib
global _ldrawlib_end
global _ldrawlib_size

_ldrawlib:      dd 0
_ldrawlib_end:
_ldrawlib_size: dd $-_ldrawlib

;  /opt/local/bin/nasm -fmacho64 ldraw_lib.asm -o ldraw_lib.o
