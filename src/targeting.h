#ifndef _TARGETING_H_
#define _TARGETING_H_

#include <algorithm>
#include <memory>
#include <type_traits>
#include <vector>

#include "guid.hpp"

#include "environment.h"

template<typename T>
bool isUnique(const std::vector<T>& x) {
	std::vector< T const * > vp;
	vp.reserve(x.size());
	for (size_t i = 0; i < x.size(); ++i) vp.push_back(&x[i]);
	std::sort(vp.begin(), vp.end(), [](auto l, auto r) -> bool { *l < *r; }); // O(N log N)
	return std::adjacent_find(vp.begin(), vp.end(), [](auto l, auto r) -> bool { *l >= *r; }) // "opposite functor"
		== vp.end(); // if no adjacent pair (vp_n,vp_n+1) has *vp_n >= *vp_n+1
}

class TargetingRestriction {
public:
	virtual bool validFirstN(const std::vector<xg::Guid>& targets, const Environment& env) = 0;
	virtual bool validTargets(const std::vector<xg::Guid>& targets, const Environment& env) = 0;

	const int minTargets;
	const int maxTargets;

	TargetingRestriction(int min, int max)
		: minTargets(min), maxTargets(max)
	{}
};

class NoTargets : public TargetingRestriction {
public:
	bool validFirstN(const std::vector<xg::Guid>& targets, const Environment& env) {
		return targets.size() == 0;
	}

	bool validTargets(const std::vector<xg::Guid>& targets, const Environment& env) {
		return targets.size() == 0;
	}

	NoTargets()
		: TargetingRestriction(0, 0)
	{}
};

template<int n, typename TargetType>
class UpToNTargets : TargetingRestriction{
public:
	bool validFirstN(const std::vector<xg::Guid>& targets, const Environment& env) {
		if (targets.size() > n || !isUnique(targets)) return false;

		for (xg::Guid target : targets) {
			if (targeting.validTargets(std::vector<xg::Guid>{target}, env)) {
				continue;
			}
			return false;
		}
	}
	bool validTargets(const std::vector<xg::Guid>& targets, const Environment& env) {
		return targets.size() <= n && this->validFirstN(targets, env);
	}

	UpToNTargets()
	: TargetingRestriction(0, n)
	{}

private:
	TargetType targeting;
};

template<int n, typename TargetType>
class ExactlyNTargets : TargetingRestriction {
public:
	bool validFirstN(const std::vector<xg::Guid>& targets, const Environment& env) {
		if (targets.size() > n || !isUnique(targets)) return false;

		for (xg::Guid target : targets) {
			if (targeting.validTargets(std::vector<xg::Guid>{target}, env)) {
				continue;
			}
			return false;
		}
	}
	bool validTargets(const std::vector<xg::Guid>& targets, const Environment& env) {
		return targets.size() == n && this->validFirstN(targets, env);
	}

	ExactlyNTargets()
		: TargetingRestriction(n, n)
	{}

private:
	TargetType targeting;
};

class AnyTarget : public TargetingRestriction {
public:
	bool validFirstN(const std::vector<xg::Guid>& targets, const Environment& env) {
		if (targets.size() > 1) return false;
		if (targets.size() == 0) return true;

		xg::Guid target = targets[0];
		std::shared_ptr<Targetable> object = env.gameObjects.at(target);
		if (std::dynamic_pointer_cast<Player>(object)) {
			return true;
		}
		else if (std::shared_ptr<Card> card = std::dynamic_pointer_cast<Card>(object)) {
			if (!env.battlefield->findObject(target)) return false;
			std::set<CardType> types = env.getTypes(card);
			if (types.find(CREATURE) != types.end()
				|| types.find(PLANESWALKER) != types.end()) {
				return true;
			}
		}
		return false;
	}
	
	bool validTargets(const std::vector<xg::Guid>& targets, const Environment& env) {
		return targets.size() == 1 && this->validFirstN(targets, env);
	}

	AnyTarget()
		: TargetingRestriction(1, 1)
	{}
};

class TargetPlayer : public TargetingRestriction {
public:
	bool validFirstN(const std::vector<xg::Guid>& targets, const Environment& env) {
		if (targets.size() > 1) return false;
		if (targets.size() == 0) return true;

		xg::Guid target = targets[0];
		std::shared_ptr<Targetable> object = env.gameObjects.at(target);
		if (std::dynamic_pointer_cast<Player>(object)) {
			return true;
		}
		return false;
	}

	bool validTargets(const std::vector<xg::Guid>& targets, const Environment& env) {
		return targets.size() == 1 && this->validFirstN(targets, env);
	}

	TargetPlayer()
		: TargetingRestriction(1, 1)
	{}
};

#endif