#include <stdio.h>

#include <filesystem>
#include <fstream>
#include <iostream>
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
	};

	size_t beg;
	size_t len;
	Type type;
};

#define TOK_OP(CHAR, TOKEN)                            \
	tks.push_back(Token {index, 1, Token::Type::TOKEN}); \
	index += 1;

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
			case '+': TOK_OP('+', Plus); break;
			case '-': TOK_OP('-', Minus); break;
			case '*': TOK_OP('*', Star); break;
			case '=': TOK_OP('=', Equal); break;
			case '&': TOK_OP('&', Ampersand); break;
			case '/': TOK_OP('/', Slash); break;
			case '!': TOK_OP('!', Bang); break;
			case '%': TOK_OP('%', Percent); break;
			case '^': TOK_OP('^', Caret); break;
			case '(': TOK_OP('(', ParenOpen); break;
			case ')': TOK_OP(')', ParenClose); break;
			case '[': TOK_OP('[', BracketOpen); break;
			case ']': TOK_OP(']', BracketClose); break;
			case '{': TOK_OP('{', BraceOpen); break;
			case '}': TOK_OP('}', BraceClose); break;
			case '\n': TOK_OP('\n', Newline); break;
			case ';': TOK_OP(';', Semicolon); break;
			case ',': TOK_OP(',', Comma); break;
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
int main(int argc, char *argv[argc]) {
	str src = read_file(argv[1]);
	print_str(src);
	const auto tks = tokenize(src);
	for (auto &tk : tks) {
		print_token(tk, src);
	}
	return 0;
}
