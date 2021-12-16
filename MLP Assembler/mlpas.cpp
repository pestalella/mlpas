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
    LOAD = 2,
    STORE = 3,
    ADDRR = 4,
    ADDI = 5,
    SUBRR = 6,
    SUBI = 7,
    JNZI = 8,
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

const char *ws = " \t\n\r\f\v";

// trim from end of string (right)
std::string &rtrim(std::string &s, const char *t = ws)
{
    s.erase(s.find_last_not_of(t) + 1);
    return s;
}

// trim from beginning of string (left)
std::string &ltrim(std::string &s, const char *t = ws)
{
    s.erase(0, s.find_first_not_of(t));
    return s;
}

std::string &removeComment(std::string &line)
{
    size_t comment_start_pos = line.find_first_of(";");
    if (comment_start_pos == std::string::npos)
        return line;

    line.erase(comment_start_pos);
    return line;
}

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

    if (opcode == "add") {
        if (arg0.empty() || arg1.empty()) {
            std::cerr << "ERROR: Wrong number of arguments for 'add'" <<
                " (" << filename << ":" << lineNum << ")" << std::endl;
        } else {
            if (arg0[0] != 'r') {
                std::cerr << "ERROR: First argument for 'add' must be the destination register, not '" << arg0 << "'" <<
                    " (" << filename << ":" << lineNum << ")" << std::endl;
            } else {
                unsigned int reg0 = (unsigned)std::stoi(arg0.substr(1));
                if (arg1[0] == '#') {
                    // Add immediate to a register
                    unsigned int reg0 = (unsigned)std::stoi(arg0.substr(1));
                    parsed_inst = Instruction({.label = label, .opcode = ADDI,
                        .packed_args = ((reg0 &0xF) << 8) | ((unsigned)std::stoi(arg1.substr(1)) & 0xFF)});
                } else if (arg1[0] == 'r' && arg2[0] == 'r') {
                    unsigned int reg1 = (unsigned)std::stoi(arg1.substr(1));
                    unsigned int reg2 = (unsigned)std::stoi(arg2.substr(1));
                    parsed_inst = Instruction({.label = label, .opcode = ADDRR,
                        .packed_args = ((reg0 &0xF) << 8) |((reg1 & 0xF) << 4) | ((reg2 & 0xF))});
                } else {
                    std::cerr << "ERROR: Second and third arguments for 'add' must be the two source registers, not '" <<
                        arg1 << "' and '" << arg2 << "'" <<
                        " (" << filename << ":" << lineNum << ")" << std::endl;
                }
            }
        }

    } else if (opcode == "jnz") {
        if (arg0.empty() || !arg1.empty() || !arg2.empty()) {
            std::cerr << "ERROR: Wrong number of arguments for 'jz'" <<
                " (" << filename << ":" << lineNum << ")" << std::endl;
        } else {
            if (arg0[0] == '#') {
                // Relative jump
                parsed_inst = Instruction({.label = label, .opcode = JNZI, .packed_args = 0});
            } else if (arg0[0] == '@') {
                // Jump to address in register
                parsed_inst = Instruction({.label = label, .opcode = JZR, .packed_args = 0});
            } else {
                // Jump to label
                parsed_inst = Instruction({.label = label, .opcode = JNZI,
                    .packed_args = 0, .address = 0, .target_label = arg0});
            }
        }

    } else if (opcode == "mov") {
        if (arg0.empty() || arg1.empty() || !arg2.empty()) {
            std::cerr << "ERROR: Wrong number of arguments for 'mov'"<<
                " (" << filename << ":" << lineNum << ")" << std::endl;
        } else {
            unsigned int reg0 = (unsigned)std::stoi(arg0.substr(1));
            if (arg0[0] != 'r') {
                std::cerr << "ERROR: First argument for 'mov' must be a register" <<
                    " (" << filename << ":" << lineNum << ")" << std::endl;
            } else if (arg1[0] == '#') {
                parsed_inst = Instruction({.label = label, .opcode = MOVIR,
                    .packed_args = ((reg0 &0xF) << 8) | ((unsigned)std::stoi(arg1.substr(1)) & 0xFF)});
            } else {
                std::cerr << "ERROR: Wrong argument type of second argument for 'mov'" <<
                    " (" << filename << ":" << lineNum << ")" << std::endl;
            }
        }

    } else if (opcode == "nop") {
        parsed_inst = Instruction({.label = label, .opcode = NOP});

    } else if (opcode == "load") {
        if (arg1[0] == '@') {
            unsigned int reg = (unsigned)std::stoi(arg0.substr(1));
            parsed_inst = Instruction({.label = label, .opcode = LOAD,
                .packed_args = ((reg & 0xF) << 8) | ((unsigned)std::stoi(arg1.substr(1)) & 0xFF)});
        } else {
            parsed_inst = Instruction({.label = label, .opcode = LOAD,
                .packed_args = (((unsigned)std::stoi(arg0.substr(1)) & 0xF) << 8), .address = 0, .target_label = arg1});
        }

    } else if (opcode == "store") {
        if (arg0[0] == '@') {
            unsigned int reg = (unsigned)std::stoi(arg1.substr(1));
            parsed_inst = Instruction({.label = label, .opcode = STORE,
                .packed_args = ((reg & 0xF) << 8) | ((unsigned)std::stoi(arg0.substr(1)) & 0xFF)});
        } else {
            parsed_inst = Instruction({.label = label, .opcode = STORE,
                .packed_args = (((unsigned)std::stoi(arg1.substr(1)) & 0xF) << 8), .address = 0, .target_label = arg0});
        }

    }  else if (opcode == "sub") {
        if (arg0.empty() || arg1.empty() || arg2.empty()) {
            std::cerr << "ERROR: Wrong number of arguments for 'sub'" <<
                " (" << filename << ":" << lineNum << ")" << std::endl;
        } else {
            if (arg0[0] != 'r') {
                std::cerr << "ERROR: First argument for 'sub' must be the destination register, not '" << arg0 << "'" <<
                    " (" << filename << ":" << lineNum << ")" << std::endl;
            } else {
                unsigned int reg0 = (unsigned)std::stoi(arg0.substr(1));
                if (arg1[0] == '#') {
                    // Subtract immediate from a register
                    parsed_inst = Instruction({.label = label, .opcode = SUBI, .packed_args = (unsigned)std::stoi(arg1.substr(1))});
                } else if (arg1[0] == 'r' && arg2[0] == 'r') {
                    unsigned int reg1 = (unsigned)std::stoi(arg1.substr(1));
                    unsigned int reg2 = (unsigned)std::stoi(arg2.substr(1));
                    parsed_inst = Instruction({.label = label, .opcode = SUBRR,
                        .packed_args = ((reg0 &0xF) << 8) |((reg1 & 0xF) << 4) | ((reg2 & 0xF))});
                } else {
                    std::cerr << "ERROR: Second and third arguments for 'sub' must be the two source registers, not '" <<
                        arg1 << "' and '" << arg2 << "'" <<
                        " (" << filename << ":" << lineNum << ")" << std::endl;
                }
            }
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
        return std::vector<Instruction>();
    }

    int lineNum = 1;
    std::string curLine;
    int byte_counter = 0;
    while (std::getline(in_file, curLine)) {
        curLine = ltrim(rtrim(removeComment(curLine)));
        if (curLine.empty()) continue;

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
        curLine = ltrim(rtrim(curLine));
        std::cout << "[@" << std::setw(2) << std::setfill('0') << std::hex << inst.address << "] " <<
                     std::setw(25) << std::setfill(' ') << curLine << std::endl;
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
                inst.packed_args |= (label_iter->second & 0xFF);
            }
        }
    }
    return parsed_instructions;
}

