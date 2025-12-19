/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 TheSuperHackers
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "always.h"
#include "utf8.h"


// Returns true if c is a single-byte UTF-8 character (i.e., 0xxxxxxx).
static bool utf8_is_single_byte(char c)
{
  return (c & 0x80) == 0x0;
}

// Returns true if c is a UTF-8 lead byte (i.e., 110xxxxx, 1110xxxx, or 11110xxx).
static bool utf8_is_lead_byte(char c)
{
  unsigned char uc = static_cast<unsigned char>(c);
  return uc >= 0xC2 && uc <= 0xF7;
}

// Returns true if c is a UTF-8 trail byte (i.e., 10xxxxxx).
static bool utf8_is_trail_byte(char c)
{
  return (c & 0xC0) == 0x80;
}

// Returns true if c is an invalid UTF-8 value (overlong encoding or 5/6 byte sequences).
static bool utf8_is_invalid_value(char c)
{
  unsigned char uc = static_cast<unsigned char>(c);
  return uc == 0xC0 || uc == 0xC1 || uc >= 0xF8;
}

size_t utf8_num_bytes(char lead)
{
  if (utf8_is_single_byte(lead))
  {
    return 1;
  }

  if (utf8_is_lead_byte(lead))
  {
    if ((lead & 0xE0) == 0xC0)
    {
      return 2;
    }
    else if ((lead & 0xF0) == 0xE0)
    {
      return 3;
    }
    else if ((lead & 0xF8) == 0xF0)
    {
      return 4;
    }
  }

  // Invalid lead byte
  return 0;
}

// Validates a 2-byte UTF-8 sequence given the lead byte and one trail byte.
static bool utf8_is_valid_lead_and_trail_2(char lead, char trail)
{
  return utf8_is_lead_byte(lead) && utf8_num_bytes(lead) == 2 && utf8_is_trail_byte(trail);
}

// Validates a 3-byte UTF-8 sequence given the lead byte and two trail bytes.
static bool utf8_is_valid_lead_and_trail_3(char lead, char trail1, char trail2)
{
  return utf8_is_lead_byte(lead) && utf8_num_bytes(lead) == 3 && utf8_is_trail_byte(trail1) && utf8_is_trail_byte(trail2);
}

// Validates a 4-byte UTF-8 sequence given the lead byte and three trail bytes.
static bool utf8_is_valid_lead_and_trail_4(char lead, char trail1, char trail2, char trail3)
{
  return utf8_is_lead_byte(lead) && utf8_num_bytes(lead) == 4 && utf8_is_trail_byte(trail1) && utf8_is_trail_byte(trail2) && utf8_is_trail_byte(trail3);
}

bool utf8_validate_string(const char* str)
{
  if (!str)
  {
    WWDEBUG_WARNING(("utf8_validate_string: str is null"));
    return false;
  }

  size_t len = strlen_t(str);
  return utf8_validate_string(str, len);
}

bool utf8_validate_string(const char* str, const size_t length)
{
  if (!str)
  {
    WWDEBUG_WARNING(("utf8_validate_string: str is null"));
    return false;
  }

  size_t i = 0;
  while (i < length)
  {
    const char c = str[i];

    if (utf8_is_single_byte(c))
    {
      i++;
    }
    else if (utf8_is_invalid_value(c))
    {
      // Invalid byte value
      WWDEBUG_WARNING(("utf8_validate_string: failed, offset=%u, invalid byte value", i));
      return false;
    }
    else if (utf8_is_lead_byte(c))
    {
      const size_t num_bytes = utf8_num_bytes(c);

      if (num_bytes == 0 || i + num_bytes > length)
      {
        WWDEBUG_WARNING(("utf8_validate_string: failed, offset=%u, num_bytes=%u", i, num_bytes));
        return false;
      }

      if (num_bytes == 2)
      {
        if (!utf8_is_valid_lead_and_trail_2(c, str[i + 1]))
        {
          WWDEBUG_WARNING(("utf8_validate_string: failed, offset=%u, invalid lead and trail 2", i));
          return false;
        }
      }
      else if (num_bytes == 3)
      {
        if (!utf8_is_valid_lead_and_trail_3(c, str[i + 1], str[i + 2]))
        {
          WWDEBUG_WARNING(("utf8_validate_string: failed, offset=%u, invalid lead and trail 3", i));
          return false;
        }
      }
      else if (num_bytes == 4)
      {
        if (!utf8_is_valid_lead_and_trail_4(c, str[i + 1], str[i + 2], str[i + 3]))
        {
          WWDEBUG_WARNING(("utf8_validate_string: failed, offset=%u, invalid lead and trail 4", i));
          return false;
        }
      }

      i += num_bytes;
    }
    else
    {
      // Invalid byte (trail byte without lead)
      WWDEBUG_WARNING(("utf8_validate_string: failed, offset=%u, trail without lead", i));
      return false;
    }
  }

  return true;
}

