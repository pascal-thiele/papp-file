struct papp_file_header
{
  unsigned int width;
  unsigned int height;
  unsigned int depth;
  unsigned int trength;
};


// Returns the amount of bytes needed to encode source_size bytes from source_address to a papp file. Source lists two bytes red, two bytes green, two bytes blue and two bytes opacity.
unsigned long papp_file_encode_size(unsigned char* source_address, unsigned int source_size)
{
  unsigned long file_size = 16;
  unsigned long red = 0;
  unsigned long green = 0;
  unsigned long blue = 0;
  unsigned long opacity = 0;
  while (source_size >= 8)
  {
    // Calculate the smallest difference from the previous cell, where 0 - 1 = 65535.
    unsigned long red_target = *(unsigned short*)source_address;
    signed long red_difference = 65535 & (65536 + (red_target - red));
    signed long inverse = 65535 & (65536 - (red_target - red));
    if (red_difference >= inverse)
    { red_difference = -inverse; }

    unsigned long green_target = *(unsigned short*)(source_address + 2);
    signed long green_difference = 65535 & (65536 + (green_target - green));
    inverse = 65535 & (65536 - (green_target - green));
    if (green_difference >= inverse)
    { green_difference = -inverse; }

    unsigned long blue_target = *(unsigned short*)(source_address + 4);
    signed long blue_difference = 65535 & (65536 + (blue_target - blue));
    inverse = 65535 & (65536 - (blue_target - blue));
    if (blue_difference >= inverse)
    { blue_difference = -inverse; }

    unsigned long opacity_target = *(unsigned short*)(source_address + 6);
    signed long opacity_difference = 65535 & (65536 + (opacity_target - opacity));
    inverse = 65535 & (65536 - (opacity_target - opacity));
    if (opacity_difference >= inverse)
    { opacity_difference = -inverse; }

    // Keep the color for following cells.
    red = red_target;
    green = green_target;
    blue = blue_target;
    opacity = opacity_target;

    // Toggles the bits of negative values and prevents negative values to become zero.
    red_difference ^= red_difference >> 14;
    green_difference ^= green_difference >> 14;
    blue_difference ^= blue_difference >> 14;
    opacity_difference ^= opacity_difference >> 14;

    // Counts the smallest amount of chunks to encode the difference.
    red_difference |= green_difference | blue_difference | opacity_difference;
    if (red_difference >> 13 != 0)
    { file_size += 12; }
    else if (red_difference >> 6 != 0)
    { file_size += 8; }
    else if (red_difference != 0)
    { file_size += 4; }

    source_address += 8;
    source_size -= 8;
  }
  return file_size;
}


