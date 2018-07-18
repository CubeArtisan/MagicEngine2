#ifndef _ETB_H_
#define _ETB_H_

#include "../changeset.h"
#include "../environment.h"
#include "../util.h"

class EtbTrigger {
public:
	std::vector<QueueTrigger> operator()(const Changeset& changes, const Environment& env) const {
		std::vector<QueueTrigger> result;
		for (const ObjectMovement& move : changes.moves) {
			if (move.destinationZone == env.battlefield->id) {
				if (std::shared_ptr<CardToken> card = std::dynamic_pointer_cast<CardToken>(env.gameObjects.at(move.object))) {
					std::shared_ptr<const std::set<CardType>> types = env.getTypes(card);
					if (intersect(watchFor.begin(), watchFor.end(), types->begin(), types->end())) {
						Changeset triggered;
						triggered.moves.push_back(move);
						result.push_back(QueueTrigger{ env.getController(card), triggered, this->createAbility(card, move) });
					}
				}
			}
			for (const ObjectCreation& create : changes.create) {
				if (create.zone == env.battlefield->id) {
					if (std::shared_ptr<CardToken> card = std::dynamic_pointer_cast<CardToken>(create.object)) {
						std::shared_ptr<const std::set<CardType>> types = env.getTypes(card);
						if (intersect(watchFor.begin(), watchFor.end(), types->begin(), types->end())) {
							Changeset triggered;
							triggered.moves.push_back(move);
							result.push_back(QueueTrigger{ env.getController(card), triggered, this->createAbility(card, move) });
						}
					}
				}
		}
		return result;
	}

	template<typename Trigger>
	EtbTrigger(Trigger func)
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