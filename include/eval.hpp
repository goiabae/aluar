#ifndef ALUAR_EVAL_HPP
#define ALUAR_EVAL_HPP

#include <cstdint>
#include <memory>
#include <string>
#include <variant>

#include "parse.hpp"

struct Value {
	enum class Type {
		Number,
		Symbol,
		String,
		Error,
	};

	Type type;
	std::variant<int64_t, std::shared_ptr<std::string>> v;

	int64_t num() { return std::get<int64_t>(v); }

	std::shared_ptr<std::string> &str() {
		return std::get<std::shared_ptr<std::string>>(v);
	}
};

Value eval(const CST &tree, const std::string &src);
void value_deinit(Value val);

#endif
