#ifndef _DAMAGEABILITY_H_
#define _DAMAGEABILITY_H_

#include "../ability.h"
#include "../targeting.h"

struct DamageAbility : public Ability {
	Changeset applyEffect(const Environment& env) const {
		std::vector<std::pair<xg::Guid, int>> targets = this->getTargets(env);
		Changeset damage;
		for (const std::pair<xg::Guid, int>& target : targets) {
			damage.damage.push_back(DamageToTarget{ target.first, target.second });
		}
		return damage;
	}

	DamageAbility(std::shared_ptr<const TargetingRestriction> targeting)
		: Ability(targeting)
	{}

protected:
	virtual std::vector<std::pair<xg::Guid, int>> getTargets(const Environment& env) const = 0;
};

struct EqualDamageAbility : public DamageAbility {
	EqualDamageAbility(std::shared_ptr<const TargetingRestriction> targeting, int amount)
		: DamageAbility(targeting), amount(amount)
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

struct EqualDamageEachOpponentAbility : public EqualDamageAbility {
	EqualDamageEachOpponentAbility(int amount)
		: EqualDamageAbility(std::shared_ptr<TargetingRestriction>(new NoTargets()), amount)
	{}

	std::shared_ptr<Ability> clone() const {
		return std::shared_ptr<Ability>(new EqualDamageEachOpponentAbility(amount));
	}

protected:
	std::vector<xg::Guid> getTargetSet(const Environment& env) const {
		std::vector<xg::Guid> result;
		result.reserve(env.players.size() - 1);
		for (const std::shared_ptr<const Player>& player : env.players) {
			if (player->id != this->owner) result.push_back(player->id);
		}
		return result;
	}
};
#endif