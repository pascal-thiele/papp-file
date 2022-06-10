#include "console.c"

// 0 
unsigned char portable_network_graphics_filter_type = 0;
unsigned char portable_network_graphics_remainder_size = 0;
unsigned long long portable_network_graphics_remainder = 0;
unsigned long long portable_network_graphics_adler32 = 0;


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


unsigned char portable_network_graphics_compare_PLTE(unsigned char* address)
{
  unsigned char PLTE = address[0] ^ 80;
  PLTE |= address[1] ^ 76;
  PLTE |= address[2] ^ 84;
  PLTE |= address[3] ^ 69;
  return PLTE;
}


unsigned char portable_network_graphics_compare_IEND(unsigned char* address)
{
  unsigned char IEND = address[0] ^ 73;
  IEND |= address[1] ^ 69;
  IEND |= address[2] ^ 78;
  IEND |= address[3] ^ 68;
  return IEND;
}


unsigned char portable_network_graphics_compare_IDAT(unsigned char* address)
{
  unsigned char IDAT = address[0] ^ 73;
  IDAT |= address[1] ^ 68;
  IDAT |= address[2] ^ 65;
  IDAT |= address[3] ^ 84;
  return IDAT;
}


unsigned char portable_network_graphics_compare_tRNS(unsigned char* address)
{
  unsigned char tRNS = address[0] ^ 116;
  tRNS |= address[1] ^ 82;
  tRNS |= address[2] ^ 78;
  tRNS |= address[3] ^ 83;
  return tRNS;
}


unsigned int portable_network_graphics_read_int(unsigned char* address)
{
  unsigned int value = address[0];
  value = (value << 8) | address[1];
  value = (value << 8) | address[2];
  value = (value << 8) | address[3];
  return value;
}


void portable_network_graphics_write_int(unsigned int bytes, unsigned char* address)
{
  address[0] = bytes >> 24;
  address[1] = bytes >> 16;
  address[2] = bytes >> 8;
  address[3] = bytes & 255;
}


unsigned int portable_network_graphics_cyclic_redundancy_check(unsigned char* address, unsigned int size)
{
  unsigned int cyclic_redundancy_check = 4294967295;
  unsigned int polynomial = 3988292384;
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
  return ~cyclic_redundancy_check;
}


unsigned long long portable_network_graphics_inflate(unsigned char* source_address, unsigned long long source_size, unsigned char* destination_address, unsigned long long destination_size)
{
  unsigned long long inflated_size = 0;

  if (destination_size == 0)
  {
    // Validate compression method and window size in the zlib header.
    unsigned char method = source_address[0];
    if (((method & 7) != 8) || ((method >> 4) >= 8))
    { source_size = 0; }

    // Jump the dictionary identification if existing.
    unsigned char flags = source_address[1];
    if ((flags & 32) != 0)
    { source_address += 4; }

    portable_network_graphics_adler32 = 1;
    source_address += 2;
  }

  while (source_size != 0)
  {
    // Extend remaining bits by one byte.
    unsigned long long value = (portable_network_graphics_remainder << 8) | *source_address;
    portable_network_graphics_remainder_size += 8;
    source_size -= 1;

    unsigned char block_type = (value >> (portable_network_graphics_remainder_size - 3)) & 3;
    if (block_type == 0)
    {
      // Copy the data.
      // to do
    }
    else if (block_type == 1)
    {
      // Inflate with static Huffman codes.
      // to do
    }
    else if (block_type == 2)
    {
      // Inflate with dynamic Huffman codes.
      // to do
    }
    else
    {
      source_size = 0;
      inflated_size = 0;
    }

    // Increment adler32.
    //unsigned long long adler32_byte = ((portable_network_graphics_adler32 + addend_byte) % 65521);
    //portable_network_graphics_adler32 = (((portable_network_graphics_adler32 >> 16) + adler32_byte) % 65521) << 16;
    //portable_network_graphics_adler32 |= adler32_byte;
  }
  return inflated_size;
}


unsigned long long portable_network_graphics_pollute_scanlines_RGBA32(unsigned char* source_address, unsigned long long source_size)
{
  unsigned long long polluted_size = 0;
  // to do
  return polluted_size;
}


unsigned long long portable_network_graphics_pollute_scanlines_RGBA64(unsigned char* source_address, unsigned long long source_size)
{
  unsigned long long polluted_size = 0;
  // to do
  return polluted_size;
}


// to do Write pollute functions for adam7 interlacing.
// to do Write functions for grey8, grey16, RGB32, RGB64, index8.
// to do For grey functions, index functions and RGB functions tRNS may be used.
// to do For index functions PLTE has to be used.


