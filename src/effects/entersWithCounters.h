#ifndef _ENTERSWITHCOUNTERS_H_
#define _ENTERSWITHCOUNTERS_H_

#include "../changeset.h"
#include "../propositions/proposition.h"

class EntersWithCounters : public EventHandler {
public:
	std::variant<std::vector<Changeset>, PassPriority> handleEvent(Changeset& changes, const Environment& env) const {
		bool replaced = false;
		for (const auto& create : changes.create) {
			if (create.created->id == this->owner) {
				if (!prop(env)) return PassPriority();
				replaced = true;
				bool existing = false;
				for (auto& counter : changes.permanentCounters) {
					if (counter.target == this->owner && counter.counterType == PLUSONEPLUSONECOUNTER) {
						existing = true;
						counter.amount += this->amount;
						break;
					}
				}
				if (!existing) changes.permanentCounters.push_back(AddPermanentCounter{ this->owner, PLUSONEPLUSONECOUNTER, this->amount });
				break;
			}
		}

		if(replaced) return std::vector<Changeset>{ changes };
		else return PassPriority();
	}

	EntersWithCounters(int amount, PropositionValue prop = TrueProposition())
		: EventHandler({}, {BATTLEFIELD}), amount(amount), prop(prop)
	{}

private:
	int amount;
	PropositionValue prop;
};

#endif