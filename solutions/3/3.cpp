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

#define HEX_BITS 4
#define CHAR_BITS 8

struct Plaintext {
  double score;
  char decryption_key;
  std::vector< std::bitset<CHAR_BITS> > text_bin;
  std::string text;
};

std::vector< std::bitset<CHAR_BITS> > read_input();
std::set<std::string> read_wordlist(std::string filename);
std::bitset<HEX_BITS> hex_to_bin(const char ch);
bool is_reasonable_plaintext(const std::string text);
double score_plaintext(const std::set<std::string> wordlist, const std::string text);
bool compare_plaintexts(const Plaintext &p1, const Plaintext &p2);
void print_plaintext(const Plaintext& p);


int main(void)
{
  std::vector< std::bitset<CHAR_BITS> > input = read_input();
  std::set<std::string> wordlist = read_wordlist("wordlist.txt");

  std::vector<Plaintext> plaintexts;
  for (int key = CHAR_MIN; key <= CHAR_MAX; key++) {
    Plaintext plaintext;
    plaintext.decryption_key = key;

    std::bitset<CHAR_BITS> key_bits(key);
    std::ostringstream text_os;
    for (auto& encrypted_ch_bin : input) {
      std::bitset<CHAR_BITS> decrypted_ch_bin = key_bits ^ encrypted_ch_bin;
      plaintext.text_bin.push_back(decrypted_ch_bin);
      text_os << (char) decrypted_ch_bin.to_ulong();
    }

    plaintext.text = text_os.str();

    if (is_reasonable_plaintext(plaintext.text)) {
      plaintext.score = score_plaintext(wordlist, plaintext.text);
      plaintexts.push_back(plaintext);
    }
  }

  if (plaintexts.size() > 0) {
    std::sort(plaintexts.begin(), plaintexts.end(), compare_plaintexts);
    print_plaintext(plaintexts[0]);
  }

  return 0;
}

std::vector< std::bitset<CHAR_BITS> > read_input()
{
  std::vector< std::bitset<CHAR_BITS> > input;
  std::bitset<CHAR_BITS> ch_bin;
  std::bitset<HEX_BITS> upper_bits;
  std::bitset<HEX_BITS> lower_bits;
  bool save_upper_bits = true;
  char hex_ch;  

  // read input
  while(std::cin >> hex_ch) {
    if (save_upper_bits) {
      upper_bits = hex_to_bin(hex_ch);
      save_upper_bits = false;
    } else {
      lower_bits = hex_to_bin(hex_ch);
      save_upper_bits = true;
      // combine upper and lower bits and save into input vector
      ch_bin = (upper_bits.to_ulong() << HEX_BITS) | lower_bits.to_ulong();
      input.push_back(ch_bin);
      ch_bin.reset();
    }
  }

  if (!save_upper_bits) {
    throw std::invalid_argument("bad input");
  }

  return input;
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
