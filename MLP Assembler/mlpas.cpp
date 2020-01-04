
#include <bitset>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <regex>
#include <string>

enum OpCode
{
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

struct Instruction
{
    std::string label;
    OpCode opcode;
    unsigned int packed_args;
    unsigned int address;
    std::string target_label;
};

std::map<std::string, int> jump_table;

Instruction parseLine(std::string line, int lineNum, std::string filename)
{
    Instruction parsed_inst = Instruction({"", UNKNOWN, 0});
    std::string match_expr = "^\\s*(([a-zA-Z][a-zA-Z0-9]*):)?\\s*([a-z]+)"   // (optional label:) opcode
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

    if (opcode == "jz") {
        if (arg0.empty() || !arg1.empty() || !arg2.empty()) {
            std::cerr << "ERROR: Wrong number of arguments for 'jz'" << 
                " (" << filename << ":" << lineNum << ")" << std::endl;
        } else {
            if (arg0[0] == '#') {
                // Relative jump
                parsed_inst = Instruction({.label = label, .opcode = JZI, .packed_args = 0});
            } else if (arg0[0] == '@') {
                // Jump to address in register
                parsed_inst = Instruction({.label = label, .opcode = JZR, .packed_args = 0});
            } else {
                // Jump to label
                parsed_inst = Instruction({.label = label, .opcode = JZI,
                    .packed_args = 0, .address = 0, .target_label = arg0});
            }
        }
    } else if (opcode == "mov") {
        if (arg0.empty() || arg1.empty() || !arg2.empty()) {
            std::cerr << "ERROR: Wrong number of arguments for 'mov'"<<
                " (" << filename << ":" << lineNum << ")" << std::endl;
        } else {
            if (arg0[0] != 'r') {
                std::cerr << "ERROR: First argument for 'mov' must be a register" <<
                    " (" << filename << ":" << lineNum << ")" << std::endl;
            }
            if (arg1[0] == '#') {
                parsed_inst = Instruction({.label = label, .opcode = MOVIR, .packed_args = 0});
            } else if (arg1[0] == 'r') {
                parsed_inst = Instruction({.label = label, .opcode = MOVRR, .packed_args = 0});
            } else {
                std::cerr << "ERROR: Wrong argument type of second argument for 'mov'" <<
                    " (" << filename << ":" << lineNum << ")" << std::endl;
            }
        }
    } else if (opcode == "nop") {
        parsed_inst = Instruction({.label = label, .opcode = NOP});
    } else if (opcode == "store") {
        if (arg0[0] == '@') {
            parsed_inst = Instruction({.label = label, .opcode = STORE, 
                .packed_args = (unsigned)std::stoi(arg0.substr(1))});
        } else {
            parsed_inst = Instruction({.label = label, .opcode = STORE, 
                .packed_args = 0, .address = 0, .target_label = arg0});
        }
    } else {
        std::cerr << "opcode '" << opcode << "' not supported" << 
            " (" << filename << ":" << lineNum << ")" << std::endl;
    }
    return parsed_inst;
}

std::vector<Instruction> parseInput(std::string infile_path)
{
    std::ifstream in_file(infile_path);
    std::vector<Instruction> parsed_instructions;

    if (!in_file) {
        std::cerr << "Error opening file '" << infile_path << "'";
        return parsed_instructions;
    }


    int lineNum = 1;
    std::string curLine;
    int byte_counter = 0;
    while (std::getline(in_file, curLine)) {
        Instruction inst = parseLine(curLine, lineNum, infile_path);
        inst.address = byte_counter;
        if (!inst.label.empty()) {
            if (jump_table.count(inst.label) != 0) {
                std::cerr << "ERROR: Duplicated label '"<< inst.label << "'" << 
                    " (" << infile_path << ":" << lineNum << ")" << std::endl;
            } else {
                jump_table[inst.label] = inst.address;
            }
        }
        std::cout << "%% " << curLine << ": [@" << inst.address << "] " << inst.opcode << std::endl;
        parsed_instructions.push_back(inst);
        lineNum += 1;
        byte_counter += 2;
    }
    // Resolve target labels
    for (auto &inst : parsed_instructions) {
        if (!inst.target_label.empty()) {
            auto label_iter = jump_table.find(inst.target_label);
            if (label_iter == jump_table.end()) {
                std::cerr << "ERROR: unknown jump target label '"<< inst.target_label << "'" << 
                    " (" << infile_path << ":" << lineNum << ")" << std::endl;
            } else {
                inst.packed_args = label_iter->second;
            }
        }
    }
    return parsed_instructions;
}

int main(int argc, char **argv)
{
    auto parsed_instructions = parseInput(argv[1]);
    for (auto inst : parsed_instructions) {
        std::cout << "[" << std::setw(2) << std::setfill('0') << std::hex << inst.address << "] " <<
            std::setw(4) << std::setfill('0') << std::bitset<4>(inst.opcode) << " " <<
            std::setw(4) << std::setfill('0') << std::bitset<4>(inst.packed_args >> 8) << " " <<
            std::setw(8) << std::setfill('0') << std::bitset<4>(inst.packed_args & 0xFF) << std::endl;
    }
}