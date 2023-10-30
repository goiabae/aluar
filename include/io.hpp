#ifndef ALUAR_IO_HPP
#define ALUAR_IO_HPP

#include <filesystem>
#include <string>

#include "eval.hpp"

void print_str(const std::string &src);
std::string read_file(std::filesystem::path path);
void print_value(const Value &val);

#endif
