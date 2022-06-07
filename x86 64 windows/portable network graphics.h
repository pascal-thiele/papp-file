unsigned char portable_network_graphics_identify(unsigned char* address)
{
  unsigned char signature = address[0] ^ 137;
  signature |= address[1] ^ 80;
  signature |= address[2] ^ 78;
  signature |= address[3] ^ 71;
  signature |= address[4] ^ 13;
  signature |= address[5] ^ 10;
  signature |= address[6] ^ 26;
  signature |= address[7] ^ 10;
  return signature;
}


unsigned char portable_network_graphics_compare_IHDR(unsigned char* address)
{
  unsigned char IHDR = address[0] ^ 73;
  IHDR |= address[1] ^ 72;
  IHDR |= address[2] ^ 68;
  IHDR |= address[3] ^ 82;
  return IHDR;
}


unsigned long long portable_network_graphics_read_int(unsigned char* address)
{
  unsigned long long value = address[0];
  value = (value << 8) | address[1];
  value = (value << 8) | address[2];
  value = (value << 8) | address[3];
  return value;
}


void portable_network_graphics_write_int(unsigned long long bytes, unsigned char* address)
{
  address[0] = bytes >> 24;
  address[1] = bytes >> 16;
  address[2] = bytes >> 8;
  address[3] = bytes & 255;
}


unsigned long long portable_network_graphics_cyclic_redundancy_check(unsigned char* address, unsigned long long size)
{
  unsigned long long cyclic_redundancy_check = 4294967295;
  unsigned long long polynomial = 3988292384;
  while (size != 0)
  {
    cyclic_redundancy_check ^= *address;
    cyclic_redundancy_check = ((cyclic_redundancy_check & 1) * polynomial) ^ (cyclic_redundancy_check >> 1);
    cyclic_redundancy_check = ((cyclic_redundancy_check & 1) * polynomial) ^ (cyclic_redundancy_check >> 1);
    cyclic_redundancy_check = ((cyclic_redundancy_check & 1) * polynomial) ^ (cyclic_redundancy_check >> 1);
    cyclic_redundancy_check = ((cyclic_redundancy_check & 1) * polynomial) ^ (cyclic_redundancy_check >> 1);
    cyclic_redundancy_check = ((cyclic_redundancy_check & 1) * polynomial) ^ (cyclic_redundancy_check >> 1);
    cyclic_redundancy_check = ((cyclic_redundancy_check & 1) * polynomial) ^ (cyclic_redundancy_check >> 1);
    cyclic_redundancy_check = ((cyclic_redundancy_check & 1) * polynomial) ^ (cyclic_redundancy_check >> 1);
    cyclic_redundancy_check = ((cyclic_redundancy_check & 1) * polynomial) ^ (cyclic_redundancy_check >> 1);
    address += 1;
    size -= 1;
  }
  return cyclic_redundancy_check ^ 4294967295;
}


// Pascal Thiele: I do not care about encoding portable network graphics for now!


unsigned long long portable_network_graphics_decode_size(unsigned char* address)
{
  if (portable_network_graphics_identify(address) == 0)
  {
    unsigned long long chunk_length = portable_network_graphics_read_int(address + 8);
    unsigned long long chunk_cyclic_redundancy_check = portable_network_graphics_read_int(address + chunk_length + 16);
    if ((portable_network_graphics_cyclic_redundancy_check(address + 12, chunk_length + 4) == chunk_cyclic_redundancy_check)
      && (portable_network_graphics_compare_IHDR(address + 12) == 0))
    {
      unsigned long long width = portable_network_graphics_read_int(address + 16);
      unsigned long long height = portable_network_graphics_read_int(address + 20);
      unsigned long long decode_size = width * height * 8;
      if ((decode_size != 0) && ((2305843009213693951 / width / height) != 0))
      { return decode_size; }
    }
  }
  return 0;
}


unsigned char portable_network_graphics_decode(unsigned char* address)
{
  // to do
  return 1;
}