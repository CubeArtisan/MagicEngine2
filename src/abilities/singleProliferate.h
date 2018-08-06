#ifndef _SINGLEPROLIFERATE_H_
#define _SINGLEPROLIFERATE_H_

#include "../ability.h"
#include "../environment.h"
#include "../targeting.h"

class SingleProliferateAbility {
public:
	std::optional<Changeset> operator()(xg::Guid source, const Environment& env) const {
		Changeset changes;
		xg::Guid target = env.targets.at(source)[0];
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
};

#endif