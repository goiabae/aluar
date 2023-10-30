#include "lex.hpp"

#include <string>
#include <vector>

using str = std::string;

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
	Token res {index, 1, Token::Type::Word};
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
			case '<': TOK_FIXED_SZ(1, LeftAngled); break;
			case '>': TOK_FIXED_SZ(1, RightAngled); break;
			case '\n': TOK_FIXED_SZ(1, Newline); break;
			case ';': TOK_FIXED_SZ(1, Semicolon); break;
			case ',': TOK_FIXED_SZ(1, Comma); break;
			case '\'': TOK_FIXED_SZ(1, SingleQuote); break;
			case '"': {
				size_t read = 1;
				while (src[index + read++] != '"')
					;
				Token tk = Token {index, read, Token::Type::String};
				tks.push_back(tk);
				index += read;
				break;
			}
			case ' ': {
				size_t read = 1;
				while (src[index + read] == ' ') read++;
				Token tk = Token {index, read, Token::Type::Spaces};
				tks.push_back(tk);
				index += read;
				break;
			}
			case '\t': {
				size_t read = 1;
				while (src[index + read] == '\t') read++;
				Token tk = Token {index, read, Token::Type::Tabs};
				tks.push_back(tk);
				index += read;
				break;
			}
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
