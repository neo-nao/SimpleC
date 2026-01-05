#include "assembler.hpp"

int error_quit(string msg)
{
  std::cerr << msg << std::endl;
  return EXIT_FAILURE;
}

int main(int argc, char *argv[])
{
  if (argc < 2)
    return error_quit("Input file to be compiled is missing...");

  string content;

  string line{};
  int line_number{0};
  {
    std::ifstream input(argv[1], std::ios::binary);

    if (input.fail() || !input.is_open())
      return error_quit("Unable to open the file");
    else
    {
      while (getline(input, line))
      {
        content.append(line);
      }
    }
  }

  Lexer lexer(content);

  std::vector<Token> tokens = lexer.tokenize();

  Parser parser(tokens);

  ASTNode *parsed_tree = parser.parse();

  CodeGenerator codegen;

  string asm_file_name = "dist/output.asm";

  std::cout << asm_file_name << "\n\n";

  codegen.generate(parsed_tree, asm_file_name);
  std::cout << "Assembly code generated: " << asm_file_name << std::endl;

  string exe_file_name = "dist/program";
  if (assembleAndLink(asm_file_name, exe_file_name))
  {
    std::cout << "\n=== COMPILATION SUCCESS ===" << std::endl;
  }
  else
  {
    std::cout << "\n=== COMPILATION FAILED ===\n";
    std::cout << "Make sure you have 'nasm' and 'ld' installed:\n";
    std::cout << "sudo apt-get install nasm / sudo apt-get install binutils" << std::endl;
  }

  delete parsed_tree;

  return EXIT_SUCCESS;
}
