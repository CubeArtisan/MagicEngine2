#ifndef _DAMAGEABILITY_H_
#define _DAMAGEABILITY_H_

#include "../ability.h"
#include "../targeting.h"

struct DamageAbility : public clone_inherit<abstract_method<DamageAbility>, Ability> {
	Changeset applyEffect(const Environment& env) const {
		std::vector<std::pair<xg::Guid, int>> targets = this->getTargets(env);
		Changeset damage;
		for (const std::pair<xg::Guid, int>& target : targets) {
			xg::Guid origin = getBaseClassPtr<const Targetable>(this->source)->id;
			damage.damage.push_back(DamageToTarget{ target.first, target.second, origin });
		}
		return damage;
	}

	DamageAbility(std::shared_ptr<const TargetingRestriction> targeting)
		: clone_inherit(targeting)
	{}

protected:
	virtual std::vector<std::pair<xg::Guid, int>> getTargets(const Environment& env) const = 0;
};

struct EqualDamageAbility : public clone_inherit<abstract_method<EqualDamageAbility>, DamageAbility> {
	EqualDamageAbility(std::shared_ptr<const TargetingRestriction> targeting, int amount)
		: clone_inherit(targeting), amount(amount)
	{}

protected:
	int amount;

	std::vector<std::pair<xg::Guid, int>> getTargets(const Environment& env) const {
		std::vector<std::pair<xg::Guid, int>> result;
		for (const xg::Guid& target : this->getTargetSet(env)) {
			result.push_back(std::make_pair(target, amount));
		}
		return result;
	}
	virtual std::vector<xg::Guid> getTargetSet(const Environment& env) const = 0;
};

struct EqualDamageEachOpponentAbility : public clone_inherit<EqualDamageEachOpponentAbility, EqualDamageAbility> {
	EqualDamageEachOpponentAbility(int amount)
		: clone_inherit(std::shared_ptr<TargetingRestriction>(new NoTargets()), amount)
	{}

protected:
	std::vector<xg::Guid> getTargetSet(const Environment& env) const {
		std::vector<xg::Guid> result;
		result.reserve(env.players.size() - 1);
		for (const std::shared_ptr<const Player>& player : env.players) {
			if (player->id != env.getController(this->owner)) result.push_back(player->id);
		}
		return result;
	}
};
#endif