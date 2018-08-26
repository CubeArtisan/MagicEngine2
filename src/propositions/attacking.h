#ifndef _ATTACKING_H_
#define _ATTACKING_H_

#include "../util.h"
#include "proposition.h"

class AttackingProposition : public Proposition<Environment, xg::Guid> {
public:
	bool operator()(const Environment& env, const xg::Guid& target) const override {
		for (auto attack : env.declaredAttacks) {
			if (attack.first->id == target) return true;
		}
		return false;
	}
};

#endif