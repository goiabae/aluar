#include <assert.h>
#include <stdio.h>

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <map>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "eval.hpp"
#include "io.hpp"
#include "lex.hpp"
#include "parse.hpp"

using str = std::string;

int run_file(const char *filename) {
	const auto src = read_file(filename);
	auto tks = tokenize(src);
	const auto tree = parse(tks);
	if (!tree.success) {
		printf("Parsing error!\n");
		return 1;
	}
	const auto val = eval(tree, src);
	print_value(val);
	return val.type == Value::Type::Error;
}

int run_repl() {
	std::string src;

	while (true) {
		printf("> ");
		std::getline(std::cin, src);
		if (src == "") break;
		auto tks = tokenize(src);
		const auto tree = parse(tks);
		if (!tree.success) {
			printf("Parsing error!\n");
			continue;
		}
		const auto val = eval(tree, src);
		print_value(val);
	}

	return 0;
}

int main(int argc, char *argv[]) {
	if (argc == 1) {
		return run_repl();
	} else {
		return run_file(argv[1]);
	}
}
