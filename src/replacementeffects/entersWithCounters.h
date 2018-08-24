#ifndef _ENTERSWITHCOUNTERS_H_
#define _ENTERSWITHCOUNTERS_H_

#include "../changeset.h"
#include "../propositions/proposition.h"

class EntersWithCounters : public clone_inherit<EntersWithCounters, EventHandler> {
public:
	std::optional<std::vector<Changeset>> handleEvent(Changeset& changes, const Environment& env) const {
		bool replaced = false;
		for (const auto& create : changes.create) {
			if (create.created->id == this->owner) {
				if (!prop(env)) return std::nullopt;
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
		else return std::nullopt;
	}

	EntersWithCounters(int amount, PropositionValue<Environment> prop = TrueProposition<Environment>())
		: clone_inherit({}, {BATTLEFIELD}), amount(amount), prop(prop)
	{}

private:
	int amount;
	PropositionValue<Environment> prop;
};

#endif