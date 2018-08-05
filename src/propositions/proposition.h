#ifndef _PROPOSITION_H_
#define _PROPOSITION_H_

#include <type_traits>

#include "../environment.h"
#include "../util.h"

class Proposition {
public:
	virtual bool operator()(const Environment& env) const = 0;
};

class PropositionValue : public polyValue<Proposition>, public Proposition {
public:
	using polyValue<Proposition>::polyValue;

	bool operator()(const Environment& env) const {
		return this->value()(env);
	}
};

template<typename T>
constexpr bool isProposition = std::is_base_of<Proposition, T>::value;

class TrueProposition : public Proposition {
	bool operator()(const Environment&) const override {
		return true;
	}
};

class FalseProposition : public Proposition {
	bool operator()(const Environment&) const override {
		return false;
	}
};

template<typename A, typename B>
class AndProposition : public Proposition {
	bool operator()(const Environment& env) const override {
		return a(env) && b(env);
	}

	AndProposition()
	{
		static_assert(isProposition<A> && isProposition<B>, "A and B must be Propositions");
	}

	AndProposition(A a, B b)
		: a(a), b(b)
	{}

private:
	A a;
	B b;
};

template<typename A, typename B>
class OrProposition : public Proposition {
	bool operator()(const Environment& env) const override {
		return a(env) || b(env);
	}

	OrProposition()
	{}

	OrProposition(A a, B b)
		: a(a), b(b)
	{}

private:
	A a;
	B b;
};


#endif