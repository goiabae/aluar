#include "eval.hpp"

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "parse.hpp"

Value::Value(int64_t num) {
	type = Type::Number;
	value.number = new int64_t;
	*value.number = num;
}

Value::Value(std::string &sym) {
	type = Type::Symbol;
	value.symbol = new std::string;
	*value.symbol = sym;
}

Value value_string(std::string &str) {
	Value ret;
	ret.type = Value::Type::String;
	ret.value.string = new std::string;
	*ret.value.string = str;
	return ret;
}

Value value_error(const char *error_msg) {
	Value ret;
	ret.type = Value::Type::Error;
	ret.value.error_msg = new std::string;
	*ret.value.error_msg = error_msg;
	return ret;
}

using Evaluator = Value(std::vector<Value> &);

Value eval_node(const CST::Node *node, const std::string &src);

Value eval_add(std::vector<Value> &args) {
	int64_t acc = 0;
	for (auto arg : args) {
		if (arg.type != Value::Type::Number)
			return value_error("Type Error: expected Number");
		acc += *(arg.value.number);
	}
	return Value(acc);
}

Value eval_sub(std::vector<Value> &args) {
	int64_t acc = 0;
	for (auto arg : args)
		if (arg.type != Value::Type::Number)
			return value_error("Type Error: expected Number");
	if (args.size() == 1) {
		acc = -*(args[0].value.number);
	} else {
		acc = *(args[0].value.number);
		for (size_t i = 1; i < args.size(); i++) {
			acc -= *(args[i].value.number);
		}
	}
	return Value(acc);
}

Value eval_concat(std::vector<Value> &args) {
	std::string acc {""};
	for (auto arg : args) {
		if (arg.type != Value::Type::String)
			return value_error("Type Error: expected String");
		acc += *(arg.value.string);
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
			return val;
		else
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
			return Value(num);
			break;
		}
		case CST::Type::Symbol: {
			std::string sym_repr = src.substr(node->beg, node->len);
			return Value(sym_repr);
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
