#pragma once
#include <iostream>
#include <string>
#include <vector>

using std::string;

/*
 *
 *      Lexing/tokenizing, converting sets of characters iterated line by line
 * or character by character into series of tokens
 *
 */

enum class TokenType {
  _return,
  let,
  equal,
  varriable_name,
  variable_value,
  double_quotes,
  int_lit,
  semi
};

struct Token {
  TokenType type;
  string value;
};

std::vector<Token> tokenize(const string &str, const int line_number) {
  std::vector<Token> tokens{};
  string str_buf{};
  bool isStringStarted{false};

  for (size_t i = 0; i < str.length(); i++) {
    char c = str.at(i);

    if (std::isalpha(static_cast<unsigned char>(c)) || c == '=' || c == '"' ||
        std::isdigit(c)) {
      str_buf.push_back(c);
    } else if (std::isspace(static_cast<unsigned char>(c))) {
      if (!str_buf.empty()) {
        if (str_buf == "let") {
          tokens.push_back({.type = TokenType::let, .value = "let"});
        } else if (str_buf == "return") {
          tokens.push_back({.type = TokenType::_return, .value = "return"});
        } else if (str_buf == "=") {
          tokens.push_back({.type = TokenType::equal, .value = "equal"});
        } else {
          // std::cout << str_buf << "\n";
          if (!tokens.empty() && tokens.back().type == TokenType::let) {
            tokens.push_back(
                {.type = TokenType::varriable_name, .value = str_buf});
          } else if (!tokens.empty() &&
                     tokens.back().type == TokenType::equal) {
            tokens.push_back({.type = TokenType::variable_value,
                              .value = str_buf.substr(0, i - 1)});
          } else {
            std::cerr << "Incorrect syntax, on line " << line_number << ":" << i
                      << std::endl;
            std::exit(EXIT_FAILURE);
          }
        }

        str_buf.clear();
      }
    } else if (c == ';') {
      if (str_buf.at(0) == '"' && str_buf.at(str_buf.length() - 1) == '"') {
        for (int j = 1; j < str_buf.length() - 1; j++) {
          char k = str_buf.at(j);

          if (k == '"') {
            std::cerr << "Incorrect syntax, \"additional \" inside the "
                         "string\", on line "
                      << line_number << ":" << i << std::endl;
            std::exit(EXIT_FAILURE);
          }
        }

        tokens.push_back({.type = TokenType::variable_value,
                          .value = str_buf.substr(1, str_buf.length() - 2)});
      } else {
        for (int j = 0; j < str_buf.length(); j++) {
          char k = str_buf.at(j);

          if (!std::isdigit(k)) {
            std::cerr
                << "Incorrect syntax, strings need \" at the beginning and "
                   "ending, otherwise place a number in the variable. on line "
                << line_number << ":" << i << std::endl;
            std::exit(EXIT_FAILURE);
          }
        }

        tokens.push_back({.type = TokenType::variable_value, .value = str_buf});
      }

      tokens.push_back({.type = TokenType::semi, .value = "semi"});
      str_buf.clear();
    }
  }

  return tokens;
}

// std::vector<Token> tokenize(const string &str, const int line_number)
// {
//     std::vector<Token> tokens{};
//     string str_buf{};

//     return tokens;
// }
