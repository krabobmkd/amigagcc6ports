/*
 * HEIF codec.
 * Copyright (c) 2022 Dirk Farin <dirk.farin@gmail.com>
 *
 * This file is part of libheif.
 *
 * libheif is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * libheif is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with libheif.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cassert>
#include "exif.h"

#define EXIF_TYPE_SHORT 3
#define DEFAULT_EXIF_ORIENTATION 1
#define EXIF_TAG_ORIENTATION 0x112

// Note: As far as I can see, it is not defined in the EXIF standard whether the offsets and counts of the IFD is signed or unsigned.
// We assume that these are all unsigned.

static uint32_t read32(const uint8_t* data, uint32_t size, uint32_t pos, bool littleEndian)
{
  assert(pos <= size - 4);

  const uint8_t* p = data + pos;

  if (littleEndian) {
    return (p[3] << 24) | (p[2] << 16) | (p[1] << 8) | p[0];
  }
  else {
    return (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
  }
}


static uint16_t read16(const uint8_t* data, uint32_t size, uint32_t pos, bool littleEndian)
{
  assert(pos <= size - 2);

  const uint8_t* p = data + pos;

  if (littleEndian) {
    return static_cast<uint16_t>((p[1] << 8) | p[0]);
  }
  else {
    return static_cast<uint16_t>((p[0] << 8) | p[1]);
  }
}


static void write16(uint8_t* data, uint32_t size, uint32_t pos, uint16_t value, bool littleEndian)
{
  assert(pos <= size - 2);

  uint8_t* p = data + pos;

  if (littleEndian) {
    p[0] = (uint8_t) (value & 0xFF);
    p[1] = (uint8_t) (value >> 8);
  }
  else {
    p[0] = (uint8_t) (value >> 8);
    p[1] = (uint8_t) (value & 0xFF);
  }
}

// Returns 0 if the query_tag was not found.
static uint32_t find_exif_tag(const uint8_t* exif, uint32_t size, uint16_t query_tag, bool* out_littleEndian)
{
  if (size < 4) {
    return 0;
  }

  if ((exif[0] != 'I' && exif[0] != 'M') ||
      (exif[1] != 'I' && exif[1] != 'M')) {
    return 0;
  }

  bool littleEndian = (exif[0] == 'I');

  assert(out_littleEndian);
  *out_littleEndian = littleEndian;

  uint32_t offset = read32(exif, size, 4, littleEndian);

  if (size - 2 < offset) {
    return 0;
  }

  uint16_t cnt = read16(exif, size, offset, littleEndian);

  // Does the IFD table fit into our memory range? We need this to prevent an underflow in the following statement.
  if (2U + cnt * 12U > size) {
    return 0;
  }

  // end of IFD table would exceed the end of the EXIF data
  if (size - 2U - cnt * 12U > offset) {
    return 0;
  }

  for (int i = 0; i < cnt; i++) {
    int tag = read16(exif, size, offset + 2 + i * 12, littleEndian);
    if (tag == query_tag) {
      return offset + 2 + i * 12;
    }
  }

  // TODO: do we have to also scan the next IFD table ?

  return 0;
}


void modify_exif_tag_if_it_exists(uint8_t* exif, uint32_t size, uint16_t modify_tag, uint16_t modify_value)
{
  bool little_endian;
  uint32_t pos = find_exif_tag(exif, size, modify_tag, &little_endian);
  if (pos == 0) {
    return;
  }

  uint16_t type = read16(exif, size, pos + 2, little_endian);
  uint32_t count = read32(exif, size, pos + 4, little_endian);

  if (type == EXIF_TYPE_SHORT && count == 1) {
    write16(exif, size, pos + 8, modify_value, little_endian);
  }
}


void modify_exif_orientation_tag_if_it_exists(uint8_t* exifData, uint32_t size, uint16_t orientation)
{
  modify_exif_tag_if_it_exists(exifData, size, EXIF_TAG_ORIENTATION, orientation);
}


int read_exif_orientation_tag(const uint8_t* exif, uint32_t size)
{
  bool little_endian;
  uint32_t pos = find_exif_tag(exif, size, EXIF_TAG_ORIENTATION, &little_endian);
  if (pos == 0) {
    return DEFAULT_EXIF_ORIENTATION;
  }

  uint16_t type = read16(exif, size, pos + 2, little_endian);
  uint32_t count = read32(exif, size, pos + 4, little_endian);

  if (type == EXIF_TYPE_SHORT && count == 1) {
    return read16(exif, size, pos + 8, little_endian);
  }

  return DEFAULT_EXIF_ORIENTATION;
}
