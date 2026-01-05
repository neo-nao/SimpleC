#pragma once

#include <iostream>
#include <string>
#include <vector>

using std::string;

// ============================================
// LEXER (TOKENIZER) : Breaks the source code into meaningful chunks
// ============================================

enum class TokenType
{
  STRING,
  INT,
  IDENTIFIER,
  NUMBER,
  STRING_LITERAL,
  ASSIGN,
  PLUS,
  MINUS,
  MULTIPLY,
  DIVIDE,
  SEMICOLON,
  L_PAREN,
  R_PAREN,
  L_BRACE,
  R_BRACE,
  IF,
  PRINTLN,
  EXIT,
  GREATER,
  LESS,
  EQUAL,
  END_OF_FILE
};

struct Token
{
  TokenType type;
  string value;
  int line;
};

class Lexer
{
private:
  string source{};
  size_t pos{};
  int line{};

  char peek()
  {
    if (pos >= source.length())
      return '\0';
    return source[pos];
  }

  char advance()
  {
    return source[pos++];
  }

  void skip_white_space()
  {
    while (std::isspace(peek()))
    {
      if (peek() == '\n')
        ++line;
      advance();
    }
  }

  Token make_token(TokenType type, string value)
  {
    return {type, value, line};
  }

  Token read_number()
  {
    string num{};
    while (std::isdigit(peek()))
    {
      num += advance();
    }

    return make_token(TokenType::NUMBER, num);
  }

  Token read_string()
  {
    advance(); // skip opening quote

    string str;

    while (peek() != '"' && peek() != '\0')
    {
      str += advance();
    }

    advance(); // skip closing quote

    return make_token(TokenType::STRING_LITERAL, str);
  }

  Token read_identifier()
  {
    string id{};

    while (std::isalnum(peek()) || peek() == '_')
    {
      id += advance();
    }

    if (id == "int")
      return make_token(TokenType::INT, id);
    if (id == "string")
      return make_token(TokenType::STRING, id);
    if (id == "if")
      return make_token(TokenType::IF, id);
    if (id == "println")
      return make_token(TokenType::PRINTLN, id);
    if (id == "exit")
      return make_token(TokenType::EXIT, id);

    return make_token(TokenType::IDENTIFIER, id);
  }

public:
  Lexer(string src) : source(src) {}

  std::vector<Token> tokenize()
  {
    std::vector<Token> tokens{};

    while (peek() != '\0')
    {
      skip_white_space();

      char c = peek();

      if (isdigit(c))
      {
        tokens.push_back(read_number());
      }
      else if (c == '"')
      {
        tokens.push_back(read_string());
      }
      else if (isalpha(c) || c == '_')
      {
        tokens.push_back(read_identifier());
      }
      else if (c == '=')
      {
        advance();
        tokens.push_back(make_token(TokenType::ASSIGN, "="));
      }
      else if (c == '+')
      {
        advance();
        tokens.push_back(make_token(TokenType::PLUS, "+"));
      }
      else if (c == '-')
      {
        advance();
        tokens.push_back(make_token(TokenType::MINUS, "-"));
      }
      else if (c == '*')
      {
        advance();
        tokens.push_back(make_token(TokenType::MULTIPLY, "*"));
      }
      else if (c == '/')
      {
        advance();
        tokens.push_back(make_token(TokenType::DIVIDE, "/"));
      }
      else if (c == ';')
      {
        advance();
        tokens.push_back(make_token(TokenType::SEMICOLON, ";"));
      }
      else if (c == '(')
      {
        advance();
        tokens.push_back(make_token(TokenType::L_PAREN, "("));
      }
      else if (c == ')')
      {
        advance();
        tokens.push_back(make_token(TokenType::R_PAREN, ")"));
      }
      else if (c == '{')
      {
        advance();
        tokens.push_back(make_token(TokenType::L_BRACE, "{"));
      }
      else if (c == '}')
      {
        advance();
        tokens.push_back(make_token(TokenType::R_BRACE, "}"));
      }
      else if (c == '>')
      {
        advance();
        tokens.push_back(make_token(TokenType::GREATER, ">"));
      }
      else if (c == '<')
      {
        advance();
        tokens.push_back(make_token(TokenType::LESS, "<"));
      }
      else
      {
        advance();
      }
    }

    return tokens;
  }
};