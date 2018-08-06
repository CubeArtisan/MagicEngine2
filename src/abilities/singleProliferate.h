#ifndef _SINGLEPROLIFERATE_H_
#define _SINGLEPROLIFERATE_H_

#include "../ability.h"
#include "../environment.h"
#include "../targeting.h"

class SingleProliferateAbility : public clone_inherit<SingleProliferateAbility, Ability> {
public:
	Changeset applyEffect(const Environment& env) const {
		Changeset changes;
		xg::Guid target = env.targets.at(this->id)[0];
		if (env.gameObjects.find(target) != env.gameObjects.end()) {
			if(env.permanentCounters.find(target) != env.permanentCounters.end())
				for (const auto& counter : env.permanentCounters.at(target)) {
					changes.permanentCounters.push_back(AddPermanentCounter{ target, counter.first, 1 });
				}
			if (env.playerCounters.find(target) != env.playerCounters.end())
				for (const auto& counter : env.playerCounters.at(target)) {
					changes.playerCounters.push_back(AddPlayerCounter{ target, counter.first, 1 });
				}
		}
		return changes;
	}

	SingleProliferateAbility()
		: clone_inherit(std::shared_ptr<TargetingRestriction>(new PermanentOrPlayerTarget()))
	{}
};

#endif