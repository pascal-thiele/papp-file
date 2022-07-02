#ifndef papp_file
#define papp_file

struct papp_file_header
{
  unsigned int width;
  unsigned int height;
  unsigned int depth;
  unsigned int trength;
};


// Returns the amount of bytes needed to encode source_size bytes from source_address to a papp file.
// The source shall list two bytes red, two bytes green, two bytes blue and two bytes opacity.
unsigned long long papp_file_encode_size(unsigned char* source_address, unsigned long long source_size)
{
  unsigned long long file_size = 16;
  unsigned long long red = 0;
  unsigned long long green = 0;
  unsigned long long blue = 0;
  unsigned long long opacity = 0;
  while (source_size >= 8)
  {
    // Calculate the smallest difference from the previous cell, where 0 - 1 = 65535.
    unsigned long long red_target = *(unsigned short*)source_address;
    signed long long red_difference = 65535 & (65536 + (red_target - red));
    if (red_difference >= 32768)
    { red_difference -= 65536; }

    unsigned long long green_target = *(unsigned short*)(source_address + 2);
    signed long long green_difference = 65535 & (65536 + (green_target - green));
    if (green_difference >= 32768)
    { green_difference -= 65536; }

    unsigned long long blue_target = *(unsigned short*)(source_address + 4);
    signed long long blue_difference = 65535 & (65536 + (blue_target - blue));
    if (blue_difference >= 32768)
    { blue_difference -= 65536; }

    unsigned long long opacity_target = *(unsigned short*)(source_address + 6);
    signed long long opacity_difference = 65535 & (65536 + (opacity_target - opacity));
    if (opacity_difference >= 32768)
    { opacity_difference -= 65536; }

    // Keep the color for following cells.
    red = red_target;
    green = green_target;
    blue = blue_target;
    opacity = opacity_target;

    // Count the smallest amount of chunks to encode the difference.
    unsigned long long mask = red_difference ^ (red_difference >> 15);
    mask |= green_difference ^ (green_difference >> 15);
    mask |= blue_difference ^ (blue_difference >> 15);
    mask |= opacity_difference ^ (opacity_difference >> 15);
    mask &= 32767;
    file_size += 7;
    if (mask >= 4096)
    { file_size += 2; }

    // Skip up to fourteen additional occurrences.
    unsigned long long occurrence_count = 1;
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
  }
  return file_size;
}


// Encodes source_size bytes from source_address to a papp file at destination_address. The source shall list two bytes red, two bytes green, two bytes blue and two bytes opacity. The width, height, depth and trength of the resulting papp file are undefined and have to be set before or after calling this function.
void papp_file_encode(unsigned char* source_address, unsigned long long source_size, unsigned char* destination_address)
{
  destination_address += 16;
  unsigned long long red = 0;
  unsigned long long green = 0;
  unsigned long long blue = 0;
  unsigned long long opacity = 0;
  while (source_size >= 8)
  {
    // Calculate the smallest difference from the previous cell, where 0 - 1 = 65535.
    unsigned long long red_target = *(unsigned short*)source_address;
    signed long long red_difference = 65535 & (65536 + (red_target - red));
    if (red_difference >= 32768)
    { red_difference -= 65536; }

    unsigned long long green_target = *(unsigned short*)(source_address + 2);
    signed long long green_difference = 65535 & (65536 + (green_target - green));
    if (green_difference >= 32768)
    { green_difference -= 65536; }

    unsigned long long blue_target = *(unsigned short*)(source_address + 4);
    signed long long blue_difference = 65535 & (65536 + (blue_target - blue));
    if (blue_difference >= 32768)
    { blue_difference -= 65536; }

    unsigned long long opacity_target = *(unsigned short*)(source_address + 6);
    signed long long opacity_difference = 65535 & (65536 + (opacity_target - opacity));
    if (opacity_difference >= 32768)
    { opacity_difference -= 65536; }

    // Keep the color for following cells.
    red = red_target;
    green = green_target;
    blue = blue_target;
    opacity = opacity_target;

    // Count the occurrences in following cells.
    unsigned long long occurrence_count = 1;
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

    // Write the chunk.
    unsigned long long chunk = opacity_difference;
    chunk = (chunk << 13) | (blue_difference & 8191);
    chunk = (chunk << 13) | (green_difference & 8191);
    chunk = (chunk << 13) | (red_difference & 8191);
    chunk = (chunk << 4) | occurrence_count;
    *destination_address = chunk;
    *(destination_address + 1) = chunk >> 8;
    *(destination_address + 2) = chunk >> 16;
    *(destination_address + 3) = chunk >> 24;
    *(destination_address + 4) = chunk >> 32;
    *(destination_address + 5) = chunk >> 40;
    *(destination_address + 6) = chunk >> 48;
    destination_address += 7;

    // Write rubble if the difference exceeds the range -4096 to +4095.
    unsigned long long mask = red_difference ^ (red_difference >> 15);
    mask |= green_difference ^ (green_difference >> 15);
    mask |= blue_difference ^ (blue_difference >> 15);
    mask |= opacity_difference ^ (opacity_difference >> 15);
    mask &= 32767;
    if (mask >= 4096)
    {
      *destination_address &= 240;
      unsigned long long rubble = ((opacity_difference + 4096) >> 13) & 7;
      rubble = (rubble << 3) | (((blue_difference + 4096) >> 13) & 7);
      rubble = (rubble << 3) | (((green_difference + 4096) >> 13) & 7);
      rubble = (rubble << 3) | (((red_difference + 4096) >> 13) & 7);
      rubble = (rubble << 4) | occurrence_count;
      *destination_address = rubble;
      *(destination_address + 1) = rubble >> 8;
      *(destination_address + 2) = rubble >> 16;
      destination_address += 3;
    }
  }
}