unsigned long long portable_network_graphics_extend_eight_bits(unsigned char* address, unsigned long long size)
{
  unsigned long long extended_size = size << 1;
  unsigned char* extended_address = address + extended_size;
  address += size;
  while (size != 0)
  {
    address -= 1;
    size -= 1;
    extended_address -= 2;
    *(unsigned short*)extended_address = *address * 257;
  }
  return extended_size;
}


// Pascal Thiele: I do not care about encoding portable network graphics for now!


// to do Take inflation into account.
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


unsigned char portable_network_graphics_decode(unsigned char* source_address, unsigned char* destination_address)
{
  if (portable_network_graphics_identify(source_address) == 0)
  {
    source_address += 8;
    unsigned long long destination_size = 0;
    unsigned char* IHDR_address = 0;
    unsigned char* PLTE_address = 0;
    unsigned char* tRNS_address = 0;

    while (1)
    {
      unsigned long long chunk_length = portable_network_graphics_read_int(source_address);
      unsigned long long chunk_cyclic_redundancy_check = portable_network_graphics_read_int(source_address + chunk_length + 8);
      if (portable_network_graphics_cyclic_redundancy_check(source_address + 4, chunk_length + 4) != chunk_cyclic_redundancy_check)
      { break; }

      if (portable_network_graphics_compare_IHDR(source_address + 4) == 0)
      {
        if (IHDR_address != 0)
        { break; }
        IHDR_address = source_address;
        source_address += chunk_length + 12;
      }
      else if (portable_network_graphics_compare_PLTE(source_address + 4) == 0)
      {
        if (PLTE_address != 0)
        { break; }
        PLTE_address = source_address;
        source_address += chunk_length + 12;
      }
      else if (portable_network_graphics_compare_tRNS(source_address + 4) == 0)
      {
        if (tRNS_address != 0)
        { break; }
        tRNS_address = source_address;
        source_address += chunk_length + 12;
      }
      else if (portable_network_graphics_compare_IDAT(source_address + 4) == 0)
      {
        if (((IHDR_address[16] ^ 8) | IHDR_address[17] | IHDR_address[18] | IHDR_address[19] | IHDR_address[20]) == 0)
        {
          // to do
        }
        else if (((IHDR_address[16] ^ 16) | IHDR_address[17] | IHDR_address[18] | IHDR_address[19] | IHDR_address[20]) == 0)
        {
          // to do
        }
        else if (((IHDR_address[16] ^ 8) | (IHDR_address[17] ^ 2) | IHDR_address[18] | IHDR_address[19] | IHDR_address[20]) == 0)
        {
          // to do
        }
        else if (((IHDR_address[16] ^ 16) | (IHDR_address[17] ^ 2) | IHDR_address[18] | IHDR_address[19] | IHDR_address[20]) == 0)
        {
          // to do
        }
        else if (((IHDR_address[16] ^ 8) | (IHDR_address[17] ^ 3) | IHDR_address[18] | IHDR_address[19] | IHDR_address[20]) == 0)
        {
          // to do
        }
        else if (((IHDR_address[16] ^ 8) | (IHDR_address[17] ^ 4) | IHDR_address[18] | IHDR_address[19] | IHDR_address[20]) == 0)
        {
          // to do
        }
        else if (((IHDR_address[16] ^ 16) | (IHDR_address[17] ^ 4) | IHDR_address[18] | IHDR_address[19] | IHDR_address[20]) == 0)
        {
          // to do
        }
        else if (((IHDR_address[16] ^ 8) | (IHDR_address[17] ^ 6) | IHDR_address[18] | IHDR_address[19] | IHDR_address[20]) == 0)
        {
          unsigned long long data_size = portable_network_graphics_inflate(source_address + 8, chunk_length, destination_address, destination_size);
          if (data_size == 0)
          { break; }
          data_size = portable_network_graphics_pollute_scanlines_RGBA32(destination_address, data_size);
          data_size = portable_network_graphics_extend_eight_bits(destination_address, data_size);
          destination_address += data_size;
          destination_size += data_size;
          source_address += chunk_length + 12;
        }
        else if (((IHDR_address[16] ^ 16) | (IHDR_address[17] ^ 6) | IHDR_address[18] | IHDR_address[19] | IHDR_address[20]) == 0)
        {
          unsigned long long data_size = portable_network_graphics_inflate(source_address + 8, chunk_length, destination_address, destination_size);
          if (data_size == 0)
          { break; }
          data_size = portable_network_graphics_pollute_scanlines_RGBA64(destination_address, data_size);
          destination_address += data_size;
          destination_size += data_size;
          source_address += chunk_length + 12;
        }
        else
        { break; }
      }
      else if (portable_network_graphics_compare_IEND(source_address + 4) == 0)
      { return 0; }
      else
      { source_address += chunk_length + 12; }
    }
  }
  return 1;
}