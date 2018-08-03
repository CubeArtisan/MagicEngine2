#ifndef _PLUSFOREACHEFFECT_H_
#define _PLUSFOREACHEFFECT_H_

#include "../changeset.h"
#include "../combat.h"
#include "../environment.h"

class PlusForEachHandler : public StaticEffectHandler {
public:
	StaticEffectQuery & handleEvent(StaticEffectQuery& query, const Environment& env) const override {
		if (power){
			if (PowerQuery* power = std::get_if<PowerQuery>(&query)) {
				xg::Guid controller = env.getController(this->owner);
				std::vector<std::shared_ptr<const Targetable>> items = this->getObjectsFromZone(env);
				int count = 0;
				for (auto& item : items) if (filter(item, env, controller)) count++;
				power->currentValue += count;
			}
		}
		if (toughness) {
			if (ToughnessQuery* toughness = std::get_if<ToughnessQuery>(&query)) {
				xg::Guid controller = env.getController(this->owner);
				std::vector<std::shared_ptr<const Targetable>> items = this->getObjectsFromZone(env);
				int count = 0;
				for (auto& item : items) if (filter(item, env, controller)) count++;
				toughness->currentValue += count;
			}
		}
		return query;
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
		: StaticEffectHandler({}, { BATTLEFIELD }), zone(zone), filter(filter), power(power), toughness(toughness)
	{}

private:
	ZoneType zone;
	std::function<bool(std::shared_ptr<const Targetable>, const Environment&, xg::Guid)> filter;
	bool power;
	bool toughness;
};

#endif