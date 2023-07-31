#include <algorithm>
#include <bitset>
#include <cctype>
#include <climits>
#include <cmath>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <stdexcept>
#include <vector>

#define NUM_HEX_BITS 4
#define NUM_CHAR_BITS 8

typedef std::bitset<NUM_HEX_BITS> HEX_BITS;
typedef std::bitset<NUM_CHAR_BITS> CHAR_BITS;
typedef std::vector<CHAR_BITS> STR_BITS;

struct Plaintext {
  double score;
  char decryption_key;
  STR_BITS text_bin;
  std::string text;
};

// read a wordlist file into a set of strings
std::set<std::string> read_wordlist(const std::string filename);

// convert a hexadecimal string to a bit representation
STR_BITS hex_str_to_bits(const std::string hex_str);

// attempt to decrypt an encrypted string
void attempt_decrypt(const std::set<std::string> wordlist, const STR_BITS encrypted);

// convert a hex character to a bit representation
HEX_BITS hex_to_bin(const char ch);

// return true if text is > 70% alpha characters
bool is_reasonable_plaintext(const std::string text);

// return a value 0 <= score <= 1 that based on the percentage of real words in text
double score_plaintext(const std::set<std::string> wordlist, const std::string text);

// compare the scores of two plaintexts
bool compare_plaintexts(const Plaintext &p1, const Plaintext &p2);

// print a plaintext and its key
void print_plaintext(const Plaintext& p);


int main(void)
{
  std::set<std::string> wordlist = read_wordlist("wordlist.txt");

  std::vector<STR_BITS> encrypted_list;
  std::string encrypted_str;
  STR_BITS encrypted_bits;

  while (std::cin >> encrypted_str) {
    encrypted_bits = hex_str_to_bits(encrypted_str);
    encrypted_list.push_back(encrypted_bits);
  }

  for (auto &encrypted : encrypted_list) {
    attempt_decrypt(wordlist, encrypted);
  }

  return 0;
}

void attempt_decrypt(const std::set<std::string> wordlist, const STR_BITS encrypted)
{
  std::vector<Plaintext> plaintexts;

  // try every possible key and attempt to decrypt
  for (int key = CHAR_MIN; key <= CHAR_MAX; key++) {
    Plaintext plaintext;
    plaintext.decryption_key = key;

    CHAR_BITS key_bits(key);
    std::ostringstream text_os;
    for (auto& encrypted_ch_bin : encrypted) {
      CHAR_BITS decrypted_ch_bin = key_bits ^ encrypted_ch_bin;
      plaintext.text_bin.push_back(decrypted_ch_bin);
      text_os << (char) decrypted_ch_bin.to_ulong();
    }

    plaintext.text = text_os.str();

    // only consider reasonable plaintexts
    if (is_reasonable_plaintext(plaintext.text)) {
      plaintext.score = score_plaintext(wordlist, plaintext.text);
      plaintexts.push_back(plaintext);
    }
  }

  if (plaintexts.size() > 0) {
    std::sort(plaintexts.begin(), plaintexts.end(), compare_plaintexts);
    // output only if plaintext has a good score
    if (plaintexts[0].score > 0.5) {
      print_plaintext(plaintexts[0]);
    }
  }
}

STR_BITS hex_str_to_bits(const std::string hex_str)
{
  STR_BITS bitstring;
  CHAR_BITS ch_bin;
  HEX_BITS upper_bits;
  HEX_BITS lower_bits;
  bool save_upper_bits = true;
  char hex_ch;

  // read hex string character by character
  std::istringstream iss(hex_str);
  while(iss >> hex_ch) {
    if (save_upper_bits) {
      upper_bits = hex_to_bin(hex_ch);
      save_upper_bits = false;
    } else {
      lower_bits = hex_to_bin(hex_ch);
      save_upper_bits = true;
      // combine upper and lower bits and save into input vector
      ch_bin = (upper_bits.to_ulong() << NUM_HEX_BITS) | lower_bits.to_ulong();
      bitstring.push_back(ch_bin);
      ch_bin.reset();
    }
  }

  if (!save_upper_bits) {
    throw std::invalid_argument("bad hex string input");
  }

  return bitstring;
}

std::set<std::string> read_wordlist(std::string filename)
{
  std::ifstream word_file(filename);

  if (!word_file.is_open()) {
    throw std::invalid_argument("unable to open wordlist file");
  }

  std::string word;
  std::set<std::string> wordlist;
  while (getline(word_file, word)) {
    wordlist.insert(word);
  }
  word_file.close();

  return wordlist;
}

HEX_BITS hex_to_bin(const char ch)
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

bool is_reasonable_plaintext(const std::string text)
{
  int num_letters = 0;
  for (auto ch : text) {
    ch = tolower(ch);
    if (ch >= 'a' && ch <= 'z') {
      num_letters++;
    }
  }

  float percent_letters = (float) num_letters / text.length();
  return (percent_letters >= 0.7);
}

double score_plaintext(const std::set<std::string> wordlist, const std::string text)
{
  int num_words = 0;
  int num_real_words = 0;
  std::istringstream iss(text);
  std::string word;

  while (iss >> word) {
    std::transform(word.begin(), word.end(), word.begin(), ::tolower);
    if (wordlist.find(word) != wordlist.end()) {
      num_real_words++;
    }
    num_words++;
  }
  return (double) num_real_words / num_words;
}

bool compare_plaintexts(const Plaintext &p1, const Plaintext &p2)
{
  return p1.score > p2.score;
}

void print_plaintext(const Plaintext& p)
{
  std::cout << p.decryption_key << ": " << p.text << std::endl;
}
