#include "console.c"

unsigned char portable_network_graphics_block_final = 0;
unsigned char portable_network_graphics_block_type = 0;
unsigned char portable_network_graphics_remainder_size = 0;
unsigned int portable_network_graphics_block_length = 0;
unsigned int portable_network_graphics_remainder = 0;
unsigned int portable_network_graphics_skip = 0;
unsigned int portable_network_graphics_adler32 = 0;


unsigned long long portable_network_graphics_identify(unsigned char* address)
{
  unsigned long long signature = address[0] ^ 137;
  signature |= address[1] ^ 80;
  signature |= address[2] ^ 78;
  signature |= address[3] ^ 71;
  signature |= address[4] ^ 13;
  signature |= address[5] ^ 10;
  signature |= address[6] ^ 26;
  signature |= address[7] ^ 10;
  return signature;
}


unsigned long long portable_network_graphics_compare_IHDR(unsigned char* address)
{
  unsigned long long IHDR = address[0] ^ 73;
  IHDR |= address[1] ^ 72;
  IHDR |= address[2] ^ 68;
  IHDR |= address[3] ^ 82;
  return IHDR;
}


unsigned long long portable_network_graphics_compare_PLTE(unsigned char* address)
{
  unsigned long long PLTE = address[0] ^ 80;
  PLTE |= address[1] ^ 76;
  PLTE |= address[2] ^ 84;
  PLTE |= address[3] ^ 69;
  return PLTE;
}


unsigned long long portable_network_graphics_compare_IEND(unsigned char* address)
{
  unsigned long long IEND = address[0] ^ 73;
  IEND |= address[1] ^ 69;
  IEND |= address[2] ^ 78;
  IEND |= address[3] ^ 68;
  return IEND;
}


unsigned long long portable_network_graphics_compare_IDAT(unsigned char* address)
{
  unsigned long long IDAT = address[0] ^ 73;
  IDAT |= address[1] ^ 68;
  IDAT |= address[2] ^ 65;
  IDAT |= address[3] ^ 84;
  return IDAT;
}


