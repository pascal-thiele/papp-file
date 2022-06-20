#ifndef CONSOLE
#define CONSOLE
unsigned char console[1024];
unsigned long long console_size = 0;


void console_write()
{
  void* console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
  unsigned long byte_count = 0;
  WriteFile(console_handle, console, console_size, &byte_count, 0);
  console_size = 0;
}


void console_append_character(unsigned long long value)
{
  if (console_size != 1024)
  {
    console[console_size] = value;
    console_size += 1;
  }
}


void console_append_unsigned_integer(unsigned long long value)
{
  if (value == 0)
  {
    console[console_size] = 48;
    console_size += 1;
  }
  else
  {
    unsigned char characters[32];
    unsigned char* character_address = characters;
    unsigned long long character_count = 0;
    while (value != 0)
    {
      *character_address = (value % 10) + 48;
      value /= 10;
      character_address += 1;
      character_count += 1;
    }
    unsigned char* console_address = console + console_size;
    console_size += character_count;
    while (character_count != 0)
    {
      character_address -= 1;
      *console_address = *character_address;
      console_address += 1;
      character_count -= 1;
    }
  }
}
#endif