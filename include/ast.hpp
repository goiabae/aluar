#ifndef ALUAR_AST_HPP
#define ALUAR_AST_HPP

#include <cstdint>
#include <map>
#include <string>
#include <vector>

using std::map;
using std::string;
using std::vector;

namespace AST {

enum class Type {
	App,
	Arrow,
	Error,
	List,
	Nil,
	Number,
	Program,
	String,
	Symbol,
};

struct Node;
struct Symbol;

using Env = map<Symbol, const Node*>;

struct Node {
	virtual Type type() const = 0;
	virtual bool is_leaf() const = 0;
	virtual const Node* reduce(Env& env) const = 0;
};

struct Branch : Node {
	vector<Node*> children;

	bool is_leaf() const override { return false; }
};

struct Leaf : Node {
	bool is_leaf() const override { return true; }
	const Node* reduce([[maybe_unused]] Env& env) const override { return this; }
};

struct Arrow : Branch {
	Type type() const override { return Type::Arrow; }
	virtual const vector<Node*>& arg_list() const = 0;
};

struct Builtin : Arrow {
	using Func = const Node* (*)(Env& env);

	Func impl;

	Builtin(Func impl) : Arrow(), impl {impl} {}
	const Node* reduce([[maybe_unused]] Env& env) const override {
		return impl(env);
	}
	const vector<Node*>& arg_list() const override { return children; }
};

struct Number : Leaf {
	int64_t val;

	Number(int64_t num) : Leaf(), val {num} {}

	Type type() const override { return Type::Number; }
};

struct String : Leaf {
	std::string val;

	Type type() const override { return Type::String; }
};

struct Symbol : Leaf {
	std::string val;

	Symbol(const char* val) : Leaf(), val {val} {}

	Type type() const override { return Type::Symbol; }

	friend bool operator<(const Symbol& l, const Symbol& r) {
		return l.val < r.val;
	}
};

struct Nil : Leaf {
	Type type() const override { return Type::Nil; }
};

struct Error : Leaf {
	string msg;

	Error(const char* msg) : Leaf(), msg {msg} {}

	Type type() const override { return Type::Nil; }
};

struct List : Branch {
	Type type() const override { return Type::List; }
};

struct App : Branch {
	Type type() const override { return Type::App; }

	const Node* reduce(Env& env) const override {
		if (children.size() < 1) return new Error("Empty application");

		const Node* fst = children[0];
		if (fst->type() != Type::Symbol)
			return new Error("Application first value must be a symbol");

		const Symbol* func_ident = dynamic_cast<const Symbol*>(fst);

		// func not in env
		if (env.find(*func_ident) == env.end())
			return new Error("Symbol doesn't refer to any value");

		const Node* sym_val = env[*func_ident];

		if (sym_val->type() != Type::Arrow)
			return new Error("Symbol does not refer to an arrow");

		const Arrow* arrow = dynamic_cast<const Arrow*>(sym_val);

		const vector<Node*>& arg_list = arrow->arg_list();

		if ((children.size() - 1) != arg_list.size())
			return new Error("Wrong number of arguments");

		// assign arguments to parameter symbols on environment
		for (size_t i = 0; i < arg_list.size(); i++) {
			const Node* arg = arg_list[i];
			if (arg->type() != Type::Symbol)
				return new Error("Parameter must be a symbol");

			const Symbol* param_sym = dynamic_cast<const Symbol*>(arg);

			env[*param_sym] = children[i + 1]->reduce(env);
		}

		const Node* ret = arrow->reduce(env);

		// remove parameters from environment
		for (size_t i = 0; i < arg_list.size(); i++) {
			const Symbol* param_sym = dynamic_cast<const Symbol*>(arg_list[i]);
			env.erase(env.find(*param_sym));
		}

		return ret;
	}
};

struct Program : Branch {
	Type type() const override { return Type::Program; }

	const Node* reduce(Env& env) const override {
		if (children.size() == 0) return new Nil;

		const Node* res = nullptr;
		for (auto& c : children) {
			res = c->reduce(env);
		}
		return res;
	};
};

struct AST {
	const Node* root;
	Env env;

	AST();

	void interpret();
};

}; // namespace AST

#endif
