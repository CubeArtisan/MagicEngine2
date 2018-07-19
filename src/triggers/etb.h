#ifndef _ETB_H_
#define _ETB_H_

#include <optional>

#include "../changeset.h"
#include "../environment.h"
#include "../util.h"

class EtbTriggerHandler : public TriggerHandler {
public:
	std::vector<QueueTrigger> operator()(const Changeset& changes, const Environment& env) const {
		std::vector<QueueTrigger> result;
		for (const ObjectMovement& move : changes.moves) {
			if (move.destinationZone == env.battlefield->id) {
				if (std::shared_ptr<CardToken> card = std::dynamic_pointer_cast<CardToken>(env.gameObjects.at(move.newObject))) {
					if (selfOnly && card->id != this->owner) continue;
					if (controlled && env.getController(card) != env.getController(this->owner)) continue;
					std::shared_ptr<const std::set<CardType>> types = env.getTypes(card);
					if (intersect(watchFor.begin(), watchFor.end(), types->begin(), types->end())) {
						Changeset triggered;
						triggered.moves.push_back(move);
						// CodeReview: Have trigger controlled by correct player
						result.push_back(QueueTrigger{ env.getController(card), triggered, this->createAbility(card, move.sourceZone) });
					}
				}
			}
		}
		for (const ObjectCreation& create : changes.create) {
			if (create.zone == env.battlefield->id) {
				if (std::shared_ptr<CardToken> card = std::dynamic_pointer_cast<CardToken>(create.created)) {
					if (selfOnly && card->id != this->owner) continue;
					std::shared_ptr<const std::set<CardType>> types = env.getTypes(card);
					if (controlled && env.getController(card) != env.getController(this->owner)) continue;
					if (intersect(watchFor.begin(), watchFor.end(), types->begin(), types->end())) {
						Changeset triggered;
						triggered.create.push_back(create);
						// CodeReview: Have trigger controlled by correct player
						result.push_back(QueueTrigger{ env.getController(card), triggered, this->createAbility(card, std::nullopt) });
					}
				}
			}
		}
		return result;
	}

	template<typename Trigger>
	EtbTriggerHandler(Trigger func, bool selfOnly=false)
		: TriggerHandler(std::set<ZoneType>{}, std::set<ZoneType>{ BATTLEFIELD }), watchFor{ CREATURE }, controlled(true), selfOnly(selfOnly), createAbility(func)
	{}

	template<typename Trigger>
	EtbTriggerHandler(std::set<CardType> watchFor, bool controlled, Trigger func, std::set<ZoneType> activeSourceZones, std::set<ZoneType> activeDestinationZones)
		: TriggerHandler(activeSourceZones, activeDestinationZones), watchFor(watchFor), controlled(controlled), createAbility(func)
	{}

protected:
	std::vector<QueueTrigger> createTriggers(const Changeset& changes, const Environment& env) const {
		return this->operator()(changes, env);
	}

private:
	const std::set<CardType> watchFor;
	const bool controlled;
	const bool selfOnly;
	const std::function<std::shared_ptr<Ability>(std::shared_ptr<CardToken>, std::optional<xg::Guid>)> createAbility;
};

#endif