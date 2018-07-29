#ifndef _RULESEFFECTS_H_
#define _RULESEFFECTS_H_

#include "../changeset.h"
#include "../environment.h"
#include "../util.h"

class PanharmoniconEffect : public EventHandler {
public:
	std::variant<std::vector<Changeset>, PassPriority> handleEvent(Changeset& changeset, const Environment& env) const {
		std::vector<QueueTrigger> toAdd;
		// CodeReview: Currently doubles instead of just adding one
		for (QueueTrigger& trigger : changeset.trigger) {
			if (env.getController(this->owner) != trigger.player) continue;
			for (ObjectCreation& create : trigger.triggered.create) {
				if (create.zone != env.battlefield->id) continue;
				std::shared_ptr<CardToken> card = std::dynamic_pointer_cast<CardToken>(create.created);
				std::shared_ptr<const std::set<CardType>> types = env.getTypes(card);
				if (types->find(CREATURE) != types->end() || types->find(ARTIFACT) != types->end()) {
					toAdd.push_back(trigger);
					break;
				}
			}
		}
		changeset.trigger.insert(changeset.trigger.end(), toAdd.begin(), toAdd.end());
		return std::vector<Changeset>{ changeset };
	}

	PanharmoniconEffect()
		: EventHandler({}, {BATTLEFIELD})
	{}
};
#endif