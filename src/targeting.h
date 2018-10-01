#ifndef _TARGETING_H_
#define _TARGETING_H_

#include <algorithm>
#include <memory>
#include <type_traits>
#include <vector>

#include "guid.hpp"

#include "ability.h"
#include "environment.h"

template<typename T>
bool isUnique(const std::vector<T>& x) noexcept {
	std::vector< T const * > vp;
	vp.reserve(x.size());
	for (size_t i = 0; i < x.size(); ++i) vp.push_back(&x[i]);
	std::sort(vp.begin(), vp.end(), [](auto l, auto r) -> bool { return *l < *r; }); // O(N log N)
	return std::adjacent_find(vp.begin(), vp.end(), [](auto l, auto r) -> bool { return *l >= *r; }) // "opposite functor"
		== vp.end(); // if no adjacent pair (vp_n,vp_n+1) has *vp_n >= *vp_n+1
}

class TargetingRestriction {
public:
	virtual bool validFirstN(const std::vector<xg::Guid>& targets, const HasEffect& source, const Environment& env) const noexcept {
		if (targets.size() == 0) return true;
		if (targets.size() > this->maxTargets || !isUnique(targets)) return false;

		size_t index = 0;
		for (const xg::Guid& target : targets) {
			if (!this->validTarget(target, index++, source, env)) return false;
		}
		return true;
	}
	
	virtual bool validTargets(const std::vector<xg::Guid>& targets, const HasEffect& source, const Environment& env) const noexcept = 0;
	
	virtual bool anyValidTarget(const std::vector<xg::Guid>& targets, const HasEffect& source, const Environment& env) const noexcept {
		if (targets.size() == 0) return true;
		if (targets.size() > this->maxTargets || !isUnique(targets)) return false;

		size_t index = 0;
		for (const xg::Guid& target : targets) {
			if (this->validTarget(target, index++, source, env)) return true;
		}
		return false;
	}

	virtual bool validTarget(const xg::Guid& target, size_t index, const HasEffect& source, const Environment& env) const noexcept = 0;

	const int minTargets;
	const int maxTargets;

	TargetingRestriction(int min, int max) noexcept
		: minTargets(min), maxTargets(max)
	{}

	virtual ~TargetingRestriction() noexcept {}
};

class NoTargets : public TargetingRestriction {
public:
	bool validTargets(const std::vector<xg::Guid>& targets, const HasEffect&, const Environment&) const noexcept override {
		return targets.size() == 0;
	}

	virtual bool validTarget(const xg::Guid&, size_t, const HasEffect&, const Environment&) const noexcept override {
		return false;
	}

	NoTargets() noexcept
		: TargetingRestriction(0, 0)
	{}
};

template<typename TargetType, int n_=1>
class UpToNTargets : TargetingRestriction{
public:
	bool validTargets(const std::vector<xg::Guid>& targets, const HasEffect& source, const Environment& env) const noexcept override {
		return targets.size() <= n && this->validFirstN(targets, source, env);
	}

	virtual bool validTarget(const xg::Guid& target, size_t index, const HasEffect& source, const Environment& env) const noexcept override {
		return targeting.validTarget(target, 0, source, env);
	}

	UpToNTargets(int n = n_) noexcept
	: TargetingRestriction(0, n), n(n)
	{}

private:
	int n;
	TargetType targeting;
};

template<typename TargetType, int n_=1>
class ExactlyNTargets : TargetingRestriction {
public:
	bool validTargets(const std::vector<xg::Guid>& targets, const HasEffect& source, const Environment& env) const noexcept override {
		return targets.size() == n && this->validFirstN(targets, source, env);
	}

	virtual bool validTarget(const xg::Guid& target, size_t index, const HasEffect& source, const Environment& env) const noexcept override {
		return targeting.validTarget(target, 0, source, env);
	}

	ExactlyNTargets(int n=n_) noexcept
		: TargetingRestriction(n, n), n(n)
	{}

private:
	int n;
	TargetType targeting;
};

