#ifndef MEMORY
#define MEMORY
#define memory_capacity 134217728

unsigned char* memory;
unsigned long long memory_size = 0;


unsigned char* memory_allocate(unsigned long long size)
{
  unsigned char* address = 0;
  size += 7;
  size ^= size & 7;
  if ((memory_capacity >= size) && ((memory_capacity - size) >= memory_size))
  {
    address = memory + memory_size;
    memory_size += size;
  }
  return address;
}


void memory_free(unsigned long long size)
{
  if (memory_size >= size)
  {
    size += 7;
    size ^= size & 7;
    memory_size -= size;
  }
  else
  { memory_size = 0; }
}
#endif