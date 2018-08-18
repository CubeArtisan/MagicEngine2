#ifndef _PLUSFOREACHEFFECT_H_
#define _PLUSFOREACHEFFECT_H_

#include "../changeset.h"
#include "../combat.h"
#include "../environment.h"

class PlusForEachHandler : public clone_inherit<PlusForEachHandler, StaticEffectHandler> {
public:
	StaticEffectQuery & handleEvent(StaticEffectQuery& query, const Environment& env) const override {
		if (power){
			if (PowerQuery* power2 = std::get_if<PowerQuery>(&query)) {
				if (power2->target.id != this->owner) return query;
				xg::Guid controller = env.getController(this->owner);
				std::vector<std::shared_ptr<const Targetable>> items = this->getObjectsFromZone(env);
				int count = 0;
				for (auto& item : items) if (filter(item, env, controller)) count++;
				power2->power += count;
			}
		}
		if (toughness) {
			if (ToughnessQuery* toughness2 = std::get_if<ToughnessQuery>(&query)) {
				if (toughness2->target.id != this->owner) return query;
				xg::Guid controller = env.getController(this->owner);
				std::vector<std::shared_ptr<const Targetable>> items = this->getObjectsFromZone(env);
				int count = 0;
				for (auto& item : items) if (filter(item, env, controller)) count++;
				toughness2->toughness += count;
			}
		}
		return query;
	}

	bool appliesTo(StaticEffectQuery& query, const Environment&) const override {
		if (power) {
			if (PowerQuery* power2 = std::get_if<PowerQuery>(&query)) {
				return power2->target.id == this->owner;
			}
		}
		if (toughness) {
			if (ToughnessQuery* toughness2 = std::get_if<ToughnessQuery>(&query)) {
				return toughness2->target.id == this->owner;
			}
		}
		return false;
	}

	bool dependsOn(StaticEffectQuery&, StaticEffectQuery&, const Environment&) const override {
		// CodeReview: check if resulting state end changes filter
		return false;
	}

	std::vector<std::shared_ptr<const Targetable>> getObjectsFromZone(const Environment& env) const {
		std::shared_ptr<ZoneInterface> inter;
		switch (this->zone) {
		case BATTLEFIELD:
			inter = env.battlefield;
			break;
		case HAND:
			inter = env.hands.at(env.getController(this->owner));
			break;
		case GRAVEYARD:
			inter = env.graveyards.at(env.getController(this->owner));
			break;
		case LIBRARY:
			inter = env.libraries.at(env.getController(this->owner));
			break;
		default:
			throw "Not supported zone";
		}
		return inter->getObjects();
	}

	PlusForEachHandler(ZoneType zone, std::function<bool(std::shared_ptr<const Targetable>, const Environment&, xg::Guid)> filter, bool power=true, bool toughness=false)
		: clone_inherit({}, { BATTLEFIELD }), zone(zone), filter(filter), power(power), toughness(toughness)
	{}

private:
	ZoneType zone;
	std::function<bool(std::shared_ptr<const Targetable>, const Environment&, xg::Guid)> filter;
	bool power;
	bool toughness;
};

#endif