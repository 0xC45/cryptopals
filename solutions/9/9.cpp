#include <iostream>
#include <sstream>
#include <string>

#define BLOCK_SIZE 20

int main(void)
{
  std::ostringstream oss;
  char ch;
  while (std::cin >> std::noskipws >> ch) {
    if (ch != '\n') {
      oss << ch;
    }
  }

  std::string plaintext = oss.str();
  size_t num_missing_bytes = BLOCK_SIZE - (plaintext.length() % BLOCK_SIZE);
  std::string padding(num_missing_bytes, (char) num_missing_bytes);
  plaintext += padding;

  std::cout << plaintext << std::endl;
  
  return 0;
}
