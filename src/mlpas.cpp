#include <iostream>
#include "parser_driver.h"

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

Instruction encodeInstruction(ParsedLine const &line)
{
    Instruction parsed_inst = Instruction({"", UNKNOWN, 0, 0, ""});

    /*
        |         inst   | opcode | op1 | op2 | op3 |       packing       |
        |----------------|--------|-----|-----|-----|---------------------|
        |mov rd, i8      |  0000  | 4x  | 8x  | --  | 0000 rdrd #### #### |
        |load rd, rm:rn  |  0010  | 4x  | 4x  | 4x  | 0010 rdrd rmrm rnrn |
        |store rm:rn, rs |  0011  | 4x  | 4x  | 4x  | 0011 rsrs rmrm rnrn |
        |addr rd, ra, rb |  0100  | 4x  | 4x  | 4x  | 0100 rdrd rara rbrb |
        |addi rd, i8     |  0101  | 4x  | 8x  | --  | 0101 rdrd #### #### |
        |subr rd, ra, rb |  0110  | 4x  | 4x  | 4x  | 0110 rdrd rara rbrb |
        |subi rd, i8     |  0111  | 4x  | 8x  | --  | 0111 rdrd #### #### |
        |jnz  i8         |  1000  | 8x  | --  | --  | 1000 0000 @@@@ @@@@ |
        |jz  r           |  1001  | 4x  | --  | --  | 1001 rrrr 0000 0000 |
        |jmp r, r        |  1010  | 4x  | 4x  | --  | 1010 rrrr rrrr 0000 |
        |nop             |  1111  | --  | --  | --  | 1111 0000 0000 0000 |
    */
    if (line.opcode == "addi") {
        // Add immediate to a register
        parsed_inst = Instruction({
            .label = line.label, 
            .opcode = ADDI,
            .packed_args = ((line.arg0_reg & 0xF) << 8) | 
                            (line.arg1_imm & 0xFF),
            .address = 0});
    } else if (line.opcode == "addr") {
        // Add two registers
        parsed_inst = Instruction({
            .label = line.label, 
            .opcode = ADDRR,
            .packed_args = ((line.arg0_reg & 0xF) << 8) |
                           ((line.arg1_reg & 0xF) << 4) | 
                           ((line.arg2_reg & 0xF)),
            .address = 0});
    } else if (line.opcode == "subi") {
        // Subtract immediate from a register
        parsed_inst = Instruction({
            .label = line.label, 
            .opcode = SUBI,
            .packed_args = ((line.arg0_reg & 0xF) << 8) | 
                            (line.arg1_imm & 0xFF),
            .address = 0});
    } else if (line.opcode == "subr") {
        // Subtract two registers
        parsed_inst = Instruction({
            .label = line.label, 
            .opcode = SUBRR,
            .packed_args = ((line.arg0_reg & 0xF) << 8) |
                           ((line.arg1_reg & 0xF) << 4) | 
                           ((line.arg2_reg & 0xF)),
            .address = 0});
    } else if (line.opcode == "jnz") {
        // Jump to label
        parsed_inst = Instruction({
            .label = line.label, 
            .opcode = JNZI,
            .packed_args = 0, 
            .address = 0, 
            .target_label = line.arg0_label});
    } else if (line.opcode == "mov") {
        parsed_inst = Instruction({
            .label = line.label, 
            .opcode = MOVIR,
            .packed_args = ((line.arg0_reg & 0xF) << 8) | 
                            (line.arg1_imm & 0xFF),
            .address = 0});
    } else if (line.opcode == "nop") {
        parsed_inst = Instruction({
            .label = line.label, 
            .opcode = NOP,
            .packed_args = 0,
            .address = 0});
    } else if (line.opcode == "load") {
        parsed_inst = Instruction({
            .label = line.label,
            .opcode = LOAD,
            .packed_args = ((line.arg0_reg & 0xF) << 8) | 
                           ((line.arg1_reg & 0xF) << 4) |
                           ((line.arg2_reg & 0xF)),
            .address = 0});
    } else if (line.opcode == "store") {
        parsed_inst = Instruction({
            .label = line.label, 
            .opcode = STORE,
            .packed_args = ((line.arg0_reg & 0xF) << 8) | 
                           ((line.arg1_reg & 0xF) << 4) |
                           ((line.arg2_reg & 0xF)),
            .address = 0});
    } else {
        std::cerr << "opcode '" << line.opcode << "' not supported" << std::endl;
    }
    return parsed_inst;
}

std::vector<Instruction> parseInput(std::vector<ParsedLine> const &parsedProgram)
{
    std::vector<Instruction> parsed_instructions;

    int lineNum = 1;
    int byte_counter = 0;
    for (auto &line : parsedProgram) {
        Instruction inst = encodeInstruction(line);
        inst.address = byte_counter;
        if (!inst.label.empty()) {
            if (jump_table.count(inst.label) != 0) {
                std::cerr << "ERROR: Duplicated label '"<< inst.label << "'" <<
                    " at line" << lineNum << std::endl;
            } else {
                jump_table[inst.label] = inst.address;
            }
        }
        // std::cout << "[@" << std::setw(2) << std::setfill('0') << std::hex << inst.address << "] " <<
        //              std::setw(25) << std::setfill(' ') << curLine << std::endl;
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
                    "(line " << lineNum << ")" << std::endl;
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

void write_program_hex(std::ostream &out_file, std::vector<Instruction> const &parsed_instructions)
{
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
//    out_file.close();
}

void write_mem_hex(std::string out_file_path, std::vector<Instruction> const &parsed_instructions)
{
    std::ofstream out_file(out_file_path, std::ios::out);
    if (!out_file) {
        std::cerr << "Error opening file '" << out_file_path << "'";
        return;
    }

    for (auto inst : parsed_instructions) {
        out_file << 
            "[" << std::setw(2) << std::setfill('0') << std::hex << inst.address << "] " <<
            std::setw(4) << std::setfill('0') << std::bitset<4>(inst.opcode) << " " <<
            std::setw(4) << std::setfill('0') << std::bitset<4>(inst.packed_args >> 8) << " " <<
            std::setw(8) << std::setfill('0') << std::bitset<8>(inst.packed_args & 0xFF) << std::endl;
    }
}

int main(int argc, char *argv[])
{
    ParserDriver drv;
    std::string infile;
    for (int i = 1; i < argc; ++i) {
        if (argv[i] == std::string("-p"))
            drv.trace_parsing = true;
        else if (argv[i] == std::string("-s"))
            drv.trace_scanning = true;
        else 
            infile = argv[i];
    }
    int res = drv.parse(infile);
    if (res == 0) {
        unsigned int nlines = drv.lines.size(); 
        std::cout << nlines << " line" << (nlines == 1? "":"s") << " parsed successfully." << std::endl;
        auto parsed_instructions = parseInput(drv.lines);
    //    write_program_binary(argv[2], parsed_instructions);
        if (false /* output file was given as argument */) {
            std::string out_file_path = "out.hex";
            std::ofstream out_file(out_file_path, std::ios::out);
            if (!out_file) {
                std::cerr << "Error opening file '" << out_file_path << "'";
            }
            write_program_hex(out_file, parsed_instructions);
            out_file.close();
        } else {
            std::cout << "Hex-encoded binary:" << std::endl;
            write_program_hex(std::cout, parsed_instructions);
            std::cout << std::endl;
        }
        write_mem_hex("mem.hex", parsed_instructions);
    }
}