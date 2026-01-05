#include "tokenizer.hpp"

enum NodeType
{
    PROGRAM,
    VAR_DEC,
    ASSIGNMENT,
    IF_STATEMENT,
    PRINT,
    EXIT,
    BINARY_OP,
    NUMBER_LITERAL,
    STRING_LITERAL,
    IDENTIFIER
};

// ============================================
// ABSTRACT SYSTANX TREE (AST)
// ============================================

struct ASTNode
{
    NodeType type;
    string value;
    std::vector<ASTNode *> children;

    ~ASTNode()
    {
        for (ASTNode *child : children)
            delete child;
    }
};

// ============================================
// PARSER
// ============================================

class Parser
{
private:
    std::vector<Token> tokens;
    size_t pos;

    Token peek()
    {
        if (pos >= tokens.size())
        {
            return {TokenType::END_OF_FILE, "", 0};
        }
        return tokens[pos];
    }

    Token advance()
    {
        if (pos >= tokens.size())
        {
            return {TokenType::END_OF_FILE, "", 0};
        }
        return tokens[pos++];
    }

    bool match(TokenType type)
    {
        if (peek().type == type)
        {
            advance();
            return true;
        }
        return false;
    }

    ASTNode *parseExpression()
    {
        ASTNode *left = parsePrimary();

        while (peek().type == TokenType::PLUS || peek().type == TokenType::MINUS ||
               peek().type == TokenType::MULTIPLY || peek().type == TokenType::DIVIDE ||
               peek().type == TokenType::GREATER || peek().type == TokenType::LESS)
        {
            Token op = advance();
            ASTNode *right = parsePrimary();

            ASTNode *binary = new ASTNode{NodeType::BINARY_OP, op.value};
            binary->children.push_back(left);
            binary->children.push_back(right);
            left = binary;
        }

        return left;
    }

    ASTNode *parsePrimary()
    {
        if (peek().type == TokenType::NUMBER)
        {
            Token t = advance();
            return new ASTNode{NodeType::NUMBER_LITERAL, t.value};
        }
        if (peek().type == TokenType::STRING_LITERAL)
        {
            Token t = advance();
            return new ASTNode{NodeType::STRING_LITERAL, t.value};
        }
        if (peek().type == TokenType::IDENTIFIER)
        {
            Token t = advance();
            return new ASTNode{NodeType::IDENTIFIER, t.value};
        }
        if (match(TokenType::L_PAREN))
        {
            ASTNode *expr = parseExpression();
            match(TokenType::R_PAREN);
            return expr;
        }
        return nullptr;
    }

    ASTNode *parseStatement()
    {
        if (peek().type == TokenType::END_OF_FILE)
        {
            return nullptr;
        }

        if (peek().type == TokenType::INT || peek().type == TokenType::STRING)
        {
            advance();
            Token name = advance();
            match(TokenType::ASSIGN);
            ASTNode *expr = parseExpression();
            match(TokenType::SEMICOLON);

            ASTNode *decl = new ASTNode{NodeType::VAR_DEC, name.value};
            decl->children.push_back(expr);
            return decl;
        }

        if (peek().type == TokenType::IDENTIFIER)
        {
            Token name = peek();
            size_t savedPos = pos;
            advance();
            if (match(TokenType::ASSIGN))
            {
                ASTNode *expr = parseExpression();
                match(TokenType::SEMICOLON);

                ASTNode *assign = new ASTNode{NodeType::ASSIGNMENT, name.value};
                assign->children.push_back(expr);
                return assign;
            }

            pos = savedPos;
        }

        if (match(TokenType::IF))
        {
            match(TokenType::L_PAREN);
            ASTNode *condition = parseExpression();
            match(TokenType::R_PAREN);
            match(TokenType::L_BRACE);

            ASTNode *ifNode = new ASTNode{NodeType::IF_STATEMENT, ""};
            ifNode->children.push_back(condition);

            while (!match(TokenType::R_BRACE) && peek().type != TokenType::END_OF_FILE)
            {
                ASTNode *stmt = parseStatement();
                if (stmt)
                {
                    ifNode->children.push_back(stmt);
                }
            }

            return ifNode;
        }

        if (match(TokenType::PRINTLN))
        {
            match(TokenType::L_PAREN);
            ASTNode *expr = parseExpression();
            match(TokenType::R_PAREN);
            match(TokenType::SEMICOLON);

            ASTNode *print = new ASTNode{NodeType::PRINT, ""};
            print->children.push_back(expr);
            return print;
        }

        if (match(TokenType::EXIT))
        {
            match(TokenType::L_PAREN);
            ASTNode *code = parseExpression();
            match(TokenType::R_PAREN);
            match(TokenType::SEMICOLON);

            ASTNode *exitNode = new ASTNode{NodeType::EXIT, ""};
            exitNode->children.push_back(code);
            return exitNode;
        }

        return nullptr;
    }

public:
    Parser(std::vector<Token> toks) : tokens(toks), pos(0) {}

    ASTNode *parse()
    {
        ASTNode *program = new ASTNode{NodeType::PROGRAM, ""};

        while (peek().type != TokenType::END_OF_FILE)
        {
            ASTNode *stmt = parseStatement();
            if (stmt)
            {
                program->children.push_back(stmt);
            }
            else
            {
                if (peek().type == TokenType::END_OF_FILE)
                {
                    break;
                }
                Token current = peek();
                std::cerr << "Warning: Skipping unexpected token '" << current.value << "'. at line: " << current.line << std::endl;
                advance();
            }
        }

        return program;
    }
};