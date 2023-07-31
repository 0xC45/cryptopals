#include <bitset>
#include <iostream>
#include <sstream>
#include <stdexcept>

#define HEX_BITS 4

std::bitset<HEX_BITS> hex_to_bin(const char ch);
char bin_to_hex(const std::bitset<HEX_BITS> bits);


int main(void)
{
  unsigned input_length;
  std::cin >> input_length;

  std::bitset<HEX_BITS> *value1 = new std::bitset<HEX_BITS>[input_length];
  for (int i = 0; i < input_length; i++) {
    char ch;
    std::cin >> ch;
    value1[i] = hex_to_bin(ch);
  }

  std::bitset<HEX_BITS> *value2 = new std::bitset<HEX_BITS>[input_length];
  for (int i = 0; i < input_length; i++) {
    char ch;
    std::cin >> ch;
    value2[i] = hex_to_bin(ch);
  }

  std::bitset<HEX_BITS> *output = new std::bitset<HEX_BITS>[input_length];
  std::ostringstream os;
  for (int i = 0; i < input_length; i++) {
    output[i] = value1[i] ^ value2[i];
    os << bin_to_hex(output[i]);
  }

  std::cout << os.str() << std::endl;

  delete[] value1;
  delete[] value2;
  delete[] output;
  
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
  default: throw std::invalid_argument("character is not a hex value");
  }
}

char bin_to_hex(const std::bitset<HEX_BITS> bits)
{
  int decimal = bits.to_ulong();
  if (decimal >= 0 && decimal <= 9) {
    return '0' + decimal;
  } else if (decimal >= 10 && decimal <= 15) {
    return 'a' + decimal - 10;
  } else {
    throw std::invalid_argument("bitset is not a hex value");
  }
}
