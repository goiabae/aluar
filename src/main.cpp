#include <assert.h>
#include <stdio.h>

#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "eval.hpp"
#include "lex.hpp"
#include "parse.hpp"

using str = std::string;

str read_file(std::filesystem::path path) {
	std::ifstream f(path, std::ios::in | std::ios::binary);
	const auto sz = std::filesystem::file_size(path);
	str result(sz, '\0');
	f.read(result.data(), sz);
	return result;
}

void print_token(Token tk, const str &src) {
	printf("Token (beg: %lu, len: %lu, str: \"", tk.beg, tk.len);
	for (size_t i = 0; i < tk.len; i++)
		if (src[tk.beg + i] == '\n')
			printf("\\n");
		else
			printf("%c", src[tk.beg + i]);
	printf("\")\n");
}

void print_str(const str &src) {
	for (size_t i = 0; i < src.length(); i++) printf("%c", src[i]);
}

void print_node(CST::Node *node, const str &src) {
	switch (node->type) {
		case CST::Type::App:
			printf("(");
			for (auto child : node->children) {
				print_node(child, src);
				printf(" ");
			}
			printf(")");
			break;
		case CST::Type::Number:
		case CST::Type::Symbol: {
			str num = src.substr(node->beg, node->len);
			printf("'");
			print_str(num);
			break;
		}
		case CST::Type::String: {
			str num = src.substr(node->beg, node->len);
			printf("\"");
			print_str(num);
			printf("\"");
			break;
		}
	}
	return;
}

void print_tree(const CST &tree, const str &src) { print_node(tree.root, src); }

void print_value(Value val) {
	switch (val.type) {
		case Value::Type::Number: {
			printf("%ld", *val.value.number);
			break;
		}
		case Value::Type::Symbol: {
			printf("\'");
			print_str(*val.value.symbol);
			break;
		}
		case Value::Type::String: {
			printf("\"");
			print_str(*val.value.string);
			printf("\"");
			break;
		}
		case Value::Type::Error:
			printf("ERROR: ");
			print_str(*val.value.error_msg);
			break;
	}
}

int main(int argc, char *argv[]) {
	if ((argc - 1) < 1) {
		return 1;
	}

	const auto src = read_file(argv[1]);
	print_str(src);
	auto tks = tokenize(src);
	for (auto &tk : tks) {
		print_token(tk, src);
	}
	const auto tree = parse(tks);
	print_tree(tree, src);
	printf("\n");
	Value res = eval(tree, src);

	print_value(res);
	printf("\n");
	return res.type == Value::Type::Error;
}
