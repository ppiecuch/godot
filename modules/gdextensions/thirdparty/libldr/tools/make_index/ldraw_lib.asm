bits 64

section .rodata

global _ldrawlibData
global _ldrawlibEnd
global _ldrawlibSize

_ldrawlibData:      incbin "parts.db.gz"
_ldrawlibEnd:
_ldrawlibSize: dd $-_ldrawlibData

;  /opt/local/bin/nasm -fmacho64 ldraw_lib.asm -o ldraw_lib.o
