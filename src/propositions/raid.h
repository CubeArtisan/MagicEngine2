#ifndef _RAID_H_
#define _RAID_H_

#include "../util.h"
#include "proposition.h"

class RaidProposition : public Proposition {
public:
	bool operator()(const Environment& env) const override {
		if (controller.isValid() && env.players[env.turnPlayer]->id != controller) return false;

		for (const auto& change : reverse(env.changes)) {
			if (!change.attacks.empty()) return true;
			else if (change.phaseChange.changed && change.phaseChange.starting == PRECOMBATMAIN) return false;
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