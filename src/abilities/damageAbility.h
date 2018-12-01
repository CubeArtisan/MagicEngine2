#ifndef _DAMAGEABILITY_H_
#define _DAMAGEABILITY_H_

#include "../ability.h"
#include "../targeting.h"

struct DamageAbility : public clone_inherit<abstract_method<DamageAbility>> {
	std::optional<Changeset> operator()(xg::Guid source, const Environment& env) const {
		std::vector<std::pair<xg::Guid, unsigned int>> targets = this->getTargets(source, env);
		Changeset damage;
		for (const std::pair<xg::Guid, unsigned int>& target : targets) {
			std::shared_ptr<Targetable> sourcePtr = env.gameObjects.at(source);
			xg::Guid origin = source;
			if (std::shared_ptr<Ability> aSource = std::dynamic_pointer_cast<Ability>(sourcePtr)) {
				origin = getBaseClassPtr<const Targetable>(aSource->source)->id;
			}
			damage.push_back(DamageToTarget{ target.first, target.second, origin });
		}
		return damage;
	}

protected:
	virtual std::vector<std::pair<xg::Guid, unsigned int>> getTargets(xg::Guid source, const Environment& env) const = 0;
};

struct EqualDamageAbility : public clone_inherit<abstract_method<EqualDamageAbility>, DamageAbility> {
	EqualDamageAbility(unsigned int amount)
		: amount(amount)
	{}

protected:
	unsigned int amount;

	std::vector<std::pair<xg::Guid, unsigned int>> getTargets(xg::Guid source, const Environment& env) const override {
		std::vector<std::pair<xg::Guid, unsigned int>> result;
		for (const xg::Guid& target : this->getTargetSet(source, env)) {
			result.push_back(std::make_pair(target, amount));
		}
		return result;
	}
	virtual std::vector<xg::Guid> getTargetSet(xg::Guid source, const Environment& env) const = 0;
};

struct EqualDamageEachOpponentAbility : public clone_inherit<EqualDamageEachOpponentAbility, EqualDamageAbility> {
	EqualDamageEachOpponentAbility(unsigned int amount)
		: clone_inherit(amount)
	{}

protected:
	std::vector<xg::Guid> getTargetSet(xg::Guid source, const Environment& env) const override {
		std::vector<xg::Guid> result;
		result.reserve(env.players.size() - 1);
		for (const std::shared_ptr<const Player>& player : env.players) {
			if (player->id != env.getController(source)) result.push_back(player->id);
		}
		return result;
	}
};
#endif