#include "tokenizer.hpp"
#include <fstream>
#include <unistd.h>

using std::ifstream;
using std::string;

int error_quit(string msg);

int main(int argc, char *argv[]) {
  if (argc != 2)
    return error_quit("Input file to be compiled is missing...");

  string line{};
  int line_number{0};
  {
    std::ifstream input(argv[1], std::ios::binary);

    if (input.fail() || !input.is_open())
      return error_quit("Unable to open the file");
    else {
      std::vector<std::vector<Token>> tokens{};
      while (getline(input, line)) {
        if (line.empty())
          continue;

        tokens.push_back(tokenize(line, line_number));
        line_number++;
      }

      for (std::vector<Token> el_v : tokens) {
        for (Token token : el_v) {
          std::cout << token.value << "\n";
        }
        std::cout << "\n===================\n\n";
      }
    }
  }

  return EXIT_SUCCESS;
}

int error_quit(string msg) {
  std::cerr << msg << std::endl;
  return EXIT_FAILURE;
}
