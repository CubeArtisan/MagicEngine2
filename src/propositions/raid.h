#ifndef _RAID_H_
#define _RAID_H_

#include "../linq/linq.h"
#include "../linq/util.h"
#include "proposition.h"

class RaidProposition : public Proposition<Environment> {
public:
	bool operator()(const Environment& env) const override {
		if (controller.isValid() && env.players[env.turnPlayer]->id != controller) return false;

		for (const Changeset& change : reverse(env.changes)) {
			auto attacks = change.ofType<DeclareAttack>();
			if (attacks.begin() != attacks.end()) return true;
			else {
				auto stepChanges = change.ofType<StepOrPhaseChange>();
				if (!stepChanges.empty() && stepChanges.first()->starting == PRECOMBATMAIN) return false;
			}
		}
		return false;
	}

	// Allows creating without controller where we'll assume that it is our turn always
	RaidProposition()
	{}

	RaidProposition(xg::Guid controller)
		: controller(controller)
	{}

private:
	xg::Guid controller;
};

#endif