:: Files that need to be provided externally:
:: Windows.h kernel32.lib libvcruntime.lib libucrt.lib
cl /c "x86 64 windows.c"
link /SUBSYSTEM:CONSOLE /ENTRY:main "/OUT:papp file.exe" "x86 64 windows.obj" kernel32.lib libvcruntime.lib libucrt.lib