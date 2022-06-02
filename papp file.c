struct papp_file_header
{
  unsigned int width;
  unsigned int height;
  unsigned int depth;
  unsigned int trength;
};


// Returns the smallest difference from origin to target, in the range 0 to 65535, where 65535 + 1 = 0.
signed int papp_file_difference(unsigned int origin, unsigned int target)
{
  signed int difference = origin - target;
  if ((-32768 < difference) && (32768 >= difference))
  { return 0 - difference; }
  return 65536 - difference;
}


// Returns the amount of bytes needed to encode a papp file from buffer_address and buffer_size.
unsigned int papp_file_encode_size(unsigned char* buffer_address, unsigned int buffer_size)
{
  unsigned int file_size = 16;
  unsigned int red = 0;
  unsigned int green = 0;
  unsigned int blue = 0;
  unsigned int opacity = 0;
  while (buffer_size >= 8)
  {
    unsigned int red_target = buffer_address[0] + (buffer_address[1] << 8);
    signed int red_difference = papp_file_difference(red, red_target);
    unsigned int green_target = buffer_address[2] + (buffer_address[3] << 8);
    signed int green_difference = papp_file_difference(green, green_target)
    unsigned int blue_target = buffer_address[4] + (buffer_address[5] << 8);
    signed int blue_difference = papp_file_difference(blue, blue_target)
    unsigned int opacity_target = buffer_address[6] + (buffer_address[7] << 8);
    signed int opacity_difference = papp_file_difference(opacity, opacity_target)

    if (red_difference < 0)
    { red_difference = ~red_difference | 1; }
    if (green_difference < 0)
    { green_difference = ~green_difference | 1; }
    if (blue_difference < 0)
    { blue_difference = ~blue_difference | 1; }
    if (opacity_difference < 0)
    { opacity_difference = ~opacity_difference | 1; }

    red_difference |= green_difference | blue_difference | opacity_difference;
    if (red_difference >> 13 != 0)
    { file_size += 12; }
    else if (red_difference >> 6 != 0)
    { file_size += 8; }
    else if (red_difference != 0)
    { file_size += 4; }

    red = red_target;
    green = green_target;
    blue = blue_target;
    opacity = opacity_target;
    buffer_address += 8;
    buffer_size -= 8;
  }
  return file_size;
}


void papp_file_encode(unsigned char* buffer_address, unsigned int buffer_size, unsigned char* file_address)
{
  unsigned char occurrence_count = 0;
  unsigned int red = 0;
  unsigned int green = 0;
  unsigned int blue = 0;
  unsigned int opacity = 0;
  file_address += 16;
  while (buffer_size >= 8)
  {
    unsigned int red_target = buffer_address[0] + (buffer_address[1] << 8);
    signed int red_difference = papp_file_difference(red, red_target);
    unsigned int green_target = buffer_address[2] + (buffer_address[3] << 8);
    signed int green_difference = papp_file_difference(green, green_target)
    unsigned int blue_target = buffer_address[4] + (buffer_address[5] << 8);
    signed int blue_difference = papp_file_difference(blue, blue_target)
    unsigned int opacity_target = buffer_address[6] + (buffer_address[7] << 8);
    signed int opacity_difference = papp_file_difference(opacity, opacity_target)

    signed int difference = red_difference;
    if (red_difference < 0)
    { difference = ~red_difference | 1; }
    if (green_difference < 0)
    { difference |= ~green_difference | 1; }
    if (blue_difference < 0)
    { difference |= ~blue_difference | 1; }
    if (opacity_difference < 0)
    { difference |= ~opacity_difference | 1; }

    if ((difference == 0) && (occurrence_count != 15))
    {
      occurrence_count += 1;
      buffer_address += 8;
      buffer_size -= 8;
    }
    else if ((difference >> 6) == 0)
    {
      unsigned int chunk = occurrence_count + 1;
      chunk += (red_difference & 127) << 4;
      chunk += (green_difference & 127) << 11;
      chunk += (blue_difference & 127) << 18;
      chunk += (opacity_difference) << 25;
      file_address[0] = chunk;
      file_address[1] = chunk >> 8;
      file_address[2] = chunk >> 16;
      file_address[3] = chunk >> 24;

      occurrence_count = 0;
      red = red_target;
      green = green_target;
      blue = blue_target;
      opacity = opacity_target;
      buffer_address += 8;
      buffer_size -= 8;
      file_address += 4;
    }
    else if ((difference >> 13) == 0)
    {
      unsigned int chunk = (red_difference & 127) << 4;
      chunk += (green_difference & 127) << 11;
      chunk += (blue_difference & 127) << 18;
      chunk += (opacity_difference & 127) << 25;
      file_address[0] = chunk;
      file_address[1] = chunk >> 8;
      file_address[2] = chunk >> 16;
      file_address[3] = chunk >> 24;

      red_difference >>= 7;
      green_difference >>= 7;
      blue_difference >>= 7;
      opacity_difference >>= 7;

      chunk = occurrence_count + 1;
      chunk += (red_difference & 127) << 4;
      chunk += (green_difference & 127) << 11;
      chunk += (blue_difference & 127) << 18;
      chunk += (opacity_difference & 127) << 25;
      file_address[4] = chunk;
      file_address[5] = chunk >> 8;
      file_address[6] = chunk >> 16;
      file_address[7] = chunk >> 24;

      occurrence_count = 0;
      red = red_target;
      green = green_target;
      blue = blue_target;
      opacity = opacity_target;
      buffer_address += 8;
      buffer_size -= 8;
      file_address += 8;
    }
    else
    {
      unsigned int chunk = (red_difference & 127) << 4;
      chunk += (green_difference & 127) << 11;
      chunk += (blue_difference & 127) << 18;
      chunk += (opacity_difference & 127) << 25;
      file_address[0] = chunk;
      file_address[1] = chunk >> 8;
      file_address[2] = chunk >> 16;
      file_address[3] = chunk >> 24;

      red_difference >>= 7;
      green_difference >>= 7;
      blue_difference >>= 7;
      opacity_difference >>= 7;

      chunk = (red_difference & 127) << 4;
      chunk += (green_difference & 127) << 11;
      chunk += (blue_difference & 127) << 18;
      chunk += (opacity_difference & 127) << 25;
      file_address[4] = chunk;
      file_address[5] = chunk >> 8;
      file_address[6] = chunk >> 16;
      file_address[7] = chunk >> 24;

      red_difference >>= 7;
      green_difference >>= 7;
      blue_difference >>= 7;
      opacity_difference >>= 7;

      chunk = (red_difference & 127) << 4;
      chunk += (green_difference & 127) << 11;
      chunk += (blue_difference & 127) << 18;
      chunk += (opacity_difference & 127) << 25;
      file_address[8] = chunk;
      file_address[9] = chunk >> 8;
      file_address[10] = chunk >> 16;
      file_address[11] = chunk >> 24;

      occurrence_count = 0;
      red = red_target;
      green = green_target;
      blue = blue_target;
      opacity = opacity_target;
      buffer_address += 8;
      buffer_size -= 8;
      file_address += 12;
    }
  }
}


unsigned int papp_file_decode_size(unsigned char* file_address)
{
  // to do
  return 0;
}


void papp_file_decode(unsigned char* file_address, unsigned char* buffer_address)
{
  // to do
  return 0;
}