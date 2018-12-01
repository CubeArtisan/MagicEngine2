#ifndef _RAID_H_
#define _RAID_H_

#include "../util.h"
#include "proposition.h"

class RaidProposition : public Proposition<Environment> {
public:
	bool operator()(const Environment& env) const override {
		if (controller.isValid() && env.players[env.turnPlayer]->id != controller) return false;

		for (const Changeset& change : reverse(env.changes)) {
			cast<GameChange, DeclareAttack> attacks(change.changes);
			if (attacks.begin() != attacks.end()) return true;
			else {
				cast<GameChange, StepOrPhaseChange> stepChanges(change.changes);
				if (stepChanges.begin() != stepChanges.end() && (*stepChanges.begin())->starting == PRECOMBATMAIN) return false;
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