#include <assert.h>
#include <stdio.h>

#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <span>
#include <string>
#include <string_view>
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
		Word,
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

void print_token(Token tk, const str &src) {
	printf("Token (len: %lu, str: \"", tk.len);
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

// Concrete Syntax Tree
struct CST {
	enum class Type {
		App,
		Number,
		Symbol,
		/* List, Map, String, Assignment, Hole */
	};
	struct Node {
		size_t beg;
		size_t len;
		Type type;
		std::vector<Node *> children;
	};

	Node *root;
};

bool is_app(const std::span<Token> &tks) {
	return tks[0].type == Token::Type::ParenOpen;
}

size_t parse_node(const std::span<Token> &tks, CST::Node **root);

using Parser = size_t(const std::span<Token> &, CST::Node **);

// parses a sequence of nodes with the given function
size_t sequence_of(
	Parser p, const std::span<Token> &tks, std::vector<CST::Node *> &cs
) {
	CST::Node *tmp = new CST::Node;
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

bool is_number(const std::span<Token> &tks) {
	return tks[0].type == Token::Type::Number;
}

size_t parse_number(const std::span<Token> &tks, CST::Node **root) {
	*root = new CST::Node;
	(*root)->type = CST::Type::Number;
	(*root)->beg = tks[0].beg;
	(*root)->len = tks[0].len;
	return 1;
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
		case Token::Type::Word:
		case Token::Type::Semicolon:
		case Token::Type::Comma:
		case Token::Type::DoubleQuote: return true;

		case Token::Type::Number:
		case Token::Type::ParenOpen:
		case Token::Type::ParenClose:
		case Token::Type::Newline: return false;
	}
	return false;
}

size_t parse_word(const std::span<Token> &tks, CST::Node **root) {
	*root = new CST::Node;
	(*root)->type = CST::Type::Symbol;
	(*root)->beg = tks[0].beg;
	(*root)->len = tks[0].len;
	return 1;
}

size_t parse_node(const std::span<Token> &tks, CST::Node **root) {
	size_t read = 0;

	if (is_app(tks)) {
		read = parse_app(tks, root);
	} else if (is_number(tks)) {
		read = parse_number(tks, root);
	} else if (is_word(tks)) {
		read = parse_word(tks, root);
	} else if (tks[0].type == Token::Type::Newline) {
		read = 1 + parse_node(tks.subspan(1), root);
	}

	return read;
}

CST parse(const std::span<Token> &tks) {
	CST tree {};
	parse_node(tks, &tree.root);
	return tree;
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
		case CST::Type::Symbol:
			str num = src.substr(node->beg, node->len);
			print_str(num);
			break;
	}
	return;
}

struct Value {
	enum class Type {
		Number,
		Symbol,
		Error,
	};

	Type type;

	union {
		int64_t *number;
		str *symbol;
	} value;

	Value(int64_t num) {
		type = Type::Number;
		value.number = new int64_t;
		*value.number = num;
	}

	Value(str &sym) {
		type = Type::Symbol;
		value.symbol = new str;
		*value.symbol = sym;
	}

	Value() { type = Type::Error; }
};

Value eval_node(const CST::Node *node, const str &src);

Value eval_app(const CST::Node *node, const str &src) {
	CST::Node *func_node = node->children[0];
	std::string func = src.substr(func_node->beg, func_node->len);
	std::vector<Value> args;

	for (size_t i = 1; i < node->children.size(); i++) {
		Value val = eval_node(node->children[i], src);
		if (val.type == Value::Type::Error)
			return Value();
		else
			args.push_back(val);
	}

	if (func == "+") {
		int64_t acc = 0;
		for (auto arg : args) {
			acc += *(arg.value.number);
		}
		return acc;
	} else if (func == "-") {
		int64_t acc = 0;
		if (args.size() == 1) {
			acc = -*(args[0].value.number);
		} else {
			acc = *(args[0].value.number);
			for (size_t i = 1; i < args.size(); i++) {
				acc -= *(args[i].value.number);
			}
		}
		return Value(acc);
	} else
		return Value();
}

// can also be called replace_node or reduce_node
Value eval_node(const CST::Node *node, const str &src) {
	switch (node->type) {
		case CST::Type::App: return eval_app(node, src);
		case CST::Type::Number: {
			str num_repr = src.substr(node->beg, node->len);
			int64_t num = std::stol(num_repr, nullptr);
			return Value(num);
			break;
		}
		case CST::Type::Symbol: {
			str sym_repr = src.substr(node->beg, node->len);
			return Value(sym_repr);
			break;
		}
	}
	return Value();
}

Value eval(const CST &tree, const str &src) {
	return eval_node(tree.root, src);
}

void print_tree(const CST &tree, const str &src) { print_node(tree.root, src); }

void print_value(Value val) {
	switch (val.type) {
		case Value::Type::Number: {
			printf("%ld", *val.value.number);
			break;
		}
		case Value::Type::Symbol: {
			printf("\"");
			print_str(*val.value.symbol);
			printf("\"");
			break;
		}
		case Value::Type::Error: break;
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

	if (res.type == Value::Type::Error)
		return 1;
	else {
		print_value(res);
		return 0;
	}
}
