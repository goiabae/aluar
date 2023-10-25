#ifndef ALUAR_LEX_HPP
#define ALUAR_LEX_HPP

#include <cstdlib>
#include <vector>
#include <string>

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
		Word,
		Newline,
		Semicolon,
		Comma,
		String,
		SingleQuote,
		Spaces,
		Tabs,
	};

	size_t beg;
	size_t len;
	Type type;
};

std::vector<Token> tokenize(std::string src);

#endif