template<typename Target1, typename Target2>
class AndTarget : public TargetingRestriction {
public:
	bool validFirstN(const std::vector<xg::Guid>& targets, const HasEffect& source, const Environment& env) const noexcept override {
		return target1.validFirstN(targets, source, env) && target2.validFirstN(targets, source, env);
	}

	bool validTargets(const std::vector<xg::Guid>& targets, const HasEffect& source, const Environment& env) const noexcept override {
		return target1.validTargets(targets, source, env) && target2.validTargets(targets, source, env);
	}

	bool validTarget(const xg::Guid& target, size_t index, const HasEffect& source, const Environment& env) const noexcept override {
		return target1.validTarget(target, index, source, env) && target2.validTarget(target, index, source, env);
	}

	AndTarget() noexcept
		: TargetingRestriction(target1.minTargets, target1.maxTargets)
	{}

private:
	Target1 target1;
	Target2 target2;
};

template<typename Target1, typename Target2>
class OrTarget : public TargetingRestriction {
public:
	bool validFirstN(const std::vector<xg::Guid>& targets, const HasEffect& source, const Environment& env) const noexcept override {
		return target1.validFirstN(targets, source, env) || target2.validFirstN(targets, source, env);
	}

	bool validTargets(const std::vector<xg::Guid>& targets, const HasEffect& source, const Environment& env) const noexcept override {
		return target1.validTargets(targets, source, env) || target2.validTargets(targets, source, env);
	}

	bool validTarget(const xg::Guid& target, size_t index, const HasEffect& source, const Environment& env) const noexcept override {
		return target1.validTarget(target, index, source, env) && target2.validTarget(target, index, source, env);
	}

	OrTarget() noexcept
		: TargetingRestriction(1, 1)
	{
/*
		assert(target1.minTargets == target2.minTargets);
		assert(target1.minTargets == 1);
		assert(target1.maxTargets == target2.maxTargets);
		assert(target1.maxTargets == 1);
*/
	}

private:
	Target1 target1;
	Target2 target2;
};

class AnyTarget : public TargetingRestriction {
public:
	bool validTarget(const xg::Guid& target, size_t index, const HasEffect&, const Environment& env) const noexcept override {
		if (index != 0) return false;
		std::shared_ptr<Targetable> object = env.gameObjects.at(target);
		if (std::dynamic_pointer_cast<Player>(object)) {
			return true;
		}
		else if (std::shared_ptr<Card> card = std::dynamic_pointer_cast<Card>(object)) {
			if (!env.battlefield->findObject(target)) return false;
			std::shared_ptr<const std::set<CardType>> types = env.getTypes(card);
			if (types->find(CREATURE) != types->end()
				|| types->find(PLANESWALKER) != types->end()) {
				return true;
			}
		}
		return false;
	}
	
	bool validTargets(const std::vector<xg::Guid>& targets, const HasEffect& source, const Environment& env) const noexcept override {
		return targets.size() == 1 && this->validFirstN(targets, source, env);
	}

	AnyTarget() noexcept
		: TargetingRestriction(1, 1)
	{}
};

class PlayerTarget : public TargetingRestriction {
public:
	bool validTarget(const xg::Guid& target, size_t index, const HasEffect&, const Environment& env) const noexcept override {
		if (index != 0) return false;
		std::shared_ptr<Targetable> object = env.gameObjects.at(target);
		if (std::dynamic_pointer_cast<Player>(object)) {
			return true;
		}
		return false;
	}

	bool validTargets(const std::vector<xg::Guid>& targets, const HasEffect& source, const Environment& env) const noexcept override {
		return targets.size() == 1 && this->validFirstN(targets, source, env);
	}

	PlayerTarget() noexcept
		: TargetingRestriction(1, 1)
	{}
};

class PermanentTarget : public TargetingRestriction {
public:
	bool validTarget(const xg::Guid& target, size_t index, const HasEffect&, const Environment& env) const noexcept override {
		if (index != 0) return false;
		return (bool)env.battlefield->findObject(target);
	}

	bool validTargets(const std::vector<xg::Guid>& targets, const HasEffect& source, const Environment& env) const noexcept override {
		return targets.size() == 1 && this->validFirstN(targets, source, env);
	}

	PermanentTarget() noexcept
		: TargetingRestriction(1, 1)
	{}
};

