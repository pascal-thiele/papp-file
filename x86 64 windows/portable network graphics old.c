#include "memory.c"
#include "console.c"

struct portable_network_graphics_huffman
{
  unsigned int length;
  unsigned int code;
};

struct portable_network_graphics_huffman code_list[2048];


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


void portable_network_graphics_copy(unsigned char* source_address, unsigned long long source_size, unsigned char* destination_address)
{
  while (source_size != 0)
  {
    *destination_address = *source_address;
    source_address += 1;
    source_size -= 1;
    destination_address += 1;
  }
}


// Reorders sixteen bits, where the less significant a bit is, the more significant a bit will be.
unsigned long long portable_network_graphics_reverse_bit_order(unsigned long long value)
{
  unsigned long long reversed = value & 65535;
  reversed = ((reversed & 65280) >> 8) | ((reversed & 255) << 8);
  reversed = ((reversed & 61680) >> 4) | ((reversed & 3855) << 4);
  reversed = ((reversed & 52428) >> 2) | ((reversed & 13107) << 2);
  reversed = ((reversed & 43690) >> 1) | ((reversed & 21845) << 1);
  return reversed;
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
  unsigned long long code_list_index = 0;
  while (code_list_index < code_list_size)
  {
    *(count_list + code_list_address->length) += 1;
    code_list_address += 1;
    code_list_index += 1;
  }
  *count_list = 0;

  // Calculate the smallest code for each length.
  unsigned long long next_list[16];
  unsigned long long* count_list_address = count_list;
  unsigned long long* next_list_address = next_list;
  unsigned long long next_list_size = 15;
  unsigned long long code = 0;
  while (next_list_size != 0)
  {
    code = (code + *count_list_address) << 1;
    next_list_address += 1;
    *next_list_address = code;
    count_list_address += 1;
    next_list_size -= 1;
  }

  // Write the actual code.
  code_list_address -= code_list_size;
  code_list_index = 0;
  while (code_list_index < code_list_size)
  {
    if (code_list_address->length != 0)
    {
      code_list_address->code = *(next_list + code_list_address->length);
      *(next_list + code_list_address->length) += 1;
    }
    code_list_address += 1;
    code_list_index += 1;
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
      remainder |= *source_address << remainder_size;
      remainder_size += 5;
      block_final = remainder & 1;
      remainder >>= 1;
      block_type = remainder & 3;
      remainder >>= 2;
      source_address += 1;

      if (block_type == 0)
      {
        // Read uncompressed bytes.
        unsigned long long block_length = *source_address | (*(source_address + 1) << 8);
        unsigned long long block_length_complement = ~(*(source_address + 2) | (*(source_address + 3) << 8));
        if (block_length != block_length_complement)
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
        // to do
        return !0;
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

        // TEMPORARY
        console_append_unsigned_integer(literal_code_count); console_append_character(32);
        console_append_unsigned_integer(distance_code_count); console_append_character(32);
        console_append_unsigned_integer(code_code_count); console_append_character(13); console_append_character(10);
        console_write();

        // Construct the code code list.
        struct portable_network_graphics_huffman code_code_list[19];
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
          code_code_list_address->length = remainder & 7;
          remainder >>= 3;
          remainder_size -= 3;
          code_code_list_address += 1;
          code_code_list_size -= 1;
        }
        portable_network_graphics_construct_huffman(code_code_list, code_code_count);

console_append_character(45); console_append_character(45); console_append_character(45); console_append_character(13); console_append_character(10);
struct portable_network_graphics_huffman* temporary_address = code_code_list;
unsigned long long temporary_size = code_code_count;
while (temporary_size != 0)
{
  console_append_unsigned_integer(temporary_address->length); console_append_character(32);
  console_append_unsigned_integer(temporary_address->code); console_append_character(13); console_append_character(10);
  console_write();
  temporary_address += 1;
  temporary_size -= 1;
}

        // Construct the literal code list and distance code list with the code code list.
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
          while (code_code_list_index < code_code_count)
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

          if (code_code_list_index == 0)
          {
            if (remainder_size < 2)
            {
              remainder |= *source_address << remainder_size;
              remainder_size += 8;
              source_address += 1;
            }
            //unsigned long long repeat = 3 + portable_network_graphics_reverse_bit_order(remainder << 14);
            unsigned long long repeat = (remainder & 3) + 3;
            remainder >>= 2;
            remainder_size -= 2;
            code_list_size -= 1;
            while (repeat != 0)
            {
              code_list_address->length = (code_list_address - 1)->length;
              code_list_address += 1;
              repeat -= 1;
            }
          }
          else if (code_code_list_index == 1)
          {
            if (remainder_size < 3)
            {
              remainder |= *source_address << remainder_size;
              remainder_size += 8;
              source_address += 1;
            }
            //unsigned long long repeat = 3 + portable_network_graphics_reverse_bit_order(remainder << 13);
            unsigned long long repeat = (remainder & 7) + 3;
            remainder >>= 3;
            remainder_size -= 3;
            code_list_size -= 1;
            while (repeat != 0)
            {
              code_list_address->length = 0;
              code_list_address += 1;
              repeat -= 1;
            }
          }
          else if (code_code_list_index == 2)
          {
            if (remainder_size < 7)
            {
              remainder |= *source_address << remainder_size;
              remainder_size += 8;
              source_address += 1;
            }
            //unsigned long long repeat = 11 + portable_network_graphics_reverse_bit_order(remainder << 9);
            unsigned long long repeat = (remainder & 127) + 11;
            remainder >>= 7;
            remainder_size -= 7;
            code_list_size -= 1;
            while (repeat != 0)
            {
              code_list_address->length = 0;
              code_list_address += 1;
              repeat -= 1;
            }
          }
          else if (code_code_list_index == 3)
          {
            code_list_address->length = 0;
            code_list_address += 1;
            code_list_size -= 1;
          }
          else if (code_code_list_index == 4)
          {
            code_list_address->length = 8;
            code_list_address += 1;
            code_list_size -= 1;
          }
          else if (code_code_list_index == 5)
          {
            code_list_address->length = 7;
            code_list_address += 1;
            code_list_size -= 1;
          }
          else if (code_code_list_index == 6)
          {
            code_list_address->length = 9;
            code_list_address += 1;
            code_list_size -= 1;
          }
          else if (code_code_list_index == 7)
          {
            code_list_address->length = 6;
            code_list_address += 1;
            code_list_size -= 1;
          }
          else if (code_code_list_index == 8)
          {
            code_list_address->length = 10;
            code_list_address += 1;
            code_list_size -= 1;
          }
          else if (code_code_list_index == 9)
          {
            code_list_address->length = 5;
            code_list_address += 1;
            code_list_size -= 1;
          }
          else if (code_code_list_index == 10)
          {
            code_list_address->length = 11;
            code_list_address += 1;
            code_list_size -= 1;
          }
          else if (code_code_list_index == 11)
          {
            code_list_address->length = 4;
            code_list_address += 1;
            code_list_size -= 1;
          }
          else if (code_code_list_index == 12)
          {
            code_list_address->length = 12;
            code_list_address += 1;
            code_list_size -= 1;
          }
          else if (code_code_list_index == 13)
          {
            code_list_address->length = 3;
            code_list_address += 1;
            code_list_size -= 1;
          }
          else if (code_code_list_index == 14)
          {
            code_list_address->length = 13;
            code_list_address += 1;
            code_list_size -= 1;
          }
          else if (code_code_list_index == 15)
          {
            code_list_address->length = 2;
            code_list_address += 1;
            code_list_size -= 1;
          }
          else if (code_code_list_index == 16)
          {
            code_list_address->length = 14;
            code_list_address += 1;
            code_list_size -= 1;
          }
          else if (code_code_list_index == 17)
          {
            code_list_address->length = 1;
            code_list_address += 1;
            code_list_size -= 1;
          }
          else if (code_code_list_index == 18)
          {
            code_list_address->length = 15;
            code_list_address += 1;
            code_list_size -= 1;
          }
          else
          { return !0; }
        }
        portable_network_graphics_construct_huffman(code_list, literal_code_count);
        portable_network_graphics_construct_huffman(code_list + literal_code_count, distance_code_count);
/*
console_append_character(45); console_append_character(45); console_append_character(45); console_append_character(13); console_append_character(10);
struct portable_network_graphics_huffman* temporary_address = code_list;
unsigned long long temporary_size = literal_code_count + distance_code_count;
while (temporary_size != 0)
{
  console_append_unsigned_integer(temporary_address->length); console_append_character(32);
  console_append_unsigned_integer(temporary_address->code); console_append_character(13); console_append_character(10);
  console_write();
  temporary_address += 1;
  temporary_size -= 1;
}
*/
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

          // Translate the symbol to destination.
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
            if (remainder_size == 0)
            { remainder = *source_address; remainder_size = 8; source_address += 1; }
            symbol_length = 11 + (remainder & 1);
            remainder >>= 1;
            remainder_size -= 1;
          }
          else if (literal_list_index == 266)
          {
            if (remainder_size == 0)
            { remainder = *source_address; remainder_size = 8; source_address += 1; }
            symbol_length = 13 + (remainder & 1);
            remainder >>= 1;
            remainder_size -= 1;
          }
          else if (literal_list_index == 267)
          {
            if (remainder_size == 0)
            { remainder = *source_address; remainder_size = 8; source_address += 1; }
            symbol_length = 15 + (remainder & 1);
            remainder >>= 1;
            remainder_size -= 1;
          }
          else if (literal_list_index == 268)
          {
            if (remainder_size == 0)
            { remainder = *source_address; remainder_size = 8; source_address += 1; }
            symbol_length = 17 + (remainder & 1);
            remainder >>= 1;
            remainder_size -= 1;
          }
          else if (literal_list_index == 269)
          {
            if (remainder_size < 2)
            { remainder |= *source_address << remainder_size; remainder_size += 8; source_address += 1; }
            symbol_length = 19 + portable_network_graphics_reverse_bit_order(remainder << 14);
            remainder >>= 2;
            remainder_size -= 2;
          }
          else if (literal_list_index == 270)
          {
            if (remainder_size < 2)
            { remainder |= *source_address << remainder_size; remainder_size += 8; source_address += 1; }
            symbol_length = 23 + portable_network_graphics_reverse_bit_order(remainder << 14);
            remainder >>= 2;
            remainder_size -= 2;
          }
          else if (literal_list_index == 271)
          {
            if (remainder_size < 2)
            { remainder |= *source_address << remainder_size; remainder_size += 8; source_address += 1; }
            symbol_length = 27 + portable_network_graphics_reverse_bit_order(remainder << 14);
            remainder >>= 2;
            remainder_size -= 2;
          }
          else if (literal_list_index == 272)
          {
            if (remainder_size < 2)
            { remainder |= *source_address << remainder_size; remainder_size += 8; source_address += 1; }
            symbol_length = 31 + portable_network_graphics_reverse_bit_order(remainder << 14);
            remainder >>= 2;
            remainder_size -= 2;
          }
          else if (literal_list_index == 273)
          {
            if (remainder_size < 3)
            { remainder |= *source_address << remainder_size; remainder_size += 8; source_address += 1; }
            symbol_length = 35 + portable_network_graphics_reverse_bit_order(remainder << 13);
            remainder >>= 3;
            remainder_size -= 3;
          }
          else if (literal_list_index == 274)
          {
            if (remainder_size < 3)
            { remainder |= *source_address << remainder_size; remainder_size += 8; source_address += 1; }
            symbol_length = 43 + portable_network_graphics_reverse_bit_order(remainder << 13);
            remainder >>= 3;
            remainder_size -= 3;
          }
          else if (literal_list_index == 275)
          {
            if (remainder_size < 3)
            { remainder |= *source_address << remainder_size; remainder_size += 8; source_address += 1; }
            symbol_length = 51 + portable_network_graphics_reverse_bit_order(remainder << 13);
            remainder >>= 3;
            remainder_size -= 3;
          }
          else if (literal_list_index == 276)
          {
            if (remainder_size < 3)
            { remainder |= *source_address << remainder_size; remainder_size += 8; source_address += 1; }
            symbol_length = 59 + portable_network_graphics_reverse_bit_order(remainder << 13);
            remainder >>= 3;
            remainder_size -= 3;
          }
          else if (literal_list_index == 277)
          {
            if (remainder_size < 4)
            { remainder |= *source_address << remainder_size; remainder_size += 8; source_address += 1; }
            symbol_length = 67 + portable_network_graphics_reverse_bit_order(remainder << 12);
            remainder >>= 4;
            remainder_size -= 4;
          }
          else if (literal_list_index == 278)
          {
            if (remainder_size < 4)
            { remainder |= *source_address << remainder_size; remainder_size += 8; source_address += 1; }
            symbol_length = 83 + portable_network_graphics_reverse_bit_order(remainder << 12);
            remainder >>= 4;
            remainder_size -= 4;
          }
          else if (literal_list_index == 279)
          {
            if (remainder_size < 4)
            { remainder |= *source_address << remainder_size; remainder_size += 8; source_address += 1; }
            symbol_length = 99 + portable_network_graphics_reverse_bit_order(remainder << 12);
            remainder >>= 4;
            remainder_size -= 4;
          }
          else if (literal_list_index == 280)
          {
            if (remainder_size < 4)
            { remainder |= *source_address << remainder_size; remainder_size += 8; source_address += 1; }
            symbol_length = 115 + portable_network_graphics_reverse_bit_order(remainder << 12);
            remainder >>= 4;
            remainder_size -= 4;
          }
          else if (literal_list_index == 281)
          {
            if (remainder_size < 5)
            { remainder |= *source_address << remainder_size; remainder_size += 8; source_address += 1; }
            symbol_length = 131 + portable_network_graphics_reverse_bit_order(remainder << 11);
            remainder >>= 5;
            remainder_size -= 5;
          }
          else if (literal_list_index == 282)
          {
            if (remainder_size < 5)
            { remainder |= *source_address << remainder_size; remainder_size += 8; source_address += 1; }
            symbol_length = 163 + portable_network_graphics_reverse_bit_order(remainder << 11);
            remainder >>= 5;
            remainder_size -= 5;
          }
          else if (literal_list_index == 283)
          {
            if (remainder_size < 5)
            { remainder |= *source_address << remainder_size; remainder_size += 8; source_address += 1; }
            symbol_length = 195 + portable_network_graphics_reverse_bit_order(remainder << 11);
            remainder >>= 5;
            remainder_size -= 5;
          }
          else if (literal_list_index == 284)
          {
            if (remainder_size < 5)
            { remainder |= *source_address << remainder_size; remainder_size += 8; source_address += 1; }
            symbol_length = 227 + portable_network_graphics_reverse_bit_order(remainder << 11);
            remainder >>= 5;
            remainder_size -= 5;
          }
          else if (literal_list_index == 285)
          { symbol_length = 258; }
          else
          { return !0; }

          // Translate the distance code.
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

          unsigned long long symbol_distance;
          if (distance_list_index < 4)
          { symbol_distance = distance_list_index + 1; }
          else if (distance_list_index == 4)
          {
            if (remainder_size == 0)
            { remainder = *source_address; remainder_size = 8; source_address += 1; }
            symbol_distance = 5 + (remainder & 1);
            remainder >>= 1;
            remainder_size -= 1;
          }
          else if (distance_list_index == 5)
          {
            if (remainder_size == 0)
            { remainder = *source_address; remainder_size = 8; source_address += 1; }
            symbol_distance = 7 + (remainder & 1);
            remainder >>= 1;
            remainder_size -= 1;
          }
          else if (distance_list_index == 6)
          {
            if (remainder_size < 2)
            { remainder |= *source_address << remainder_size; remainder_size += 8; source_address += 1; }
            symbol_distance = 9 + portable_network_graphics_reverse_bit_order(remainder << 14);
            remainder >>= 2;
            remainder_size -= 2;
          }
          else if (distance_list_index == 7)
          {
            if (remainder_size < 2)
            { remainder |= *source_address << remainder_size; remainder_size += 8; source_address += 1; }
            symbol_distance = 13 + portable_network_graphics_reverse_bit_order(remainder << 14);
            remainder >>= 2;
            remainder_size -= 2;
          }
          else if (distance_list_index == 8)
          {
            if (remainder_size < 3)
            { remainder |= *source_address << remainder_size; remainder_size += 8; source_address += 1; }
            symbol_distance = 17 + portable_network_graphics_reverse_bit_order(remainder << 13);
            remainder >>= 3;
            remainder_size -= 3;
          }
          else if (distance_list_index == 9)
          {
            if (remainder_size < 3)
            { remainder |= *source_address << remainder_size; remainder_size += 8; source_address += 1; }
            symbol_distance = 25 + portable_network_graphics_reverse_bit_order(remainder << 13);
            remainder >>= 3;
            remainder_size -= 3;
          }
          else if (distance_list_index == 10)
          {
            if (remainder_size < 4)
            { remainder |= *source_address << remainder_size; remainder_size += 8; source_address += 1; }
            symbol_distance = 33 + portable_network_graphics_reverse_bit_order(remainder << 12);
            remainder >>= 4;
            remainder_size -= 4;
          }
          else if (distance_list_index == 11)
          {
            if (remainder_size < 4)
            { remainder |= *source_address << remainder_size; remainder_size += 8; source_address += 1; }
            symbol_distance = 49 + portable_network_graphics_reverse_bit_order(remainder << 12);
            remainder >>= 4;
            remainder_size -= 4;
          }
          else if (distance_list_index == 12)
          {
            if (remainder_size < 5)
            { remainder |= *source_address << remainder_size; remainder_size += 8; source_address += 1; }
            symbol_distance = 65 + portable_network_graphics_reverse_bit_order(remainder << 11);
            remainder >>= 5;
            remainder_size -= 5;
          }
          else if (distance_list_index == 13)
          {
            if (remainder_size < 5)
            { remainder |= *source_address << remainder_size; remainder_size += 8; source_address += 1; }
            symbol_distance = 97 + portable_network_graphics_reverse_bit_order(remainder << 11);
            remainder >>= 5;
            remainder_size -= 5;
          }
          else if (distance_list_index == 14)
          {
            if (remainder_size < 6)
            { remainder |= *source_address << remainder_size; remainder_size += 8; source_address += 1; }
            symbol_distance = 129 + portable_network_graphics_reverse_bit_order(remainder << 10);
            remainder >>= 6;
            remainder_size -= 6;
          }
          else if (distance_list_index == 15)
          {
            if (remainder_size < 6)
            { remainder |= *source_address << remainder_size; remainder_size += 8; source_address += 1; }
            symbol_distance = 193 + portable_network_graphics_reverse_bit_order(remainder << 10);
            remainder >>= 6;
            remainder_size -= 6;
          }
          else if (distance_list_index == 16)
          {
            if (remainder_size < 7)
            { remainder |= *source_address << remainder_size; remainder_size += 8; source_address += 1; }
            symbol_distance = 257 + portable_network_graphics_reverse_bit_order(remainder << 9);
            remainder >>= 7;
            remainder_size -= 7;
          }
          else if (distance_list_index == 17)
          {
            if (remainder_size < 7)
            { remainder |= *source_address << remainder_size; remainder_size += 8; source_address += 1; }
            symbol_distance = 385 + portable_network_graphics_reverse_bit_order(remainder << 9);
            remainder >>= 7;
            remainder_size -= 7;
          }
          else if (distance_list_index == 18)
          {
            if (remainder_size < 8)
            { remainder |= *source_address << remainder_size; remainder_size += 8; source_address += 1; }
            symbol_distance = 513 + portable_network_graphics_reverse_bit_order(remainder << 8);
            remainder >>= 8;
            remainder_size -= 8;
          }
          else if (distance_list_index == 19)
          {
            if (remainder_size < 8)
            { remainder |= *source_address << remainder_size; remainder_size += 8; source_address += 1; }
            symbol_distance = 769 + portable_network_graphics_reverse_bit_order(remainder << 8);
            remainder >>= 8;
            remainder_size -= 8;
          }
          else if (distance_list_index == 20)
          {
            if (remainder_size < 9)
            { remainder |= *source_address << remainder_size; remainder_size += 8; source_address += 1; }
            if (remainder_size < 9)
            { remainder |= *source_address << remainder_size; remainder_size += 8; source_address += 1; }
            symbol_distance = 1025 + portable_network_graphics_reverse_bit_order(remainder << 7);
            remainder >>= 9;
            remainder_size -= 9;
          }
          else if (distance_list_index == 21)
          {
            if (remainder_size < 9)
            { remainder |= *source_address << remainder_size; remainder_size += 8; source_address += 1; }
            if (remainder_size < 9)
            { remainder |= *source_address << remainder_size; remainder_size += 8; source_address += 1; }
            symbol_distance = 1537 + portable_network_graphics_reverse_bit_order(remainder << 7);
            remainder >>= 9;
            remainder_size -= 9;
          }
          else if (distance_list_index == 22)
          {
            if (remainder_size < 10)
            { remainder |= *source_address << remainder_size; remainder_size += 8; source_address += 1; }
            if (remainder_size < 10)
            { remainder |= *source_address << remainder_size; remainder_size += 8; source_address += 1; }
            symbol_distance = 2049 + portable_network_graphics_reverse_bit_order(remainder << 6);
            remainder >>= 10;
            remainder_size -= 10;
          }
          else if (distance_list_index == 23)
          {
            if (remainder_size < 10)
            { remainder |= *source_address << remainder_size; remainder_size += 8; source_address += 1; }
            if (remainder_size < 10)
            { remainder |= *source_address << remainder_size; remainder_size += 8; source_address += 1; }
            symbol_distance = 3073 + portable_network_graphics_reverse_bit_order(remainder << 6);
            remainder >>= 10;
            remainder_size -= 10;
          }
          else if (distance_list_index == 24)
          {
            if (remainder_size < 11)
            { remainder |= *source_address << remainder_size; remainder_size += 8; source_address += 1; }
            if (remainder_size < 11)
            { remainder |= *source_address << remainder_size; remainder_size += 8; source_address += 1; }
            symbol_distance = 4097 + portable_network_graphics_reverse_bit_order(remainder << 5);
            remainder >>= 11;
            remainder_size -= 11;
          }
          else if (distance_list_index == 25)
          {
            if (remainder_size < 11)
            { remainder |= *source_address << remainder_size; remainder_size += 8; source_address += 1; }
            if (remainder_size < 11)
            { remainder |= *source_address << remainder_size; remainder_size += 8; source_address += 1; }
            symbol_distance = 6145 + portable_network_graphics_reverse_bit_order(remainder << 5);
            remainder >>= 11;
            remainder_size -= 11;
          }
          else if (distance_list_index == 26)
          {
            if (remainder_size < 12)
            { remainder |= *source_address << remainder_size; remainder_size += 8; source_address += 1; }
            if (remainder_size < 12)
            { remainder |= *source_address << remainder_size; remainder_size += 8; source_address += 1; }
            symbol_distance = 8193 + portable_network_graphics_reverse_bit_order(remainder << 4);
            remainder >>= 12;
            remainder_size -= 12;
          }
          else if (distance_list_index == 27)
          {
            if (remainder_size < 12)
            { remainder |= *source_address << remainder_size; remainder_size += 8; source_address += 1; }
            if (remainder_size < 12)
            { remainder |= *source_address << remainder_size; remainder_size += 8; source_address += 1; }
            symbol_distance = 12289 + portable_network_graphics_reverse_bit_order(remainder << 4);
            remainder >>= 12;
            remainder_size -= 12;
          }
          else if (distance_list_index == 28)
          {
            if (remainder_size < 13)
            { remainder |= *source_address << remainder_size; remainder_size += 8; source_address += 1; }
            if (remainder_size < 13)
            { remainder |= *source_address << remainder_size; remainder_size += 8; source_address += 1; }
            symbol_distance = 16385 + portable_network_graphics_reverse_bit_order(remainder << 3);
            remainder >>= 13;
            remainder_size -= 13;
          }
          else if (distance_list_index == 29)
          {
            if (remainder_size < 13)
            { remainder |= *source_address << remainder_size; remainder_size += 8; source_address += 1; }
            if (remainder_size < 13)
            { remainder |= *source_address << remainder_size; remainder_size += 8; source_address += 1; }
            symbol_distance = 24577 + portable_network_graphics_reverse_bit_order(remainder << 3);
            remainder >>= 13;
            remainder_size -= 13;
          }
          else
          { return !0; }

          // Relocate all bytes of the symbol that directly preceed the current destination address.
          if (symbol_distance < symbol_length)
          {
            portable_network_graphics_copy(destination_address - symbol_distance, symbol_distance, destination_address);
            symbol_distance = 0;
          }

          // Copy length bytes, distance bytes back in the destination.
          portable_network_graphics_copy(destination_address - symbol_distance, symbol_length, destination_address);
          destination_address += symbol_length;
          destination_size += symbol_length;
        }
      }
      else
      { break; }

      if (block_final != 0)
      {
        // Calculate the adler32 from the inflated data.
        unsigned long long adler32 = 1;
        // to do
        // Compare the adler32 with the source.
        // to do
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
  if (portable_network_graphics_identify(source_address) != 0)
  { return !0;}

  source_address += 8;
  unsigned char* decode_address = source_address;
  unsigned long long decode_size = 0;
  unsigned char* IHDR_address = 0;
  unsigned char* PLTE_address = 0;
  unsigned char* tRNS_address = 0;

  // Validate and extract basic information.
  while (1)
  {
    unsigned long long chunk_length = portable_network_graphics_read_int(decode_address);
    unsigned long long chunk_cyclic_redundancy_check = portable_network_graphics_read_int(decode_address + chunk_length + 8);
    if (chunk_cyclic_redundancy_check != portable_network_graphics_cyclic_redundancy_check(decode_address + 4, chunk_length + 4))
    { return !0; }
    
    if (portable_network_graphics_compare_IHDR(decode_address + 4) == 0)
    {
      if (IHDR_address != 0)
      { return !0; }
      IHDR_address = decode_address;
      decode_address += chunk_length + 12;
    }
    else if (portable_network_graphics_compare_PLTE(decode_address + 4) == 0)
    {
      if (PLTE_address != 0)
      { return !0; }
      PLTE_address = decode_address;
      decode_address += chunk_length + 12;
    }
    else if (portable_network_graphics_compare_tRNS(decode_address + 4) == 0)
    {
      if (tRNS_address != 0)
      { return !0; }
      tRNS_address = decode_address;
      decode_address += chunk_length + 12;
    }
    else if (portable_network_graphics_compare_IDAT(decode_address + 4) == 0)
    {
      decode_size += chunk_length;
      decode_address += chunk_length + 12;
    }
    else if (portable_network_graphics_compare_IEND(decode_address + 4) == 0)
    { break; }
    else
    { decode_address += chunk_length + 12; }
  }

  // Copy IDAT to be contiguous.
  if (decode_size == 0)
  { return !0; }

  decode_address = memory_allocate(decode_size);
  if (decode_address == 0)
  { return !0; }

  while (1)
  {
    unsigned long long chunk_length = portable_network_graphics_read_int(source_address);
    if (portable_network_graphics_compare_IDAT(source_address + 4) == 0)
    {
      portable_network_graphics_copy(source_address + 8, chunk_length, decode_address);
      source_address += chunk_length + 12;
      decode_address += chunk_length;
    }
    else if (portable_network_graphics_compare_IEND(source_address + 4) == 0)
    { break; }
    else
    { source_address += chunk_length + 12; }
  }

  // Translate the contiguous data according to IHDR.
  source_address = decode_address - decode_size;
  unsigned long long compressed_size = decode_size;
  if (((IHDR_address[16] ^ 8) | IHDR_address[17] | IHDR_address[18] | IHDR_address[19] | IHDR_address[20]) == 0)
  {
    unsigned long long filtered_size = portable_network_graphics_read_int(IHDR_address + 8);
    filtered_size += 1;
    filtered_size *= portable_network_graphics_read_int(IHDR_address + 12);
    if (filtered_size == 0)
    { memory_free(compressed_size); return !0; }

    decode_address = memory_allocate(filtered_size);
    if (decode_address == 0)
    { memory_free(compressed_size); return !0; }

    if (portable_network_graphics_inflate(source_address, decode_address) != 0)
    { memory_free(filtered_size); memory_free(compressed_size); return !0; }

    decode_size = 0;
    unsigned long long filter_type = 0;
    unsigned long long width = portable_network_graphics_read_int(IHDR_address + 8);
    unsigned long long scanline_size = width + 1;
    while (decode_size != filtered_size)
    {
      if (decode_size % scanline_size == 0)
      {
        filter_type = *decode_address;
        decode_address += 1;
        decode_size += 1;
      }
      else if (filter_type == 0)
      {
        unsigned long long sample = *decode_address * 257;
        *(unsigned short*)destination_address = sample;
        *(unsigned short*)(destination_address + 2) = sample;
        *(unsigned short*)(destination_address + 4) = sample;
        *(unsigned short*)(destination_address + 6) = 65535;

        if ((tRNS_address != 0) && (tRNS_address[9] * 257 == sample))
        { *(unsigned short*)(destination_address + 6) = 0; }

        destination_address += 8;
        decode_address += 1;
        decode_size += 1;
      }
      else if (filter_type == 1)
      {
        unsigned long long sample = *decode_address * 257;
        if (decode_size % scanline_size >= 5)
        {
          sample += *(unsigned short*)(destination_address - 8);
          sample %= 65536;
        }
        *(unsigned short*)destination_address = sample;
        *(unsigned short*)(destination_address + 2) = sample;
        *(unsigned short*)(destination_address + 4) = sample;
        *(unsigned short*)(destination_address + 6) = 65535;

        if ((tRNS_address != 0) && (tRNS_address[9] * 257 == sample))
        { *(unsigned short*)(destination_address + 6) = 0; }

        destination_address += 8;
        decode_address += 1;
        decode_size += 1;
      }
      else if (filter_type == 2)
      {
        unsigned long long sample = *decode_address * 257;
        if (decode_size >= scanline_size)
        {
          sample += *(unsigned short*)(destination_address - (width * 8) - 1);
          sample %= 65536;
        }
        *(unsigned short*)destination_address = sample;
        *(unsigned short*)(destination_address + 2) = sample;
        *(unsigned short*)(destination_address + 4) = sample;
        // to do
      }
      else if (filter_type == 3)
      {
        // to do
      }
      else if (filter_type == 4)
      {
        // to do
      }
      else
      { memory_free(filtered_size); memory_free(compressed_size); return !0; }
    }
    memory_free(filtered_size);
    memory_free(compressed_size);
    return 0;
  }
  else if (((IHDR_address[16] ^ 16) | IHDR_address[17] | IHDR_address[18] | IHDR_address[19] | IHDR_address[20]) == 0)
  {
    unsigned long long filtered_size = portable_network_graphics_read_int(IHDR_address + 8);
    filtered_size = (filtered_size * 2) + 1;
    filtered_size *= portable_network_graphics_read_int(IHDR_address + 16);
    // to do
  }
  else if (((IHDR_address[16] ^ 8) | (IHDR_address[17] ^ 2) | IHDR_address[18] | IHDR_address[19] | IHDR_address[20]) == 0)
  {
    unsigned long long filtered_size = portable_network_graphics_read_int(IHDR_address + 8);
    filtered_size = (filtered_size * 3) + 1;
    filtered_size *= portable_network_graphics_read_int(IHDR_address + 16);
    // to do
  }
  else if (((IHDR_address[16] ^ 16) | (IHDR_address[17] ^ 2) | IHDR_address[18] | IHDR_address[19] | IHDR_address[20]) == 0)
  {
    unsigned long long filtered_size = portable_network_graphics_read_int(IHDR_address + 8);
    filtered_size = (filtered_size * 6) + 1;
    filtered_size *= portable_network_graphics_read_int(IHDR_address + 16);
    // to do
  }
  else if (((IHDR_address[16] ^ 8) | (IHDR_address[17] ^ 3) | IHDR_address[18] | IHDR_address[19] | IHDR_address[20]) == 0)
  {
    unsigned long long filtered_size = portable_network_graphics_read_int(IHDR_address + 8);
    filtered_size += 1;
    filtered_size *= portable_network_graphics_read_int(IHDR_address + 16);
    // to do
  }
  else if (((IHDR_address[16] ^ 8) | (IHDR_address[17] ^ 4) | IHDR_address[18] | IHDR_address[19] | IHDR_address[20]) == 0)
  {
    unsigned long long filtered_size = portable_network_graphics_read_int(IHDR_address + 8);
    filtered_size = (filtered_size * 2) + 1;
    filtered_size *= portable_network_graphics_read_int(IHDR_address + 16);
    // to do
  }
  else if (((IHDR_address[16] ^ 16) | (IHDR_address[17] ^ 4) | IHDR_address[18] | IHDR_address[19] | IHDR_address[20]) == 0)
  {
    unsigned long long filtered_size = portable_network_graphics_read_int(IHDR_address + 8);
    filtered_size = (filtered_size * 4) + 1;
    filtered_size *= portable_network_graphics_read_int(IHDR_address + 16);
    // to do
  }
  else if (((IHDR_address[16] ^ 8) | (IHDR_address[17] ^ 6) | IHDR_address[18] | IHDR_address[19] | IHDR_address[20]) == 0)
  {
    unsigned long long filtered_size = portable_network_graphics_read_int(IHDR_address + 8);
    filtered_size = (filtered_size * 4) + 1;
    filtered_size *= portable_network_graphics_read_int(IHDR_address + 12);
    if (filtered_size == 0)
    { memory_free(compressed_size); return !0; }

    decode_address = memory_allocate(filtered_size);
    if (decode_address == 0)
    { memory_free(compressed_size); return !0; }

    if (portable_network_graphics_inflate(source_address, decode_address) != 0)
    { memory_free(filtered_size); memory_free(compressed_size); return !0; }
console_append_character(45); console_append_character(45); console_append_character(45);
console_append_character(13); console_append_character(10);
console_append_character(67); console_append_character(13); console_append_character(10);
console_write();
    decode_size = 0;
    unsigned long long filter_type = 0;
    unsigned long long width = portable_network_graphics_read_int(IHDR_address + 8);
    unsigned long long scanline_size = (width * 4) + 1;
    while (decode_size < filtered_size)
    {
      if (decode_size % scanline_size == 0)
      {
        filter_type = *decode_address;
        decode_address += 1;
        decode_size += 1;
      }
      else if (filter_type == 0)
      {
        *(unsigned short*)destination_address = decode_address[0] * 257;
        destination_address += 2;
        decode_address += 1;
        decode_size += 1;
      }
      else if (filter_type == 1)
      {
        unsigned long long sample = *decode_address * 257;
        if (decode_size % scanline_size >= 5)
        { sample += *(unsigned short*)(destination_address - 8); }
        *(unsigned short*)destination_address = sample;
        destination_address += 2;
        decode_address += 1;
        decode_size += 1;
      }
      else if (filter_type == 2)
      {
        unsigned long long sample = *decode_address * 257;
        if (decode_size >= scanline_size)
        { sample += *(unsigned short*)(destination_address - scanline_size); }
        *(unsigned short*)destination_address = sample;
        destination_address += 2;
        decode_address += 1;
        decode_size += 1;
      }
      else if (filter_type == 3)
      {
        unsigned long long sample = *decode_address * 257;
        unsigned long long addend = 0;

        if (decode_size % scanline_size >= 5)
        { addend = *(unsigned short*)(destination_address - 8); }

        if (decode_size >= scanline_size)
        { addend += *(unsigned short*)(destination_address - scanline_size); }

        sample += (addend % 65536) >> 1;
        *(unsigned short*)destination_address = sample;
        destination_address += 2;
        decode_address += 1;
        decode_size += 1;
      }
      else if (filter_type == 4)
      {
        unsigned long long sample = *decode_address * 257;

        unsigned long long horizontal = 0;
        if (decode_size % scanline_size >= 5)
        { horizontal = *(unsigned short*)(destination_address - 8); }

        unsigned long long vertical = 0;
        if (decode_size >= scanline_size)
        { vertical = *(unsigned short*)(destination_address - scanline_size); }

        unsigned long long diagonal = 0;
        if ((decode_size % scanline_size >= 5) && (decode_size >= scanline_size))
        { diagonal = *(unsigned short*)(destination_address - scanline_size - 8); }

        sample += portable_network_graphics_paeth(horizontal, vertical, diagonal) * 257;
        *(unsigned short*)destination_address = sample % 65536;
        destination_address += 2;
        decode_address += 1;
        decode_size += 1;
      }
      else
      { memory_free(filtered_size); memory_free(compressed_size); return !0; }
    }
    memory_free(filtered_size);
    memory_free(compressed_size);
    return 0;
  }
  else if (((IHDR_address[16] ^ 16) | (IHDR_address[17] ^ 6) | IHDR_address[18] | IHDR_address[19] | IHDR_address[20]) == 0)
  {
    unsigned long long inflate_size = portable_network_graphics_read_int(IHDR_address + 8);
    inflate_size = (inflate_size * 8) + 1;
    inflate_size *= portable_network_graphics_read_int(IHDR_address + 16);
    // to do
  }
  memory_free(compressed_size);
  return !0;
}