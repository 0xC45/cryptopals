#include <iostream>
#include <set>
#include <string>

#define BLOCK_SIZE 16

int main(void)
{
  std::string encrypted_str;
  while (std::cin >> encrypted_str) {
    std::set<std::string> block_set;
    for (int i = 0; i < encrypted_str.length(); i += BLOCK_SIZE) {
      std::string block = encrypted_str.substr(i, BLOCK_SIZE);
      if (block_set.find(block) == block_set.end()) {
	block_set.insert(block);
      } else {
	std::cout << "Duplicate block" << std::endl;
      }
    }
  }

  return 0;
}
