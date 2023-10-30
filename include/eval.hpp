#ifndef ALUAR_EVAL_HPP
#define ALUAR_EVAL_HPP

#include <cstdint>
#include <memory>
#include <string>

#include "parse.hpp"

struct Value {
	enum class Type {
		Number,
		Symbol,
		String,
		Error,
		Nil,
	};

	Type type;
	void *box; // type-erased value
};

Value eval(const CST &tree, const std::string &src);

#endif
