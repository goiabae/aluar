#include "io.hpp"

#include <filesystem>
#include <fstream>

#include "eval.hpp"
#include "lex.hpp"
#include "parse.hpp"

using str = std::string;

void print_str(const std::string &src) {
	for (size_t i = 0; i < src.length(); i++) printf("%c", src[i]);
}

std::string read_file(std::filesystem::path path) {
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

void print_value(const Value &val) {
	switch (val.type) {
		case Value::Type::Number: {
			printf("= %ld : Number", *(int64_t *)val.box);
			break;
		}
		case Value::Type::Symbol: {
			printf("= \'");
			print_str(*(std::string *)val.box);
			printf(" : Symbol");
			break;
		}
		case Value::Type::String: {
			printf("= \"");
			print_str(*(std::string *)val.box);
			printf("\"");
			printf(" : String");
			break;
		}
		case Value::Type::Nil: {
			printf("nil");
			break;
		}
		case Value::Type::Error:
			printf("= ");
			print_str(*(std::string *)val.box);
			printf(" : Error");
			break;
	}
	printf("\n");
}
