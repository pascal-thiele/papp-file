:: Files that need to be provided externally:
:: Windows.h kernel32.lib libvcruntime.lib libucrt.lib
cl /c adler32.c compress.c crc32.c deflate.c gzclose.c gzlib.c gzread.c gzwrite.c infback.c inffast.c inflate.c inftrees.c trees.c uncompr.c "x86 64 windows.c" zutil.c
link /SUBSYSTEM:CONSOLE /ENTRY:main "/OUT:papp file.exe" adler32.obj compress.obj crc32.obj deflate.obj gzclose.obj gzlib.obj gzread.obj gzwrite.obj infback.obj inffast.obj inflate.obj inftrees.obj trees.obj uncompr.obj "x86 64 windows.obj" zutil.obj kernel32.lib libvcruntime.lib libucrt.lib