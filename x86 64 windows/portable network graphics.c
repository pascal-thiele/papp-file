#ifndef portable_network_graphics
#define portable_network_graphics
#include "memory.c"

struct portable_network_graphics_huffman
{
  unsigned int length;
  unsigned int code;
};


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
  unsigned long long value = *address;
  value = (value << 8) | *(address + 1);
  value = (value << 8) | *(address + 2);
  value = (value << 8) | *(address + 3);
  return value;
}


// Orders sixteen bits, the less significant a bit is, the more significant a bit will be.
unsigned long long portable_network_graphics_reverse_bit_order(unsigned long long value)
{
  value = ((value & 65280) >> 8) | ((value & 255) << 8);
  value = ((value & 61680) >> 4) | ((value & 3855) << 4);
  value = ((value & 52428) >> 2) | ((value & 13107) << 2);
  value = ((value & 43690) >> 1) | ((value & 21845) << 1);
  return value;
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


void portable_network_graphics_construct_huffman(struct portable_network_graphics_huffman* code_list_address, unsigned long long code_list_size)
{
  // Count the occurrences of each length.
  unsigned long long count_list[16] = {0};
  unsigned long long code_list_count = code_list_size;
  while (code_list_count != 0)
  {
    *(count_list + code_list_address->length) += 1;
    code_list_address += 1;
    code_list_count -= 1;
  }

  // Calculate the smallest code for each length.
  unsigned long long next_list[16];
  unsigned long long* count_list_address = count_list;
  unsigned long long* next_list_address = next_list;
  unsigned long long next_list_count = 15;
  *count_list_address = 0;
  *next_list_address = 0;
  while (next_list_count != 0)
  {
    *(next_list_address + 1) = (*next_list_address + *count_list_address) << 1;
    next_list_address += 1;
    count_list_address += 1;
    next_list_count -= 1;
  }

  // Write the actual code.
  code_list_address -= code_list_size;
  code_list_count = code_list_size;
  while (code_list_count != 0)
  {
    if (code_list_address->length != 0)
    {
      code_list_address->code = *(next_list + code_list_address->length);
      *(next_list + code_list_address->length) += 1;
    }
    code_list_address += 1;
    code_list_count -= 1;
  }
}


// Pascal Thiele: I do not care about encoding portable network graphics for now!


unsigned long long portable_network_graphics_inflate(unsigned char* source_address, unsigned char* destination_address)
{
  // Validate the header.
  unsigned char compression_method = *source_address & 15;
  unsigned char compression_information = *source_address >> 4;
  unsigned char preset_dictionary = *(source_address + 1) & 32;
  if ((compression_method == 8) && (compression_information < 8) && (preset_dictionary == 0))
  {
    source_address += 2;
    unsigned long long remainder = 0;
    unsigned long long remainder_size = 0;
    unsigned long long block_final = 0;
    unsigned long long block_type = 0;
    unsigned long long destination_size = 0;
    while (1)
    {
      // Read the block header.
      if (remainder_size < 3)
      {
        remainder |= *source_address << remainder_size;
        remainder_size += 8;
        source_address += 1;
      }
      block_final = remainder & 1;
      remainder >>= 1;
      block_type = remainder & 3;
      remainder >>= 2;
      remainder_size -= 3;

      if (block_type == 0)
      {
        // Drop the remainder.
        source_address -= remainder_size >> 3;
        remainder = 0;
        remainder_size = 0;

        // Validate block length.
        unsigned long long block_length = *source_address | (*(source_address + 1) << 8);
        unsigned long long block_length_complement = *(source_address + 2) | (*(source_address + 3) << 8);
        if (block_length != ~block_length_complement)
        { break; }

        // Copy bytes.
        source_address += 4;
        destination_size += block_length;
        while (block_length != 0)
        {
          *destination_address = *source_address;
          destination_address += 1;
          source_address += 1;
          block_length -= 1;
        }
      }
      else if (block_type == 1)
      {
        // Read fixed huffman codes.
        while (1)
        {
          // Lengths are up to eight bits long, with up to five extra bits.
          // Literals are nine bits long at most.
          if (remainder_size < 13)
          {
            remainder |= *source_address << remainder_size;
            remainder_size += 8;
            source_address += 1;
            if (remainder_size < 13)
            {
              remainder |= *source_address << remainder_size;
              remainder_size += 8;
              source_address += 1;
            }
          }

          // Translate low literals with eight bit codes.
          unsigned long long symbol_code = portable_network_graphics_reverse_bit_order(remainder << 8);
          if ((symbol_code >= 48) && (symbol_code < 192))
          {
            *destination_address = symbol_code - 48;
            destination_address += 1;
            destination_size += 1;
            remainder >>= 8;
            remainder_size -= 8;
            continue;
          }

          // Translate high literals with nine bit codes.
          symbol_code = portable_network_graphics_reverse_bit_order(remainder << 7);
          if (symbol_code >= 400)
          {
            *destination_address = symbol_code - 256;
            destination_address += 1;
            destination_size += 1;
            remainder >>= 9;
            remainder_size -= 9;
            continue;
          }

          // Translate low lengths with seven bit codes.
          unsigned long long symbol_length;
          symbol_code = portable_network_graphics_reverse_bit_order(remainder << 9);
          if (symbol_code == 0)
          {
            remainder >>= 7;
            remainder_size -= 7;
            break;
          }
          else if (symbol_code < 9)
          {
            symbol_length = symbol_code + 2;
            remainder >>= 7;
            remainder_size -= 7;
          }
          else if (symbol_code == 9)
          {
            remainder >>= 7;
            symbol_length = (remainder & 1) + 11;
            remainder >>= 1;
            remainder_size -= 8;
          }
          else if (symbol_code == 10)
          {
            remainder >>= 7;
            symbol_length = (remainder & 1) + 13;
            remainder >>= 1;
            remainder_size -= 8;
          }
          else if (symbol_code == 11)
          {
            remainder >>= 7;
            symbol_length = (remainder & 1) + 15;
            remainder >>= 1;
            remainder_size -= 8;
          }
          else if (symbol_code == 12)
          {
            remainder >>= 7;
            symbol_length = (remainder & 1) + 17;
            remainder >>= 1;
            remainder_size -= 8;
          }
          else if (symbol_code == 13)
          {
            remainder >>= 7;
            symbol_length = (remainder & 3) + 19;
            remainder >>= 2;
            remainder_size -= 9;
          }
          else if (symbol_code == 14)
          {
            remainder >>= 7;
            symbol_length = (remainder & 3) + 23;
            remainder >>= 2;
            remainder_size -= 9;
          }
          else if (symbol_code == 15)
          {
            remainder >>= 7;
            symbol_length = (remainder & 3) + 27;
            remainder >>= 2;
            remainder_size -= 9;
          }
          else if (symbol_code == 16)
          {
            remainder >>= 7;
            symbol_length = (remainder & 3) + 31;
            remainder >>= 2;
            remainder_size -= 9;
          }
          else if (symbol_code == 17)
          {
            remainder >>= 7;
            symbol_length = (remainder & 7) + 35;
            remainder >>= 3;
            remainder_size -= 10;
          }
          else if (symbol_code == 18)
          {
            remainder >>= 7;
            symbol_length = (remainder & 7) + 43;
            remainder >>= 3;
            remainder_size -= 10;
          }
          else if (symbol_code == 19)
          {
            remainder >>= 7;
            symbol_length = (remainder & 7) + 51;
            remainder >>= 3;
            remainder_size -= 10;
          }
          else if (symbol_code == 20)
          {
            remainder >>= 7;
            symbol_length = (remainder & 7) + 59;
            remainder >>= 3;
            remainder_size -= 10;
          }
          else if (symbol_code == 21)
          {
            remainder >>= 7;
            symbol_length = (remainder & 15) + 67;
            remainder >>= 4;
            remainder_size -= 11;
          }
          else if (symbol_code == 22)
          {
            remainder >>= 7;
            symbol_length = (remainder & 15) + 83;
            remainder >>= 4;
            remainder_size -= 11;
          }
          else if (symbol_code == 23)
          {
            remainder >>= 7;
            symbol_length = (remainder & 15) + 99;
            remainder >>= 4;
            remainder_size -= 11;
          }
          else
          {
            // Translate high lengths with eight bit codes.
            symbol_code = portable_network_graphics_reverse_bit_order(remainder << 8);
            if (symbol_code == 192)
            {
              remainder >>= 8;
              symbol_length = (remainder & 15) + 115;
              remainder >>= 4;
              remainder_size -= 12;
            }
            else if (symbol_code == 193)
            {
              remainder >>= 8;
              symbol_length = (remainder & 31) + 131;
              remainder >>= 5;
              remainder_size -= 13;
            }
            else if (symbol_code == 194)
            {
              remainder >>= 8;
              symbol_length = (remainder & 31) + 163;
              remainder >>= 5;
              remainder_size -= 13;
            }
            else if (symbol_code == 195)
            {
              remainder >>= 8;
              symbol_length = (remainder & 31) + 195;
              remainder >>= 5;
              remainder_size -= 13;
            }
            else if (symbol_code == 196)
            {
              remainder >>= 8;
              symbol_length = (remainder & 31) + 227;
              remainder >>= 5;
              remainder_size -= 13;
            }
            else if (symbol_code == 197)
            {
              symbol_length = 258;
              remainder >>= 8;
              remainder_size -= 8;
            }
            else
            { return !0; }
          }

          // Distance codes are five bits long, with up to thirteen extra bits.
          if (remainder_size < 18)
          {
            remainder |= *source_address << remainder_size;
            remainder_size += 8;
            source_address += 1;
            if (remainder_size < 18)
            {
              remainder |= *source_address << remainder_size;
              remainder_size += 8;
              source_address += 1;
              if (remainder_size < 18)
              {
                remainder |= *source_address << remainder_size;
                remainder_size += 8;
                source_address += 1;
              }
            }
          }

          // Translate the distance.
          unsigned long long symbol_distance;
          symbol_code = portable_network_graphics_reverse_bit_order(remainder << 11);
          if (symbol_code < 4)
          {
            symbol_distance = symbol_code + 1;
            remainder >>= 5;
            remainder_size -= 5;
          }
          else if (symbol_code == 4)
          {
            remainder >>= 5;
            symbol_distance = (remainder & 1) + 5;
            remainder >>= 1;
            remainder_size -= 6;
          }
          else if (symbol_code == 5)
          {
            remainder >>= 5;
            symbol_distance = (remainder & 1) + 7;
            remainder >>= 1;
            remainder_size -= 6;
          }
          else if (symbol_code == 6)
          {
            remainder >>= 5;
            symbol_distance = (remainder & 3) + 9;
            remainder >>= 2;
            remainder_size -= 7;
          }
          else if (symbol_code == 7)
          {
            remainder >>= 5;
            symbol_distance = (remainder & 3) + 13;
            remainder >>= 2;
            remainder_size -= 7;
          }
          else if (symbol_code == 8)
          {
            remainder >>= 5;
            symbol_distance = (remainder & 7) + 17;
            remainder >>= 3;
            remainder_size -= 8;
          }
          else if (symbol_code == 9)
          {
            remainder >>= 5;
            symbol_distance = (remainder & 7) + 25;
            remainder >>= 3;
            remainder_size -= 8;
          }
          else if (symbol_code == 10)
          {
            remainder >>= 5;
            symbol_distance = (remainder & 15) + 33;
            remainder >>= 4;
            remainder_size -= 9;
          }
          else if (symbol_code == 11)
          {
            remainder >>= 5;
            symbol_distance = (remainder & 15) + 49;
            remainder >>= 4;
            remainder_size -= 9;
          }
          else if (symbol_code == 12)
          {
            remainder >>= 5;
            symbol_distance = (remainder & 31) + 65;
            remainder >>= 5;
            remainder_size -= 10;
          }
          else if (symbol_code == 13)
          {
            remainder >>= 5;
            symbol_distance = (remainder & 31) + 97;
            remainder >>= 5;
            remainder_size -= 10;
          }
          else if (symbol_code == 14)
          {
            remainder >>= 5;
            symbol_distance = (remainder & 63) + 129;
            remainder >>= 6;
            remainder_size -= 11;
          }
          else if (symbol_code == 15)
          {
            remainder >>= 5;
            symbol_distance = (remainder & 63) + 193;
            remainder >>= 6;
            remainder_size -= 11;
          }
          else if (symbol_code == 16)
          {
            remainder >>= 5;
            symbol_distance = (remainder & 127) + 257;
            remainder >>= 7;
            remainder_size -= 12;
          }
          else if (symbol_code == 17)
          {
            remainder >>= 5;
            symbol_distance = (remainder & 127) + 385;
            remainder >>= 7;
            remainder_size -= 12;
          }
          else if (symbol_code == 18)
          {
            remainder >>= 5;
            symbol_distance = (remainder & 255) + 513;
            remainder >>= 8;
            remainder_size -= 13;
          }
          else if (symbol_code == 19)
          {
            remainder >>= 5;
            symbol_distance = (remainder & 255) + 769;
            remainder >>= 8;
            remainder_size -= 13;
          }
          else if (symbol_code == 20)
          {
            remainder >>= 5;
            symbol_distance = (remainder & 511) + 1025;
            remainder >>= 9;
            remainder_size -= 14;
          }
          else if (symbol_code == 21)
          {
            remainder >>= 5;
            symbol_distance = (remainder & 511) + 1537;
            remainder >>= 9;
            remainder_size -= 14;
          }
          else if (symbol_code == 22)
          {
            remainder >>= 5;
            symbol_distance = (remainder & 1023) + 2049;
            remainder >>= 10;
            remainder_size -= 15;
          }
          else if (symbol_code == 23)
          {
            remainder >>= 5;
            symbol_distance = (remainder & 1023) + 3073;
            remainder >>= 10;
            remainder_size -= 15;
          }
          else if (symbol_code == 24)
          {
            remainder >>= 5;
            symbol_distance = (remainder & 2047) + 4097;
            remainder >>= 11;
            remainder_size -= 16;
          }
          else if (symbol_code == 25)
          {
            remainder >>= 5;
            symbol_distance = (remainder & 2047) + 6145;
            remainder >>= 11;
            remainder_size -= 16;
          }
          else if (symbol_code == 26)
          {
            remainder >>= 5;
            symbol_distance = (remainder & 4095) + 8193;
            remainder >>= 12;
            remainder_size -= 17;
          }
          else if (symbol_code == 27)
          {
            remainder >>= 5;
            symbol_distance = (remainder & 4095) + 12289;
            remainder >>= 12;
            remainder_size -= 17;
          }
          else if (symbol_code == 28)
          {
            remainder >>= 5;
            symbol_distance = (remainder & 8191) + 16385;
            remainder >>= 13;
            remainder_size -= 18;
          }
          else if (symbol_code == 29)
          {
            remainder >>= 5;
            symbol_distance = (remainder & 8191) + 24577;
            remainder >>= 13;
            remainder_size -= 18;
          }
          else
          { return !0; }

          // Copy length bytes, from distance bytes back in destination.
          unsigned char* symbol_address = destination_address - symbol_distance;
          destination_size += symbol_length;
          while (symbol_length != 0)
          {
            *destination_address = *symbol_address;
            destination_address += 1;
            symbol_address += 1;
            symbol_length -= 1;
          }
        }
      }
      else if (block_type == 2)
      {
        // Read dynamic huffman codes.
        if (remainder_size < 14)
        {
          remainder |= *source_address << remainder_size;
          remainder_size += 8;
          source_address += 1;
          if (remainder_size < 14)
          {
            remainder |= *source_address << remainder_size;
            remainder_size += 8;
            source_address += 1;
          }
        }

        unsigned long long literal_code_count = (remainder & 31) + 257;
        remainder >>= 5;
        unsigned long long distance_code_count = (remainder & 31) + 1;
        remainder >>= 5;
        unsigned long long code_code_count = (remainder & 15) + 4;
        remainder >>= 4;
        remainder_size -= 14;

        // Temporarily write the shuffled code codes to the code element of the huffman.
        struct portable_network_graphics_huffman code_code_list[19] = {0};
        struct portable_network_graphics_huffman* code_code_list_address = code_code_list;
        unsigned long long code_code_list_size = code_code_count;
        while (code_code_list_size != 0)
        {
          if (remainder_size < 3)
          {
            remainder |= *source_address << remainder_size;
            remainder_size += 8;
            source_address += 1;
          }
          code_code_list_address->code = remainder & 7;
          remainder >>= 3;
          remainder_size -= 3;
          code_code_list_address += 1;
          code_code_list_size -= 1;
        }

        // Order the code codes.
        code_code_list[0].length = code_code_list[3].code;
        code_code_list[1].length = code_code_list[17].code;
        code_code_list[2].length = code_code_list[15].code;
        code_code_list[3].length = code_code_list[13].code;
        code_code_list[4].length = code_code_list[11].code;
        code_code_list[5].length = code_code_list[9].code;
        code_code_list[6].length = code_code_list[7].code;
        code_code_list[7].length = code_code_list[5].code;
        code_code_list[8].length = code_code_list[4].code;
        code_code_list[9].length = code_code_list[6].code;
        code_code_list[10].length = code_code_list[8].code;
        code_code_list[11].length = code_code_list[10].code;
        code_code_list[12].length = code_code_list[12].code;
        code_code_list[13].length = code_code_list[14].code;
        code_code_list[14].length = code_code_list[16].code;
        code_code_list[15].length = code_code_list[18].code;
        code_code_list[16].length = code_code_list[0].code;
        code_code_list[17].length = code_code_list[1].code;
        code_code_list[18].length = code_code_list[2].code;
        portable_network_graphics_construct_huffman(code_code_list, 19);

        // Construct the literal code list and distance code list with the code code list.
        struct portable_network_graphics_huffman code_list[320];
        struct portable_network_graphics_huffman* code_list_address = code_list;
        unsigned long long code_list_size = literal_code_count + distance_code_count;
        while (code_list_size != 0)
        {
          if (remainder_size < 7)
          {
            remainder |= *source_address << remainder_size;
            remainder_size += 8;
            source_address += 1;
          }

          // Look up the symbol in the code code alphabet.
          code_code_list_address = code_code_list;
          unsigned long long code_code_list_index = 0;
          while (code_code_list_index < 19)
          {
            if (code_code_list_address->length != 0)
            {
              unsigned long long code_code = portable_network_graphics_reverse_bit_order(remainder << (16 - code_code_list_address->length));
              if (code_code == code_code_list_address->code)
              {
                remainder >>= code_code_list_address->length;
                remainder_size -= code_code_list_address->length;
                break;
              }
            }
            code_code_list_address += 1;
            code_code_list_index += 1;
          }

          if (remainder_size < 7)
          {
            remainder |= *source_address << remainder_size;
            remainder_size += 8;
            source_address += 1;
          }

          // Translate the code.
          if (code_code_list_index < 16)
          {
            code_list_address->length = code_code_list_index;
            code_list_address += 1;
            code_list_size -= 1;
          }
          else if (code_code_list_index == 16)
          {
            unsigned long long repeat = (remainder & 3) + 3;
            if (code_list_size < repeat)
            { return !0; }
            code_list_size -= repeat;
            remainder >>= 2;
            remainder_size -= 2;
            while (repeat != 0)
            {
              code_list_address->length = (code_list_address - 1)->length;
              code_list_address += 1;
              repeat -= 1;
            }
          }
          else if (code_code_list_index == 17)
          {
            unsigned long long repeat = (remainder & 7) + 3;
            if (code_list_size < repeat)
            { return !0; }
            code_list_size -= repeat;
            remainder >>= 3;
            remainder_size -= 3;
            while (repeat != 0)
            {
              code_list_address->length = 0;
              code_list_address += 1;
              repeat -= 1;
            }
          }
          else if (code_code_list_index == 18)
          {
            unsigned long long repeat = (remainder & 127) + 11;
            if (code_list_size < repeat)
            { return !0; }
            code_list_size -= repeat;
            remainder >>= 7;
            remainder_size -= 7;
            while (repeat != 0)
            {
              code_list_address->length = 0;
              code_list_address += 1;
              repeat -= 1;
            }
          }
          else
          { return !0; }
        }

        portable_network_graphics_construct_huffman(code_list, literal_code_count);
        portable_network_graphics_construct_huffman(code_list + literal_code_count, distance_code_count);

        // Translate the compressed data.
        while (1)
        {
          if (remainder_size < 15)
          {
            remainder |= *source_address << remainder_size;
            remainder_size += 8;
            source_address += 1;
            if (remainder_size < 15)
            {
              remainder |= *source_address << remainder_size;
              remainder_size += 8;
              source_address += 1;
            }
          }

          // Look up the symbol in the literal alphabet.
          struct portable_network_graphics_huffman* literal_list_address = code_list;
          unsigned long long literal_list_index = 0;
          while (literal_list_index < literal_code_count)
          {
            if (literal_list_address->length != 0)
            {
              unsigned long long literal_code = portable_network_graphics_reverse_bit_order(remainder << (16 - literal_list_address->length));
              if (literal_code == literal_list_address->code)
              {
                remainder >>= literal_list_address->length;
                remainder_size -= literal_list_address->length;
                break;
              }
            }
            literal_list_address += 1;
            literal_list_index += 1;
          }

          if (remainder_size < 5)
          {
            remainder |= *source_address << remainder_size;
            remainder_size += 8;
            source_address += 1;
          }

          // Translate the literal or length.
          unsigned long long symbol_length;
          if (literal_list_index < 256)
          {
            *destination_address = literal_list_index;
            destination_address += 1;
            destination_size += 1;
            continue;
          }
          else if (literal_list_index == 256)
          { break; }
          else if (literal_list_index < 265)
          { symbol_length = literal_list_index - 254; }
          else if (literal_list_index == 265)
          {
            symbol_length = (remainder & 1) + 11;
            remainder >>= 1;
            remainder_size -= 1;
          }
          else if (literal_list_index == 266)
          {
            symbol_length = (remainder & 1) + 13;
            remainder >>= 1;
            remainder_size -= 1;
          }
          else if (literal_list_index == 267)
          {
            symbol_length = (remainder & 1) + 15;
            remainder >>= 1;
            remainder_size -= 1;
          }
          else if (literal_list_index == 268)
          {
            symbol_length = (remainder & 1) + 17;
            remainder >>= 1;
            remainder_size -= 1;
          }
          else if (literal_list_index == 269)
          {
            symbol_length = (remainder & 3) + 19;
            remainder >>= 2;
            remainder_size -= 2;
          }
          else if (literal_list_index == 270)
          {
            symbol_length = (remainder & 3) + 23;
            remainder >>= 2;
            remainder_size -= 2;
          }
          else if (literal_list_index == 271)
          {
            symbol_length = (remainder & 3) + 27;
            remainder >>= 2;
            remainder_size -= 2;
          }
          else if (literal_list_index == 272)
          {
            symbol_length = (remainder & 3) + 31;
            remainder >>= 2;
            remainder_size -= 2;
          }
          else if (literal_list_index == 273)
          {
            symbol_length = (remainder & 7) + 35;
            remainder >>= 3;
            remainder_size -= 3;
          }
          else if (literal_list_index == 274)
          {
            symbol_length = (remainder & 7) + 43;
            remainder >>= 3;
            remainder_size -= 3;
          }
          else if (literal_list_index == 275)
          {
            symbol_length = (remainder & 7) + 51;
            remainder >>= 3;
            remainder_size -= 3;
          }
          else if (literal_list_index == 276)
          {
            symbol_length = (remainder & 7) + 59;
            remainder >>= 3;
            remainder_size -= 3;
          }
          else if (literal_list_index == 277)
          {
            symbol_length = (remainder & 15) + 67;
            remainder >>= 4;
            remainder_size -= 4;
          }
          else if (literal_list_index == 278)
          {
            symbol_length = (remainder & 15) + 83;
            remainder >>= 4;
            remainder_size -= 4;
          }
          else if (literal_list_index == 279)
          {
            symbol_length = (remainder & 15) + 99;
            remainder >>= 4;
            remainder_size -= 4;
          }
          else if (literal_list_index == 280)
          {
            symbol_length = (remainder & 15) + 115;
            remainder >>= 4;
            remainder_size -= 4;
          }
          else if (literal_list_index == 281)
          {
            symbol_length = (remainder & 31) + 131;
            remainder >>= 5;
            remainder_size -= 5;
          }
          else if (literal_list_index == 282)
          {
            symbol_length = (remainder & 31) + 163;
            remainder >>= 5;
            remainder_size -= 5;
          }
          else if (literal_list_index == 283)
          {
            symbol_length = (remainder & 31) + 195;
            remainder >>= 5;
            remainder_size -= 5;
          }
          else if (literal_list_index == 284)
          {
            symbol_length = (remainder & 31) + 227;
            remainder >>= 5;
            remainder_size -= 5;
          }
          else if (literal_list_index == 285)
          { symbol_length = 258; }
          else
          { return !0; }

          if (remainder_size < 15)
          {
            remainder |= *source_address << remainder_size;
            remainder_size += 8;
            source_address += 1;
            if (remainder_size < 15)
            {
              remainder |= *source_address << remainder_size;
              remainder_size += 8;
              source_address += 1;
            }
          }

          // Look up the symbol in the distance list.
          struct portable_network_graphics_huffman* distance_list_address = code_list + literal_code_count;
          unsigned long long distance_list_index = 0;
          while (distance_list_index < distance_code_count)
          {
            if (distance_list_address->length != 0)
            {
              unsigned long long distance_code = portable_network_graphics_reverse_bit_order(remainder << (16 - distance_list_address->length));
              if (distance_code == distance_list_address->code)
              {
                remainder >>= distance_list_address->length;
                remainder_size -= distance_list_address->length;
                break;
              }
            }
            distance_list_address += 1;
            distance_list_index += 1;
          }

          if (remainder_size < 13)
          {
            remainder |= *source_address << remainder_size;
            remainder_size += 8;
            source_address += 1;
            if (remainder_size < 13)
            {
              remainder |= *source_address << remainder_size;
              remainder_size += 8;
              source_address += 1;
            }
          }

          // Translate the distance.
          unsigned long long symbol_distance;
          if (distance_list_index < 4)
          { symbol_distance = distance_list_index + 1; }
          else if (distance_list_index == 4)
          {
            symbol_distance = (remainder & 1) + 5;
            remainder >>= 1;
            remainder_size -= 1;
          }
          else if (distance_list_index == 5)
          {
            symbol_distance = (remainder & 1) + 7;
            remainder >>= 1;
            remainder_size -= 1;
          }
          else if (distance_list_index == 6)
          {
            symbol_distance = (remainder & 3) + 9;
            remainder >>= 2;
            remainder_size -= 2;
          }
          else if (distance_list_index == 7)
          {
            symbol_distance = (remainder & 3) + 13;
            remainder >>= 2;
            remainder_size -= 2;
          }
          else if (distance_list_index == 8)
          {
            symbol_distance = (remainder & 7) + 17;
            remainder >>= 3;
            remainder_size -= 3;
          }
          else if (distance_list_index == 9)
          {
            symbol_distance = (remainder & 7) + 25;
            remainder >>= 3;
            remainder_size -= 3;
          }
          else if (distance_list_index == 10)
          {
            symbol_distance = (remainder & 15) + 33;
            remainder >>= 4;
            remainder_size -= 4;
          }
          else if (distance_list_index == 11)
          {
            symbol_distance = (remainder & 15) + 49;
            remainder >>= 4;
            remainder_size -= 4;
          }
          else if (distance_list_index == 12)
          {
            symbol_distance = (remainder & 31) + 65;
            remainder >>= 5;
            remainder_size -= 5;
          }
          else if (distance_list_index == 13)
          {
            symbol_distance = (remainder & 31) + 97;
            remainder >>= 5;
            remainder_size -= 5;
          }
          else if (distance_list_index == 14)
          {
            symbol_distance = (remainder & 63) + 129;
            remainder >>= 6;
            remainder_size -= 6;
          }
          else if (distance_list_index == 15)
          {
            symbol_distance = (remainder & 63) + 193;
            remainder >>= 6;
            remainder_size -= 6;
          }
          else if (distance_list_index == 16)
          {
            symbol_distance = (remainder & 127) + 257;
            remainder >>= 7;
            remainder_size -= 7;
          }
          else if (distance_list_index == 17)
          {
            symbol_distance = (remainder & 127) + 385;
            remainder >>= 7;
            remainder_size -= 7;
          }
          else if (distance_list_index == 18)
          {
            symbol_distance = (remainder & 255) + 513;
            remainder >>= 8;
            remainder_size -= 8;
          }
          else if (distance_list_index == 19)
          {
            symbol_distance = (remainder & 255) + 769;
            remainder >>= 8;
            remainder_size -= 8;
          }
          else if (distance_list_index == 20)
          {
            symbol_distance = (remainder & 511) + 1025;
            remainder >>= 9;
            remainder_size -= 9;
          }
          else if (distance_list_index == 21)
          {
            symbol_distance = (remainder & 511) + 1537;
            remainder >>= 9;
            remainder_size -= 9;
          }
          else if (distance_list_index == 22)
          {
            symbol_distance = (remainder & 1023) + 2049;
            remainder >>= 10;
            remainder_size -= 10;
          }
          else if (distance_list_index == 23)
          {
            symbol_distance = (remainder & 1023) + 3073;
            remainder >>= 10;
            remainder_size -= 10;
          }
          else if (distance_list_index == 24)
          {
            symbol_distance = (remainder & 2047) + 4097;
            remainder >>= 11;
            remainder_size -= 11;
          }
          else if (distance_list_index == 25)
          {
            symbol_distance = (remainder & 2047) + 6145;
            remainder >>= 11;
            remainder_size -= 11;
          }
          else if (distance_list_index == 26)
          {
            symbol_distance = (remainder & 4095) + 8193;
            remainder >>= 12;
            remainder_size -= 12;
          }
          else if (distance_list_index == 27)
          {
            symbol_distance = (remainder & 4095) + 12289;
            remainder >>= 12;
            remainder_size -= 12;
          }
          else if (distance_list_index == 28)
          {
            symbol_distance = (remainder & 8191) + 16385;
            remainder >>= 13;
            remainder_size -= 13;
          }
          else if (distance_list_index == 29)
          {
            symbol_distance = (remainder & 8191) + 24577;
            remainder >>= 13;
            remainder_size -= 13;
          }
          else
          { return !0; }

          // Copy length bytes, from distance bytes back in destination.
          unsigned char* symbol_address = destination_address - symbol_distance;
          destination_size += symbol_length;
          while (symbol_length != 0)
          {
            *destination_address = *symbol_address;
            destination_address += 1;
            symbol_address += 1;
            symbol_length -= 1;
          }
        }
      }
      else
      { break; }

      if (block_final != 0)
      {
        // Calculate the adler32 from the inflated data.
        unsigned long long adler32_low = 1;
        unsigned long long adler32_high = 0;
        destination_address -= destination_size;
        while (destination_size != 0)
        {
          adler32_low = (adler32_low + *destination_address) % 65521;
          adler32_high = (adler32_high + adler32_low) % 65521;
          destination_address += 1;
          destination_size -= 1;
        }

        // Drop the remainder.
        source_address -= remainder_size >> 3;

        // Compare the adler32.
        unsigned long long adler32_high_source = (*source_address << 8) | *(source_address + 1);
        unsigned long long adler32_low_source = *(source_address + 2) << 8 | *(source_address + 3);
        if ((adler32_high != adler32_high_source) || (adler32_low != adler32_low_source))
        { break; }

        // We made it out alive... at last.
        return 0;
      }
    }
  }
  return !0;
}


unsigned long long portable_network_graphics_paeth(unsigned long long horizontal, unsigned long long vertical, unsigned long long diagonal)
{
  unsigned long long horizontal_difference = vertical - diagonal;
  if (vertical < diagonal)
  { horizontal_difference = diagonal - vertical; }

  unsigned long long vertical_difference = horizontal - diagonal;
  if (horizontal < diagonal)
  { vertical_difference = diagonal - horizontal; }

  unsigned long long diagonal_difference = horizontal + vertical - diagonal - diagonal;
  if ((horizontal + vertical) < (diagonal + diagonal))
  { diagonal_difference = diagonal + diagonal - horizontal - vertical; }
  diagonal_difference &= 255;

  if ((horizontal_difference <= vertical_difference) && (horizontal_difference <= diagonal_difference))
  { return horizontal; }
  else if (vertical_difference <= diagonal_difference)
  { return vertical; }
  else
  { return diagonal; }
}


unsigned long long portable_network_graphics_decode_size(unsigned char* source_address)
{
  if (portable_network_graphics_identify(source_address) == 0)
  {
    unsigned long long chunk_length = portable_network_graphics_read_int(source_address + 8);
    unsigned long long chunk_cyclic_redundancy_check = portable_network_graphics_read_int(source_address + chunk_length + 16);
    if ((portable_network_graphics_cyclic_redundancy_check(source_address + 12, chunk_length + 4) == chunk_cyclic_redundancy_check)
      && (portable_network_graphics_compare_IHDR(source_address + 12) == 0))
    {
      unsigned long long width = portable_network_graphics_read_int(source_address + 16);
      unsigned long long height = portable_network_graphics_read_int(source_address + 20);
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
    // Validate and extract basic information.
    unsigned char* chunk_address = source_address + 8;
    unsigned long long compressed_size = 0;
    unsigned char* IHDR_address = 0;
    unsigned char* PLTE_address = 0;
    unsigned char* tRNS_address = 0;
    while (1)
    {
      unsigned long long chunk_length = portable_network_graphics_read_int(chunk_address);
      unsigned long long chunk_cyclic_redundancy_check = portable_network_graphics_read_int(chunk_address + chunk_length + 8);
      if (chunk_cyclic_redundancy_check != portable_network_graphics_cyclic_redundancy_check(chunk_address + 4, chunk_length + 4))
      { return !0; }
      
      if (portable_network_graphics_compare_IHDR(chunk_address + 4) == 0)
      {
        if (IHDR_address != 0)
        { return !0; }
        IHDR_address = chunk_address;
        chunk_address += chunk_length + 12;
      }
      else if (portable_network_graphics_compare_PLTE(chunk_address + 4) == 0)
      {
        if (PLTE_address != 0)
        { return !0; }
        PLTE_address = chunk_address;
        chunk_address += chunk_length + 12;
      }
      else if (portable_network_graphics_compare_tRNS(chunk_address + 4) == 0)
      {
        if (tRNS_address != 0)
        { return !0; }
        tRNS_address = chunk_address;
        chunk_address += chunk_length + 12;
      }
      else if (portable_network_graphics_compare_IDAT(chunk_address + 4) == 0)
      {
        compressed_size += chunk_length;
        chunk_address += chunk_length + 12;
      }
      else if (portable_network_graphics_compare_IEND(chunk_address + 4) == 0)
      { break; }
      else
      { chunk_address += chunk_length + 12; }
    }

    // Copy IDAT to be contiguous.
    if (compressed_size == 0)
    { return !0; }

    unsigned char* compressed_address = memory_allocate(compressed_size);
    if (compressed_address == 0)
    { return !0; }

    chunk_address = source_address + 8;
    while (1)
    {
      unsigned long long chunk_length = portable_network_graphics_read_int(chunk_address);
      if (portable_network_graphics_compare_IDAT(chunk_address + 4) == 0)
      {
        chunk_address += 8;
        while (chunk_length != 0)
        {
          *compressed_address = *chunk_address;
          compressed_address += 1;
          chunk_address += 1;
          chunk_length -= 1;
        }
        chunk_address += 4;
      }
      else if (portable_network_graphics_compare_IEND(chunk_address + 4) == 0)
      { break; }
      else
      { chunk_address += chunk_length + 12; }
    }

    // TO DO Write translation routines for adam7 interlacing.

    // Translate the contiguous data according to IHDR.
    compressed_address -= compressed_size;
    if ((IHDR_address[16] == 8) && (IHDR_address[17] == 0) && (IHDR_address[18] == 0) && (IHDR_address[19] == 0) && (IHDR_address[20] == 0))
    {
      unsigned long long width = portable_network_graphics_read_int(IHDR_address + 8);
      unsigned long long filtered_size = width + 1;
      filtered_size *= portable_network_graphics_read_int(IHDR_address + 12);
      if (filtered_size == 0)
      {
        memory_free(compressed_size);
        return !0;
      }

      unsigned char* filtered_address = memory_allocate(filtered_size);
      if (filtered_address == 0)
      {
        memory_free(compressed_size);
        return !0;
      }

      if (portable_network_graphics_inflate(compressed_address, filtered_address) != 0)
      {
        memory_free(filtered_size);
        memory_free(compressed_size);
        return !0;
      }

      unsigned long long filter_type;
      unsigned long long filtered_count = 0;
      while (filtered_count < filtered_size)
      {
        if ((filtered_count % (width + 1)) == 0)
        {
          filter_type = *filtered_address;
          filtered_address += 1;
          filtered_count += 1;
        }
        else if (filter_type == 0)
        {
          *destination_address = *filtered_address;
          *(destination_address + 1) = *filtered_address;
          *(destination_address + 2) = *filtered_address;
          *(destination_address + 3) = *filtered_address;
          *(destination_address + 4) = *filtered_address;
          *(destination_address + 5) = *filtered_address;
          *(destination_address + 6) = 255;
          *(destination_address + 7) = 255;
          if ((tRNS_address != 0) && (*(tRNS_address + 9) == *filtered_address))
          {
            *(destination_address + 6) = 0;
            *(destination_address + 7) = 0;
          }

          destination_address += 8;
          filtered_address += 1;
          filtered_count += 1;
        }
        else if (filter_type == 1)
        {
          unsigned long long sample = *filtered_address;
          if ((filtered_count % (width + 1)) >= 2)
          {
            sample += *(destination_address - 8);
            sample &= 255;
          }

          *destination_address = sample;
          *(destination_address + 1) = sample;
          *(destination_address + 2) = sample;
          *(destination_address + 3) = sample;
          *(destination_address + 4) = sample;
          *(destination_address + 5) = sample;
          *(destination_address + 6) = 255;
          *(destination_address + 7) = 255;
          if ((tRNS_address != 0) && (*(tRNS_address + 9) == sample))
          {
            *(destination_address + 6) = 0;
            *(destination_address + 7) = 0;
          }

          destination_address += 8;
          filtered_address += 1;
          filtered_count += 1;
        }
        else if (filter_type == 2)
        {
          unsigned long long sample = *filtered_address;
          if (filtered_count >= (width + 1))
          {
            sample += *(destination_address - (width * 8));
            sample &= 255;
          }

          *destination_address = sample;
          *(destination_address + 1) = sample;
          *(destination_address + 2) = sample;
          *(destination_address + 3) = sample;
          *(destination_address + 4) = sample;
          *(destination_address + 5) = sample;
          *(destination_address + 6) = 255;
          *(destination_address + 7) = 255;
          if ((tRNS_address != 0) && (*(tRNS_address + 9) == sample))
          {
            *(destination_address + 6) = 0;
            *(destination_address + 7) = 0;
          }

          destination_address += 8;
          filtered_address += 1;
          filtered_count += 1;
        }
        else if (filter_type == 3)
        {
          unsigned long long sample = 0;
          if ((filtered_count % (width + 1)) >= 2)
          { sample = *(destination_address - 8); }

          if (filtered_count >= (width + 1))
          {
            sample += *(destination_address - (width * 8));
            sample &= 255;
          }

          sample = ((sample / 2) + *filtered_address) & 255;
          *destination_address = sample;
          *(destination_address + 1) = sample;
          *(destination_address + 2) = sample;
          *(destination_address + 3) = sample;
          *(destination_address + 4) = sample;
          *(destination_address + 5) = sample;
          *(destination_address + 6) = 255;
          *(destination_address + 7) = 255;
          if ((tRNS_address != 0) && (*(tRNS_address + 9) == sample))
          {
            *(destination_address + 6) = 0;
            *(destination_address + 7) = 0;
          }

          destination_address += 8;
          filtered_address += 1;
          filtered_count += 1;
        }
        else if (filter_type == 4)
        {
          unsigned long long dimension_count = 0;
          unsigned long long horizontal = 0;
          if ((filtered_count % (width + 1)) >= 2)
          {
            horizontal = *(destination_address - 8);
            dimension_count += 1;
          }

          unsigned long long vertical = 0;
          if (filtered_count >= (width + 1))
          {
            vertical = *(destination_address - (width * 8));
            dimension_count += 1;
          }

          unsigned long long diagonal = 0;
          if (dimension_count == 2)
          { diagonal = *((destination_address - 8) - (width * 8)); }

          unsigned long long sample = portable_network_graphics_paeth(horizontal, vertical, diagonal);
          sample = (sample + *filtered_address) & 255;
          *destination_address = sample;
          *(destination_address + 1) = sample;
          *(destination_address + 2) = sample;
          *(destination_address + 3) = sample;
          *(destination_address + 4) = sample;
          *(destination_address + 5) = sample;
          *(destination_address + 6) = 255;
          *(destination_address + 7) = 255;
          if ((tRNS_address != 0) && (*(tRNS_address + 9) == sample))
          {
            *(destination_address + 6) = 0;
            *(destination_address + 7) = 0;
          }

          destination_address += 8;
          filtered_address += 1;
          filtered_count += 1;
        }
        else
        {
          memory_free(filtered_size);
          memory_free(compressed_size);
          return !0;
        }
      }
      memory_free(filtered_size);
      memory_free(compressed_size);
      return 0;
    }
    else if ((IHDR_address[16] == 16) && (IHDR_address[17] == 0) && (IHDR_address[18] == 0) && (IHDR_address[19] == 0) && (IHDR_address[20] == 0))
    {
      unsigned long long width = portable_network_graphics_read_int(IHDR_address + 8);
      unsigned long long filtered_size = (width * 2) + 1;
      filtered_size *= portable_network_graphics_read_int(IHDR_address + 12);
      if (filtered_size == 0)
      {
        memory_free(compressed_size);
        return !0;
      }

      unsigned char* filtered_address = memory_allocate(filtered_size);
      if (filtered_address == 0)
      {
        memory_free(compressed_size);
        return !0;
      }

      if (portable_network_graphics_inflate(compressed_address, filtered_address) != 0)
      {
        memory_free(filtered_size);
        memory_free(compressed_size);
        return !0;
      }

      unsigned long long filter_type;
      unsigned long long filtered_count = 0;
      while (filtered_count < filtered_size)
      {
        if ((filtered_count % ((width * 2) + 1)) == 0)
        {
          filter_type = *filtered_address;
          filtered_address += 1;
          filtered_count += 1;
        }
        else if (filter_type == 0)
        {
          unsigned long long high = *filtered_address;
          unsigned long long low = *(filtered_address + 1);
          *destination_address = low;
          *(destination_address + 1) = high;
          *(destination_address + 2) = low;
          *(destination_address + 3) = high;
          *(destination_address + 4) = low;
          *(destination_address + 5) = high;
          *(destination_address + 6) = 255;
          *(destination_address + 7) = 255;
          if ((tRNS_address != 0) && (*(tRNS_address + 8) == high) && (*(tRNS_address + 9) == low))
          {
            *(destination_address + 6) = 0;
            *(destination_address + 7) = 0;
          }

          destination_address += 8;
          filtered_address += 2;
          filtered_count += 2;
        }
        else if (filter_type == 1)
        {
          unsigned long long high = *filtered_address;
          unsigned long long low = *(filtered_address + 1);
          if ((filtered_count % ((width * 2) + 1)) >= 3)
          {
            low += *(destination_address - 8);
            low &= 255;
            high += *(destination_address - 7);
            high &= 255;
          }

          *destination_address = low;
          *(destination_address + 1) = high;
          *(destination_address + 2) = low;
          *(destination_address + 3) = high;
          *(destination_address + 4) = low;
          *(destination_address + 5) = high;
          *(destination_address + 6) = 255;
          *(destination_address + 7) = 255;
          if ((tRNS_address != 0) && (*(tRNS_address + 8) == high) && (*(tRNS_address + 9) == low))
          {
            *(destination_address + 6) = 0;
            *(destination_address + 7) = 0;
          }

          destination_address += 8;
          filtered_address += 2;
          filtered_count += 2;
        }
        else if (filter_type == 2)
        {
          unsigned long long high = *filtered_address;
          unsigned long long low = *(filtered_address + 1);
          if (filtered_count >= ((width * 2) + 1))
          {
            low += *(destination_address - (width * 8));
            low &= 255;
            high += *(destination_address - (width * 8) + 1);
            high &= 255;
          }

          *destination_address = low;
          *(destination_address + 1) = high;
          *(destination_address + 2) = low;
          *(destination_address + 3) = high;
          *(destination_address + 4) = low;
          *(destination_address + 5) = high;
          *(destination_address + 6) = 255;
          *(destination_address + 7) = 255;
          if ((tRNS_address != 0) && (*(tRNS_address + 8) == high) && (*(tRNS_address + 9) == low))
          {
            *(destination_address + 6) = 0;
            *(destination_address + 7) = 0;
          }

          destination_address += 8;
          filtered_address += 2;
          filtered_count += 2;
        }
        else if (filter_type == 3)
        {
          unsigned long long high = 0;
          unsigned long long low = 0;
          if ((filtered_count % ((width * 2) + 1)) >= 3)
          {
            low = *(destination_address - 8);
            high = *(destination_address - 7);
          }

          if (filtered_count >= ((width * 2) + 1))
          {
            low += *(destination_address - (width * 8));
            low &= 255;
            high += *(destination_address - (width * 8) + 1);
            high &= 255;
          }

          high = ((high / 2) + *filtered_address) & 255;
          low = ((low / 2) + *(filtered_address + 1)) & 255;
          *destination_address = low;
          *(destination_address + 1) = high;
          *(destination_address + 2) = low;
          *(destination_address + 3) = high;
          *(destination_address + 4) = low;
          *(destination_address + 5) = high;
          *(destination_address + 6) = 255;
          *(destination_address + 7) = 255;
          if ((tRNS_address != 0) && (*(tRNS_address + 8) == high) && (*(tRNS_address + 9) == low))
          {
            *(destination_address + 6) = 0;
            *(destination_address + 7) = 0;
          }

          destination_address += 8;
          filtered_address += 2;
          filtered_count += 2;
        }
        else if (filter_type == 4)
        {
          unsigned long long dimension_count = 0;
          unsigned long long horizontal_low = 0;
          unsigned long long horizontal_high = 0;
          if ((filtered_count % ((width * 2) + 1)) >= 3)
          {
            horizontal_low = *(destination_address - 7);
            horizontal_high = *(destination_address - 8);
            dimension_count += 1;
          }

          unsigned long long vertical_low = 0;
          unsigned long long vertical_high = 0;
          if (filtered_count >= ((width * 2) + 1))
          {
            vertical_low = *(destination_address - (width * 8));
            vertical_high = *(destination_address - (width * 8) + 1);
            dimension_count += 1;
          }

          unsigned long long diagonal_low = 0;
          unsigned long long diagonal_high = 0;
          if (dimension_count == 2)
          {
            diagonal_low = *((destination_address - 8) - (width * 8));
            diagonal_high = *((destination_address - 7) - (width * 8));
          }

          unsigned long long low = portable_network_graphics_paeth(horizontal_low, vertical_low, diagonal_low);
          unsigned long long high = portable_network_graphics_paeth(horizontal_high, vertical_high, diagonal_high);

          high = (high + *filtered_address) & 255;
          low = (low + *(filtered_address + 1)) & 255;
          *destination_address = low;
          *(destination_address + 1) = high;
          *(destination_address + 2) = low;
          *(destination_address + 3) = high;
          *(destination_address + 4) = low;
          *(destination_address + 5) = high;
          *(destination_address + 6) = 255;
          *(destination_address + 7) = 255;
          if ((tRNS_address != 0) && (*(tRNS_address + 8) == high) && (*(tRNS_address + 9) == low))
          {
            *(destination_address + 6) = 0;
            *(destination_address + 7) = 0;
          }

          destination_address += 8;
          filtered_address += 2;
          filtered_count += 2;
        }
        else
        {
          memory_free(filtered_size);
          memory_free(compressed_size);
          return !0;
        }
      }
    }
    else if ((IHDR_address[16] == 8) && (IHDR_address[17] == 2) && (IHDR_address[18] == 0) && (IHDR_address[19] == 0) && (IHDR_address[20] == 0))
    {
      unsigned long long width = portable_network_graphics_read_int(IHDR_address + 8);
      unsigned long long filtered_size = (width * 3) + 1;
      filtered_size *= portable_network_graphics_read_int(IHDR_address + 12);
      if (filtered_size == 0)
      {
        memory_free(compressed_size);
        return !0;
      }

      unsigned char* filtered_address = memory_allocate(filtered_size);
      if (filtered_address == 0)
      {
        memory_free(compressed_size);
        return !0;
      }

      if (portable_network_graphics_inflate(compressed_address, filtered_address) != 0)
      {
        memory_free(filtered_size);
        memory_free(compressed_size);
        return !0;
      }

      unsigned long long filter_type;
      unsigned long long filtered_count = 0;
      while (filtered_count < filtered_size)
      {
        if ((filtered_count % ((width * 3) + 1)) == 0)
        {
          filter_type = *filtered_address;
          filtered_address += 1;
          filtered_count += 1;
        }
        else if (filter_type == 0)
        {
          unsigned char red = *filtered_address;
          unsigned char green = *(filtered_address + 1);
          unsigned char blue = *(filtered_address + 2);
          *destination_address = red;
          *(destination_address + 1) = red;
          *(destination_address + 2) = green;
          *(destination_address + 3) = green;
          *(destination_address + 4) = blue;
          *(destination_address + 5) = blue;
          *(destination_address + 6) = 255;
          *(destination_address + 7) = 255;
          if ((tRNS_address != 0) && (*(tRNS_address + 9) == red) && (*(tRNS_address + 11) == green) && (*(tRNS_address + 13) == blue))
          {
            *(destination_address + 6) = 0;
            *(destination_address + 7) = 0;
          }

          destination_address += 8;
          filtered_address += 3;
          filtered_count += 3;
        }
        else if (filter_type == 1)
        {
          unsigned long long red = *filtered_address;
          unsigned long long green = *(filtered_address + 1);
          unsigned long long blue = *(filtered_address + 2);
          if ((filtered_count % ((width * 3) + 1)) >= 4)
          {
            red += *(destination_address - 8);
            red &= 255;
            green += *(destination_address - 6);
            green &= 255;
            blue += *(destination_address - 4);
            blue &= 255;
          }

          *destination_address = red;
          *(destination_address + 1) = red;
          *(destination_address + 2) = green;
          *(destination_address + 3) = green;
          *(destination_address + 4) = blue;
          *(destination_address + 5) = blue;
          *(destination_address + 6) = 255;
          *(destination_address + 7) = 255;
          if ((tRNS_address != 0) && (*(tRNS_address + 9) == red) && (*(tRNS_address + 11) == green) && (*(tRNS_address + 13) == blue))
          {
            *(destination_address + 6) = 0;
            *(destination_address + 7) = 0;
          }

          destination_address += 8;
          filtered_address += 3;
          filtered_count += 3;
        }
        else if (filter_type == 2)
        {
          unsigned long long red = *filtered_address;
          unsigned long long green = *(filtered_address + 1);
          unsigned long long blue = *(filtered_address + 2);
          if (filtered_count >= ((width * 3) + 1))
          {
            red += *(destination_address - (width * 8));
            red &= 255;
            green += *((destination_address + 2) - (width * 8));
            green &= 255;
            blue += *((destination_address + 4) - (width * 8));
            blue &= 255;
          }

          *destination_address = red;
          *(destination_address + 1) = red;
          *(destination_address + 2) = green;
          *(destination_address + 3) = green;
          *(destination_address + 4) = blue;
          *(destination_address + 5) = blue;
          *(destination_address + 6) = 255;
          *(destination_address + 7) = 255;
          if ((tRNS_address != 0) && (*(tRNS_address + 9) == red) && (*(tRNS_address + 11) == green) && (*(tRNS_address + 13) == blue))
          {
            *(destination_address + 6) = 0;
            *(destination_address + 7) = 0;
          }

          destination_address += 8;
          filtered_address += 3;
          filtered_count += 3;
        }
        else if (filter_type == 3)
        {
          unsigned long long red = 0;
          unsigned long long green = 0;
          unsigned long long blue = 0;
          if ((filtered_count % ((width * 3) + 1)) >= 4)
          {
            red = *(destination_address - 8);
            green = *(destination_address - 6);
            blue = *(destination_address - 4);
          }

          if (filtered_count >= ((width * 3) + 1))
          {
            red += *(destination_address - (width * 8));
            red &= 255;
            green += *((destination_address + 2) - (width * 8));
            green &= 255;
            blue += *((destination_address + 4) - (width * 8));
            blue &= 255;
          }

          red = ((red / 2) + *filtered_address) & 255;
          green = ((green / 2) + *(filtered_address + 1)) & 255;
          blue = ((blue / 2) + *(filtered_address + 2)) & 255;
          *destination_address = red;
          *(destination_address + 1) = red;
          *(destination_address + 2) = green;
          *(destination_address + 3) = green;
          *(destination_address + 4) = blue;
          *(destination_address + 5) = blue;
          *(destination_address + 6) = 255;
          *(destination_address + 7) = 255;
          if ((tRNS_address != 0) && (*(tRNS_address + 9) == red) && (*(tRNS_address + 11) == green) && (*(tRNS_address + 13) == blue))
          {
            *(destination_address + 6) = 0;
            *(destination_address + 7) = 0;
          }

          destination_address += 8;
          filtered_address += 3;
          filtered_count += 3;
        }
        else if (filter_type == 4)
        {
          unsigned long long dimension_count = 0;
          unsigned long long red_horizontal = 0;
          unsigned long long green_horizontal = 0;
          unsigned long long blue_horizontal = 0;
          if ((filtered_count % ((width * 3) + 1)) >= 4)
          {
            red_horizontal = *(destination_address - 8);
            green_horizontal = *(destination_address - 6);
            blue_horizontal = *(destination_address - 4);
            dimension_count += 1;
          }

          unsigned long long red_vertical = 0;
          unsigned long long green_vertical = 0;
          unsigned long long blue_vertical = 0;
          if (filtered_count >= ((width * 3) + 1))
          {
            red_vertical = *(destination_address - (width * 8));
            green_vertical = *((destination_address + 2) - (width * 8));
            blue_vertical = *((destination_address + 4) - (width * 8));
            dimension_count += 1;
          }

          unsigned long long red_diagonal = 0;
          unsigned long long green_diagonal = 0;
          unsigned long long blue_diagonal = 0;
          if (dimension_count == 2)
          {
            red_diagonal = *((destination_address - 8) - (width * 8));
            green_diagonal = *((destination_address - 6) - (width * 8));
            blue_diagonal = *((destination_address - 4) - (width * 8));
          }

          unsigned long long red = portable_network_graphics_paeth(red_horizontal, red_vertical, red_diagonal);
          unsigned long long green = portable_network_graphics_paeth(green_horizontal, green_vertical, green_diagonal);
          unsigned long long blue = portable_network_graphics_paeth(blue_horizontal, blue_vertical, blue_diagonal);

          red = (red + *filtered_address) & 255;
          green = (green + *(filtered_address + 1)) & 255;
          blue = (blue + *(filtered_address + 2)) & 255;
          *destination_address = red;
          *(destination_address + 1) = red;
          *(destination_address + 2) = green;
          *(destination_address + 3) = green;
          *(destination_address + 4) = blue;
          *(destination_address + 5) = blue;
          *(destination_address + 6) = 255;
          *(destination_address + 7) = 255;
          if ((tRNS_address != 0) && (*(tRNS_address + 9) == red) && (*(tRNS_address + 11) == green) && (*(tRNS_address + 13) == blue))
          {
            *(destination_address + 6) = 0;
            *(destination_address + 7) = 0;
          }

          destination_address += 8;
          filtered_address += 3;
          filtered_count += 3;
        }
        else
        {
          memory_free(filtered_size);
          memory_free(compressed_size);
          return !0;
        }
      }
      memory_free(filtered_size);
      memory_free(compressed_size);
      return 0;
    }
    else if ((IHDR_address[16] == 16) && (IHDR_address[17] == 2) && (IHDR_address[18] == 0) && (IHDR_address[19] == 0) && (IHDR_address[20] == 0))
    {
      unsigned long long width = portable_network_graphics_read_int(IHDR_address + 8);
      unsigned long long filtered_size = (width * 6) + 1;
      filtered_size *= portable_network_graphics_read_int(IHDR_address + 12);
      if (filtered_size == 0)
      {
        memory_free(compressed_size);
        return !0;
      }

      unsigned char* filtered_address = memory_allocate(filtered_size);
      if (filtered_address == 0)
      {
        memory_free(compressed_size);
        return !0;
      }

      if (portable_network_graphics_inflate(compressed_address, filtered_address) != 0)
      {
        memory_free(filtered_size);
        memory_free(compressed_size);
        return !0;
      }

      unsigned long long filter_type;
      unsigned long long filtered_count = 0;
      while (filtered_count < filtered_size)
      {
        if ((filtered_count % ((width * 6) + 1)) == 0)
        {
          filter_type = *filtered_address;
          filtered_address += 1;
          filtered_count += 1;
        }
        else if (filter_type == 0)
        {
          *destination_address = *(filtered_address + 1);
          *(destination_address + 1) = *filtered_address;
          *(destination_address + 2) = *(filtered_address + 3);
          *(destination_address + 3) = *(filtered_address + 2);
          *(destination_address + 4) = *(filtered_address + 5);
          *(destination_address + 5) = *(filtered_address + 4);
          *(destination_address + 6) = 255;
          *(destination_address + 7) = 255;
          if ((tRNS_address != 0)
            && (*(tRNS_address + 8) == *filtered_address)
            && (*(tRNS_address + 9) == *(filtered_address + 1))
            && (*(tRNS_address + 10) == *(filtered_address + 2))
            && (*(tRNS_address + 11) == *(filtered_address + 3))
            && (*(tRNS_address + 12) == *(filtered_address + 4))
            && (*(tRNS_address + 13) == *(filtered_address + 5)))
          {
            *(destination_address + 6) = 0;
            *(destination_address + 7) = 0;
          }

          destination_address += 8;
          filtered_address += 6;
          filtered_count += 6;
        }
        else if (filter_type == 1)
        {
          unsigned long long red_high = *filtered_address;
          unsigned long long red_low = *(filtered_address + 1);
          unsigned long long green_high = *(filtered_address + 2);
          unsigned long long green_low = *(filtered_address + 3);
          unsigned long long blue_high = *(filtered_address + 4);
          unsigned long long blue_low = *(filtered_address + 5);
          if ((filtered_count % ((width * 6) + 1)) >= 7)
          {
            red_low += *(destination_address - 8);
            red_low &= 255;
            red_high += *(destination_address - 7);
            red_high &= 255;
            green_low += *(destination_address - 6);
            green_low &= 255;
            green_high += *(destination_address - 5);
            green_high &= 255;
            blue_low += *(destination_address - 4);
            blue_low &= 255;
            blue_high += *(destination_address - 3);
            blue_high &= 255;
          }

          *destination_address = red_low;
          *(destination_address + 1) = red_high;
          *(destination_address + 2) = green_low;
          *(destination_address + 3) = green_high;
          *(destination_address + 4) = blue_low;
          *(destination_address + 5) = blue_high;
          *(destination_address + 6) = 255;
          *(destination_address + 7) = 255;
          if ((tRNS_address != 0)
            && (*(tRNS_address + 8) == red_high)
            && (*(tRNS_address + 9) == red_low)
            && (*(tRNS_address + 10) == green_high)
            && (*(tRNS_address + 11) == green_low)
            && (*(tRNS_address + 12) == blue_high)
            && (*(tRNS_address + 13) == blue_low))
          {
            *(destination_address + 6) = 0;
            *(destination_address + 7) = 0;
          }

          destination_address += 8;
          filtered_address += 6;
          filtered_count += 6;
        }
        else if (filter_type == 2)
        {
          unsigned long long red_high = *filtered_address;
          unsigned long long red_low = *(filtered_address + 1);
          unsigned long long green_high = *(filtered_address + 2);
          unsigned long long green_low = *(filtered_address + 3);
          unsigned long long blue_high = *(filtered_address + 4);
          unsigned long long blue_low = *(filtered_address + 5);
          if (filtered_count >= ((width * 6) + 1))
          {
            red_low += *(destination_address - (width * 8));
            red_low &= 255;
            red_high += *((destination_address + 1) - (width * 8));
            red_high &= 255;
            green_low += *((destination_address + 2) - (width * 8));
            green_low &= 255;
            green_high += *((destination_address + 3) - (width * 8));
            green_high &= 255;
            blue_low += *((destination_address + 4) - (width * 8));
            blue_low &= 255;
            blue_high += *((destination_address + 5) - (width * 8));
            blue_high &= 255;
          }

          *destination_address = red_low;
          *(destination_address + 1) = red_high;
          *(destination_address + 2) = green_low;
          *(destination_address + 3) = green_high;
          *(destination_address + 4) = blue_low;
          *(destination_address + 5) = blue_high;
          *(destination_address + 6) = 255;
          *(destination_address + 7) = 255;
          if ((tRNS_address != 0)
            && (*(tRNS_address + 8) == red_high)
            && (*(tRNS_address + 9) == red_low)
            && (*(tRNS_address + 10) == green_high)
            && (*(tRNS_address + 11) == green_low)
            && (*(tRNS_address + 12) == blue_high)
            && (*(tRNS_address + 13) == blue_low))
          {
            *(destination_address + 6) = 0;
            *(destination_address + 7) = 0;
          }

          destination_address += 8;
          filtered_address += 6;
          filtered_count += 6;
        }
        else if (filter_type == 3)
        {
          unsigned long long red_low = 0;
          unsigned long long red_high = 0;
          unsigned long long green_low = 0;
          unsigned long long green_high = 0;
          unsigned long long blue_low = 0;
          unsigned long long blue_high = 0;
          if ((filtered_count % ((width * 6) + 1)) >= 7)
          {
            red_low = *(destination_address - 8);
            red_high = *(destination_address - 7);
            green_low = *(destination_address - 6);
            green_high = *(destination_address - 5);
            blue_low = *(destination_address - 4);
            blue_high = *(destination_address - 3);
          }

          if (filtered_count >= ((width * 6) + 1))
          {
            red_low += *(destination_address - (width * 8));
            red_low &= 255;
            red_high += *((destination_address + 1) - (width * 8));
            red_high &= 255;
            green_low += *((destination_address + 2) - (width * 8));
            green_low &= 255;
            green_high += *((destination_address + 3) - (width * 8));
            green_high &= 255;
            blue_low += *((destination_address + 4) - (width * 8));
            blue_low &= 255;
            blue_high += *((destination_address + 5) - (width * 8));
            blue_high &= 255;
          }

          red_high = ((red_high / 2) + *filtered_address) & 255;
          red_low = ((red_low / 2) + *(filtered_address + 1)) & 255;
          green_high = ((green_high / 2) + *(filtered_address + 2)) & 255;
          green_low = ((green_low / 2) + *(filtered_address + 3)) & 255;
          blue_high = ((blue_high / 2) + *(filtered_address + 4)) & 255;
          blue_low = ((blue_low / 2) + *(filtered_address + 5)) & 255;

          *destination_address = red_low;
          *(destination_address + 1) = red_high;
          *(destination_address + 2) = green_low;
          *(destination_address + 3) = green_high;
          *(destination_address + 4) = blue_low;
          *(destination_address + 5) = blue_high;
          *(destination_address + 6) = 255;
          *(destination_address + 7) = 255;
          if ((tRNS_address != 0)
            && (*(tRNS_address + 8) == red_high)
            && (*(tRNS_address + 9) == red_low)
            && (*(tRNS_address + 10) == green_high)
            && (*(tRNS_address + 11) == green_low)
            && (*(tRNS_address + 12) == blue_high)
            && (*(tRNS_address + 13) == blue_low))
          {
            *(destination_address + 6) = 0;
            *(destination_address + 7) = 0;
          }

          destination_address += 8;
          filtered_address += 6;
          filtered_count += 6;
        }
        else if (filter_type == 4)
        {
          unsigned long long dimension_count = 0;
          unsigned long long red_horizontal_low = 0;
          unsigned long long red_horizontal_high = 0;
          unsigned long long green_horizontal_low = 0;
          unsigned long long green_horizontal_high = 0;
          unsigned long long blue_horizontal_low = 0;
          unsigned long long blue_horizontal_high = 0;
          if ((filtered_count % ((width * 6) + 1)) >= 7)
          {
            red_horizontal_low = *(destination_address - 8);
            red_horizontal_high = *(destination_address - 7);
            green_horizontal_low = *(destination_address - 6);
            green_horizontal_high = *(destination_address - 5);
            blue_horizontal_low = *(destination_address - 4);
            blue_horizontal_high = *(destination_address - 3);
            dimension_count += 1;
          }

          unsigned long long red_vertical_low = 0;
          unsigned long long red_vertical_high = 0;
          unsigned long long green_vertical_low = 0;
          unsigned long long green_vertical_high = 0;
          unsigned long long blue_vertical_low = 0;
          unsigned long long blue_vertical_high = 0;
          if (filtered_count >= ((width * 6) + 1))
          {
            red_vertical_low = *(destination_address - (width * 8));
            red_vertical_high = *((destination_address + 1) - (width * 8));
            green_vertical_low = *((destination_address + 2) - (width * 8));
            green_vertical_high = *((destination_address + 3) - (width * 8));
            blue_vertical_low = *((destination_address + 4) - (width * 8));
            blue_vertical_high = *((destination_address + 5) - (width * 8));
            dimension_count += 1;
          }

          unsigned long long red_diagonal_low = 0;
          unsigned long long red_diagonal_high = 0;
          unsigned long long green_diagonal_low = 0;
          unsigned long long green_diagonal_high = 0;
          unsigned long long blue_diagonal_low = 0;
          unsigned long long blue_diagonal_high = 0;
          if (dimension_count == 2)
          {
            red_diagonal_low = *((destination_address - 8) - (width * 8));
            red_diagonal_high = *((destination_address - 7) - (width * 8));
            green_diagonal_low = *((destination_address - 6) - (width * 8));
            green_diagonal_high = *((destination_address - 5) - (width * 8));
            blue_diagonal_low = *((destination_address - 4) - (width * 8));
            blue_diagonal_high = *((destination_address - 3) - (width * 8));
          }

          unsigned long long red_low = portable_network_graphics_paeth(red_horizontal_low, red_vertical_low, red_diagonal_low);
          unsigned long long red_high = portable_network_graphics_paeth(red_horizontal_high, red_vertical_high, red_diagonal_high);
          unsigned long long green_low = portable_network_graphics_paeth(green_horizontal_low, green_vertical_low, green_diagonal_low);
          unsigned long long green_high = portable_network_graphics_paeth(green_horizontal_high, green_vertical_high, green_diagonal_high);
          unsigned long long blue_low = portable_network_graphics_paeth(blue_horizontal_low, blue_vertical_low, blue_diagonal_low);
          unsigned long long blue_high = portable_network_graphics_paeth(blue_horizontal_high, blue_vertical_high, blue_diagonal_high);

          red_high = (red_high + *filtered_address) & 255;
          red_low = (red_low + *(filtered_address + 1)) & 255;
          green_high = (green_high + *(filtered_address + 2)) & 255;
          green_low = (green_low + *(filtered_address + 3)) & 255;
          blue_high = (blue_high + *(filtered_address + 4)) & 255;
          blue_low = (blue_low + *(filtered_address + 5)) & 255;

          *destination_address = red_low;
          *(destination_address + 1) = red_high;
          *(destination_address + 2) = green_low;
          *(destination_address + 3) = green_high;
          *(destination_address + 4) = blue_low;
          *(destination_address + 5) = blue_high;
          *(destination_address + 6) = 255;
          *(destination_address + 7) = 255;
          if ((tRNS_address != 0)
            && (*(tRNS_address + 8) == red_high)
            && (*(tRNS_address + 9) == red_low)
            && (*(tRNS_address + 10) == green_high)
            && (*(tRNS_address + 11) == green_low)
            && (*(tRNS_address + 12) == blue_high)
            && (*(tRNS_address + 13) == blue_low))
          {
            *(destination_address + 6) = 0;
            *(destination_address + 7) = 0;
          }

          destination_address += 8;
          filtered_address += 6;
          filtered_count += 6;
        }
        else
        {
          memory_free(filtered_size);
          memory_free(compressed_size);
          return !0;
        }
      }
      memory_free(filtered_size);
      memory_free(compressed_size);
      return 0;
    }
    else if ((IHDR_address[16] == 8) && (IHDR_address[17] == 3) && (IHDR_address[18] == 0) && (IHDR_address[19] == 0) && (IHDR_address[20] == 0))
    {
      // TO DO
    }
    else if ((IHDR_address[16] == 8) && (IHDR_address[17] == 4) && (IHDR_address[18] == 0) && (IHDR_address[19] == 0) && (IHDR_address[20] == 0))
    {
      // TO DO
    }
    else if ((IHDR_address[16] == 16) && (IHDR_address[17] == 4) && (IHDR_address[18] == 0) && (IHDR_address[19] == 0) && (IHDR_address[20] == 0))
    {
      // TO DO
    }
    else if ((IHDR_address[16] == 8) && (IHDR_address[17] == 6) && (IHDR_address[18] == 0) && (IHDR_address[19] == 0) && (IHDR_address[20] == 0))
    {
      unsigned long long width = portable_network_graphics_read_int(IHDR_address + 8);
      unsigned long long filtered_size = (width * 4) + 1;
      filtered_size *= portable_network_graphics_read_int(IHDR_address + 12);
      if (filtered_size == 0)
      {
        memory_free(compressed_size);
        return !0;
      }

      unsigned char* filtered_address = memory_allocate(filtered_size);
      if (filtered_address == 0)
      {
        memory_free(compressed_size);
        return !0;
      }

      if (portable_network_graphics_inflate(compressed_address, filtered_address) != 0)
      {
        memory_free(filtered_size);
        memory_free(compressed_size);
        return !0;
      }

      unsigned long long filter_type;
      unsigned long long filtered_count = 0;
      while (filtered_count < filtered_size)
      {
        if ((filtered_count % ((width * 4) + 1)) == 0)
        {
          filter_type = *filtered_address;
          filtered_address += 1;
          filtered_count += 1;
        }
        else if (filter_type == 0)
        {
          *destination_address = *filtered_address;
          *(destination_address + 1) = *filtered_address;
          destination_address += 2;
          filtered_address += 1;
          filtered_count += 1;
        }
        else if (filter_type == 1)
        {
          unsigned long long sample = *filtered_address;
          if ((filtered_count % ((width * 4) + 1)) >= 5)
          { sample += *(destination_address - 8); }

          *destination_address = sample;
          *(destination_address + 1) = sample;
          destination_address += 2;
          filtered_address += 1;
          filtered_count += 1;
        }
        else if (filter_type == 2)
        {
          unsigned long long sample = *filtered_address;
          if (filtered_count >= ((width * 4) + 1))
          { sample += *(destination_address - (width * 8)); }

          *destination_address = sample;
          *(destination_address + 1) = sample;
          destination_address += 2;
          filtered_address += 1;
          filtered_count += 1;
        }
        else if (filter_type == 3)
        {
          unsigned long long sample = 0;
          if ((filtered_count % ((width * 4) + 1)) >= 5)
          { sample = *(destination_address - 8); }

          if (filtered_count >= ((width * 4) + 1))
          {
            sample += *(destination_address - (width * 8));
            sample &= 255;
          }

          sample = (sample / 2) + *filtered_address;
          *destination_address = sample;
          *(destination_address + 1) = sample;
          destination_address += 2;
          filtered_address += 1;
          filtered_count += 1;
        }
        else if (filter_type == 4)
        {
          unsigned long long dimension_count = 0;
          unsigned long long horizontal = 0;
          if ((filtered_count % ((width * 4) + 1)) >= 5)
          {
            horizontal = *(destination_address - 8);
            dimension_count += 1;
          }

          unsigned long long vertical = 0;
          if (filtered_count >= ((width * 4) + 1))
          {
            vertical = *(destination_address - (width * 8));
            dimension_count += 1;
          }

          unsigned long long diagonal = 0;
          if (dimension_count == 2)
          { diagonal = *((destination_address - 8) - (width * 8)); }

          unsigned long long sample = portable_network_graphics_paeth(horizontal, vertical, diagonal);
          *destination_address = sample + *filtered_address;
          *(destination_address + 1) = sample + *filtered_address;
          destination_address += 2;
          filtered_address += 1;
          filtered_count += 1;
        }
        else
        {
          memory_free(filtered_size);
          memory_free(compressed_size);
          return !0;
        }
      }
      memory_free(filtered_size);
      memory_free(compressed_size);
      return 0;
    }
    else if ((IHDR_address[16] == 16) && (IHDR_address[17] == 6) && (IHDR_address[18] == 0) && (IHDR_address[19] == 0) && (IHDR_address[20] == 0))
    {
      unsigned long long width = portable_network_graphics_read_int(IHDR_address + 8);
      unsigned long long filtered_size = (width * 8) + 1;
      filtered_size *= portable_network_graphics_read_int(IHDR_address + 12);
      if (filtered_size == 0)
      {
        memory_free(compressed_size);
        return !0;
      }

      unsigned char* filtered_address = memory_allocate(filtered_size);
      if (filtered_address == 0)
      {
        memory_free(compressed_size);
        return !0;
      }

      if (portable_network_graphics_inflate(compressed_address, filtered_address) != 0)
      {
        memory_free(filtered_size);
        memory_free(compressed_size);
        return !0;
      }

      unsigned long long filter_type;
      unsigned long long filtered_count = 0;
      while (filtered_count < filtered_size)
      {
        if ((filtered_count % ((width * 8) + 1)) == 0)
        {
          filter_type = *filtered_address;
          filtered_address += 1;
          filtered_count += 1;
        }
        else if (filter_type == 0)
        {
          *destination_address = *(filtered_address + 1);
          *(destination_address + 1) = *filtered_address;
          destination_address += 2;
          filtered_address += 2;
          filtered_count += 2;
        }
        else if (filter_type == 1)
        {
          unsigned long long high = *filtered_address;
          unsigned long long low = *(filtered_address + 1);
          if ((filtered_count % ((width * 8) + 1)) >= 9)
          {
            low += *(destination_address - 8);
            high += *(destination_address - 7);
          }

          *destination_address = low;
          *(destination_address + 1) = high;
          destination_address += 2;
          filtered_address += 2;
          filtered_count += 2;
        }
        else if (filter_type == 2)
        {
          unsigned long long high = *filtered_address;
          unsigned long long low = *(filtered_address + 1);
          if (filtered_count >= ((width * 8) + 1))
          {
            low += *(destination_address - (width * 8));
            high += *((destination_address + 1) - (width * 8));
          }

          *destination_address = low;
          *(destination_address + 1) = high;
          destination_address += 2;
          filtered_address += 2;
          filtered_count += 2;
        }
        else if (filter_type == 3)
        {
          unsigned long long low = 0;
          unsigned long long high = 0;
          if ((filtered_count % ((width * 8) + 1)) >= 9)
          {
            low = *(destination_address - 8);
            high = *(destination_address - 7);
          }

          if (filtered_count >= ((width * 8) + 1))
          {
            low += *(destination_address - (width * 8));
            low &= 255;
            high += *((destination_address + 1) - (width * 8));
            high &= 255;
          }

          *destination_address = (low / 2) + *(filtered_address + 1);
          *(destination_address + 1) = (high / 2) + *filtered_address;
          destination_address += 2;
          filtered_address += 2;
          filtered_count += 2;
        }
        else if (filter_type == 4)
        {
          unsigned long long dimension_count = 0;
          unsigned long long horizontal_low = 0;
          unsigned long long horizontal_high = 0;
          if ((filtered_count % ((width * 8) + 1)) >= 9)
          {
            horizontal_low = *(destination_address - 8);
            horizontal_high = *(destination_address - 7);
            dimension_count += 1;
          }

          unsigned long long vertical_low = 0;
          unsigned long long vertical_high = 0;
          if (filtered_count >= ((width * 8) + 1))
          {
            vertical_low = *(destination_address - (width * 8));
            vertical_high = *((destination_address + 1) - (width * 8));
            dimension_count += 1;
          }

          unsigned long long diagonal_low = 0;
          unsigned long long diagonal_high = 0;
          if (dimension_count == 2)
          {
            diagonal_low = *((destination_address - 8) - (width * 8));
            diagonal_high = *((destination_address - 7) - (width * 8));
          }

          unsigned long long low = portable_network_graphics_paeth(horizontal_low, vertical_low, diagonal_low);
          unsigned long long high = portable_network_graphics_paeth(horizontal_high, vertical_high, diagonal_high);
          *destination_address = low + *(filtered_address + 1);
          *(destination_address + 1) = high + *filtered_address;
          destination_address += 2;
          filtered_address += 2;
          filtered_count += 2;
        }
        else
        {
          memory_free(filtered_size);
          memory_free(compressed_size);
          return !0;
        }
      }
      memory_free(filtered_size);
      memory_free(compressed_size);
      return 0;
    }
    else
    { memory_free(compressed_size); }
  }
  return !0;
}
#endif