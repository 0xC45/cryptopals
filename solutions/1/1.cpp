#include <bitset>
#include <iostream>
#include <sstream>
#include <stdexcept>

#define HEX_BITS 4
#define BASE64_BITS 6
#define CHUNK_BITS 24
#define BASE64_CHARS_IN_CHUNK 4

std::bitset<HEX_BITS> hex_to_bin(const char ch);
char base64_to_char(const std::bitset<BASE64_BITS> base64_bits);
std::string chunk_to_base64(const std::bitset<CHUNK_BITS> chunk, int bits_filled);

int main(void)
{
  std::ostringstream base64_os;
  std::bitset<CHUNK_BITS> chunk;
  int chunk_pos = CHUNK_BITS - 1;

  char ch;
  while (std::cin >> ch) {
    // convert hex to binary
    std::bitset<HEX_BITS> hex = hex_to_bin(ch);

    // copy hex bits to chunk
    for (int i = HEX_BITS - 1; i >= 0; i--) {
      chunk[chunk_pos--] = hex[i];
    }

    // once chunk is written, convert to base64 and reset
    if (chunk_pos < 0) {
      base64_os << chunk_to_base64(chunk, CHUNK_BITS);
      chunk_pos = CHUNK_BITS - 1;
      chunk.reset();
    }
  }

  // handle remaining bits, if any
  if (chunk_pos != CHUNK_BITS - 1) {
    base64_os << chunk_to_base64(chunk, CHUNK_BITS - chunk_pos - 1);
  }

  std::cout << base64_os.str() << std::endl;

  return 0;
}

std::bitset<HEX_BITS> hex_to_bin(const char ch)
{
  switch(ch) {
  case '0': return 0;
  case '1': return 1;
  case '2': return 2;
  case '3': return 3;
  case '4': return 4;
  case '5': return 5;
  case '6': return 6;
  case '7': return 7;
  case '8': return 8;
  case '9': return 9;
  case 'a': case 'A': return 10;
  case 'b': case 'B': return 11;
  case 'c': case 'C': return 12;
  case 'd': case 'D': return 13;
  case 'e': case 'E': return 14;
  case 'f': case 'F': return 15;
  default: throw std::invalid_argument("not a hex value");
  }
}

char base64_to_char(const std::bitset<BASE64_BITS> base64_bits)
{
  int base64_decimal = base64_bits.to_ulong();
  if (base64_decimal >= 0 && base64_decimal <= 25) {
    return 'A' + base64_decimal;
  } else if (base64_decimal >= 26 && base64_decimal <= 51) {
    return 'a' + base64_decimal - 26;
  } else if (base64_decimal >= 52 && base64_decimal <= 61) {
    return '0' + base64_decimal - 52;
  } else if (base64_decimal == 62) {
    return '+';
  } else if (base64_decimal == 63) {
    return '/';
  } else {
    throw std::invalid_argument("not a base64 value");
  }
}

std::string chunk_to_base64(const std::bitset<CHUNK_BITS> chunk, int bits_filled)
{
  char base64_chars[BASE64_CHARS_IN_CHUNK+1];
  int char_num = 0;

  std::bitset<BASE64_BITS> base64_bits;
  int base64_pos = BASE64_BITS - 1;

  for (int chunk_pos = CHUNK_BITS - 1; chunk_pos >= 0; chunk_pos--) {
    // copy chunk bits to base64 bits
    base64_bits[base64_pos--] = chunk[chunk_pos];

    // once base64 bits are written, convert to char
    if (base64_pos < 0) {
      base64_chars[char_num++] = base64_to_char(base64_bits);
      base64_pos = BASE64_BITS - 1;
      base64_bits.reset();
    }
  }

  // handle incomplete chunk
  if (bits_filled != CHUNK_BITS) {
    if (bits_filled >=4 && bits_filled < 12) {
      base64_chars[2] = '=';
      base64_chars[3] = '=';
    } else {
      base64_chars[3] = '=';
    }
  }

  base64_chars[BASE64_CHARS_IN_CHUNK] = '\0';
  return base64_chars;
}
