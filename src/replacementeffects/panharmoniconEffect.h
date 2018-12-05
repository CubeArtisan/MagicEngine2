#ifndef _RULESEFFECTS_H_
#define _RULESEFFECTS_H_

#include "../changeset.h"
#include "../environment.h"
#include "../linq/util.h"

class PanharmoniconEffect : public clone_inherit<PanharmoniconEffect, EventHandler> {
public:
	std::optional<std::vector<Changeset>> handleEvent(Changeset& changeset, const Environment& env) const {
		std::vector<std::shared_ptr<QueueTrigger>> toAdd;
		// CodeReview: Currently doubles instead of just adding one
		for (const std::shared_ptr<QueueTrigger>& trigger : changeset.ofType<QueueTrigger>()) {
			if (env.getController(this->owner) != trigger->player) continue;
			for (const std::shared_ptr<CreateObject>& create : trigger->triggered.ofType<CreateObject>()) {
				if (create->zone != env.battlefield->id) continue;
				std::shared_ptr<CardToken> card = std::dynamic_pointer_cast<CardToken>(create->created);
				if (!card) continue;
				std::shared_ptr<const std::set<CardType>> types = env.getTypes(card);
				if (types->find(CREATURE) != types->end() || types->find(ARTIFACT) != types->end()) {
					toAdd.push_back(trigger);
					break;
				}
			}
		}
		changeset.changes.insert(changeset.changes.end(), toAdd.begin(), toAdd.end());
		return std::vector<Changeset>{ changeset };
	}

	PanharmoniconEffect()
		: clone_inherit({}, { BATTLEFIELD })
	{}
};
#endif