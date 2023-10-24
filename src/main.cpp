#include <stdio.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <span>
#include <string>
#include <vector>

using str = std::string;

str read_file(std::filesystem::path path) {
	std::ifstream f(path, std::ios::in | std::ios::binary);
	const auto sz = std::filesystem::file_size(path);
	str result(sz, '\0');
	f.read(result.data(), sz);
	return result;
}

struct Token {
	enum class Type {
		Plus,
		Minus,
		Star,
		Slash,
		Equal,
		Ampersand,
		Bang,
		Percent,
		Caret,
		ParenOpen,
		ParenClose,
		BracketOpen,
		BracketClose,
		BraceOpen,
		BraceClose,
		Number,
		Letter,
		Newline,
		Semicolon,
		Comma,
		DoubleQuote,
	};

	size_t beg;
	size_t len;
	Type type;
};

#define TOK_FIXED_SZ(SZ, TOKEN)                         \
	tks.push_back(Token {index, SZ, Token::Type::TOKEN}); \
	index += SZ;

Token tokenize_number(str &src, size_t index) {
	Token res {index, 1, Token::Type::Number};
	if (src[index] == '0' && (src[index + 1] == 'x' || src[index + 1] == 'b')) {
		res.len += 1;
	}
	while (true)
		if ('0' <= src[index + res.len] && src[index + res.len] <= '9')
			res.len += 1;
		else
			break;
	return res;
}

Token tokenize_word(str &src, size_t index) {
	Token res {index, 1, Token::Type::Letter};
	while (true)
		if (('a' <= src[index + res.len] && src[index + res.len] <= 'z') || ('A' <= src[index + res.len] && src[index + res.len] <= 'Z'))
			res.len += 1;
		else
			break;
	return res;
}

std::vector<Token> tokenize(str src) {
	std::vector<Token> tks;
	size_t index = 0;
	printf("src.length() is %lu\n", src.length());
	while (true) {
		if (index >= src.length()) return tks;
		switch (src[index]) {
			case '+': TOK_FIXED_SZ(1, Plus); break;
			case '-': TOK_FIXED_SZ(1, Minus); break;
			case '*': TOK_FIXED_SZ(1, Star); break;
			case '=': TOK_FIXED_SZ(1, Equal); break;
			case '&': TOK_FIXED_SZ(1, Ampersand); break;
			case '/': TOK_FIXED_SZ(1, Slash); break;
			case '!': TOK_FIXED_SZ(1, Bang); break;
			case '%': TOK_FIXED_SZ(1, Percent); break;
			case '^': TOK_FIXED_SZ(1, Caret); break;
			case '(': TOK_FIXED_SZ(1, ParenOpen); break;
			case ')': TOK_FIXED_SZ(1, ParenClose); break;
			case '[': TOK_FIXED_SZ(1, BracketOpen); break;
			case ']': TOK_FIXED_SZ(1, BracketClose); break;
			case '{': TOK_FIXED_SZ(1, BraceOpen); break;
			case '}': TOK_FIXED_SZ(1, BraceClose); break;
			case '\n': TOK_FIXED_SZ(1, Newline); break;
			case ';': TOK_FIXED_SZ(1, Semicolon); break;
			case ',': TOK_FIXED_SZ(1, Comma); break;
			case '"': TOK_FIXED_SZ(1, DoubleQuote); break;
			case ' ':
			case '\t': index += 1; break;
			default:
				if (('a' <= src[index] && src[index] <= 'z') || ('A' <= src[index] && src[index] <= 'Z')) {
					Token tk = tokenize_word(src, index);
					tks.push_back(tk);
					index += tk.len;
				} else if ('0' <= src[index] && src[index] <= '9') {
					Token tk = tokenize_number(src, index);
					tks.push_back(tk);
					index += tk.len;
				}
				break;
		}
	}
}

void print_token(Token tk, str &src) {
	printf("Token (len: %lu, str: \"", tk.len);
	for (int i = 0; i < tk.len; i++)
		if (src[tk.beg + i] == '\n')
			printf("\\n");
		else
			printf("%c", src[tk.beg + i]);
	printf("\")\n");
}

void print_str(str &src) {
	printf("from source:\n");
	printf("\"\"\"\n");
	for (int i = 0; i < src.length(); i++) printf("%c", src[i]);
	printf("\"\"\"\n");
	printf("\n");
}

// Concrete Syntax Tree
struct CST {
	enum class Type { Map, List, Number, String, App, Assignment, Symbol, Hole };
	struct Node {
		Type type;
		std::vector<Node *> children;
	};

	Node *root;
};

CST::Node *parse_node(CST::Node *root, const std::span<Token> &tks) {
	switch (tks[0].type) {
		// <map> ::= { exps }
		case Token::Type::BraceOpen: {
			root = new CST::Node;
			root->type = CST::Type::Map;
			CST::Node *child = nullptr;
			while ((child = parse_node(nullptr, tks.subspan(1, tks.size() - 1)))) {
				root->children.push_back(child);
			}
			break;
		}
	}
}

CST parse(const std::span<Token> &tks) {
	CST tree {};
	tree.root = parse_node(tree.root, tks);
}

int main(int argc, char *argv[]) {
	str src = read_file(argv[1]);
	print_str(src);
	auto tks = tokenize(src);
	/*
	for (auto &tk : tks) {
		print_token(tk, src);
	}
	*/
	const auto tree = parse(tks);
	return 0;
}
