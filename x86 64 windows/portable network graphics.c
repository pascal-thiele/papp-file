#include "memory.c"
#include "console.c"

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
  unsigned long long value = address[0];
  value = (value << 8) | address[1];
  value = (value << 8) | address[2];
  value = (value << 8) | address[3];
  return value;
}


// Orders sixteen bits, the less significant a bit is, the more significant a bit will be.
unsigned long long portable_network_graphics_reverse_bit_order(unsigned long long value)
{
  value &= 65535;
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
        // Read uncompressed bytes.
        unsigned long long block_length = *source_address | (*(source_address + 1) << 8);
        unsigned long long block_length_complement = *(source_address + 2) | (*(source_address + 3) << 8);
        block_length_complement ^= block_length;
        if (block_length_complement != 0)
        { break; }
        source_address += 4;
        while (block_length != 0)
        {
          *destination_address = *source_address;
          destination_address += 1;
          destination_size += 1;
          source_address += 1;
          block_length -= 1;
        }
      }
      else if (block_type == 1)
      {
        // Read fixed huffman codes.
        while (1)
        {
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

          // Translate literals or lengths with nine bit codes.
          unsigned long long symbol_code = portable_network_graphics_reverse_bit_order(remainder << 7);
          if (symbol_code >= 400)
          {
            *destination_address = symbol_code - 256;
            destination_address += 1;
            destination_size += 1;
            remainder >>= 9;
            remainder_size -= 9;
            continue;
          }

          // Translate literals or lengths with eight bit codes.
          symbol_code >>= 1;
          unsigned long long symbol_length;
          if ((symbol_code >= 48) && (symbol_code < 192))
          {
            *destination_address = (symbol_code >> 1) - 48;
            destination_address += 1;
            destination_size += 1;
            remainder >>= 8;
            remainder_size -= 8;
            continue;
          }
          else if (symbol_code == 192)
          {
            remainder >>= 8;
            symbol_length = 115 + portable_network_graphics_reverse_bit_order(remainder << 12);
            remainder >>= 4;
            remainder_size -= 12;
          }
          else if (symbol_code == 193)
          {
            remainder >>= 8;
            symbol_length = 131 + portable_network_graphics_reverse_bit_order(remainder << 11);
            remainder >>= 5;
            remainder_size -= 13;
          }
          else if (symbol_code == 194)
          {
            remainder >>= 8;
            symbol_length = 163 + portable_network_graphics_reverse_bit_order(remainder << 11);
            remainder >>= 5;
            remainder_size -= 13;
          }
          else if (symbol_code == 195)
          {
            remainder >>= 8;
            symbol_length = 195 + portable_network_graphics_reverse_bit_order(remainder << 11);
            remainder >>= 5;
            remainder_size -= 13;
          }
          else if (symbol_code == 196)
          {
            remainder >>= 8;
            symbol_length = 227 + portable_network_graphics_reverse_bit_order(remainder << 11);
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
          {
            // Translate literals or lengths with seven bit codes.
            symbol_code >>= 1;
            if (symbol_code == 0)
            { break; }
            else if (symbol_code < 9)
            {
              symbol_length = symbol_code + 2;
              remainder >>= 7;
              remainder_size -= 7;
            }
            else if (symbol_code == 9)
            {
              remainder >>= 7;
              symbol_length = 11 + (remainder & 1);
              remainder >>= 1;
              remainder_size -= 8;
            }
            else if (symbol_code == 10)
            {
              remainder >>= 7;
              symbol_length = 13 + (remainder & 1);
              remainder >>= 1;
              remainder_size -= 8;
            }
            else if (symbol_code == 11)
            {
              remainder >>= 7;
              symbol_length = 15 + (remainder & 1);
              remainder >>= 1;
              remainder_size -= 8;
            }
            else if (symbol_code == 12)
            {
              remainder >>= 7;
              symbol_length = 17 + (remainder & 1);
              remainder >>= 1;
              remainder_size -= 8;
            }
            else if (symbol_code == 13)
            {
              remainder >>= 7;
              symbol_length = 19 + portable_network_graphics_reverse_bit_order(remainder << 14);
              remainder >>= 2;
              remainder_size -= 9;
            }
            else if (symbol_code == 14)
            {
              remainder >>= 7;
              symbol_length = 23 + portable_network_graphics_reverse_bit_order(remainder << 14);
              remainder >>= 2;
              remainder_size -= 9;
            }
            else if (symbol_code == 15)
            {
              remainder >>= 7;
              symbol_length = 27 + portable_network_graphics_reverse_bit_order(remainder << 14);
              remainder >>= 2;
              remainder_size -= 9;
            }
            else if (symbol_code == 16)
            {
              remainder >>= 7;
              symbol_length = 31 + portable_network_graphics_reverse_bit_order(remainder << 14);
              remainder >>= 2;
              remainder_size -= 9;
            }
            else if (symbol_code == 17)
            {
              remainder >>= 7;
              symbol_length = 35 + portable_network_graphics_reverse_bit_order(remainder << 13);
              remainder >>= 3;
              remainder_size -= 10;
            }
            else if (symbol_code == 18)
            {
              remainder >>= 7;
              symbol_length = 43 + portable_network_graphics_reverse_bit_order(remainder << 13);
              remainder >>= 3;
              remainder_size -= 10;
            }
            else if (symbol_code == 19)
            {
              remainder >>= 7;
              symbol_length = 51 + portable_network_graphics_reverse_bit_order(remainder << 13);
              remainder >>= 3;
              remainder_size -= 10;
            }
            else if (symbol_code == 20)
            {
              remainder >>= 7;
              symbol_length = 59 + portable_network_graphics_reverse_bit_order(remainder << 13);
              remainder >>= 3;
              remainder_size -= 10;
            }
            else if (symbol_code == 21)
            {
              remainder >>= 7;
              symbol_length = 67 + portable_network_graphics_reverse_bit_order(remainder << 12);
              remainder >>= 4;
              remainder_size -= 11;
            }
            else if (symbol_code == 22)
            {
              remainder >>= 7;
              symbol_length = 83 + portable_network_graphics_reverse_bit_order(remainder << 12);
              remainder >>= 4;
              remainder_size -= 11;
            }
            else if (symbol_code == 23)
            {
              remainder >>= 7;
              symbol_length = 99 + portable_network_graphics_reverse_bit_order(remainder << 12);
              remainder >>= 4;
              remainder_size -= 11;
            }
            else
            { return !0; }
          }

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
          symbol_code = portable_network_graphics_reverse_bit_order(remainder << 11);
          unsigned long long symbol_distance;
          if (symbol_code < 4)
          {
            symbol_distance = symbol_code + 1;
            remainder >>= 5;
            remainder_size -= 5;
          }
          else if (symbol_code == 4)
          {
            remainder >>= 5;
            symbol_distance = 5 + (remainder & 1);
            remainder >>= 1;
            remainder_size -= 6;
          }
          else if (symbol_code == 5)
          {
            remainder >>= 5;
            symbol_distance = 7 + (remainder & 1);
            remainder >>= 1;
            remainder_size -= 6;
          }
          else if (symbol_code == 6)
          {
            remainder >>= 5;
            symbol_distance = 9 + portable_network_graphics_reverse_bit_order(remainder << 14);
            remainder >>= 2;
            remainder_size -= 7;
          }
          else if (symbol_code == 7)
          {
            remainder >>= 5;
            symbol_distance = 13 + portable_network_graphics_reverse_bit_order(remainder << 14);
            remainder >>= 2;
            remainder_size -= 7;
          }
          else if (symbol_code == 8)
          {
            remainder >>= 5;
            symbol_distance = 13 + portable_network_graphics_reverse_bit_order(remainder << 13);
            remainder >>= 3;
            remainder_size -= 8;
          }
          else if (symbol_code == 9)
          {
            remainder >>= 5;
            symbol_distance = 25 + portable_network_graphics_reverse_bit_order(remainder << 13);
            remainder >>= 3;
            remainder_size -= 9;
          }
          else if (symbol_code == 10)
          {
            remainder >>= 5;
            symbol_distance = 33 + portable_network_graphics_reverse_bit_order(remainder << 12);
            remainder >>= 4;
            remainder_size -= 9;
          }
          else if (symbol_code == 11)
          {
            remainder >>= 5;
            symbol_distance = 49 + portable_network_graphics_reverse_bit_order(remainder << 12);
            remainder >>= 4;
            remainder_size -= 9;
          }
          else if (symbol_code == 12)
          {
            remainder >>= 5;
            symbol_distance = 65 + portable_network_graphics_reverse_bit_order(remainder << 11);
            remainder >>= 5;
            remainder_size -= 10;
          }
          else if (symbol_code == 13)
          {
            remainder >>= 5;
            symbol_distance = 97 + portable_network_graphics_reverse_bit_order(remainder << 11);
            remainder >>= 5;
            remainder_size -= 10;
          }
          else if (symbol_code == 14)
          {
            remainder >>= 5;
            symbol_distance = 129 + portable_network_graphics_reverse_bit_order(remainder << 10);
            remainder >>= 6;
            remainder_size -= 11;
          }
          else if (symbol_code == 15)
          {
            remainder >>= 5;
            symbol_distance = 193 + portable_network_graphics_reverse_bit_order(remainder << 10);
            remainder >>= 6;
            remainder_size -= 11;
          }
          else if (symbol_code == 16)
          {
            remainder >>= 5;
            symbol_distance = 257 + portable_network_graphics_reverse_bit_order(remainder << 9);
            remainder >>= 7;
            remainder_size -= 12;
          }
          else if (symbol_code == 17)
          {
            remainder >>= 5;
            symbol_distance = 385 + portable_network_graphics_reverse_bit_order(remainder << 9);
            remainder >>= 7;
            remainder_size -= 12;
          }
          else if (symbol_code == 18)
          {
            remainder >>= 5;
            symbol_distance = 513 + portable_network_graphics_reverse_bit_order(remainder << 8);
            remainder >>= 8;
            remainder_size -= 13;
          }
          else if (symbol_code == 19)
          {
            remainder >>= 5;
            symbol_distance = 769 + portable_network_graphics_reverse_bit_order(remainder << 8);
            remainder >>= 8;
            remainder_size -= 13;
          }
          else if (symbol_code == 20)
          {
            remainder >>= 5;
            symbol_distance = 1025 + portable_network_graphics_reverse_bit_order(remainder << 7);
            remainder >>= 9;
            remainder_size -= 14;
          }
          else if (symbol_code == 21)
          {
            remainder >>= 5;
            symbol_distance = 1537 + portable_network_graphics_reverse_bit_order(remainder << 7);
            remainder >>= 9;
            remainder_size -= 14;
          }
          else if (symbol_code == 22)
          {
            remainder >>= 5;
            symbol_distance = 2049 + portable_network_graphics_reverse_bit_order(remainder << 6);
            remainder >>= 10;
            remainder_size -= 15;
          }
          else if (symbol_code == 23)
          {
            remainder >>= 5;
            symbol_distance = 3073 + portable_network_graphics_reverse_bit_order(remainder << 6);
            remainder >>= 10;
            remainder_size -= 15;
          }
          else if (symbol_code == 24)
          {
            remainder >>= 5;
            symbol_distance = 4097 + portable_network_graphics_reverse_bit_order(remainder << 5);
            remainder >>= 11;
            remainder_size -= 16;
          }
          else if (symbol_code == 25)
          {
            remainder >>= 5;
            symbol_distance = 6145 + portable_network_graphics_reverse_bit_order(remainder << 5);
            remainder >>= 11;
            remainder_size -= 16;
          }
          else if (symbol_code == 26)
          {
            remainder >>= 5;
            symbol_distance = 8193 + portable_network_graphics_reverse_bit_order(remainder << 4);
            remainder >>= 12;
            remainder_size -= 17;
          }
          else if (symbol_code == 27)
          {
            remainder >>= 5;
            symbol_distance = 12289 + portable_network_graphics_reverse_bit_order(remainder << 4);
            remainder >>= 12;
            remainder_size -= 17;
          }
          else if (symbol_code == 28)
          {
            remainder >>= 5;
            symbol_distance = 16385 + portable_network_graphics_reverse_bit_order(remainder << 3);
            remainder >>= 13;
            remainder_size -= 18;
          }
          else if (symbol_code == 29)
          {
            remainder >>= 5;
            symbol_distance = 24577 + portable_network_graphics_reverse_bit_order(remainder << 3);
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
            symbol_length = 11 + (remainder & 1);
            remainder >>= 1;
            remainder_size -= 1;
          }
          else if (literal_list_index == 266)
          {
            symbol_length = 13 + (remainder & 1);
            remainder >>= 1;
            remainder_size -= 1;
          }
          else if (literal_list_index == 267)
          {
            symbol_length = 15 + (remainder & 1);
            remainder >>= 1;
            remainder_size -= 1;
          }
          else if (literal_list_index == 268)
          {
            symbol_length = 17 + (remainder & 1);
            remainder >>= 1;
            remainder_size -= 1;
          }
          else if (literal_list_index == 269)
          {
            symbol_length = 19 + portable_network_graphics_reverse_bit_order(remainder << 14);
            remainder >>= 2;
            remainder_size -= 2;
          }
          else if (literal_list_index == 270)
          {
            symbol_length = 23 + portable_network_graphics_reverse_bit_order(remainder << 14);
            remainder >>= 2;
            remainder_size -= 2;
          }
          else if (literal_list_index == 271)
          {
            symbol_length = 27 + portable_network_graphics_reverse_bit_order(remainder << 14);
            remainder >>= 2;
            remainder_size -= 2;
          }
          else if (literal_list_index == 272)
          {
            symbol_length = 31 + portable_network_graphics_reverse_bit_order(remainder << 14);
            remainder >>= 2;
            remainder_size -= 2;
          }
          else if (literal_list_index == 273)
          {
            symbol_length = 35 + portable_network_graphics_reverse_bit_order(remainder << 13);
            remainder >>= 3;
            remainder_size -= 3;
          }
          else if (literal_list_index == 274)
          {
            symbol_length = 43 + portable_network_graphics_reverse_bit_order(remainder << 13);
            remainder >>= 3;
            remainder_size -= 3;
          }
          else if (literal_list_index == 275)
          {
            symbol_length = 51 + portable_network_graphics_reverse_bit_order(remainder << 13);
            remainder >>= 3;
            remainder_size -= 3;
          }
          else if (literal_list_index == 276)
          {
            symbol_length = 59 + portable_network_graphics_reverse_bit_order(remainder << 13);
            remainder >>= 3;
            remainder_size -= 3;
          }
          else if (literal_list_index == 277)
          {
            symbol_length = 67 + portable_network_graphics_reverse_bit_order(remainder << 12);
            remainder >>= 4;
            remainder_size -= 4;
          }
          else if (literal_list_index == 278)
          {
            symbol_length = 83 + portable_network_graphics_reverse_bit_order(remainder << 12);
            remainder >>= 4;
            remainder_size -= 4;
          }
          else if (literal_list_index == 279)
          {
            symbol_length = 99 + portable_network_graphics_reverse_bit_order(remainder << 12);
            remainder >>= 4;
            remainder_size -= 4;
          }
          else if (literal_list_index == 280)
          {
            symbol_length = 115 + portable_network_graphics_reverse_bit_order(remainder << 12);
            remainder >>= 4;
            remainder_size -= 4;
          }
          else if (literal_list_index == 281)
          {
            symbol_length = 131 + portable_network_graphics_reverse_bit_order(remainder << 11);
            remainder >>= 5;
            remainder_size -= 5;
          }
          else if (literal_list_index == 282)
          {
            symbol_length = 163 + portable_network_graphics_reverse_bit_order(remainder << 11);
            remainder >>= 5;
            remainder_size -= 5;
          }
          else if (literal_list_index == 283)
          {
            symbol_length = 195 + portable_network_graphics_reverse_bit_order(remainder << 11);
            remainder >>= 5;
            remainder_size -= 5;
          }
          else if (literal_list_index == 284)
          {
            symbol_length = 227 + portable_network_graphics_reverse_bit_order(remainder << 11);
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
            symbol_distance = 5 + (remainder & 1);
            remainder >>= 1;
            remainder_size -= 1;
          }
          else if (distance_list_index == 5)
          {
            symbol_distance = 7 + (remainder & 1);
            remainder >>= 1;
            remainder_size -= 1;
          }
          else if (distance_list_index == 6)
          {
            symbol_distance = 9 + portable_network_graphics_reverse_bit_order(remainder << 14);
            remainder >>= 2;
            remainder_size -= 2;
          }
          else if (distance_list_index == 7)
          {
            symbol_distance = 13 + portable_network_graphics_reverse_bit_order(remainder << 14);
            remainder >>= 2;
            remainder_size -= 2;
          }
          else if (distance_list_index == 8)
          {
            symbol_distance = 17 + portable_network_graphics_reverse_bit_order(remainder << 13);
            remainder >>= 3;
            remainder_size -= 3;
          }
          else if (distance_list_index == 9)
          {
            symbol_distance = 25 + portable_network_graphics_reverse_bit_order(remainder << 13);
            remainder >>= 3;
            remainder_size -= 3;
          }
          else if (distance_list_index == 10)
          {
            symbol_distance = 33 + portable_network_graphics_reverse_bit_order(remainder << 12);
            remainder >>= 4;
            remainder_size -= 4;
          }
          else if (distance_list_index == 11)
          {
            symbol_distance = 49 + portable_network_graphics_reverse_bit_order(remainder << 12);
            remainder >>= 4;
            remainder_size -= 4;
          }
          else if (distance_list_index == 12)
          {
            symbol_distance = 65 + portable_network_graphics_reverse_bit_order(remainder << 11);
            remainder >>= 5;
            remainder_size -= 5;
          }
          else if (distance_list_index == 13)
          {
            symbol_distance = 97 + portable_network_graphics_reverse_bit_order(remainder << 11);
            remainder >>= 5;
            remainder_size -= 5;
          }
          else if (distance_list_index == 14)
          {
            symbol_distance = 129 + portable_network_graphics_reverse_bit_order(remainder << 10);
            remainder >>= 6;
            remainder_size -= 6;
          }
          else if (distance_list_index == 15)
          {
            symbol_distance = 193 + portable_network_graphics_reverse_bit_order(remainder << 10);
            remainder >>= 6;
            remainder_size -= 6;
          }
          else if (distance_list_index == 16)
          {
            symbol_distance = 257 + portable_network_graphics_reverse_bit_order(remainder << 9);
            remainder >>= 7;
            remainder_size -= 7;
          }
          else if (distance_list_index == 17)
          {
            symbol_distance = 385 + portable_network_graphics_reverse_bit_order(remainder << 9);
            remainder >>= 7;
            remainder_size -= 7;
          }
          else if (distance_list_index == 18)
          {
            symbol_distance = 513 + portable_network_graphics_reverse_bit_order(remainder << 8);
            remainder >>= 8;
            remainder_size -= 8;
          }
          else if (distance_list_index == 19)
          {
            symbol_distance = 769 + portable_network_graphics_reverse_bit_order(remainder << 8);
            remainder >>= 8;
            remainder_size -= 8;
          }
          else if (distance_list_index == 20)
          {
            symbol_distance = 1025 + portable_network_graphics_reverse_bit_order(remainder << 7);
            remainder >>= 9;
            remainder_size -= 9;
          }
          else if (distance_list_index == 21)
          {
            symbol_distance = 1537 + portable_network_graphics_reverse_bit_order(remainder << 7);
            remainder >>= 9;
            remainder_size -= 9;
          }
          else if (distance_list_index == 22)
          {
            symbol_distance = 2049 + portable_network_graphics_reverse_bit_order(remainder << 6);
            remainder >>= 10;
            remainder_size -= 10;
          }
          else if (distance_list_index == 23)
          {
            symbol_distance = 3073 + portable_network_graphics_reverse_bit_order(remainder << 6);
            remainder >>= 10;
            remainder_size -= 10;
          }
          else if (distance_list_index == 24)
          {
            symbol_distance = 4097 + portable_network_graphics_reverse_bit_order(remainder << 5);
            remainder >>= 11;
            remainder_size -= 11;
          }
          else if (distance_list_index == 25)
          {
            symbol_distance = 6145 + portable_network_graphics_reverse_bit_order(remainder << 5);
            remainder >>= 11;
            remainder_size -= 11;
          }
          else if (distance_list_index == 26)
          {
            symbol_distance = 8193 + portable_network_graphics_reverse_bit_order(remainder << 4);
            remainder >>= 12;
            remainder_size -= 12;
          }
          else if (distance_list_index == 27)
          {
            symbol_distance = 12289 + portable_network_graphics_reverse_bit_order(remainder << 4);
            remainder >>= 12;
            remainder_size -= 12;
          }
          else if (distance_list_index == 28)
          {
            symbol_distance = 16385 + portable_network_graphics_reverse_bit_order(remainder << 3);
            remainder >>= 13;
            remainder_size -= 13;
          }
          else if (distance_list_index == 29)
          {
            symbol_distance = 24577 + portable_network_graphics_reverse_bit_order(remainder << 3);
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
        // TEMPORARY
        unsigned char* temporary_address = (destination_address - destination_size) + 1;
        unsigned long long temporary_size = 256;
        while (temporary_size != 0)
        {
          console_append_unsigned_integer(*temporary_address); console_append_character(32);
          console_append_unsigned_integer(*(temporary_address + 1)); console_append_character(32);
          console_append_unsigned_integer(*(temporary_address + 2)); console_append_character(32);
          console_append_unsigned_integer(*(temporary_address + 3));
          console_append_character(13); console_append_character(10); console_write();
          temporary_address += 4;
          temporary_size -= 1;
        }

        // TO DO Make this check succeed!
        // Calculate the adler32 from the inflated data.
        unsigned long long low_adler32 = 1;
        unsigned long long high_adler32 = 0;
        destination_address -= destination_size;
        while (destination_size != 0)
        {
          low_adler32 = (low_adler32 + *destination_address) % 65521;
          high_adler32 = (high_adler32 + low_adler32) % 65521;
          destination_address += 1;
          destination_size -= 1;
        }

        // Drop the remainder.
        source_address -= remainder_size >> 3;

        // Compare the high adler32.
        unsigned long long source_high_adler32 = (*source_address << 8) | *(source_address + 1);
        if (high_adler32 != source_high_adler32)
        { break; }

        // Compare the low adler32.
        unsigned long long source_low_adler32 = (*(source_address + 2) << 8) | *(source_address + 3);
        if (low_adler32 != source_low_adler32)
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
  diagonal_difference %= 256;

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

      unsigned long long filtered_count = 0;
      unsigned long long filter_type = 0;
      while (filtered_count < filtered_size)
      {
        if (filtered_count % (width + 1) == 0)
        {
          filter_type = *filtered_address;
          filtered_address += 1;
          filtered_count += 1;
        }
        else if (filter_type == 0)
        {
          unsigned long long sample = *filtered_address * 257;
          *(unsigned short*)destination_address = sample;
          *(unsigned short*)(destination_address + 2) = sample;
          *(unsigned short*)(destination_address + 4) = sample;
          *(unsigned short*)(destination_address + 6) = 65535;
          if ((tRNS_address != 0) && ((*(tRNS_address + 9) * 257) == sample))
          { *(unsigned short*)(destination_address + 6) = 0; }

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
            sample %= 256;
          }

          sample *= 257;
          *(unsigned short*)destination_address = sample;
          *(unsigned short*)(destination_address + 2) = sample;
          *(unsigned short*)(destination_address + 4) = sample;
          *(unsigned short*)(destination_address + 6) = 65535;
          if ((tRNS_address != 0) && ((*(tRNS_address + 9) * 257) == sample))
          { *(unsigned short*)(destination_address + 6) = 0; }

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
            sample %= 256;
          }

          sample *= 257;
          *(unsigned short*)destination_address = sample;
          *(unsigned short*)(destination_address + 2) = sample;
          *(unsigned short*)(destination_address + 4) = sample;
          *(unsigned short*)(destination_address + 6) = 65535;
          if ((tRNS_address != 0) && ((*(tRNS_address + 9) * 257) == sample))
          { *(unsigned short*)(destination_address + 6) = 0; }

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
          { sample += *(destination_address - (width * 8)); }

          sample = (((sample / 2) + *filtered_address) % 256) * 257;
          *(unsigned short*)destination_address = sample;
          *(unsigned short*)(destination_address + 2) = sample;
          *(unsigned short*)(destination_address + 4) = sample;
          *(unsigned short*)(destination_address + 6) = 65535;
          if ((tRNS_address != 0) && ((*(tRNS_address + 9) * 257) == sample))
          { *(unsigned short*)(destination_address + 6) = 0; }

          destination_address += 8;
          filtered_address += 1;
          filtered_count += 1;
        }
        else if (filter_type == 4)
        {
          unsigned long long dimension_count = 2;
          unsigned long long horizontal = 0;
          if ((filtered_count % (width + 1)) >= 2)
          {
            horizontal = *(destination_address - 8);
            dimension_count -= 1;
          }

          unsigned long long vertical = 0;
          if (filtered_count >= (width + 1))
          {
            vertical = *(destination_address - (width * 8));
            dimension_count -= 1;
          }

          unsigned long long diagonal = 0;
          if (dimension_count == 0)
          { diagonal =  *(destination_address - (width * 8) - 8); }

          unsigned long long sample = portable_network_graphics_paeth(horizontal, vertical, diagonal);
          sample = ((sample + *filtered_address) % 256) * 257;
          *(unsigned short*)destination_address = sample;
          *(unsigned short*)(destination_address + 2) = sample;
          *(unsigned short*)(destination_address + 4) = sample;
          *(unsigned short*)(destination_address + 6) = 65535;
          if ((tRNS_address != 0) && ((*(tRNS_address + 9) * 257) == sample))
          { *(unsigned short*)(destination_address + 6) = 0; }

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
      unsigned long long filtered_size = portable_network_graphics_read_int(IHDR_address + 8);
      filtered_size = (filtered_size * 2) + 1;
      filtered_size *= portable_network_graphics_read_int(IHDR_address + 16);
      // to do
    }
    else if ((IHDR_address[16] == 8) && (IHDR_address[17] == 2) && (IHDR_address[18] == 0) && (IHDR_address[19] == 0) && (IHDR_address[20] == 0))
    {
      unsigned long long filtered_size = portable_network_graphics_read_int(IHDR_address + 8);
      filtered_size = (filtered_size * 3) + 1;
      filtered_size *= portable_network_graphics_read_int(IHDR_address + 16);
      // to do
    }
    else if ((IHDR_address[16] == 16) && (IHDR_address[17] == 2) && (IHDR_address[18] == 0) && (IHDR_address[19] == 0) && (IHDR_address[20] == 0))
    {
      unsigned long long filtered_size = portable_network_graphics_read_int(IHDR_address + 8);
      filtered_size = (filtered_size * 6) + 1;
      filtered_size *= portable_network_graphics_read_int(IHDR_address + 16);
      // to do
    }
    else if ((IHDR_address[16] == 8) && (IHDR_address[17] == 3) && (IHDR_address[18] == 0) && (IHDR_address[19] == 0) && (IHDR_address[20] == 0))
    {
      unsigned long long filtered_size = portable_network_graphics_read_int(IHDR_address + 8);
      filtered_size += 1;
      filtered_size *= portable_network_graphics_read_int(IHDR_address + 16);
      // to do
    }
    else if ((IHDR_address[16] == 8) && (IHDR_address[17] == 4) && (IHDR_address[18] == 0) && (IHDR_address[19] == 0) && (IHDR_address[20] == 0))
    {
      unsigned long long filtered_size = portable_network_graphics_read_int(IHDR_address + 8);
      filtered_size = (filtered_size * 2) + 1;
      filtered_size *= portable_network_graphics_read_int(IHDR_address + 16);
      // to do
    }
    else if ((IHDR_address[16] == 16) && (IHDR_address[17] == 4) && (IHDR_address[18] == 0) && (IHDR_address[19] == 0) && (IHDR_address[20] == 0))
    {
      unsigned long long filtered_size = portable_network_graphics_read_int(IHDR_address + 8);
      filtered_size = (filtered_size * 4) + 1;
      filtered_size *= portable_network_graphics_read_int(IHDR_address + 16);
      // to do
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

      // TEMPORARY
      console_append_character(65); console_append_character(13); console_append_character(10); console_write();

      unsigned long long filtered_count = 0;
      unsigned long long filter_type = 0;
      while (filtered_count < filtered_size)
      {
        if (filtered_size % ((width * 4) + 1) == 0)
        {
          filter_type = *filtered_address;
          filtered_address += 1;
          filtered_count += 1;
        }
        else if (filter_type == 0)
        {
          *(unsigned short*)destination_address = *filtered_address * 257;
          destination_address += 2;
          filtered_address += 1;
          filtered_count += 1;
        }
        else if (filter_type == 1)
        {
          unsigned long long sample = *filtered_address;
          if ((filtered_count % ((width * 4) + 1)) >= 5)
          {
            sample += *(destination_address - 8);
            sample %= 256;
          }

          *(unsigned short*)destination_address = sample * 257;
          destination_address += 2;
          filtered_address += 1;
          filtered_count += 1;
        }
        else if (filter_type == 2)
        {
          unsigned long long sample = *filtered_address;
          if (filtered_count >= ((width * 4) + 1))
          {
            sample += *(destination_address - (width * 8));
            sample %= 256;
          }

          *(unsigned short*)destination_address = sample * 257;
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
          { sample += *(destination_address - (width * 8)); }

          *(unsigned short*)destination_address = (((sample / 2) + *filtered_address) % 256) * 257;
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
          { diagonal = *(destination_address - (width * 8) - 8); }

          unsigned long long sample = portable_network_graphics_paeth(horizontal, vertical, diagonal);
          *(unsigned short*)destination_address = ((sample + *filtered_address) % 256) * 257;
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

      unsigned long long filtered_count = 0;
      unsigned long long filter_type = 0;
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
          *destination_address = *filtered_address;
          destination_address += 1;
          filtered_address += 1;
          filtered_count += 1;
        }
        else if (filter_type == 1)
        {
          unsigned long long sample = *filtered_address;
          if ((filtered_count % ((width * 8) + 1)) >= 9)
          {
            sample += *(destination_address - 8);
            sample %= 256;
          }

          *destination_address = sample;
          destination_address += 1;
          filtered_address += 1;
          filtered_size += 1;
        }
        else if (filter_type == 2)
        {
          unsigned long long sample = *filtered_address;
          if (filtered_count >= ((width * 8) + 1))
          {
            sample += *(destination_address - (width * 8));
            sample %= 256;
          }

          *destination_address = sample;
          destination_address += 1;
          filtered_address += 1;
          filtered_count += 1;
        }
        else if (filter_type == 3)
        {
          unsigned long long sample = 0;
          if ((filtered_count % ((width * 8) + 1)) >= 9)
          { sample = *(destination_address - 8); }

          if (filtered_count >= ((width * 8) + 1))
          { sample += *(destination_address - (width * 8)); }

          *destination_address = ((sample / 2) + *filtered_address) % 256;
          destination_address += 1;
          filtered_address += 1;
          filtered_count += 1;
        }
        else if (filter_type == 4)
        {
          unsigned long long dimension_count = 0;
          unsigned long long horizontal = 0;
          if ((filtered_count % ((width * 8) + 1)) >= 9)
          {
            horizontal = *(destination_address - 8);
            dimension_count += 1;
          }

          unsigned long long vertical = 0;
          if (filtered_count >= ((width * 8) + 1))
          {
            vertical = *(destination_address - (width * 8));
            dimension_count += 1;
          }

          unsigned long long diagonal = 0;
          if (dimension_count == 2)
          { diagonal = *(destination_address - (width * 8) - 8); }

          unsigned long long sample = portable_network_graphics_paeth(horizontal, vertical, diagonal);
          *destination_address = (sample + *filtered_address) % 256;
          destination_address += 1;
          filtered_address += 1;
          filtered_size += 1;
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