// Encodes source_size bytes from source_address to a papp file at destination_address. Source list two bytes red, two bytes green, two bytes blue and two bytes opacity.
void papp_file_encode(unsigned char* source_address, unsigned long source_size, unsigned char* destination_address)
{
  unsigned long red = 0;
  unsigned long green = 0;
  unsigned long blue = 0;
  unsigned long opacity = 0;
  destination_address += 16;
  while (source_size >= 8)
  {
    // Calculate the smallest difference from the previous cell, where 0 - 1 = 65535.
    unsigned long red_target = *(unsigned short*)source_address;
    signed long red_difference = 65535 & (65536 + (red_target - red));
    signed long inverse = 65535 & (65536 - (red_target - red));
    if (red_difference >= inverse)
    { red_difference = -inverse; }

    unsigned long green_target = *(unsigned short*)(source_address + 2);
    signed long green_difference = 65535 & (65536 + (green_target - green));
    inverse = 65535 & (65536 - (green_target - green));
    if (green_difference >= inverse)
    { green_difference = -inverse; }

    unsigned long blue_target = *(unsigned short*)(source_address + 4);
    signed long blue_difference = 65535 & (65536 + (blue_target - blue));
    inverse = 65535 & (65536 - (blue_target - blue));
    if (blue_difference >= inverse)
    { blue_difference = -inverse; }

    unsigned long opacity_target = *(unsigned short*)(source_address + 6);
    signed long opacity_difference = 65535 & (65536 + (opacity_target - opacity));
    inverse = 65535 & (65536 - (opacity_target - opacity));
    if (opacity_difference >= inverse)
    { opacity_difference = -inverse;}

    // Keep the color for following cells.
    red = red_target;
    green = green_target;
    blue = blue_target;
    opacity = opacity_target;

    // Count color occurrences in following cells.
    unsigned long occurrence_count = 1;
    source_address += 8;
    source_size -= 8;
    while ((occurrence_count < 15) && (source_size >= 8)
      && (red == *(unsigned short*)source_address)
      && (green == *(unsigned short*)(source_address + 2))
      && (blue == *(unsigned short*)(source_address + 4))
      && (opacity == *(unsigned short*)(source_address + 6)))
    {
      occurrence_count += 1;
      source_address += 8;
      source_size -= 8;
    }

    // Toggles the bits of negative values and prevents negative values to become zero.
    signed long mask = red_difference ^ (red_difference >> 14);
    mask |= green_difference ^ (green_difference >> 14);
    mask |= blue_difference ^ (blue_difference >> 14);
    mask |= opacity_difference ^ (opacity_difference >> 14);

    // Writes the smallest amount of chunks to encode the difference.
    if ((mask >> 6) == 0)
    {
      unsigned int chunk = occurrence_count;
      chunk += (red_difference & 127) << 4;
      chunk += (green_difference & 127) << 11;
      chunk += (blue_difference & 127) << 18;
      chunk += (opacity_difference) << 25;
      *(unsigned int*)destination_address = chunk;
      destination_address += 4;
    }
    else if ((mask >> 13) == 0)
    {
      unsigned int chunk = (red_difference & 127) << 4;
      chunk += (green_difference & 127) << 11;
      chunk += (blue_difference & 127) << 18;
      chunk += (opacity_difference & 127) << 25;
      *(unsigned int*)destination_address = chunk;

      // Increment the second chunk by one if the first chunk is negative.
      red_difference = (red_difference + 64) >> 7;
      green_difference = (green_difference + 64) >> 7;
      blue_difference = (blue_difference + 64) >> 7;
      opacity_difference = (opacity_difference + 64) >> 7;

      chunk = occurrence_count;
      chunk += (red_difference & 127) << 4;
      chunk += (green_difference & 127) << 11;
      chunk += (blue_difference & 127) << 18;
      chunk += (opacity_difference & 127) << 25;
      *(unsigned int*)(destination_address + 4) = chunk;
      destination_address += 8;
    }
    else
    {
      unsigned int chunk = (red_difference & 127) << 4;
      chunk += (green_difference & 127) << 11;
      chunk += (blue_difference & 127) << 18;
      chunk += (opacity_difference & 127) << 25;
      *(unsigned int*)destination_address = chunk;

      // Increment the second chunk by one if the first chunk is negative.
      red_difference = (red_difference + 64) >> 7;
      green_difference = (green_difference + 64) >> 7;
      blue_difference = (blue_difference + 64) >> 7;
      opacity_difference = (opacity_difference + 64) >> 7;

      chunk = (red_difference & 127) << 4;
      chunk += (green_difference & 127) << 11;
      chunk += (blue_difference & 127) << 18;
      chunk += (opacity_difference & 127) << 25;
      *(unsigned int*)(destination_address + 4) = chunk;

      // Increment the third chunk by one if the second chunk is negative.
      red_difference = (red_difference + 64) >> 7;
      green_difference = (green_difference + 64) >> 7;
      blue_difference = (blue_difference + 64) >> 7;
      opacity_difference = (opacity_difference + 64) >> 7;

      chunk = occurrence_count;
      chunk += (red_difference & 127) << 4;
      chunk += (green_difference & 127) << 11;
      chunk += (blue_difference & 127) << 18;
      chunk += (opacity_difference & 127) << 25;
      *(unsigned int*)(destination_address + 8) = chunk;
      destination_address += 12;
    }
  }
}