void write_program_binary(std::string out_file_path, std::vector<Instruction> const &parsed_instructions)
{
    std::ofstream out_file(out_file_path, std::ios::binary | std::ios::out);
    if (!out_file) {
        std::cerr << "Error opening file '" << out_file_path << "'";
        return;
    }
    std::vector<uint16_t> machine_code(parsed_instructions.size());
    unsigned char *machine_code_bytes = reinterpret_cast<unsigned char *>(&machine_code[0]);
    size_t inst_count = 0;
    for (auto inst : parsed_instructions) {
        machine_code[inst_count] = (((inst.opcode & 0xF) << 12) | (inst.packed_args & 0xFFF));
        // Change endinanness
        std::swap(machine_code_bytes[2*inst_count + 0], machine_code_bytes[2*inst_count + 1]);
        inst_count++;
    }
    out_file.write(reinterpret_cast<char *>(&machine_code[0]), inst_count*2);
    out_file.close();
}

void write_program_hex(std::string out_file_path, std::vector<Instruction> const &parsed_instructions)
{
    std::ofstream out_file(out_file_path, std::ios::out);
    if (!out_file) {
        std::cerr << "Error opening file '" << out_file_path << "'";
        return;
    }
    std::vector<uint16_t> machine_code(parsed_instructions.size());
    unsigned char *machine_code_bytes = reinterpret_cast<unsigned char *>(&machine_code[0]);
    size_t inst_count = 0;
    for (auto inst : parsed_instructions) {
        machine_code[inst_count] = (((inst.opcode & 0xF) << 12) | (inst.packed_args & 0xFFF));
        // Change endinanness
        std::swap(machine_code_bytes[2*inst_count + 0], machine_code_bytes[2*inst_count + 1]);
        inst_count++;
    }

    for (auto curInst : machine_code) {
        out_file << std::setfill('0') << std::setw(2) << std::hex << (curInst & 0x00FF);
        out_file << " ";
        out_file << std::setfill('0') << std::setw(2) << std::hex << ((curInst >> 8) & 0x00FF);
        out_file << " ";
        //        out_file << std::setfill(' ') << std::setw(3) << std::hex << 50 << ' ' << 12;
    }
    out_file.close();
}

int main(int argc, char **argv)
{
    if (argc != 3) {
        std::cerr << "Usage: mlpas <input_assembly_file> <output_binary_file>" << std::endl;
        return 1;
    }
    auto parsed_instructions = parseInput(argv[1]);
//    write_program_binary(argv[2], parsed_instructions);
    write_program_hex(argv[2], parsed_instructions);

    for (auto inst : parsed_instructions) {
        std::cout << "[" << std::setw(2) << std::setfill('0') << std::hex << inst.address << "] " <<
            std::setw(4) << std::setfill('0') << std::bitset<4>(inst.opcode) << " " <<
            std::setw(4) << std::setfill('0') << std::bitset<4>(inst.packed_args >> 8) << " " <<
            std::setw(8) << std::setfill('0') << std::bitset<8>(inst.packed_args & 0xFF) << std::endl;
    }
}
