#pragma once
#include <iostream>
#include <vector>
#include <string>

using std::string;

enum class TokenType
{
    _return,
    let,
    equal,
    varriable_name,
    variable_value,
    int_lit,
    semi
};

struct Token
{
    TokenType type;
    string value;
};

std::vector<Token> tokenize(const string &str, const int line_number)
{
    std::vector<Token> tokens{};
    string str_buf{};

    for (size_t i = 0; i < str.length(); i++)
    {
        char c = str.at(i);

        if (std::isalpha(static_cast<unsigned char>(c)) || c == '=')
        {
            str_buf.push_back(c);
        }
        else if (std::isspace(static_cast<unsigned char>(c)))
        {
            if (!str_buf.empty())
            {
                if (str_buf == "let")
                {
                    tokens.push_back({.type = TokenType::let, .value = "let"});
                }
                else if (str_buf == "return")
                {
                    tokens.push_back({.type = TokenType::_return, .value = "return"});
                }
                else if (str_buf == "=")
                {
                    tokens.push_back({.type = TokenType::equal, .value = "equal"});
                }
                else
                {
                    if (!tokens.empty() && tokens.back().type == TokenType::let)
                    {
                        tokens.push_back({.type = TokenType::varriable_name, .value = str_buf});
                    }
                    else if (!tokens.empty() && tokens.back().type == TokenType::equal)
                    {
                        tokens.push_back({.type = TokenType::variable_value, .value = str_buf.substr(0, i - 1)});
                    }
                    else
                    {
                        std::cerr << "Incorrect syntax, on line "
                                  << line_number << ":" << i << std::endl;
                        std::exit(EXIT_FAILURE);
                    }
                }

                str_buf.clear();
            }
        }
        if (c == ';')
        {
            tokens.push_back({.type = TokenType::variable_value, .value = str_buf});
            tokens.push_back({.type = TokenType::semi, .value = "semi"});
            str_buf.clear();
        }
    }

    return tokens;
}