#include "parser.hpp"
#include <map>
#include <fstream>

// ============================================
// CODE GENERATOR (x86-64 Assembly)
// ============================================

class CodeGenerator
{
private:
    std::ofstream asmFile;
    std::map<string, int> intVars;
    std::map<string, string> stringVars;
    int stackOffset;
    int labelCounter;
    int stringCounter;

    string newLabel()
    {
        return ".L" + std::to_string(labelCounter++);
    }

    string newStringLabel()
    {
        return ".STR" + std::to_string(stringCounter);
    }

    void emitDataSection()
    {
        asmFile << "section .data\n";

        for (auto &pair : stringVars)
        {
            asmFile << pair.second << ": db \"";
            asmFile << "\", 0\n";
        }
        asmFile << "\n";
    }

    void generateExpression(ASTNode *node)
    {
        if (node->type == NodeType::NUMBER_LITERAL)
        {
            asmFile << "    mov rax, " << node->value << "\n";
        }
        else if (node->type == NodeType::IDENTIFIER)
        {
            if (intVars.count(node->value))
            {
                int offset = intVars[node->value];
                asmFile << "    mov rax, [rbp - " << offset << "]\n";
            }
        }
        else if (node->type == NodeType::BINARY_OP)
        {
            generateExpression(node->children[1]);
            asmFile << "    push rax\n";

            generateExpression(node->children[0]);

            asmFile << "    pop rbx\n";

            if (node->value == "+")
            {
                asmFile << "    add rax, rbx\n";
            }
            else if (node->value == "-")
            {
                asmFile << "    sub rax, rbx\n";
            }
            else if (node->value == "*")
            {
                asmFile << "    imul rax, rbx\n";
            }
            else if (node->value == "/")
            {
                asmFile << "    xor rdx, rdx\n";
                asmFile << "    idiv rbx\n";
            }
            else if (node->value == ">")
            {
                asmFile << "    cmp rax, rbx\n";
                asmFile << "    setg al\n";
                asmFile << "    movzx rax, al\n";
            }
        }
    }

    void generateStatement(ASTNode *stmt)
    {
        if (stmt->type == NodeType::VAR_DEC)
        {
            ASTNode *expr = stmt->children[0];

            if (expr->type == NodeType::NUMBER_LITERAL || expr->type == NodeType::BINARY_OP || expr->type == NodeType::IDENTIFIER)
            {
                stackOffset += 8;
                intVars[stmt->value] = stackOffset;

                generateExpression(expr);
                asmFile << "    mov [rbp - " << stackOffset << "], rax\n";
            }
            else if (expr->type == NodeType::STRING_LITERAL)
            {
                string label = newStringLabel();
                stringVars[stmt->value] = label;

                asmFile << "; string " << stmt->value << " = " << label << "\n";
            }
        }
        else if (stmt->type == NodeType::ASSIGNMENT)
        {
            generateExpression(stmt->children[0]);
            int offset = intVars[stmt->value];
            asmFile << "    mov [rbp - " << stackOffset << "], rax\n";
        }
        else if (stmt->type == NodeType::IF_STATEMENT)
        {
            string endLabel = newLabel();

            generateExpression(stmt->children[0]);
            asmFile << "    cmp rax, 0\n";
            asmFile << "    je " << endLabel << "\n";

            for (size_t i = 1; i < stmt->children.size(); i++)
            {
                generateStatement(stmt->children[i]);
            }

            asmFile << endLabel << ":\n";
        }
        else if (stmt->type == NodeType::PRINT)
        {
            ASTNode *expr = stmt->children[0];

            if (expr->type == NodeType::IDENTIFIER && stringVars.count(expr->value))
            {
                string label = stringVars[expr->value];

                asmFile << "    ; println(" << expr->value << ")\n";
                asmFile << "    mov rax, 1       ; sys_write\n";
                asmFile << "    mov rdi, 1       ; stdout\n";
                asmFile << "    lea rsi, [rel " << label << "]\n";
                asmFile << "    mov rdx, 20      ; approximate length\n";
                asmFile << "    syscall\n";

                asmFile << "    mov rax, 1\n";
                asmFile << "    mov rdi, 1\n";
                asmFile << "    lea rsi, [rel .NEWLINE]\n";
                asmFile << "    mov rdx, 1\n";
                asmFile << "    syscall\n";
            }
        }
        else if (stmt->type == NodeType::EXIT)
        {
            generateExpression(stmt->children[0]);
            asmFile << "    mov rdi, rax     ; exit code\n";
            asmFile << "    mov rax, 60      ; sys_exit\n";
            asmFile << "    syscall\n";
        }
    }

public:
    CodeGenerator() : stackOffset(0), labelCounter(0), stringCounter(0) {}

    void generate(ASTNode *program, const string &filename)
    {
        asmFile.open(filename);

        std::map<string, string> stringLiterals;
        collectStrings(program, stringLiterals);

        asmFile << "section .data\n";
        for (auto &pair : stringLiterals)
        {
            asmFile << pair.second << ": db \"" << pair.first << "\", 0\n";
        }
        asmFile << ".NEWLINE: db 10\n\n";

        asmFile << "section .text\n";
        asmFile << "global _start\n\n";
        asmFile << "_start:\n";
        asmFile << "    push rbp\n";
        asmFile << "    mov rbp, rsp\n";
        asmFile << "    sub rsp, 128     ; allocate stack space\n\n";

        for (ASTNode *stmt : program->children)
        {
            generateStatement(stmt);
        }

        asmFile << "\n    ; default exit\n";
        asmFile << "    mov rax, 60\n";
        asmFile << "    xor rdi, rdi\n";
        asmFile << "    syscall\n";

        asmFile.close();
    }

    void collectStrings(ASTNode *node, std::map<string, string> &literals)
    {
        if (node->type == NodeType::VAR_DEC && !node->children.empty())
        {
            if (node->children[0]->type == NodeType::STRING_LITERAL)
            {
                string label = newStringLabel();
                stringVars[node->value] = label;
                literals[node->children[0]->value] = label;
            }
        }
        for (ASTNode *child : node->children)
        {
            collectStrings(child, literals);
        }
    }
};

// ============================================
// ASSEMBLER & LINKER
// ============================================

bool assembleAndLink(const string &asmFile, const string &outputExe)
{
    string objFile = "dist/output.o";
    string nasmCmd = "nasm -f elf64 " + asmFile + " -o " + objFile;

    std::cout << "Assembling: " << nasmCmd << std::endl;
    int result = system(nasmCmd.c_str());
    if (result != 0)
    {
        std::cerr << "Assembly failed!" << std::endl;
        return false;
    }

    string ldCmd = "ld " + objFile + " -o " + outputExe;

    std::cout << "Linking: " << ldCmd << std::endl;
    result = system(ldCmd.c_str());
    if (result != 0)
    {
        std::cerr << "Linking failed!" << std::endl;
        return false;
    }

    std::cout << "\nSuccessfully created executable: " << outputExe << std::endl;
    std::cout << "Run it with: ./" << outputExe << std::endl;

    return true;
}