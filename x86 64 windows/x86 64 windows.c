#include "Windows.h"
#include "memory.c"
#include "papp file.c"
#include "portable network graphics.c"
#include "console.c"


unsigned long long compare_type_RGBA64(unsigned char* address, unsigned long long size)
{
  if ((size == 7)
    && ((address[1] == 82) || (address[1] == 114))
    && ((address[2] == 71) || (address[2] == 103))
    && ((address[3] == 66) || (address[3] == 98))
    && ((address[4] == 65) || (address[4] == 97))
    && (address[5] == 54)
    && (address[6] == 52))
  { return 0; }
  return 1;
}


unsigned long long compare_type_PNG(unsigned char* address, unsigned long long size)
{
  if ((size == 4)
    && ((address[1] == 80) || (address[1] == 112))
    && ((address[2] == 78) || (address[2] == 110))
    && ((address[3] == 71) || (address[3] == 103)))
  { return 0; }
  return 1;
}


unsigned char file_name[256];

int main()
{
  // Initialize memory.
  memory = VirtualAlloc(0, memory_capacity, 12288, 4);
  if (memory == 0)
  { return 0; }
  
  // Seek the program name.
  unsigned char* command = GetCommandLineA();
  unsigned long long terminator = 32;
  while (*command == 32)
  { command += 1; }

  // Skip the program name.
  if (*command == 34)
  {
    terminator = 34;
    command += 1;
  }
  while ((*command != 0) && (*command != terminator))
  { command += 1; }
  if (*command == 34)
  { command += 1; }

  while (*command != 0)
  {
    // Seek a file name.
    while (*command == 32)
    { command += 1; }
    terminator = 32;
    if (*command == 34)
    {
      terminator = 34;
      command += 1;
    }
    if (*command == 0)
    { break; }
    
    // Copy the file name.
    unsigned char* file_name_address = file_name;
    unsigned long long file_name_size = 0;
    while ((*command != 0) && (file_name_size != 255) && (*command != terminator))
    {
      *file_name_address = *command;
      file_name_address += 1;
      file_name_size += 1;
      command += 1;
    }
    *file_name_address = 0;
    file_name_address -= file_name_size;
    if (*command == 34)
    { command += 1; }
    
    // Read the file.
    unsigned char* source_file_address = 0;
    unsigned long long source_file_size = 0;
    void* file_handle = CreateFileA(file_name_address, GENERIC_READ, 0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    if (file_handle != -1)
    {
      if (GetFileSizeEx(file_handle, &source_file_size) != 0)
      {
        source_file_address = memory_allocate(source_file_size);
        if (source_file_address != 0)
        {
          unsigned long read_file_size = 0;
          ReadFile(file_handle, source_file_address, source_file_size, &read_file_size, 0);
        }
      }
      CloseHandle(file_handle);
    }

    if ((source_file_address != 0) && (source_file_size != 0))
    {
      // Seek the file type.
      unsigned char* file_type_address = file_name_address;
      unsigned long long file_type_size = file_name_size;
      while ((file_type_size != 0) && (*file_type_address != 46))
      {
        file_type_address += 1;
        file_type_size -= 1;
      }

      // Translate the source file to the destination file.
      unsigned char* destination_file_address = 0;
      unsigned long long destination_file_size = 0;
      if (compare_type_RGBA64(file_type_address, file_type_size) == 0)
      {
        destination_file_size = papp_file_encode_size(source_file_address, source_file_size);
        destination_file_address = memory_allocate(destination_file_size);
        if (destination_file_address != 0)
        { papp_file_encode(source_file_address, source_file_size, destination_file_address); }
      }
      else if (compare_type_PNG(file_type_address, file_type_size) == 0)
      {
        unsigned long long decoded_file_size = portable_network_graphics_decode_size(source_file_address);
        if (decoded_file_size != 0)
        {
          unsigned char* decoded_file_address = memory_allocate(decoded_file_size);
          if (decoded_file_address != 0)
          {
            if (portable_network_graphics_decode(source_file_address, decoded_file_address) == 0)
            {
              destination_file_size = papp_file_encode_size(decoded_file_address, decoded_file_size);
              destination_file_address = memory_allocate(destination_file_size);
              if (destination_file_address != 0)
              { papp_file_encode(decoded_file_address, decoded_file_size, destination_file_address); }
            }
            memory_free(decoded_file_size);
          }
        }
      }

      // Write the papp file.
      if ((destination_file_address != 0) && (destination_file_size != 0))
      {
        file_type_address[1] = 112;
        file_type_address[2] = 97;
        file_type_address[3] = 112;
        file_type_address[4] = 112;
        file_type_address[5] = 0;
        file_handle = CreateFileA(file_name_address, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
        if (file_handle != -1)
        {
          unsigned long written_file_size = 0;
          WriteFile(file_handle, destination_file_address, destination_file_size, &written_file_size, 0);
          CloseHandle(file_handle);
        }
        memory_free(destination_file_size);
      }
      memory_free(source_file_size);
    }
  }
  return 0;
}