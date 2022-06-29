#ifndef memory
#define memory

unsigned char* memory_address;
unsigned long long memory_size = 0;
unsigned long long memory_capacity = 134217728;


unsigned char* memory_allocate(unsigned long long size)
{
  unsigned char* allocated_address = 0;
  size += 7;
  size ^= size & 7;
  if ((memory_capacity >= size) && ((memory_capacity - size) >= memory_size))
  {
    allocated_address = memory_address + memory_size;
    memory_size += size;
  }
  return allocated_address;
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