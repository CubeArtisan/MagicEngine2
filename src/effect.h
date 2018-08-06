#ifndef _EFFECT_H_
#define _EFFECT_H_

#include "changeset.h"
#include "util.h"

struct Environment;

class Effect {
public:
	virtual std::optional<Changeset> getChangeset(xg::Guid source, const Environment& env) = 0;
	virtual void reset() = 0;
};

class EffectValue : polyValue<Effect>, Effect {
public:
	std::optional<Changeset> getChangeset(xg::Guid source, const Environment& env) {
		return this->value().getChangeset(source, env);
	}

	void reset() {
		this->value().reset();
	}

	using polyValue<Effect>::polyValue;
};

class LambdaEffects : public Effect {
public:
	template<typename... Args>
	LambdaEffects(Args... args)
		: curIndex(0), funcs{ args... }
	{}

	LambdaEffects(std::vector<std::function<std::optional<Changeset>(xg::Guid source, const Environment& env)>> funcs)
		: curIndex(0), funcs(funcs)
	{}

	std::optional<Changeset> getChangeset(xg::Guid source, const Environment& env) {
		if (curIndex >= this->funcs.size()) return std::nullopt;

		return funcs[curIndex++](source, env);
	}

	void reset() {
		curIndex = 0;
	}

private:
	size_t curIndex;
	std::vector<std::function<std::optional<Changeset>(xg::Guid source, const Environment& env)>> funcs;
};

#endif