// Returns the amount of bytes needed to decode the papp file from address to a list of two bytes red, two bytes green, two bytes blue and two bytes opacity.
unsigned long long papp_file_decode_size(unsigned char* address)
{
  unsigned long long width = *(unsigned int*)address;
  unsigned long long height = *(unsigned int*)(address + 4);
  unsigned long long depth = *(unsigned int*)(address + 8);
  unsigned long long trength = *(unsigned int*)(address + 12);
  unsigned long long decode_size = width * height * depth * trength * 8;
  if (decode_size != 0)
  {
    unsigned long long overflow = (((2305843009213693951 / width) / height) / depth) / trength;
    if (overflow != 0)
    { return decode_size; }
  }
  return 0;
}


// Decodes a papp file from source_address to destination_address as a list of two bytes red, two bytes green, two bytes blue and two bytes opacity.
void papp_file_decode(unsigned char* source_address, unsigned char* destination_address)
{
  unsigned long long width = *(unsigned int*)source_address;
  unsigned long long height = *(unsigned int*)(source_address + 4);
  unsigned long long depth = *(unsigned int*)(source_address + 8);
  unsigned long long trength = *(unsigned int*)(source_address + 12);
  unsigned long long cell_count = width * height * depth * trength;
  if (cell_count != 0)
  {
    // Ensure all source bytes can be addressed.
    unsigned long long overflow = (((2305843009213693951 / width) / height) / depth) / trength;
    if (overflow != 0)
    {
      source_address += 16;
      unsigned long long red = 0;
      unsigned long long green = 0;
      unsigned long long blue = 0;
      unsigned long long opacity = 0;
      while (cell_count != 0)
      {
        // Read a chunk.
        unsigned long long chunk = *source_address;
        chunk |= (unsigned long long)*(source_address + 1) << 8;
        chunk |= (unsigned long long)*(source_address + 2) << 16;
        chunk |= (unsigned long long)*(source_address + 3) << 24;
        chunk |= (unsigned long long)*(source_address + 4) << 32;
        chunk |= (unsigned long long)*(source_address + 5) << 40;
        chunk |= (unsigned long long)*(source_address + 6) << 48;

        // Extract differences, extract occurrence count and sign extend differences.
        unsigned long long occurrence_count = chunk & 15;
        chunk >>= 4;
        signed long long red_difference = (chunk & 8191) - (chunk & 4096);
        chunk >>= 13;
        signed long long green_difference = (chunk & 8191) - (chunk & 4096);
        chunk >>= 13;
        signed long long blue_difference = (chunk & 8191) - (chunk & 4096);
        chunk >>= 13;
        signed long long opacity_difference = chunk - (chunk & 4096);

        // Read rubble if occurrence count is zero.
        source_address += 7;
        if (occurrence_count == 0)
        {
          unsigned long long rubble = *source_address;
          rubble |= (unsigned long long)*(source_address + 1) << 8;
          rubble |= (unsigned long long)*(source_address + 2) << 16;
          occurrence_count = rubble & 15;
          rubble >>= 4;
          red_difference |= (rubble & 7) << 13;
          rubble >>= 3;
          green_difference |= (rubble & 7) << 13;
          rubble >>= 3;
          blue_difference |= (rubble & 7) << 13;
          rubble >>= 3;
          opacity_difference |= rubble << 13;
          source_address += 3;
        }

        // Add the difference to the current color.
        red += red_difference;
        green += green_difference;
        blue += blue_difference;
        opacity += opacity_difference;

        // Only write occurrences that are contained in cells.
        if (occurrence_count >= cell_count)
        { occurrence_count = cell_count; }

        // Write all occurrences.
        cell_count -= occurrence_count;
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
}
#endif