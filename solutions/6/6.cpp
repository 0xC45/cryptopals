#include <algorithm>
#include <bitset>
#include <climits>
#include <iostream>
#include <sstream>
#include <utility>
#include <vector>

#define NUM_BASE64_BITS 6
#define NUM_CHAR_BITS 8

typedef std::bitset<NUM_CHAR_BITS> CHAR_BITS;
typedef std::bitset<NUM_BASE64_BITS> BASE64_BITS;
typedef std::bitset<NUM_BASE64_BITS * 4> BASE64_CHUNK_BITS;
typedef std::vector<CHAR_BITS> STR_BITS;

typedef std::pair<int, double> KEY_EVALUATION;

STR_BITS str_to_bits(const std::string str);
STR_BITS base64_str_to_bits(const std::string base64_str);
BASE64_BITS base64_char_to_bits(const char base64_char);
unsigned hamming_distance(const STR_BITS s1, const STR_BITS s2);
std::vector<KEY_EVALUATION> evaluate_key_lengths(const STR_BITS encrypted_bits);
bool compare_key_evaluations(const KEY_EVALUATION &ke1, const KEY_EVALUATION &ke2);
void attempt_decrypt_with_keylength(const STR_BITS encrypted, const int keylength);
std::vector<STR_BITS> generate_blocks(const STR_BITS encrypted, const int keylength);
void attempt_decrypt(const STR_BITS encrypted);
bool is_reasonable_plaintext(const std::string text);
void decrypt(const STR_BITS key, const STR_BITS encrypted);


