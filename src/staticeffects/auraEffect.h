#ifndef _AURAEFFECT_H_
#define _AURAEFFECT_H_

#include "../changeset.h"
#include "../combat.h"
#include "../environment.h"

class AuraEffect : public clone_inherit<AuraEffect, StaticEffectHandler> {
public:
	StaticEffectQuery& handleEvent(StaticEffectQuery& query, const Environment& env) const override {
		if (PowerQuery* ptr = std::get_if<PowerQuery>(&query)) {
			if (this->powerBoost != 0 && env.targets.at(this->owner)[0] == ptr->target.id) {
				ptr->power += this->powerBoost;
				return query;
			}
		}
		if (ToughnessQuery* ptr = std::get_if<ToughnessQuery>(&query)) {
			if (this->toughnessBoost != 0 && env.targets.at(this->owner)[0] == ptr->target.id) {
				ptr->toughness += this->toughnessBoost;
				return query;
			}
		}
		if (StaticEffectsQuery* ptr = std::get_if<StaticEffectsQuery>(&query)) {
			if (!this->staticEffectBoost.empty() && env.targets.at(this->owner)[0] == ptr->target.id) {
				ptr->effects.insert(ptr->effects.end(), this->staticEffectBoost.begin(), this->staticEffectBoost.end());
				return query;
			}
		}
		if (ReplacementEffectsQuery* ptr = std::get_if<ReplacementEffectsQuery>(&query)) {
			if (!this->replacementEffectBoost.empty() && env.targets.at(this->owner)[0] == ptr->target.id) {
				ptr->effects.insert(ptr->effects.end(), this->replacementEffectBoost.begin(), this->replacementEffectBoost.end());
				return query;
			}
		}
		if (TriggerEffectsQuery* ptr = std::get_if<TriggerEffectsQuery>(&query)) {
			if (!this->triggerEffectBoost.empty() && env.targets.at(this->owner)[0] == ptr->target.id) {
				ptr->effects.insert(ptr->effects.end(), this->triggerEffectBoost.begin(), this->triggerEffectBoost.end());
				return query;
			}
		}
		return query;
	}

	bool appliesTo(StaticEffectQuery& query, const Environment& env) const override {
		if (PowerQuery* ptr = std::get_if<PowerQuery>(&query)) {
			if (this->powerBoost != 0 && env.targets.at(this->owner)[0] == ptr->target.id) {
				return true;
			}
		}
		if (ToughnessQuery* ptr = std::get_if<ToughnessQuery>(&query)) {
			if (this->toughnessBoost != 0 && env.targets.at(this->owner)[0] == ptr->target.id) {
				return true;
			}
		}
		if (StaticEffectsQuery* ptr = std::get_if<StaticEffectsQuery>(&query)) {
			if (!this->staticEffectBoost.empty() && env.targets.at(this->owner)[0] == ptr->target.id) {
				return true;
			}
		}
		if (ReplacementEffectsQuery* ptr = std::get_if<ReplacementEffectsQuery>(&query)) {
			if (!this->replacementEffectBoost.empty() && env.targets.at(this->owner)[0] == ptr->target.id) {
				return true;
			}
		}
		if (TriggerEffectsQuery* ptr = std::get_if<TriggerEffectsQuery>(&query)) {
			if (!this->triggerEffectBoost.empty() && env.targets.at(this->owner)[0] == ptr->target.id) {
				return true;
			}
		}
		return false;
	}

	bool dependsOn(StaticEffectQuery&, StaticEffectQuery&, const Environment&) const override {
		return false;
	}

	AuraEffect(int powerBoost=0, int toughnessBoost=0,
		std::vector<std::shared_ptr<EventHandler>> replacementEffectBoost = {}, std::vector<std::shared_ptr<TriggerHandler>> triggerEffectBoost={},
		std::vector<std::shared_ptr<StaticEffectHandler>> staticEffectBoost={})
		: clone_inherit({}, { BATTLEFIELD }), powerBoost(powerBoost), toughnessBoost(toughnessBoost),
		  replacementEffectBoost(replacementEffectBoost), triggerEffectBoost(triggerEffectBoost), staticEffectBoost(staticEffectBoost)
	{}

private:
	int powerBoost;
	int toughnessBoost;
	std::vector<std::shared_ptr<EventHandler>> replacementEffectBoost;
	std::vector<std::shared_ptr<TriggerHandler>> triggerEffectBoost;
	std::vector<std::shared_ptr<StaticEffectHandler>> staticEffectBoost;
};

#endif