int utf8_truncate_if_incomplete(const char* str, size_t length)
{
  if (length == 0)
  {
    return 0;
  }

  char c = str[length - 1];
  if (utf8_is_single_byte(c))
  {
    // No truncation needed
    return 0;
  }
  else if (utf8_is_lead_byte(c))
  {
    // Incomplete sequence; Single lead byte at the end
    return 1;
  }
  else if (utf8_is_trail_byte(c) && (length > 1))
  {
    char c2 = str[length - 2];
    if (utf8_is_lead_byte(c2))
    {
      if (utf8_is_valid_lead_and_trail_2(c2, c))
      {
        return 0;
      }

      // Incomplete sequence
      return 2;
    }
    else if (utf8_is_trail_byte(c2) && (length > 2))
    {
      char c3 = str[length - 3];
      if (utf8_is_lead_byte(c3))
      {
        if (utf8_is_valid_lead_and_trail_3(c3, c2, c))
        {
          return 0;
        }

        // Incomplete sequence
        return 3;
      }
      else if (utf8_is_trail_byte(c3) && (length > 3))
      {
        char c4 = str[length - 4];
        if (utf8_is_lead_byte(c4))
        {
          if (utf8_is_valid_lead_and_trail_4(c4, c3, c2, c))
          {
            return 0;
          }

          // Invalid sequence
          return 4;
        }
      }
    }
  }

  // The sequence is incomplete - for example of length 2 and both are trailing bytes,
  // so we can't determine how many bytes to truncate.
  return 0;
}

#ifdef _WIN32
size_t get_size_as_utf8(const wchar_t* s)
{
  int reqBytes = WideCharToMultiByte(CP_UTF8, 0, s, -1, nullptr, 0, nullptr, nullptr);
  if (reqBytes == 0)
  {
    WWDEBUG_WARNING(("get_size_as_utf8: WideCharToMultiByte failed with error: %u", GetLastError()));
  }

  return reqBytes;
}

size_t get_size_as_widechar(const char* s)
{
  int reqChars = MultiByteToWideChar(CP_UTF8, 0, s, -1, nullptr, 0);
  if (reqChars == 0)
  {
    WWDEBUG_WARNING(("get_size_as_widechar: MultiByteToWideChar failed with error: %u", GetLastError()));
  }

  return reqChars * sizeof(wchar_t);
}

size_t convert_widechar_to_utf8(const wchar_t* orig, char* tgt, size_t tgtsize)
{
  WWASSERT_PRINT(tgtsize >= get_size_as_utf8(orig), "Insufficient buffer for UTF8 conversion");
  int bytesWritten = WideCharToMultiByte(CP_UTF8, 0, orig, -1, tgt, tgtsize, NULL, NULL);
  if (bytesWritten == 0)
  {
    WWDEBUG_WARNING(("convert_widechar_to_utf8: WideCharToMultiByte failed with error: %u", GetLastError()));
  }

  return bytesWritten;
}

size_t convert_utf8_to_widechar(const char* orig, wchar_t* tgt, size_t tgtsize)
{
  WWASSERT_PRINT(tgtsize >= get_size_as_widechar(orig), "Insufficient buffer for widechar conversion");
  int charsWritten = MultiByteToWideChar(CP_UTF8, 0, orig, -1, tgt, static_cast<int>(tgtsize / sizeof(wchar_t)));
  if (charsWritten == 0)
  {
    WWDEBUG_WARNING(("convert_utf8_to_widechar: MultiByteToWideChar failed with error: %u", GetLastError()));
  }

  return charsWritten * sizeof(wchar_t);
}
#else
#error "UTF-8 conversion functions not implemented for this platform"
#endif