int main(void)
{
  std::ostringstream oss;
  char ch;
  while (std::cin >> std::noskipws >> ch) {
    if (ch != '\n') {
      oss << ch;
    }
  }

  STR_BITS encrypted_bits = base64_str_to_bits(oss.str());

  std::vector<KEY_EVALUATION> key_evaluations = evaluate_key_lengths(encrypted_bits);

  std::cout << "Trying keylength " << key_evaluations[0].first << std::endl;
  attempt_decrypt_with_keylength(encrypted_bits, key_evaluations[0].first);

  /*
  STR_BITS key = str_to_bits("Terminator X: Bring the noise");
  decrypt(key, encrypted_bits);
  */

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

STR_BITS base64_str_to_bits(const std::string base64_str)
{
  if (base64_str.size() % 4 != 0) {
    throw std::invalid_argument("base64_str length must be a multiple of 4");
  }

  unsigned long ch_mask = (1 << NUM_CHAR_BITS) - 1;
  unsigned long ch1_mask = ch_mask << (NUM_CHAR_BITS * 2);
  unsigned long ch2_mask = ch_mask << NUM_CHAR_BITS;
  unsigned long ch3_mask = ch_mask;

  STR_BITS bitstring;

  for (int i = 0; i < base64_str.size(); i += 4) {
    BASE64_BITS b64_ch1_bits = base64_char_to_bits(base64_str[i]);
    BASE64_BITS b64_ch2_bits = base64_char_to_bits(base64_str[i+1]);
    BASE64_BITS b64_ch3_bits = base64_char_to_bits(base64_str[i+2]);
    BASE64_BITS b64_ch4_bits = base64_char_to_bits(base64_str[i+3]);
    
    BASE64_CHUNK_BITS b64_chunk_bits;
    b64_chunk_bits |= b64_ch1_bits.to_ulong() << (NUM_BASE64_BITS * 3);
    b64_chunk_bits |= b64_ch2_bits.to_ulong() << (NUM_BASE64_BITS * 2);
    b64_chunk_bits |= b64_ch3_bits.to_ulong() << NUM_BASE64_BITS;
    b64_chunk_bits |= b64_ch4_bits.to_ulong();

    unsigned long b64_chunk = b64_chunk_bits.to_ulong();
    CHAR_BITS ch1 = (b64_chunk & ch1_mask) >> (NUM_CHAR_BITS * 2);
    CHAR_BITS ch2 = (b64_chunk & ch2_mask) >> NUM_CHAR_BITS;
    CHAR_BITS ch3 = (b64_chunk & ch3_mask);

    bitstring.push_back(ch1);
    bitstring.push_back(ch2);
    bitstring.push_back(ch3);
  }

  return bitstring;
}

BASE64_BITS base64_char_to_bits(const char base64_char)
{
  if (base64_char >= 'A' && base64_char <= 'Z') {
    return base64_char - 'A';
  } else if (base64_char >= 'a' && base64_char <= 'z') {
    return base64_char - 'a' + 26;
  } else if (base64_char >= '0' && base64_char <= '9') {
    return base64_char - '0' + 52;
  } else if (base64_char == '+') {
    return 62;
  } else if (base64_char == '/') {
    return 63;
  } else if (base64_char == '=') {
    return 0;
  } else {
    throw std::invalid_argument("base64_char is not a base64 value");
  }
}

unsigned hamming_distance(const STR_BITS s1, const STR_BITS s2)
{
  if (s1.size() != s2.size()) {
    throw std::invalid_argument("strings must be equal length");
  }

  unsigned distance = 0;
  for (int i = 0; i < s1.size(); i++) {
    CHAR_BITS byte = s1[i] ^ s2[i];
    for (int i = 0; i < NUM_CHAR_BITS; i++) {
      if (byte.test(i)) {
	distance++;
      }
    }
  }

  return distance;
}

std::vector<KEY_EVALUATION> evaluate_key_lengths(const STR_BITS encrypted_bits)
{
  std::vector<KEY_EVALUATION> key_evaluations;

  for (int keysize = 2; keysize <= 40; keysize++) {
    STR_BITS block1, block2, block3, block4;
    for (int pos = 0; pos < keysize; pos++) {
      block1.push_back(encrypted_bits[pos]);
      block2.push_back(encrypted_bits[pos + keysize]);
      block3.push_back(encrypted_bits[pos + (keysize * 2)]);
      block4.push_back(encrypted_bits[pos + (keysize * 3)]);
    }

    int d1 = hamming_distance(block1, block2);
    int d2 = hamming_distance(block3, block4);
    int d3 = hamming_distance(block1, block3);
    int d4 = hamming_distance(block2, block4);
    int d5 = hamming_distance(block2, block3);
    int d6 = hamming_distance(block1, block4);
    double average_distance = (double) (d1 + d2 + d3 + d4 + d5 + d6) / 6;
    double normalized_distance = average_distance / keysize;

    KEY_EVALUATION key_evaluation(keysize, normalized_distance);
    key_evaluations.push_back(key_evaluation);
  }

  std::sort(key_evaluations.begin(), key_evaluations.end(), compare_key_evaluations);

  return key_evaluations;
}

bool compare_key_evaluations(const KEY_EVALUATION &ke1, const KEY_EVALUATION &ke2)
{
  return ke1.second < ke2.second;
}

void attempt_decrypt_with_keylength(const STR_BITS encrypted, const int keylength)
{
  std::vector<STR_BITS> encrypted_blocks = generate_blocks(encrypted, keylength);

  for (auto &encrypted_block : encrypted_blocks) {
    attempt_decrypt(encrypted_block);
  }
}

std::vector<STR_BITS> generate_blocks(const STR_BITS encrypted, const int keylength)
{
  std::vector<STR_BITS> blocks;
  for (int i = 0; i < keylength; i++) {
    STR_BITS block;
    for (int j = i; j < encrypted.size(); j += keylength) {
      block.push_back(encrypted[j]);
    }
    blocks.push_back(block);
  }
  return blocks;
}

void attempt_decrypt(const STR_BITS encrypted)
{
  for (int key = CHAR_MIN; key < CHAR_MAX; key++) {
    CHAR_BITS key_bits(key);
    std::ostringstream text_os;
    for (auto& encrypted_ch_bin : encrypted) {
      CHAR_BITS decrypted_ch_bin = key_bits ^ encrypted_ch_bin;
      text_os << (char) decrypted_ch_bin.to_ulong();
    }

    std::string plaintext = text_os.str();
    if (is_reasonable_plaintext(plaintext)) {
      std::cout << (char) key << " ";
    }
  }
  std::cout << std::endl;
}

bool is_reasonable_plaintext(const std::string text)
{
  int num_valid_chars = 0;
  for (auto ch : text) {
    if ((ch >= 'a' && ch <= 'z') ||
	(ch >= 'A' && ch <= 'Z') ||
	(ch >= '0' && ch <= '9') ||
	ch == ' ' || ch == '\n' ||
	ch == '\'' || ch == '\"' ||
	ch == '-' || ch == ',' ||
	ch == '.') {
      num_valid_chars++;
    }
  }

  float percent_valid_chars = (float) num_valid_chars / text.length();
  return (percent_valid_chars >= 0.95);
}

void decrypt(const STR_BITS key, const STR_BITS encrypted)
{
  size_t key_pos = 0;

  for (auto &ch_bits : encrypted) {
    if (key_pos >= key.size()) {
      key_pos = 0;
    }

    CHAR_BITS decrypted_ch = ch_bits ^ key[key_pos];
    std::cout << (char) decrypted_ch.to_ulong();

    key_pos++;
  }
}
