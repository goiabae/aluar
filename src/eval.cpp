#include "eval.hpp"

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "parse.hpp"

Value value_string(std::string &str) {
	Value val {Value::Type::String, std::make_shared<std::string>(str)};
	return val;
}

Value value_error(const char *error_msg) {
	Value val {Value::Type::String, std::make_shared<std::string>(error_msg)};
	return val;
}

Value value_sym(std::string &sym) {
	return Value {Value::Type::String, std::make_shared<std::string>(sym)};
}

Value value_num(int64_t num) { return Value {Value::Type::Number, num}; }

using Evaluator = Value(std::vector<Value> &);

Value eval_node(const CST::Node *node, const std::string &src);

Value eval_add(std::vector<Value> &args) {
	int64_t acc = 0;
	for (auto &arg : args) {
		if (arg.type != Value::Type::Number)
			return value_error("Type Error: expected Number");
		acc += arg.num();
	}
	return value_num(acc);
}

Value eval_sub(std::vector<Value> &args) {
	int64_t acc = 0;
	for (auto &arg : args)
		if (arg.type != Value::Type::Number)
			return value_error("Type Error: expected Number");
	if (args.size() == 1) {
		acc = -args[0].num();
	} else {
		acc = args[0].num();
		for (size_t i = 1; i < args.size(); i++) {
			acc -= args[i].num();
		}
	}
	return value_num(acc);
}

Value eval_concat(std::vector<Value> &args) {
	std::string acc {""};
	for (auto &arg : args) {
		// leaks memory if there's a type error
		if (arg.type != Value::Type::String)
			return value_error("Type Error: expected String");
		acc += *arg.str();
	}
	return value_string(acc);
}

Value eval_app(const CST::Node *node, const std::string &src) {
	CST::Node *func_node = node->children[0];
	std::string func = src.substr(func_node->beg, func_node->len);
	std::vector<Value> args;

	for (size_t i = 1; i < node->children.size(); i++) {
		Value val = eval_node(node->children[i], src);
		if (val.type == Value::Type::Error)
			return value_error((*val.str()).c_str());
		args.push_back(val);
	}

	std::map<std::string, Evaluator *> funcs;
	funcs["+"] = eval_add;
	funcs["-"] = eval_sub;
	funcs["concat"] = eval_concat;

	Evaluator *e = funcs[func];
	if (e != nullptr)
		return e(args);
	else
		return value_error("Unknown function");
}

// can also be called replace_node or reduce_node
Value eval_node(const CST::Node *node, const std::string &src) {
	switch (node->type) {
		case CST::Type::App: return eval_app(node, src);
		case CST::Type::Number: {
			std::string num_repr = src.substr(node->beg, node->len);
			int64_t num = std::stol(num_repr, nullptr);
			return value_num(num);
			break;
		}
		case CST::Type::Symbol: {
			std::string sym_repr = src.substr(node->beg, node->len);
			return value_sym(sym_repr);
			break;
		}
		case CST::Type::String: {
			std::string str_repr =
				src.substr(node->beg, node->len); // skip double quotes
			return value_string(str_repr);
		}
	}
	return value_error("Unknown syntax tree node type");
}

Value eval(const CST &tree, const std::string &src) {
	return eval_node(tree.root, src);
}
