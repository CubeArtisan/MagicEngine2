#ifndef _CREATETIMEDEFFECT_H_
#define _CREATETIMEDEFFECT_H_

#include "../changeset.h"
#include "../effect.h"
#include "../propositions/proposition.h"

class EndEffectHandler : public clone_inherit<EndEffectHandler, EventHandler> {
public:
	std::optional<std::vector<Changeset>> handleEvent(Changeset& changeset, const Environment& env) const override {
		if (prop(changeset)) {
			changeset.propertiesToRemove.push_back(this->effect);
			return std::vector<Changeset>{ changeset };
		}
		return std::nullopt;
	}

	EndEffectHandler(PropositionValue<Changeset> prop, std::shared_ptr<StaticEffectHandler> effect)
		: clone_inherit({}, {}), prop(prop), effect(effect)
	{}

private:
	PropositionValue<Changeset> prop;
	std::shared_ptr<StaticEffectHandler> effect;
};

class CreateTimedEffects {
public:
	std::optional<Changeset> operator()(xg::Guid source, const Environment& env) {
		std::shared_ptr<StaticEffectHandler> handler = this->effect->clone();
		handler->id = xg::newGuid();
		handler->owner = source;
		Changeset result;
		result.propertiesToAdd.push_back(handler);
		std::shared_ptr<EventHandler> removal = std::make_shared<EndEffectHandler>(this->prop, effect);
		result.effectsToAdd.push_back(removal);
		return result;
	}

private:
	std::shared_ptr<StaticEffectHandler> effect;
	PropositionValue<Changeset> prop;
};

#endif