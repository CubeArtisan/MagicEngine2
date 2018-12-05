#ifndef _LAMBDATRIGGER_H_
#define _LAMBDATRIGGER_H_

#include <optional>

#include "../changeset.h"
#include "../environment.h"
#include "../linq/util.h"

#include "../propositions/proposition.h"

struct TriggerInfo
{
	const Changeset& change;
	const Environment& env;
	const xg::Guid& source;
	const xg::Guid& player;
};

using LambdaTriggerProp = Proposition<Changeset, Environment>;
using LambdaTriggerPropValue = PropositionValue<Changeset, Environment>;

class LambdaTriggerHandler : public clone_inherit<LambdaTriggerHandler, TriggerHandler> {
public:
	using createFunc = std::function<std::vector<QueueTrigger>(const TriggerInfo&)>;

	std::vector<QueueTrigger> operator()(const Changeset& changes, const Environment& env) const {
		if (this->prop(changes, env)) {
			TriggerInfo info{ changes, env, this->owner, env.getController(this->owner) };
			return this->createAbility(info);
		}
		return {};
	}

	LambdaTriggerHandler(LambdaTriggerPropValue prop, createFunc createAbility)
		: clone_inherit(std::set<ZoneType>{}, std::set<ZoneType>{ BATTLEFIELD }), prop(prop), createAbility(createAbility)
	{}

protected:
	std::vector<QueueTrigger> createTriggers(const Changeset& changes, const Environment& env) const {
		return this->operator()(changes, env);
	}

private:
	const createFunc createAbility;
	const LambdaTriggerPropValue prop;
};

#endif