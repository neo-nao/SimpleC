#include "parser.hpp"
#include <map>
#include <fstream>

// ============================================
// CODE GENERATOR (x86-64 Assembly)
// ============================================

class CodeGenerator
{
private:
    std::ofstream asm_file;
    std::map<string, int> int_vars;
    std::map<string, string> string_vars;
    int stack_offset;
    int label_counter;
    int string_counter;

    string newLabel()
    {
        return ".L" + std::to_string(label_counter++);
    }

    string newStringLabel()
    {
        return ".STR" + std::to_string(string_counter);
    }

    void emitDataSection()
    {
        asm_file << "section .data\n";

        for (auto &pair : string_vars)
        {
            asm_file << pair.second << ": db \"";
            asm_file << "\", 0\n";
        }
        asm_file << "\n";
    }

    void generateExpression(ASTNode *node)
    {
        if (node->type == NodeType::NUMBER_LITERAL)
        {
            asm_file << "    mov rax, " << node->value << "\n";
        }
        else if (node->type == NodeType::IDENTIFIER)
        {
            if (int_vars.count(node->value))
            {
                int offset = int_vars[node->value];
                asm_file << "    mov rax, [rbp - " << offset << "]\n";
            }
        }
        else if (node->type == NodeType::BINARY_OP)
        {
            generateExpression(node->children[1]);
            asm_file << "    push rax\n";

            generateExpression(node->children[0]);

            asm_file << "    pop rbx\n";

            if (node->value == "+")
            {
                asm_file << "    add rax, rbx\n";
            }
            else if (node->value == "-")
            {
                asm_file << "    sub rax, rbx\n";
            }
            else if (node->value == "*")
            {
                asm_file << "    imul rax, rbx\n";
            }
            else if (node->value == "/")
            {
                asm_file << "    xor rdx, rdx\n";
                asm_file << "    idiv rbx\n";
            }
            else if (node->value == ">")
            {
                asm_file << "    cmp rax, rbx\n";
                asm_file << "    setg al\n";
                asm_file << "    movzx rax, al\n";
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
                stack_offset += 8;
                int_vars[stmt->value] = stack_offset;

                generateExpression(expr);
                asm_file << "    mov [rbp - " << stack_offset << "], rax\n";
            }
            else if (expr->type == NodeType::STRING_LITERAL)
            {
                string label = newStringLabel();
                string_vars[stmt->value] = label;

                asm_file << "; string " << stmt->value << " = " << label << "\n";
            }
        }
        else if (stmt->type == NodeType::ASSIGNMENT)
        {
            generateExpression(stmt->children[0]);
            int offset = int_vars[stmt->value];
            asm_file << "    mov [rbp - " << stack_offset << "], rax\n";
        }
        else if (stmt->type == NodeType::IF_STATEMENT)
        {
            string endLabel = newLabel();

            generateExpression(stmt->children[0]);
            asm_file << "    cmp rax, 0\n";
            asm_file << "    je " << endLabel << "\n";

            for (size_t i = 1; i < stmt->children.size(); i++)
            {
                generateStatement(stmt->children[i]);
            }

            asm_file << endLabel << ":\n";
        }
        else if (stmt->type == NodeType::PRINT)
        {
            ASTNode *expr = stmt->children[0];

            if (expr->type == NodeType::IDENTIFIER && string_vars.count(expr->value))
            {
                string label = string_vars[expr->value];

                asm_file << "    ; println(" << expr->value << ")\n";
                asm_file << "    mov rax, 1       ; sys_write\n";
                asm_file << "    mov rdi, 1       ; stdout\n";
                asm_file << "    lea rsi, [rel " << label << "]\n";
                asm_file << "    mov rdx, 20      ; approximate length\n";
                asm_file << "    syscall\n";

                asm_file << "    mov rax, 1\n";
                asm_file << "    mov rdi, 1\n";
                asm_file << "    lea rsi, [rel .NEWLINE]\n";
                asm_file << "    mov rdx, 1\n";
                asm_file << "    syscall\n";
            }
        }
        else if (stmt->type == NodeType::EXIT)
        {
            generateExpression(stmt->children[0]);
            asm_file << "    mov rdi, rax     ; exit code\n";
            asm_file << "    mov rax, 60      ; sys_exit\n";
            asm_file << "    syscall\n";
        }
    }

public:
    CodeGenerator() : stack_offset(0), label_counter(0), string_counter(0) {}

    void generate(ASTNode *program, const string &filename)
    {
        asm_file.open(filename);

        std::map<string, string> stringLiterals;
        collect_strings(program, stringLiterals);

        asm_file << "section .data\n";
        for (auto &pair : stringLiterals)
        {
            asm_file << pair.second << ": db \"" << pair.first << "\", 0\n";
        }
        asm_file << ".NEWLINE: db 10\n\n";

        asm_file << "section .text\n";
        asm_file << "global _start\n\n";
        asm_file << "_start:\n";
        asm_file << "    push rbp\n";
        asm_file << "    mov rbp, rsp\n";
        asm_file << "    sub rsp, 128     ; allocate stack space\n\n";

        for (ASTNode *stmt : program->children)
        {
            generateStatement(stmt);
        }

        asm_file << "\n    ; default exit\n";
        asm_file << "    mov rax, 60\n";
        asm_file << "    xor rdi, rdi\n";
        asm_file << "    syscall\n";

        asm_file.close();
    }

    void collect_strings(ASTNode *node, std::map<string, string> &literals)
    {
        if (node->type == NodeType::VAR_DEC && !node->children.empty())
        {
            if (node->children[0]->type == NodeType::STRING_LITERAL)
            {
                string label = newStringLabel();
                string_vars[node->value] = label;
                literals[node->children[0]->value] = label;
            }
        }
        for (ASTNode *child : node->children)
        {
            collect_strings(child, literals);
        }
    }
};

// ============================================
// ASSEMBLER & LINKER
// ============================================

bool assembleAndLink(const string &asm_file, const string &output_exe)
{
    string obj_file = "dist/output.o";
    string nasm_cmd = "nasm -f elf64 " + asm_file + " -o " + obj_file;

    std::cout << "Assembling: " << nasm_cmd << std::endl;
    int result = system(nasm_cmd.c_str());
    if (result != 0)
    {
        std::cerr << "Assembly failed!" << std::endl;
        return false;
    }

    string ld_cmd = "ld " + obj_file + " -o " + output_exe;

    std::cout << "Linking: " << ld_cmd << std::endl;
    result = system(ld_cmd.c_str());
    if (result != 0)
    {
        std::cerr << "Linking failed!" << std::endl;
        return false;
    }

    std::cout << "\nSuccessfully created executable: " << output_exe << std::endl;
    std::cout << "Run it with: ./" << output_exe << std::endl;

    return true;
}