#ifndef ALUAR_PARSE_HPP
#define ALUAR_PARSE_HPP

#include <cstdlib>
#include <span>
#include <vector>

#include "lex.hpp"

// Concrete Syntax Tree
struct CST {
	enum class Type {
		App,
		Number,
		Symbol,
		String,
		/* List, Map, String, Assignment, Hole */
	};

	struct Node {
		size_t beg;
		size_t len;
		Type type;
		std::vector<Node *> children;

		~Node() {
			for (auto c : children) delete c;
		}
	};

	Node *root;
	bool success;

	~CST() { delete root; }
};

CST parse(const std::span<Token> &tks);

#endif
