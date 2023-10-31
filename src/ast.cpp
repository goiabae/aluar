#include "ast.hpp"

namespace AST {

void AST::AST::interpret() { root = root->reduce(env); }

const Node* add(Env& env);
const Node* sub(Env& env);
const Node* mul(Env& env);
const Node* div(Env& env);

// scoped macro
// FIXME calee can override environment of caller function, messing things up.
// Create a unique hole punching system for sanitizing variable names
#define PUSH_BINARY_OP(FUNC, IDENT)       \
	b = new Builtin(FUNC);                  \
	b->children.push_back(new Symbol("x")); \
	b->children.push_back(new Symbol("y")); \
	env[Symbol(IDENT)] = b;

AST::AST() {
	Builtin* b = nullptr;

	PUSH_BINARY_OP(add, "+");
	PUSH_BINARY_OP(sub, "-");
	PUSH_BINARY_OP(mul, "*");
	PUSH_BINARY_OP(div, "/");
}

#undef PUSH_BINARY_OP

// scoped macro
#define DEF_BINARY_OP(FUNC, OP)                        \
	const Node* FUNC(Env& env) {                         \
		const Number* x = (const Number*)env[Symbol("x")]; \
		const Number* y = (const Number*)env[Symbol("y")]; \
		return new Number(x->val OP y->val);               \
	}

DEF_BINARY_OP(sub, -);
DEF_BINARY_OP(add, +);
DEF_BINARY_OP(mul, *);
DEF_BINARY_OP(div, /);

#undef DEF_BINARY_OP

} // namespace AST
