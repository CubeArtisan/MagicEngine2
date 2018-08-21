#ifndef _COMBATDAMAGETRIGGER_H_
#define _COMBATDAMAGETRIGGER_H_

#include "../changeset.h"

#include "../propositions/proposition.h"

class CombatDamageTrigger : public clone_inherit<CombatDamageTrigger, TriggerHandler> {
public:
	CombatDamageTrigger(PropositionValue<DamageToTarget, Environment> prop, std::function<std::shared_ptr<Ability>(DamageToTarget)> createAbility, bool justOwner = true)
		: clone_inherit({}, { BATTLEFIELD }), prop(prop), createAbility(createAbility), justOwner(justOwner)
	{}

protected:
	std::vector<QueueTrigger> createTriggers(const Changeset& changes, const Environment& env) const override {
		std::vector<QueueTrigger> results;
		for (const DamageToTarget& cdamage : changes.combatDamage) {
			if (cdamage.amount <= 0) continue;
			if ((!this->justOwner || cdamage.source == this->owner) && prop(cdamage, env)) {
				Changeset triggered;
				triggered.combatDamage.push_back(cdamage);
				results.push_back(QueueTrigger{ env.getController(this->owner), this->owner, triggered, this->createAbility(cdamage) });
			}
		}
		return results;
	}

private:
	const PropositionValue<DamageToTarget, Environment> prop;
	const std::function<std::shared_ptr<Ability>(DamageToTarget)> createAbility;
	const bool justOwner;
};

LambdaProposition toPlayerProp(std::function([](const DamageToTarget& damage, const Environment& env) { return (bool)std::dynamic_pointer_cast<Player>(env.gameObjects.at(damage.target)); }));

#endif 