unsigned long long portable_network_graphics_compare_tRNS(unsigned char* address)
{
  unsigned long long tRNS = address[0] ^ 116;
  tRNS |= address[1] ^ 82;
  tRNS |= address[2] ^ 78;
  tRNS |= address[3] ^ 83;
  return tRNS;
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
  address[3] = bytes;
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


// to do Inline this function in inflate. adler32 is part of the zlib specification and should not be a concern of the caller.
unsigned long long portable_network_graphics_add_adler32(unsigned long long adler32, unsigned long long addend)
{
  unsigned long long data = (adler32 + addend) % 65521;
  adler32 = (((adler32 >> 16) + data) % 65521) << 16;
  adler32 |= data;
  return adler32;
}


// Inflates up to source_size bytes from source_address. Read bytes are counted in progress across multiple sources. The returned source_address and the returned source_size shall not change between calls for one source. One inflate call returns one inflated data byte at most. If no byte could be inflated, the returned data is greater than 255 and has to be ignored. Additionally, if the source is invalid, the returned total_progress is zero.

struct portable_network_graphics_inflate_result
{ unsigned char* source_address; unsigned long long source_size; unsigned long long progress; unsigned long long data; };

struct portable_network_graphics_inflate_result portable_network_graphics_inflate(unsigned char* source_address, unsigned long long source_size, unsigned long long progress)
{
  struct portable_network_graphics_inflate_result inflate;
  inflate.source_address = source_address;
  inflate.source_size = source_size;
  inflate.progress = progress;

  while (inflate.source_size != 0)
  {
    if (portable_network_graphics_skip != 0)
    {
      portable_network_graphics_skip -= 1;
      inflate.source_address += 1;
      inflate.source_size -= 1;
      inflate.progress += 1;
    }
    else if (inflate.progress == 0)
    {
      if (((*inflate.source_address & 15) != 8) || ((*inflate.source_address >> 4) >= 8))
      { return inflate; }
      inflate.source_address += 1;
      inflate.source_size -= 1;
      inflate.progress += 1;
      portable_network_graphics_block_final = 0;
      portable_network_graphics_block_type = 3;
      portable_network_graphics_remainder = 0;
      portable_network_graphics_remainder_size = 0;
      portable_network_graphics_skip = 0;
      portable_network_graphics_adler32 = 1;
    }
    else if (inflate.progress == 1)
    {
      if ((*inflate.source_address & 32) != 0)
      { portable_network_graphics_skip = 4; }
      inflate.source_address += 1;
      inflate.source_size -= 1;
      inflate.progress += 1;
    }
    else if (portable_network_graphics_block_type == 0)
    {
      // Copy one byte.
      inflate.data = *source_address;
      inflate.source_address += 1;
      inflate.source_size -= 1;
      inflate.progress += 1;
      portable_network_graphics_block_length -= 1;
      if (portable_network_graphics_block_length == 0)
      { portable_network_graphics_block_type = 3; }
      return inflate;
    }
    else if (portable_network_graphics_block_type == 1)
    {
      // to do
    }
    else if (portable_network_graphics_block_type == 2)
    {
      // to do
    }
    else if (portable_network_graphics_block_final == 0)
    {
      // Read the block header.
      if (portable_network_graphics_remainder_size < 3)
      {
        portable_network_graphics_remainder <<= 8;
        portable_network_graphics_remainder |= *inflate.source_address;
        portable_network_graphics_remainder_size += 8;
      }
      portable_network_graphics_block_final = portable_network_graphics_remainder >> (portable_network_graphics - 3);
      portable_network_graphics_block_type = portable_network_graphics_block_final & 3;
      portable_network_graphics_block_final >>= 2;
    }
    else
    {
      // Compare adler32.
      // to do
    }
  }

  inflate.data = 256;
  return inflate;
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


unsigned long long portable_network_graphics_decode(unsigned char* source_address, unsigned char* destination_address)
{
  if (portable_network_graphics_identify(source_address) == 0)
  {
    source_address += 8;
    unsigned char* IHDR_address = 0;
    unsigned char* PLTE_address = 0;
    unsigned char* tRNS_address = 0;
    unsigned long long progress = 0;
    unsigned long long filtered_size = 0;
    unsigned long long filter_type = 0;

    while (1)
    {
      unsigned long long chunk_length = portable_network_graphics_read_int(source_address);
      unsigned long long chunk_cyclic_redundancy_check = portable_network_graphics_read_int(source_address + chunk_length + 8);
      if (portable_network_graphics_cyclic_redundancy_check(source_address + 4, chunk_length + 4) != chunk_cyclic_redundancy_check)
      { return 1; }

      if (portable_network_graphics_compare_IHDR(source_address + 4) == 0)
      {
        if (IHDR_address != 0)
        { return 1; }
        IHDR_address = source_address;
        source_address += chunk_length + 12;
      }
      else if (portable_network_graphics_compare_PLTE(source_address + 4) == 0)
      {
        if (PLTE_address != 0)
        { return 1; }
        PLTE_address = source_address;
        source_address += chunk_length + 12;
      }
      else if (portable_network_graphics_compare_tRNS(source_address + 4) == 0)
      {
        if (tRNS_address != 0)
        { return 1; }
        tRNS_address = source_address;
        source_address += chunk_length + 12;
      }
      else if (portable_network_graphics_compare_IDAT(source_address + 4) == 0)
      {
        // to do For grey functions, index functions and RGB functions tRNS may be used.
        // to do For index functions PLTE has to be used.
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
          source_address += 8;
          while (chunk_length != 0)
          {
            struct portable_network_graphics_inflate_result inflate = portable_network_graphics_inflate(source_address, chunk_length, progress);
            source_address = inflate.source_address;
            chunk_length = inflate.chunk_length;
            progress = inflate.progress;
            if (progress == 0)
            { return 1; }

            // Interpret the inflated byte.
            if (inflate.data < 256)
            {
              unsigned long long width = portable_network_graphics_read_int(IHDR_address + 8);
              unsigned long long scanline_size = (width * 4) + 1;
              if ((filtered_size % scanline_size) == 0)
              {
                // Translate the first byte of each scanline to filter type.
                if (inflate.data >= 5)
                { return 1; }
                filter_type = inflate.data;
                filtered_size += 1;
              }
              else if (filter_type == 0)
              {
                // The byte has no filter.
                *(unsigned short*)(destination_address) = inflate.data * 257;
                destination_address += 2;
                filtered_size += 1;
              }
              else if (filter_type == 1)
              {
                // Add the byte from the preceding pixel. Add zero if no pixel precedes.
                unsigned long long preceding_value = 0;
                if ((filtered_size % scanline_size) >= 5)
                { preceding_value = *(unsigned short*)(destination_address - 8); }
                *(unsigned short*)destination_address = ((inflate.data * 257) + preceding_value) % 65536;
                destination_address += 2;
                filtered_size += 1;
              }
              else if (filter_type == 2)
              {
                // Add the byte from the preceding scanline. Add zero if no scanline precedes.
                unsigned long long preceding_value = 0;
                if (filtered_size >= scanline_size)
                { preceding_value = *(unsigned short*)(destination_address - ((width * 8) + 1)); }
                *(unsigned short*)destination_address = ((inflate.data * 257) + preceding_value) % 65536;
                destination_address += 2;
                filtered_size += 1;
              }
              else if (filter_type == 3)
              {
                // DESCRIPTION MISSING
                // to do
              }
              else
              {
                // DESCRIPTION MISSING
                // to do
              }
            }
          }
          source_address += 4;
        }
        else if (((IHDR_address[16] ^ 16) | (IHDR_address[17] ^ 6) | IHDR_address[18] | IHDR_address[19] | IHDR_address[20]) == 0)
        {
          // to do
        }
        else
        { return 1; }
      }
      else if (portable_network_graphics_compare_IEND(source_address + 4) == 0)
      { return 0; }
      else
      { source_address += chunk_length + 12; }
    }
  }
  return 1;
}