:: Windows.h and kernel32.lib have to be provided externally!
cl /c "x86 64 windows.c"
link /SUBSYSTEM:CONSOLE /ENTRY:main "/OUT:papp file.exe" "x86 64 windows.obj" kernel32.lib