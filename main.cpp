#include "tokenizer.hpp"
#include <fstream>
#include <unistd.h>

using std::ifstream;
using std::string;

int error_quit(string msg);

/*
 *
 *      Lexing or tokenizing is converting sets of characters iterated line by line or
 *      character by character into series of tokens which there's a list of it above
 *
 */

int main(int argc, char *argv[])
{
    if (argc != 2)
        return error_quit("Input file to be compiled is missing...");

    string line{};
    int line_number{0};
    {
        std::ifstream input(argv[1], std::ios::binary);

        if (input.fail() || !input.is_open())
            return error_quit("Unable to open the file");
        else
        {
            std::vector<Token> tokens{};
            while (getline(input, line))
            {
                if (line.empty())
                    continue;

                tokens = tokenize(line, line_number);
                line_number++;
            }

            for (Token el : tokens)
            {
                std::cout << el.value << "\n";
            }
        }
    }

    return EXIT_SUCCESS;
}

int error_quit(string msg)
{
    std::cerr << msg << std::endl;
    return EXIT_FAILURE;
}