#ifndef _RULESEFFECTS_H_
#define _RULESEFFECTS_H_

#include "../changeset.h"
#include "../util.h"

class TokenMovementEffect : public EventHandler {
public:
	std::variant<std::vector<Changeset>, PassPriority> handleEvent(Changeset& changeset, const Environment& env) const {
		// 110.5g. A token that has left the battlefield can't move to another zone or come back onto the battlefield. If such a token would change zones, it remains in its current zone instead. It ceases to exist the next time state-based actions are checked; see rule 704.
		std::vector<size_t> toRemove;
		size_t i = 0;
		for (auto& move : changeset.moves) {
			if (move.sourceZone != env.battlefield->id && move.sourceZone != env.stack->id &&std::dynamic_pointer_cast<Token>(env.gameObjects.at(move.object))) {
				toRemove.push_back(i);
			}
			i++;
		}
		for (size_t j = toRemove.size(); j > 0; j--) {
			changeset.moves.erase(changeset.moves.begin() + toRemove[j-1]);
		}
		if (toRemove.empty()) return PassPriority();
		else return std::vector<Changeset>{ changeset };
	}

	TokenMovementEffect()
		: EventHandler({}, {})
	{}
};

class ZeroDamageEffect : public EventHandler {
public:
	std::variant<std::vector<Changeset>, PassPriority> handleEvent(Changeset& changeset, const Environment& env) const {
		// 110.5g. A token that has left the battlefield can't move to another zone or come back onto the battlefield. If such a token would change zones, it remains in its current zone instead. It ceases to exist the next time state-based actions are checked; see rule 704.
		std::vector<size_t> toRemove;
		size_t i = 0;
		for (auto& damage : changeset.damage) {
			if (damage.amount <= 0) {
				toRemove.push_back(i);
			}
			i++;
		}
		for (size_t j = toRemove.size(); j > 0; j--) {
			changeset.damage.erase(changeset.damage.begin() + toRemove[j - 1]);
		}
		if (toRemove.empty()) return PassPriority();
		else return std::vector<Changeset>{ changeset };
	}

	ZeroDamageEffect()
		: EventHandler({}, {})
	{}
};

// CodeReview: Create StateQueryHandler for +1/+1 and -1/-1 counters
class CounterPowerToughnessEffect : public StaticEffectHandler {
public:
	StaticEffectQuery& handleEvent(StaticEffectQuery& query, const Environment& env) const {
		if (PowerQuery* powerQuery = std::get_if<PowerQuery>(&query)) {
			powerQuery->currentValue += tryAtMap(tryAtMap(env.permanentCounters, powerQuery->target.id, std::map<PermanentCounterType, unsigned int>()), PLUSONEPLUSONECOUNTER, 0u);
			powerQuery->currentValue -= tryAtMap(tryAtMap(env.permanentCounters, powerQuery->target.id, std::map<PermanentCounterType, unsigned int>()), MINUSONEMINUSONECOUNTER, 0u);
		}
		else if (ToughnessQuery* toughnessQuery = std::get_if<ToughnessQuery>(&query)) {
			toughnessQuery->currentValue += tryAtMap(tryAtMap(env.permanentCounters, toughnessQuery->target.id, std::map<PermanentCounterType, unsigned int>()), PLUSONEPLUSONECOUNTER, 0u);
			toughnessQuery->currentValue -= tryAtMap(tryAtMap(env.permanentCounters, toughnessQuery->target.id, std::map<PermanentCounterType, unsigned int>()), MINUSONEMINUSONECOUNTER, 0u);
		}
		return query;
	}
	
	CounterPowerToughnessEffect()
		: StaticEffectHandler({}, {})
	{}
};
#endif