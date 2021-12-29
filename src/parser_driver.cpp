#include "parser_driver.h"
#include "grammar.hpp"

ParserDriver::ParserDriver()
    : trace_parsing(false), trace_scanning(false)
{
}

int ParserDriver::parse(const std::string &f)
{
    file = f;
    location.initialize(&file);
    scan_begin();
    yy::parser parser(*this);
    parser.set_debug_level(trace_parsing);
    result = parser.parse();
    scan_end();
    return result;
}

void yy::parser::error(yy::location const &loc, const std::string &msg) {
    std::cout << "An error occured: " << msg << std::endl;
    std::cout << "    at: " << loc << std::endl;
}