using PermanentOrPlayerTarget = OrTarget<PlayerTarget, PermanentTarget>;

class TargetsYouTarget : public TargetingRestriction {
public:
	bool validTarget(const xg::Guid& target, size_t index, const HasEffect& source, const Environment& env) const noexcept override {
		if (index != 0) return false;
		
		std::vector<xg::Guid> targets = tryAtMap(env.targets, target, {});
		return std::find(targets.begin(), targets.end(), env.getController(source.clone())) != targets.end();
	}

	bool validTargets(const std::vector<xg::Guid>& targets, const HasEffect& source, const Environment& env) const noexcept override {
		return targets.size() == 1 && this->validFirstN(targets, source, env);
	}

	TargetsYouTarget() noexcept
		: TargetingRestriction(1, 1)
	{}
};

class TargetsPermanentYouControlTarget : public TargetingRestriction {
public:
	bool validTarget(const xg::Guid& target, size_t index, const HasEffect& source, const Environment& env) const noexcept override {
		if (index != 0) return false;

		std::vector<xg::Guid> targets = tryAtMap(env.targets, target, {});
		xg::Guid controller = env.getController(source.clone());
		for (const xg::Guid& guid : targets) {
			if (env.gameObjects.find(guid) != env.gameObjects.end())
				if (env.getController(guid) == controller) return true;
		}
		return false;
	}

	bool validTargets(const std::vector<xg::Guid>& targets, const HasEffect& source, const Environment& env) const noexcept override {
		return targets.size() == 1 && this->validFirstN(targets, source, env);
	}

	TargetsPermanentYouControlTarget() noexcept
		: TargetingRestriction(1, 1)
	{}
};

using TargetsYouOrPermanentYouControlTarget = OrTarget<TargetsYouTarget, TargetsPermanentYouControlTarget>;

class NonCreatureTarget : public TargetingRestriction {
public:
	bool validTarget(const xg::Guid& target, size_t index, const HasEffect&, const Environment& env) const noexcept override {
		if (index != 0) return false;

		if (std::shared_ptr<CardToken> card = std::dynamic_pointer_cast<CardToken>(env.gameObjects.at(target))) {
			std::shared_ptr<const std::set<CardType>> types = env.getTypes(card);
			if (types->find(CREATURE) != types->end()) return false;
			else return true;
		}
		else {
			return false;
		}
	}

	bool validTargets(const std::vector<xg::Guid>& targets, const HasEffect& source, const Environment& env) const noexcept override {
		return targets.size() == 1 && this->validFirstN(targets, source, env);
	}

	NonCreatureTarget() noexcept
		: TargetingRestriction(1, 1)
	{}
};

class CreatureTarget : public TargetingRestriction {
public:
	bool validTarget(const xg::Guid& target, size_t index, const HasEffect&, const Environment& env) const noexcept override {
		if (index != 0) return false;

		if (env.gameObjects.find(target) != env.gameObjects.end()) return false;
		if (std::shared_ptr<CardToken> card = std::dynamic_pointer_cast<CardToken>(env.gameObjects.at(target))) {
			std::shared_ptr<const std::set<CardType>> types = env.getTypes(card);
			if (types->find(CREATURE) != types->end()) return true;
			else return false;
		}
		else {
			return false;
		}
	}

	bool validTargets(const std::vector<xg::Guid>& targets, const HasEffect& source, const Environment& env) const noexcept override {
		return targets.size() == 1 && this->validFirstN(targets, source, env);
	}

	CreatureTarget() noexcept
		: TargetingRestriction(1, 1)
	{}
};

class SpellTarget : public TargetingRestriction {
public:
	bool validTarget(const xg::Guid& target, size_t index, const HasEffect&, const Environment& env) const noexcept override {
		if (index != 0) return false;

		return (bool)env.stack->findObject(target);
	}

	bool validTargets(const std::vector<xg::Guid>& targets, const HasEffect& source, const Environment& env) const noexcept override {
		return targets.size() == 1 && this->validFirstN(targets, source, env);
	}

	SpellTarget() noexcept
		: TargetingRestriction(1, 1)
	{}
};

#endif
