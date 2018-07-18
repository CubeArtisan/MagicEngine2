#ifndef _ETB_H_
#define _ETB_H_

#include <optional>

#include "../changeset.h"
#include "../environment.h"
#include "../util.h"

class EtbTrigger {
public:
	std::vector<QueueTrigger> operator()(const Changeset& changes, const Environment& env) const {
		std::vector<QueueTrigger> result;
		for (const ObjectMovement& move : changes.moves) {
			if (move.destinationZone == env.battlefield->id) {
				if (std::shared_ptr<CardToken> card = std::dynamic_pointer_cast<CardToken>(env.gameObjects.at(move.newObject))) {
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
					std::shared_ptr<const std::set<CardType>> types = env.getTypes(card);
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

	EtbTrigger(std::function<std::shared_ptr<Ability>(std::shared_ptr<CardToken>, std::optional<xg::Guid>)> func)
		: watchFor{ CREATURE }, createAbility(func)
	{}

	template<typename Trigger>
	EtbTrigger(std::set<CardType> watchFor, Trigger func)
		: watchFor(watchFor), createAbility(func)
	{}

private:
	const std::set<CardType> watchFor;
	const std::function<std::shared_ptr<Ability>(std::shared_ptr<CardToken>, std::optional<xg::Guid>)> createAbility;
};

#endif