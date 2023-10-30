#include "eval.hpp"

#include <cerrno>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "io.hpp"
#include "parse.hpp"

using std::string;

Value value_string(std::string &str) {
	Value val {};
	val.type = Value::Type::String;
	val.box = (void *)new string(str);
	return val;
}

Value value_error(std::string &error_msg) {
	Value val;
	val.type = Value::Type::Error;
	val.box = (void *)new string(error_msg);
	return val;
}

Value value_sym(std::string &sym) {
	Value val;
	val.type = Value::Type::Symbol;
	val.box = (void *)new string(sym);
	return val;
}

Value value_num(int64_t num) {
	Value val;
	val.type = Value::Type::Number;
	val.box = (void *)new int64_t(num);
	return val;
}

Value value_nil() { return Value {Value::Type::Nil, nullptr}; }

using Evaluator = Value(std::vector<Value> &);

Value eval_node(const CST::Node *node, const std::string &src);

Value eval_add(std::vector<Value> &args) {
	int64_t acc = 0;
	for (auto &arg : args) {
		if (arg.type != Value::Type::Number) {
			std::string err = "Type Error: expected Number";
			return value_error(err);
		}
		acc += *(int64_t *)arg.box;
	}
	return value_num(acc);
}

Value eval_sub(std::vector<Value> &args) {
	int64_t acc = 0;
	for (auto &arg : args)
		if (arg.type != Value::Type::Number) {
			std::string err = "Type Error: expected Number";
			return value_error(err);
		}
	if (args.size() == 1) {
		acc = -*(int64_t *)args[0].box;
	} else {
		acc = *(int64_t *)args[0].box;
		for (size_t i = 1; i < args.size(); i++) {
			acc -= *(int64_t *)args[0].box;
		}
	}
	return value_num(acc);
}

Value eval_mul(std::vector<Value> &args) {
	int64_t acc = 1;
	for (auto &arg : args) {
		if (arg.type != Value::Type::Number) {
			std::string err = "Type Error: expected Number";
			return value_error(err);
		}
		acc *= *(int64_t *)arg.box;
	}

	return value_num(acc);
}

Value eval_div(std::vector<Value> &args) {
	int64_t acc = 1;
	for (auto &arg : args) {
		if (arg.type != Value::Type::Number) {
			std::string err = "Type Error: expected Number";
			return value_error(err);
		}
		acc /= *(int64_t *)arg.box;
	}

	return value_num(acc);
}

Value eval_print(std::vector<Value> &args) {
	if (args[0].type != Value::Type::String) {
		std::string err = "Type Error: expected String";
		return value_error(err);
	}

	print_str(*(string *)args[0].box);
	return value_nil();
}

Value eval_not(std::vector<Value> &args) {
	if (args.size() != 1) {
		std::string err = "Type Error: wrong number of arguments";
		return value_error(err);
	}
	if (args[0].type != Value::Type::Number) {
		std::string err = "Type Error: expected Number";
		return value_error(err);
	}

	return value_num(!*(int64_t *)(args[0].box));
}

Value eval_println(std::vector<Value> &args) {
	if (args[0].type != Value::Type::String) {
		std::string err = "Type Error: expected String";
		return value_error(err);
	}

	print_str(*(string *)args[0].box);
	printf("\n");
	return args[0];
}

Value eval_and(std::vector<Value> &args) {
	int64_t acc = 1;
	for (auto &arg : args) {
		if (arg.type != Value::Type::Number) {
			std::string err = "Type Error: expected Number";
			return value_error(err);
		}
		acc = acc & *(int64_t *)arg.box;
	}
	return value_num(acc);
}

Value eval_or(std::vector<Value> &args) {
	int64_t acc = 1;
	for (auto &arg : args) {
		if (arg.type != Value::Type::Number) {
			std::string err = "Type Error: expected Number";
			return value_error(err);
		}
		acc = acc | *(int64_t *)arg.box;
	}
	return value_num(acc);
}

Value eval_xor(std::vector<Value> &args) {
	int64_t acc = 1;
	for (auto &arg : args) {
		if (arg.type != Value::Type::Number) {
			std::string err = "Type Error: expected Number";
			return value_error(err);
		}
		acc = acc ^ *(int64_t *)arg.box;
	}
	return value_num(acc);
}

Value eval_concat(std::vector<Value> &args) {
	std::string acc {""};
	for (auto &arg : args) {
		// leaks memory if there's a type error
		if (arg.type != Value::Type::String) {
			std::string err = "Type Error: expected String";
			return value_error(err);
		}
		acc += *(string *)arg.box;
	}
	return value_string(acc);
}

Value eval_lsh(std::vector<Value> &args) {
	if (args.size() != 2) {
		std::string err = "Type Error: wrong number of arguments";
		return value_error(err);
	}
	if (args[0].type != Value::Type::Number || args[1].type != Value::Type::Number) {
		std::string err = "Type Error: expected Number";
		return value_error(err);
	}

	return value_num(*(int64_t *)args[0].box << *(int64_t *)args[1].box);
}

Value eval_rsh(std::vector<Value> &args) {
	if (args.size() != 2) {
		std::string err = "Type Error: wrong number of arguments";
		return value_error(err);
	}
	if (args[0].type != Value::Type::Number || args[1].type != Value::Type::Number) {
		std::string err = "Type Error: expected Number";
		return value_error(err);
	}

	return value_num(*(int64_t *)args[0].box >> *(int64_t *)args[1].box);
}

Value eval_app(const CST::Node *node, const std::string &src) {
	CST::Node *func_node = node->children[0];
	std::string func = src.substr(func_node->beg, func_node->len);
	std::vector<Value> args;

	for (size_t i = 1; i < node->children.size(); i++) {
		Value val = eval_node(node->children[i], src);
		if (val.type == Value::Type::Error) return value_error(*(string *)val.box);
		args.push_back(val);
	}

	std::map<std::string, Evaluator *> funcs;
	funcs["add"] = eval_add;
	funcs["sub"] = eval_sub;
	funcs["mul"] = eval_mul;
	funcs["div"] = eval_div;
	funcs["shl"] = eval_lsh;
	funcs["shr"] = eval_rsh;
	funcs["cat"] = eval_concat;
	funcs["put"] = eval_print;
	funcs["and"] = eval_and;
	funcs["or"] = eval_or;
	funcs["xor"] = eval_xor;
	funcs["not"] = eval_not;
	funcs["println"] = eval_println;

	Evaluator *e = funcs[func];
	if (e != nullptr)
		return e(args);
	else {
		std::string err = "Unknown function \"" + func + "\"";
		return value_error(err);
	}
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
	std::string err = "Unknown syntax tree node type";
	return value_error(err);
}

Value eval(const CST &tree, const std::string &src) {
	return eval_node(tree.root, src);
}
