
#include <fstream>
#include <iostream>
#include <regex>
#include <string>

enum OpCode {
    MOVIR = 0,
    MOVRR = 1,
    LOAD = 2,
    STORE = 3,
    ADDRR = 4,
    ADDI = 5,
    SUBRR = 6,
    SUBI = 7,
    JZI = 8,
    JZR = 9,
    NOP = 15,
    UNKNOWN = 9999
};

struct Instruction {
    std::string label;
    OpCode opcode;
    unsigned int packed_args;
    unsigned int address;
    std::string jump_label;
};

Instruction parseLine(std::string line, int lineNum, std::string filename)
{
    Instruction parsed_inst = Instruction({ "", UNKNOWN, 0 });
    std::string match_expr = "^\\s*(([a-zA-Z0-9]+):)?\\s*([a-z]+)"             // (optional label:) opcode
                             "(\\s+(r[0-9]+|@[0-9]+|#[0-9]+|[a-zA-Z0-9]+))?"   //               optional arg0
                             "(\\s*[,]\\s*(r[0-9]+|@[0-9]+|#[0-9]+))?"         //               optional arg1   
                             "(\\s*[,]\\s*(r[0-9]+))?";                        //               optional arg2
    std::regex re(match_expr);
    std::smatch match;

    std::string label, opcode, arg0, arg1, arg2;
    if (std::regex_search(line, match, re) && match.size() > 1) {
        label = match.str(2);
        opcode = match.str(3);
        arg0 = match.str(5);
        arg1 = match.str(7);
        arg2 = match.str(9);
    } else {
        std::cerr << "Couldn't parse '" << line << "'" << " (" << filename << ":" << lineNum << ")" << std::endl;
    }
    if (opcode == "nop") {
        parsed_inst = Instruction({ label, NOP, 0 });
    } else if (opcode == "mov") {
        if (arg0.empty() || arg1.empty() || !arg2.empty()) {
            std::cerr << "ERROR: Wrong number of arguments for 'mov'"<< " (" << filename << ":" << lineNum << ")" << std::endl;
        } else {
            if (arg0[0] != 'r') {
                std::cerr << "ERROR: First argument for 'mov' must be a register" << " (" << filename << ":" << lineNum << ")" << std::endl;
            }
            if (arg1[0] == '#') {
                parsed_inst = Instruction({ label, MOVIR, 0 });
            } else if (arg1[0] == 'r') {
                parsed_inst = Instruction({ label, MOVRR, 0 });
            } else {
                std::cerr << "ERROR: Wrong argument type of second argument for 'mov'"<< " (" << filename << ":" << lineNum << ")" << std::endl;
            }
        }
    } else if (opcode == "jz") {
        if (arg0.empty() || !arg1.empty() || !arg2.empty()) {
            std::cerr << "ERROR: Wrong number of arguments for 'jz'"<< " (" << filename << ":" << lineNum << ")" << std::endl;
        } else {
            if (arg1[0] == '#') {
                // Relative jump
                parsed_inst = Instruction({ label, JZI, 0 });
            } else if (arg1[0] == '@') {
                // Jump to address in register
                parsed_inst = Instruction({ label, JZR, 0 });
            } else {
                // Jump to label
                parsed_inst = Instruction({ label, JZI, 0, 0, arg1 });
            }
        }
    } else {
        std::cerr << "opcode '" << opcode << "' not supported" << " (" << filename << ":" << lineNum << ")" << std::endl;
    }
    return parsed_inst;
}

void parseInput(std::string infile_path)
{
    std::ifstream in_file(infile_path);

    if (!in_file) {
        std::cerr << "Error opening file '" << infile_path << "'";
        return;
    }
    std::vector<Instruction> parsed_instructions;
    int lineNum = 1;
    std::string curLine;
    int byte_counter = 0;
    while (std::getline(in_file, curLine)) {
        Instruction inst = parseLine(curLine, lineNum, infile_path);
        inst.address = byte_counter;
        std::cout << "%% " << curLine << ": [@" << inst.address << "] " << inst.opcode << std::endl;
        parsed_instructions.push_back(inst);
        lineNum += 1;
        byte_counter += 2;
    }

}

int main(int argc, char **argv)
{
    parseInput(argv[1]);
}