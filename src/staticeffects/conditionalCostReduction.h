#ifndef _CONDITIONALCOSTREDUCTION_H_
#define _CONDITIONALCOSTREDUCTION_H_

#include "../changeset.h"
#include "../combat.h"
#include "../environment.h"

#include "../propositions/proposition.h"

class ConditionalCostReduction : public clone_inherit<ConditionalCostReduction, StaticEffectHandler> {
public:
	StaticEffectQuery& handleEvent(StaticEffectQuery& query, const Environment& env) const override {
		if (CostReductionQuery* cost = std::get_if<CostReductionQuery>(&query)) {
			if(dynamic_cast<const Targetable&>(cost->costed).id == this->owner && prop(env)) *cost->reduction += this->reduction;
		}
		return query;
	}

	bool appliesTo(StaticEffectQuery& query, const Environment& env) const override {
		if (CostReductionQuery* cost = std::get_if<CostReductionQuery>(&query)) {
			if (dynamic_cast<const Targetable&>(cost->costed).id == this->owner && prop(env)) return true;
		}
		return false;
	}

	bool dependsOn(StaticEffectQuery&, StaticEffectQuery&, const Environment&) const override {
		// CodeReview: check if resulting state end changes prop
		return false;
	}

	ConditionalCostReduction(PropositionValue<Environment> prop, CostValue reduction)
		: clone_inherit({}, { BATTLEFIELD, COMMAND, EXILE, STACK, LIBRARY, GRAVEYARD, HAND }), prop(prop), reduction(reduction)
	{}

private:
	PropositionValue<Environment> prop;
	CostValue reduction;
};

#endif