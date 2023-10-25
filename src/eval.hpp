#ifndef ALUAR_EVAL_HPP
#define ALUAR_EVAL_HPP

#include <cstdint>
#include <string>
#include "parse.hpp"

struct Value {
	enum class Type {
		Number,
		Symbol,
		String,
		Error,
	};

	Type type;

	union {
		int64_t *number;
		std::string *symbol;
		std::string *string;
		std::string *error_msg;
	} value;

	Value() = default;

	Value(int64_t num);
	Value(std::string &sym);
};

Value eval(const CST &tree, const std::string &src);

#endif
