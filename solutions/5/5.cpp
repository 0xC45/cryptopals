#include <bitset>
#include <iostream>
#include <sstream>
#include <vector>

#define NUM_HEX_BITS 4
#define NUM_CHAR_BITS 8

typedef std::bitset<NUM_HEX_BITS> HEX_BITS;
typedef std::bitset<NUM_CHAR_BITS> CHAR_BITS;
typedef std::vector<CHAR_BITS> STR_BITS;

// convert a string to binary representation
STR_BITS str_to_bits(const std::string str);

// encrypt a binary string using repeating key xor
STR_BITS encrypt(const STR_BITS plaintext, const STR_BITS key);

// convert a binary string to hexadecimal representation
std::string bits_to_hex(const STR_BITS binary_string);

// convert a decmial to hex character
char int_to_hex(const unsigned decimal);


int main(void)
{
  std::string key = "ICE";
  STR_BITS key_bits = str_to_bits(key);

  char ch;
  std::ostringstream oss;
  while(std::cin >> std::noskipws >> ch) {
    oss << ch;
  }

  std::string plaintext = oss.str();
  STR_BITS plaintext_bits = str_to_bits(plaintext);
  STR_BITS encrypted_bits = encrypt(plaintext_bits, key_bits);
  std::string encrypted = bits_to_hex(encrypted_bits);
  std::cout << encrypted << std::endl;

  return 0;
}

STR_BITS str_to_bits(const std::string str)
{
  STR_BITS bitstring;
  char ch;

  for (auto &ch : str) {
    CHAR_BITS ch_bits(ch);
    bitstring.push_back(ch);
  }

  return bitstring;
}

STR_BITS encrypt(const STR_BITS plaintext, const STR_BITS key)
{
  STR_BITS encrypted;
  size_t key_pos = 0;

  for (auto &ch_bits : plaintext) {
    if (key_pos >= key.size()) {
      key_pos = 0;
    }

    CHAR_BITS encrypted_ch = ch_bits ^ key[key_pos];
    encrypted.push_back(encrypted_ch);

    key_pos++;
  }

  return encrypted;
}

std::string bits_to_hex(const STR_BITS binary_string)
{
  std::ostringstream os;

  for (auto &ch_bits : binary_string) {
    unsigned upper_bits = ch_bits.to_ulong() >> NUM_HEX_BITS;
    unsigned lower_bits = ch_bits.to_ulong() & ((1 << NUM_HEX_BITS) - 1);
    char upper_ch = int_to_hex(upper_bits);
    char lower_ch = int_to_hex(lower_bits);
    os << upper_ch << lower_ch;
  }

  return os.str();
}

char int_to_hex(const unsigned decimal)
{
  if (decimal <= 9) {
    return '0' + decimal;
  } else if (decimal >= 10 && decimal <= 15) {
    return 'a' + decimal - 10;
  } else {
    throw std::invalid_argument("bitset is not a hex value");
  }
}
