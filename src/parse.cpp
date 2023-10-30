#include "parse.hpp"

#include <assert.h>

size_t parse_word(const std::span<Token> &tks, CST::Node **root) {
	*root = new CST::Node;
	(*root)->type = CST::Type::Symbol;
	(*root)->beg = tks[0].beg;
	(*root)->len = tks[0].len;
	return 1;
}

size_t parse_string(const std::span<Token> &tks, CST::Node **root) {
	*root = new CST::Node;
	(*root)->type = CST::Type::String;
	(*root)->beg = tks[0].beg + 1;
	(*root)->len = tks[0].len - 2;
	return 1;
}

size_t parse_number(const std::span<Token> &tks, CST::Node **root) {
	*root = new CST::Node;
	(*root)->type = CST::Type::Number;
	(*root)->beg = tks[0].beg;
	(*root)->len = tks[0].len;
	return 1;
}

size_t parse_node(const std::span<Token> &tks, CST::Node **root);

using Parser = size_t(const std::span<Token> &, CST::Node **);

// parses a sequence of nodes with the given function
size_t sequence_of(
	Parser p, const std::span<Token> &tks, std::vector<CST::Node *> &cs
) {
	CST::Node *tmp = nullptr;
	size_t read = p(tks, &tmp);

	if (read > 0) {
		cs.push_back(tmp);
		return read + sequence_of(p, tks.subspan(read), cs);
	} else {
		delete tmp;
		return read;
	}
}

size_t parse_app(const std::span<Token> &tks, CST::Node **root) {
	assert(tks[0].type == Token::Type::ParenOpen);

	// set current node (application)
	CST::Node *cur = new CST::Node;
	cur->beg = tks[0].beg;
	cur->len = tks[0].len;
	cur->type = CST::Type::App;
	*root = cur;

	// ( 1 2 3 ) -> 1 2 3
	const std::span<Token> middle = tks.subspan(1, tks.size() - 2);
	size_t read = sequence_of(parse_node, middle, cur->children);

	assert(tks[read + 1].type == Token::Type::ParenClose);
	return read + 2; // +2 for open and close parens
}
bool is_app(const std::span<Token> &tks) {
	return tks[0].type == Token::Type::ParenOpen;
}

bool is_number(const std::span<Token> &tks) {
	return tks[0].type == Token::Type::Number;
}

bool is_word(const std::span<Token> &tks) {
	switch (tks[0].type) {
		case Token::Type::Plus:
		case Token::Type::Minus:
		case Token::Type::Star:
		case Token::Type::Slash:
		case Token::Type::Equal:
		case Token::Type::Ampersand:
		case Token::Type::Bang:
		case Token::Type::Percent:
		case Token::Type::Caret:
		case Token::Type::BracketOpen:
		case Token::Type::BracketClose:
		case Token::Type::BraceOpen:
		case Token::Type::BraceClose:
		case Token::Type::LeftAngled:
		case Token::Type::RightAngled:
		case Token::Type::Word:
		case Token::Type::Semicolon:
		case Token::Type::Comma: return true;

		case Token::Type::Number:
		case Token::Type::String:
		case Token::Type::SingleQuote:
		case Token::Type::ParenOpen:
		case Token::Type::ParenClose:
		case Token::Type::Tabs:
		case Token::Type::Spaces:
		case Token::Type::Newline: return false;
	}
	return false;
}
size_t parse_node(const std::span<Token> &tks, CST::Node **root) {
	size_t read = 0;

	if (is_app(tks)) {
		read = parse_app(tks, root);
	} else if (is_number(tks)) {
		read = parse_number(tks, root);
	} else if (is_word(tks)) {
		read = parse_word(tks, root);
	} else if (tks[0].type == Token::Type::String) {
		read = parse_string(tks, root);
	} else if (tks[0].type == Token::Type::Newline || tks[0].type == Token::Type::Tabs || tks[0].type == Token::Type::Spaces) {
		read = 1 + parse_node(tks.subspan(1), root);
	}

	return read;
}

CST parse(const std::span<Token> &tks) {
	CST tree {};
	size_t read = parse_node(tks, &tree.root);
	tree.success = read == tks.size();
	return tree;
}
