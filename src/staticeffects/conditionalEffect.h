#ifndef _CONDITIONALEFFECT_H_
#define _CONDITIONALEFFECT_H_

#include "../changeset.h"
#include "../combat.h"
#include "../environment.h"

#include "../propositions/proposition.h"

class ConditionalEffectHandler : public clone_inherit<ConditionalEffectHandler, StaticEffectHandler> {
public:
	StaticEffectQuery & handleEvent(StaticEffectQuery& query, const Environment& env) const override {
		if (StaticEffectsQuery* staticEffect = std::get_if<StaticEffectsQuery>(&query)) {
			if (staticEffect->target.id == this->owner && this->prop(env, this->owner)) {
				staticEffect->effects.push_back(this->effect->clone());
			}
		}
		else if (ActiveStaticEffectsQuery* staticEffects = std::get_if<ActiveStaticEffectsQuery>(&query)) {
			if (this->prop(env, this->owner)) {
				std::shared_ptr<StaticEffectHandler> handler = this->effect->clone();
				handler->id = xg::newGuid();
				handler->owner = this->owner;
				staticEffects->effects.push_back(handler);
			}
		}
		return query;
	}

	bool appliesTo(StaticEffectQuery& query, const Environment& env) const override {
		if (StaticEffectsQuery* staticEffect = std::get_if<StaticEffectsQuery>(&query)) {
			if (staticEffect->target.id == this->owner && this->prop(env, this->owner)) {
				return true;
			}
		}
		else if (ActiveStaticEffectsQuery* staticEffects = std::get_if<ActiveStaticEffectsQuery>(&query)) {
			if (this->prop(env, this->owner)) {
				return false;
			}
		}
		return false;
	}

	bool dependsOn(StaticEffectQuery&, StaticEffectQuery&, const Environment&) const override {
		// CodeReview: check if resulting state end changes prop
		return false;
	}

	ConditionalEffectHandler(PropositionValue<Environment, xg::Guid> prop, std::shared_ptr<StaticEffectHandler> effect)
		: clone_inherit({}, { BATTLEFIELD, COMMAND, EXILE, GRAVEYARD, HAND, LIBRARY, STACK }), prop(prop), effect(effect)
	{}

private:
	PropositionValue<Environment, xg::Guid> prop;
	std::shared_ptr<StaticEffectHandler> effect;
};

#endif