// Returns the amount of bytes needed to decode the papp file from address to a list of two bytes red, two bytes green, two bytes blue and two bytes opacity.
unsigned long papp_file_decode_size(unsigned char* address)
{
  unsigned long width = *(unsigned int*)address;
  unsigned long height = *(unsigned int*)(address + 4);
  unsigned long depth = *(unsigned int*)(address + 8);
  unsigned long trength = *(unsigned int*)(address + 12);
  unsigned long decode_size = width * height * depth * trength * 8;
  if ((decode_size != 0) && (width < 18446744073709551615 / height / depth / trength / 8))
  { return decode_size; }
  return 0;
}


// Decodes a papp file from source_address to destination_address as a list of two bytes red, two bytes green, two bytes blue and two bytes opacity.
void papp_file_decode(unsigned char* source_address, unsigned char* destination_address)
{
  unsigned long width = *(unsigned int*)source_address;
  unsigned long height = *(unsigned int*)(source_address + 4);
  unsigned long depth = *(unsigned int*)(source_address + 8);
  unsigned long trength = *(unsigned int*)(source_address + 12);
  unsigned long cell_count = width * height * depth * trength;
  if ((cell_count != 0) && (width < 18446744073709551615 / height / depth / trength))
  {
    unsigned long red = 0;
    unsigned long green = 0;
    unsigned long blue = 0;
    unsigned long opacity = 0;
    source_address += 16;
    while (cell_count != 0)
    {
      unsigned long occurrence_count = *(unsigned int*)source_address;
      signed long red_difference = (occurrence_count >> 4) & 127;
      red_difference |= 0 - (red_difference & 64);
      signed long green_difference = (occurrence_count >> 11) & 127;
      green_difference |= 0 - (green_difference & 64);
      signed long blue_difference = (occurrence_count >> 18) & 127;
      blue_difference |= 0 - (blue_difference & 64);
      signed long opacity_difference = (occurrence_count >> 25) & 127;
      opacity_difference |= 0 - (opacity_difference & 64);

      occurrence_count &= 15;
      source_address += 4;
      if (occurrence_count == 0)
      {
        // Add the second chunk of the cell.
        occurrence_count = *(unsigned int*)source_address;
        signed long addend = (occurrence_count >> 4) & 127;
        addend |= 0 - (addend & 64);
        red_difference += addend << 7;
        
        addend = (occurrence_count >> 11) & 127;
        addend |= 0 - (addend & 64);
        green_difference += addend << 7;
        
        addend = (occurrence_count >> 18) & 127;
        addend |= 0 - (addend & 64);
        blue_difference += addend << 7;

        addend = (occurrence_count >> 25) & 127;
        addend |= 0 - (addend & 64);
        opacity_difference += addend << 7;

        occurrence_count &= 15;
        source_address += 4;
        if (occurrence_count == 0)
        {
          // Add the third chunk of the cell.
          occurrence_count = *(unsigned int*)source_address;
          addend = (occurrence_count >> 4) & 127;
          addend |= 0 - (addend & 64);
          red_difference += addend << 14;

          addend = (occurrence_count >> 11) & 127;
          addend |= 0 - (addend & 64);
          green_difference += addend << 14;

          addend = (occurrence_count >> 18) & 127;
          addend |= 0 - (addend & 64);
          blue_difference += addend << 14;

          addend = (occurrence_count >> 25) & 127;
          addend |= 0 - (addend & 64);
          opacity_difference += addend << 14;

          occurrence_count &= 15;
          source_address += 4;
        }
      }

      // Add the difference to the current color.
      red = (red + red_difference) & 65535;
      green = (green + green_difference) & 65535;
      blue = (blue + blue_difference) & 65535;
      opacity = (opacity + opacity_difference) & 65535;

      // Write all occurrences.
      while (occurrence_count != 0)
      {
        *(unsigned short*)destination_address = red;
        *(unsigned short*)(destination_address + 2) = green;
        *(unsigned short*)(destination_address + 4) = blue;
        *(unsigned short*)(destination_address + 6) = opacity;
        destination_address += 8;
        occurrence_count -= 1;
      }
    }
  }
}